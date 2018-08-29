// Based on Hopscotch implementation by martin.ankerl@gmail.com
// https://github.com/martinus/robin-hood-hashing/
// Revised 2017-08-29
#pragma once
#include <cstdlib>
#include "numeric.hpp"
#include "hop_base.hpp"

namespace rk {
    template <typename Key,
              typename Value,
              size_t HopSize = 32,
              typename Hash = std::hash<Key>>
    struct Dict : public HopscotchBase<Key, HopSize, Hash> {
        using base_type = HopscotchBase<Key, HopSize, Hash>;
        using size_type = typename base_type::size_type;
        using base_type::size_;
        using base_type::capacity_;
        using base_type::keys_;
        using base_type::hops_;
        using key_type = Key;
        using value_type = Value;
        using hash_type = Hash;

        using traits = detail::hop_traits<HopSize>;
        using hop_type = typename traits::hop_type;

        template <bool is_const>
        struct kvp_base {
            using value_ref = typename std::conditional<is_const,
                                                        const value_type &,
                                                        value_type &>::type;
            const key_type &key;
            value_ref       value;
        };


        using kvp = kvp_base<false>;
        using const_kvp = kvp_base<true>;

        template <bool is_const>
        struct iterator_base : public base_type::template iterator<
                                   typename std::conditional<is_const, const Dict, Dict>::type> {
            using dict_type = typename std::conditional<is_const, const Dict, Dict>::type;
            using base_type::template iterator<dict_type>::index_;
            using base_type::template iterator<dict_type>::parent_;

            using difference_type = std::ptrdiff_t;
            using value_type = kvp;
            using reference = kvp;

            iterator_base(dict_type *parent, size_type index) :
                base_type::template iterator<dict_type>{parent, index}
            {
                this->next();
            }

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

            kvp operator*()
            {
                return {parent_->keys_[index_], parent_->values_[index_]};
            }

            kvp operator->()
            {
                return {parent_->keys_[index_], parent_->values_[index_]};
            }

            const key_type& key() const
            {
                return parent_->keys_[index_];
            }

            value_type& value()
            {
                return parent_->values_[index_];
            }

            const value_type& value() const
            {
                return parent_->values_[index_];
            }

            const_kvp operator*() const
            {
                return {parent_->keys_[index_], parent_->values_[index_]};
            }

            const_kvp operator->() const
            {
                return {parent_->keys_[index_], parent_->values_[index_]};
            }
        };


        using iterator = iterator_base<false>;
        using const_iterator = iterator_base<true>;
        friend struct iterator_base<false>;
        friend struct iterator_base<true>;


        Dict(size_type initial_size = HopSize) :
            HopscotchBase<Key, HopSize>{}
        {
            init_internal(initial_size);
        }

        ~Dict()
        {
            reset_internal(typename std::is_trivially_destructible<key_type>::type());
        }

        void reset()
        {
            reset_internal(typename std::is_trivially_destructible<key_type>::type());
        }

        iterator insert(const key_type &key, const value_type &value)
        {
            key_type new_key(key);
            value_type new_value(value);
            return insert(std::move(new_key), std::move(new_value));
        }

        iterator insert(key_type &&key, value_type &&value)
        {
            // maintain npot size
            const size_type bucket_index = base_type::get_bucket_index(key);
            const size_type ins_pt = base_type::find_internal(bucket_index, key);

            if(ins_pt != (capacity_ + traits::hop_bucket))
            {
                return {this, ins_pt};
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
                return insert(std::forward<key_type>(key), std::forward<value_type>(value));
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
                    return insert(std::forward<key_type>(key), std::forward<value_type>(value));
                }

                new (keys_ + idx) key_type(std::move(keys_[offset]));
                new (values_ + idx) value_type(std::move(values_[offset]));

                keys_[offset].~key_type();
                values_[offset].~value_type();

                hops_[cursor] |= 1 << (idx - cursor + 1);
                hops_[cursor] ^= 1 << (offset - cursor + 1);
                idx = offset;
            }

            new (keys_ + idx) key_type(std::forward<key_type>(key));
            new (values_ + idx) value_type(std::forward<value_type>(value));
            hops_[idx] |= 1;

