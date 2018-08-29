// Based on Hopscotch implementation by martin.ankerl@gmail.com
// https://github.com/martinus/robin-hood-hashing/
// Revised 2017-08-29
#pragma once
#include <cstdlib>
#include "numeric.hpp" // npot32
#include "hop_base.hpp"

namespace rk {
    /**
     *  @brief Hash set using hopscotch hashing.
     */
    template <typename Key,
              size_t HopSize = 32,
              typename Hash = std::hash<Key>>
    struct Set : public HopscotchBase<Key, HopSize, Hash> {
        using base_type = HopscotchBase<Key, HopSize, Hash>;
        using size_type = typename base_type::size_type;
        using base_type::size_;
        using base_type::capacity_;
        using base_type::keys_;
        using base_type::hops_;
        using key_type = Key;
        using value_type = Key;
        using hash_type = Hash;

        using traits = detail::hop_traits<HopSize>;
        using hop_type = typename traits::hop_type;


        struct iterator : public base_type::template iterator<const Set> {
            using base_type::template iterator<const Set>::iterator;
            using base_type::template iterator<const Set>::index_;
            using base_type::template iterator<const Set>::parent_;

            using difference_type = std::ptrdiff_t;
            using value_type = key_type;
            using reference = const key_type&;
            using pointer = const key_type*;

            iterator& operator++()
            {
                ++index_;
                iterator::next();
                return *this;
            }

            iterator operator++(int)
            {
                iterator ret = *this;
                ++*this;
                return ret;
            }

            iterator& operator--()
            {
                if(index_ > 0)
                {
                    --index_;
                }
                iterator::next();
                return *this;
            }

            iterator operator--(int)
            {
                iterator ret = *this;
                --*this;
                return ret;
            }

            reference operator*() const
            {
                return parent_->keys_[index_];
            }

            pointer operator->() const
            {
                return parent_->keys_ + index_;
            }
        };

        using const_iterator = iterator;
        friend struct iterator;

        Set(size_type initial_size = HopSize) :
            HopscotchBase<Key, HopSize>{}
        {
            init_internal(initial_size);
        }

        Set(const Set &other) :
            Set()
        {
            clone(other);
        }

        Set(std::initializer_list<key_type> keys) :
            Set()
        {
            init_internal();
            for(const auto &k : keys)
            {
                insert(k);
            }
        }

        Set(Set &&other) :
            HopscotchBase<Key, HopSize>{std::move(other)}
        {

        }

        ~Set()
        {
            reset_internal(typename std::is_trivially_destructible<key_type>::type());
        }

        //! Clear the contents of the set.
        void clear()
        {
            clear_internal(typename std::is_trivially_destructible<key_type>::type());
        }

        //! Clear the contents of the set, relinquish all allocated memory, and reset
        //! to initial empty state.
        void reset()
        {
            reset_internal(typename std::is_trivially_destructible<key_type>::type());
            init_internal();
        }

