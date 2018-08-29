// Based on Hopscotch implementation by martin.ankerl@gmail.com
// https://github.com/martinus/robin-hood-hashing/
// Revised 2017-08-29
#pragma once
#include <cstdint>
#include <functional>
#include <type_traits>

namespace rk {
    // could do this with std::conditional in the Set class, but rather messy
    namespace detail {
        //! Special handler for invalid hop sizes on hop_traits.
        template <size_t X> struct handle_invalid_hop_size : public std::false_type {};
        //! Base hop traits class.
        template <size_t HopSize>
        struct hop_traits {
            static_assert(handle_invalid_hop_size<HopSize>::value,
                          "Hop size must be a power of 2.");
        };

        template <>
        struct hop_traits<8> {
            //! Hop word type.
            using hop_type = uint8_t;
            //! Virtual bucket size.
            static constexpr uint32_t hop_bucket = 8 - 1;
            //! The maximum length of a linear probe before force-reallocating.
            static constexpr uint32_t probe_max = 8 * 16;
        };

        template <>
        struct hop_traits<16> {
            //! Hop word type.
            using hop_type = uint16_t;
            //! Virtual bucket size.
            static constexpr uint32_t hop_bucket = 16 - 1;
            //! The maximum length of a linear probe before force-reallocating.
            static constexpr uint32_t probe_max = 16 * 16;
        };

        template <>
        struct hop_traits<32> {
            //! Hop word type.
            using hop_type = uint32_t;
            //! Virtual bucket size.
            static constexpr uint32_t hop_bucket = 32 - 1;
            //! The maximum length of a linear probe before force-reallocating.
            static constexpr uint32_t probe_max = 32 * 16;
        };
    }

    template <typename Key,
              size_t HopSize,
              typename Hash>
    class HopscotchBase {
    public:
        using size_type = uint32_t;
        using key_type = Key;
        using hash_type = Hash;

        using traits = detail::hop_traits<HopSize>;
        using hop_type = typename traits::hop_type;

        template <typename DerivedType>
        struct iterator {
            iterator(DerivedType *parent, size_type index) :
                parent_{parent},
                index_{index}
            {
                next();
            }

            void next()
            {
                while(index_ < parent_->capacity() + traits::hop_bucket && !(parent_->hops_[index_] & 1))
                {
                    ++index_;
                }
            }

            void previous()
            {
                while(index_ > 0  && !(parent_->hops_[index_] & 1))
                {
                    --index_;
                }
            }

            size_type index() const
            {
                return index_;
            }

            bool operator==(const iterator &other) const
            {
                return index_ == other.index_;
            }

            bool operator!=(const iterator &other) const
            {
                return index_ != other.index_;
            }
        protected:
            DerivedType        *parent_;
            size_type           index_;
        };

        HopscotchBase(HopscotchBase &&other) :
            HopscotchBase()
        {
            std::swap(keys_, other.keys_);
            std::swap(hops_, other.hops_);
            std::swap(size_, other.size_);
            std::swap(capacity_, other.capacity_);
        }

        ~HopscotchBase() = default;

        //! Get tehe number of elements in the container.
        size_type size() const
        {
            return size_;
        }

        //! Get the maximum number of elements this container can hold before resizing.
        size_type capacity() const
        {
            return capacity_;
        }

        //! Check whether the container is empty.
        bool empty() const
        {
            return size_ == 0;
        }

        //! Check whether an element is present in the container.
        bool has(const key_type &key) const
        {
            const size_type bucket_index = get_bucket_index(key);
            return find_internal(bucket_index, key) != (capacity_ + traits::hop_bucket);
        }

        size_type find_index(const key_type &key) const
        {
            return find_internal(get_bucket_index(key), key);
        }
    protected:
        HopscotchBase() :
            keys_{nullptr},
            hops_{nullptr},
            size_{0},
            capacity_{0}
        {
        }

        //! Get the index of the 'virtual bucket' for a key.
        size_type get_bucket_index(const key_type &k) const
        {
            return hash_type()(k) & (capacity_ - 1);
        }

        /**
         * @brief Given a 'virtual bucket' index, return the index of the given key
         *  or the end-index if the key is not found.
         * @param index Index of virtual bucket.
         * @param key   Key to check for.
         * @return size_type
         */
        size_type find_internal(size_type index, const key_type &k) const
        {
            hop_type hops = hops_[index] >> 1;
            while(hops)
            {
                if((hops & 1) && (keys_[index] == k))
                {
                    return index;
                }
                ++index;
                hops >>= 1;
            }
            return capacity_ + traits::hop_bucket;
        }


        key_type   *keys_;      //!< Array of keys.
        hop_type   *hops_;      //!< Array of hop-information.
        size_type   size_,      //!< Number of allocated elements in set.
                    capacity_;  //!< Capacity of set.
    };
}