            hops_[bucket_index] |= 1 << (idx - bucket_index + 1);
            ++size_;
            return {this, idx};
        }

        iterator find(const key_type &key)
        {
            const size_type bucket_index = base_type::get_bucket_index(key);
            return {this, base_type::find_internal(bucket_index, key)};
        }

        iterator find(const key_type &key) const
        {
            const size_type bucket_index = base_type::get_bucket_index(key);
            return {this, base_type::find_internal(bucket_index, key)};
        }

        //! Erase a key from the container.
        bool erase(const key_type &key)
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

        //! Get a value by key, returning the passed default if no such key exists.
        const value_type& get(key_type &&key, const value_type &default_value) const
        {
            const auto iter = base_type::find_index(key);
            if(iter != capacity_ + traits::hop_bucket)
            {
                return values_[iter];
            }
            return default_value;
        }

        //! Get a value by key, returning the passed default if no such key exists.
        const value_type& get(const key_type &key, const value_type &default_value) const
        {
            const auto iter = base_type::find_index(key);
            if(iter != capacity_ + traits::hop_bucket)
            {
                return values_[iter];
            }
            return default_value;
        }

        //! Get a value by key, default-constructing a new key-value pair if no such key exists.
        value_type& operator[](const key_type &key)
        {
            const auto iter = base_type::find_index(key);
            if(iter != capacity_ + traits::hop_bucket)
            {
                return values_[iter];
            }
            key_type new_key(key);
            return insert(std::move(new_key), value_type()).operator->().value;
        }

        //! Get a value by key, default-constructing a new key-value pair if no such key exists.
        value_type& operator[](key_type &&key)
        {
            const auto iter = base_type::find_index(key);
            if(iter != capacity_ + traits::hop_bucket)
            {
                return values_[iter];
            }
            return insert(std::forward<key_type>(key), value_type()).operator->().value;
        }

        iterator begin()
        {
            return {this, 0};
        }

        iterator end()
        {
            return {this, capacity_ + traits::hop_bucket};
        }

        const_iterator begin() const
        {
            return {this, 0};
        }

        const_iterator end() const
        {
            return {this, capacity_ + traits::hop_bucket};
        }

        const_iterator cbegin() const
        {
            return {this, 0};
        }

        const_iterator cend() const
        {
            return {this, capacity_ + traits::hop_bucket};
        }
    private:
        void init_internal(size_type initial_size)
        {
            initial_size = npot32(initial_size);
            size_ = 0;
            capacity_ = initial_size;
            keys_ = reinterpret_cast<key_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(key_type)));
            values_ = reinterpret_cast<value_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(value_type)));
            hops_ = reinterpret_cast<hop_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(hop_type)));
        }

        void reset_internal(std::true_type)
        {
            std::free(keys_);
            std::free(values_);
            std::free(hops_);
            capacity_ = 0;
            size_ = 0;
        }

        void reset_internal(std::false_type)
        {
            for(size_type iter = 0; iter < capacity_ + traits::hop_bucket; ++iter)
            {
                if(hops_[iter] & 1)
                {
                    keys_[iter].~key_type();
                    values_[iter].~value_type();
                }
            }
            reset_internal(std::true_type());
        }

        void expand()
        {
            key_type *old_keys = keys_;
            value_type *old_values = values_;
            hop_type *old_hops = hops_;
            uint32_t old_cap = base_type::capacity_;
            capacity_ *= 2;
            size_ = 0;
            keys_ = reinterpret_cast<key_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(key_type)));
            hops_ = reinterpret_cast<hop_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(hop_type)));
            values_ = reinterpret_cast<value_type*>(std::calloc(capacity_ + traits::hop_bucket, sizeof(value_type)));

            for(size_type iter = 0; iter < old_cap + traits::hop_bucket; ++iter)
            {
                if(old_hops[iter] & 1)
                {
                    insert(std::move(old_keys[iter]), std::move(old_values[iter]));
                }
            }
            std::free(old_keys);
            std::free(old_hops);
            std::free(old_values);
        }

        void construct_key(size_t index, key_type &&key)
        {
            new (keys_ + index) key_type(std::move(key));
        }
        void construct_value(size_t index, value_type &&value)
        {
            new (values_ + index) value_type(std::move(value));
        }


        value_type *values_;
    };
}