        //! Replace the contents of this set with a copy of the contents of another set.
        void clone(const Set &other)
        {
            reset_internal(typename std::is_trivially_destructible<key_type>::type());
            capacity_ = other.capacity_;
            size_ = other.size_;
            keys_ = reinterpret_cast<key_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(key_type)));
            hops_ = reinterpret_cast<hop_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(hop_type)));
            std::copy(other.keys_, other.keys_ + capacity_ + traits::hop_bucket, keys_);
            std::copy(other.hops_, other.hops_ + capacity_ + traits::hop_bucket, hops_);
        }

        //! Insert an element into the set.
        bool insert(const key_type &key)
        {
            key_type new_key(key);
            return insert(std::move(new_key));
        }

        /**
         * @brief Insert an element into the set.
         * 
         * @param key Element to add to the set.
         * @return true if the element was inserted, false if it already existed
         * in the set.
         */
        bool insert(key_type &&key)
        {
            // maintain npot size
            const size_type bucket_index = base_type::get_bucket_index(key);
            const size_type ins_pt = base_type::find_internal(bucket_index, key);
            if(ins_pt != (capacity_ + traits::hop_bucket))
            {
                return false;
            }
            size_type probe_end = bucket_index + traits::probe_max;
            size_type idx = bucket_index;

            if(probe_end > capacity_ + traits::hop_bucket)
            {
                probe_end = capacity_ + traits::hop_bucket;
            }

            while((idx < probe_end) & (hops_[idx] & 1))
            {
                ++idx;
            }

            if(idx == probe_end)
            {
                expand();
                return insert(std::forward<key_type>(key));
            }

            hops_[idx] |= 1;

            while(idx > bucket_index + traits::hop_bucket - 1)
            {
                const uint32_t look_first = idx < traits::hop_bucket ? 0 : idx - traits::hop_bucket + 1;
                uint32_t cursor = 0,
                         offset = look_first - 1;

                do {
                    ++offset;
                    cursor = look_first;
                    uint32_t hop_mask = 1 << (offset - cursor + 1);
                    while(cursor <= offset && !(hops_[cursor] & hop_mask))
                    {
                        ++cursor;
                        hop_mask >>= 1;
                    }
                } while(offset < idx && cursor > offset);

                if(offset >= idx)
                {
                    hops_[idx] ^= 1;
                    expand();
                    return insert(std::forward<key_type>(key));
                }

                new (keys_ + idx) key_type(std::move(keys_[offset]));
                keys_[offset].~key_type();

                hops_[cursor] |= 1 << (idx - cursor + 1);
                hops_[cursor] ^= 1 << (offset - cursor + 1);

                idx = offset;
            }

            new (keys_ + idx) key_type(std::forward<key_type>(key));
            hops_[idx] |= 1;

            hops_[bucket_index] |= 1 << (idx - bucket_index + 1);
            ++size_;
            return true;
        }

        //! Find and return an iterator to a specific element in the set.
        iterator find(const key_type &key) const
        {
            const size_type bucket_index = base_type::get_bucket_index(key);
            return {this, base_type::find_internal(bucket_index, key)};
        }

        //! Remove an element from the set.
        bool remove(const key_type &key)
        {
            const size_type bucket_index = base_type::get_bucket_index(key);
            const size_type index = base_type::find_internal(bucket_index, key);
            if(index != capacity_ + traits::hop_bucket)
            {
                hops_[bucket_index] ^= 1 << (index - bucket_index + 1);
                hops_[index] ^= 1;
                --size_;
                return true;
            }
            return false;
        }

        //! Get an iterator to the beginning of the set.
        iterator begin()
        {
            return {this, 0};
        }

        //! Get an iterator to the end of the set.
        iterator end()
        {
            return {this, capacity_ + traits::hop_bucket};
        }

        //! Get a const iterator to the beginning of the set.
        const_iterator begin() const
        {
            return {this, 0};
        }

        //! Get a const iterator to the end of the set.
        const_iterator end() const
        {
            return {this, capacity_ + traits::hop_bucket};
        }

        //! Union operator.
        Set operator&(const Set &other)
        {
            Set ret(*this);
            ret &= other;
            return ret;
        }

        //! Union assignment operator.
        void operator&=(const Set &other)
        {
            for(const auto &e : *this)
            {
                if(!other.has(e))
                {
                    remove(e);
                }
            }
        }

        //! Intersection operator.
        Set operator|(const Set &other)
        {
            if(other.size() > this->size())
            {
                Set ret(other);
                ret |= *this;
                return ret;
            }
            else
            {
                Set ret(*this);
                ret |= other;
                return ret;
            }
        }

        //! Intersection assignment operator.
        void operator|=(const Set &other)
        {
            for(const auto &e : other)
            {
                insert(e);
            }
        }

        //! Returns true if this set contains any elements in common with another set.
        bool intersects(const Set &other)
        {
            if(other.size() > this->size())
            {
                for(const auto &e : *this)
                {
                    if(other.has(e))
                    {
                        return true;
                    }
                }
            }
            else
            {
                for(const auto &e : other)
                {
                    if(this->has(e))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        //! Difference operator.
        Set operator-(const Set &other)
        {
            Set ret(*this);
            ret -= other;
            return ret;
        }

        //! Difference assignment operator.
        void operator-=(const Set &other)
        {
            for(const auto &e : other)
            {
                if(this->has(e))
                {
                    remove(e);
                }
            }
        }

        //! Symmetric difference operator.
        Set operator^(const Set &other)
        {
            Set ret(*this);
            ret ^= other;
            return ret;
        }

        //! Symmetric difference assignment operator.
        void operator^=(const Set &other)
        {
            for(const auto &e : other)
            {
                if(this->has(e))
                {
                    remove(e);
                }
                else
                {
                    insert(e);
                }
            }
        }

        template <typename SaveSerialise>
        void save(SaveSerialise &ser) const
        {
            ser.save(size_);
            ser.save(capacity_);
            const hop_type *hops_end = hops_ + capacity_ + traits::hop_bucket;
            for(hop_type *iter = hops_; iter != hops_end; ++iter)
            {
                ser.save(*iter);
            }
            const key_type *keys_end = keys_ + capacity_ + traits::hop_bucket;
            for(key_type *iter = keys_; iter != keys_end; ++iter)
            {
                ser.save(*iter);
            }
        }

        template <typename LoadSerialise>
        void load(LoadSerialise &ser)
        {
            ser.load(size_);
            ser.load(capacity_);
            keys_ = reinterpret_cast<key_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(key_type)));
            for(size_type iter = 0; iter < capacity_ + traits::hop_bucket; ++i)
            {
                ser.load(keys_[iter]);
            }
            hops_ = reinterpret_cast<hop_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(hop_type)));
            for(size_type iter = 0; iter < capacity_ + traits::hop_bucket; ++i)
            {
                ser.load(hops_[iter]);
            }
        }
    private:
        void init_internal(size_type initial_size)
        {
            initial_size = npot32(initial_size);
            size_ = 0;
            capacity_ = initial_size;
            keys_ = reinterpret_cast<key_type*>(std::calloc(initial_size + traits::hop_bucket, sizeof(key_type)));
            hops_ = reinterpret_cast<hop_type*>(std::calloc(initial_size + traits::hop_bucket, sizeof(hop_type)));
        }

        void clear_internal(std::true_type)
        {
            std::memset(hops_, 0, capacity_ + traits::hop_bucket);
            size_ = 0;
        }

        void reset_internal(std::true_type)
        {
            std::free(keys_);
            std::free(hops_);
            keys_ = nullptr;
            hops_ = nullptr;
            size_ = 0;
            capacity_ = 0;
        }

        void reset_internal(std::false_type)
        {
            for(size_type iter = 0; iter < capacity_ + traits::hop_bucket; ++iter)
            {
                if(hops_[iter] & 1)
                {
                    keys_[iter].~key_type();
                }
            }
            reset_internal(std::true_type{});
        }

        void expand()
        {
            key_type *old_keys = keys_;
            hop_type *old_hops = hops_;
            uint32_t old_cap = base_type::capacity_;
            capacity_ *= 2;
            size_ = 0;
            keys_ = reinterpret_cast<key_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(key_type)));
            hops_ = reinterpret_cast<hop_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(hop_type)));

            for(size_type iter = 0; iter < old_cap + traits::hop_bucket; ++iter)
            {
                if(old_hops[iter] & 1)
                {
                    insert(std::move(old_keys[iter]));
                }
            }
            std::free(old_keys);
            std::free(old_hops);
        }
    };
}
