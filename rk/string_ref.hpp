#pragma once
#include <cassert>
#include <string>
#include "string_util.hpp"
#include "hash.hpp"

namespace rk {
    //! String view class.
    template <typename CharT, typename Traits = std::char_traits<CharT>>
    class basic_str_ref {
    public:
        using value_type                = CharT;
        using pointer                   = CharT*;
        using reference                 = CharT&;
        using const_pointer             = const CharT *;
        using const_reference           = const CharT &;
        using size_type                 = size_t;
        using difference_type           = uintptr_t;

        using iterator                  = const_pointer;
        using const_iterator            = const_pointer;
        using reverse_iterator          = std::reverse_iterator<iterator>;
        using const_reverse_iterator    = std::reverse_iterator<iterator>;

        using str_type = std::basic_string<CharT, Traits>;

        static constexpr size_type npos = size_type(-1);
//------------------------------------------------------------------------------
        constexpr basic_str_ref();
        ~basic_str_ref() = default;
        basic_str_ref(const CharT *str);
        basic_str_ref(const CharT *str, size_type len);
        basic_str_ref(const std::basic_string<CharT, Traits> &str);
        basic_str_ref(const std::basic_string<CharT, Traits> &str, size_type len);

        constexpr basic_str_ref(const basic_str_ref &other);
        basic_str_ref(basic_str_ref &&other);
        basic_str_ref& operator=(basic_str_ref other);
        basic_str_ref& operator=(basic_str_ref &&other) = default;

        iterator            begin() const;
        iterator            end() const;
        const_iterator      cbegin() const;
        const_iterator      cend() const;
        reverse_iterator    rbegin() const;
        reverse_iterator    rend() const;

        const_reference     front() const;
        const_reference     back() const;

        //! Check if the string is empty.
        bool                empty() const;
        //! Get the view's size in bytes.
        size_type           size() const;
        //! Alias for size().
        size_type           length() const;

        /**
         * @brief Clear the string_ref's contents
         * @details After calling, str_ == nullptr and len_ == 0
         */
        void                clear();

        //! Find the first instance of the character after the specified point.
        size_type           find(const CharT ch, size_type begin = 0) const;
        //! Find the first instance of the character after the specified point.
        size_type           find_first_of(const CharT ch, size_type begin = 0) const;
        //! Find the first character equal to one of `chs` after the specified point.
        size_type           find_first_of(const CharT *chs, size_type begin = 0) const;
        //! Find the first instance of the character after the specified point
        //! looking back through the view.
        size_type           rfind(CharT ch, size_type begin = 0) const;
        //! Get a substring.
        basic_str_ref       substr(size_type begin, size_type num = npos) const;
        //! Get a slice through the view.
        basic_str_ref       slice(size_type begin) const;
        //! Get a slice through the view.
        basic_str_ref       slice(size_type begin, size_type end) const;

        //! Strip whitespace from the left of the view.
        basic_str_ref&      lstrip();
        //! Strip whitespace from the right of the view.
        basic_str_ref&      rstrip();
        //! Strip whitespace from the left and right of the string.
        basic_str_ref&      strip();

        /**
         * @brief Drop the first n characters, returning the suffix
         * @details Invokes substr(num)
         *
         * @param num   number of characters to drop
         * @return      str_ref from num to size()
         */
        basic_str_ref       drop(size_type num);
        /**
         * @brief Return the first n characters, dropping the suffix
         * @details Invokes substr(0, len_ - num)
         *
         * @param num   number of characters to take
         * @return      str_ref from 0 to num
         */
        basic_str_ref       take(size_type num);
        //! Copy the underlying data into a new string.
        str_type            string() const;
        //! Copy the underlying data into a new string.
        operator            std::string() const { return string(); }
        //! Append this string to another string.
        void                append_to(std::string &str)
        {
            str.append(str_, len_);
        }

        /**
         * @brief Index operator, no bounds checking
         * @details Undefined behaviour if index > size()
         *
         * @param index index in string.
         * @return      character at given index.
         */
        const_reference     operator[](size_type index) const;
        /**
         * @brief Index operator, with bounds checking.
         * @details Throws std::out_of_range if index > size().
         *
         * @param index index in string.
         * @return      character at given index
         */
        const_reference     at(size_type index) const;

        static bool         lt(const basic_str_ref &lhs, const basic_str_ref &rhs);
        static bool         equal(const basic_str_ref &lhs, const basic_str_ref &rhs);
        static bool         gt(const basic_str_ref &lhs, const basic_str_ref &rhs);

        /**
         * @brief Find a substring within this string.
         * @details Uses Boyer-Moore-Horspool search.
         * @param needle    string to find
         * @return          position of substring, or basic_str_ref::npos if not found
         */
        size_type           find(basic_str_ref needle) const;

        // Return a copy of the str_ref.
        basic_str_ref       copy() const { return {str_, len_}; }
        // Return the underlying raw pointer.
        const_pointer       data() const { return str_; }
        // Return he underlying raw pointer as a uint8_t*.
        const uint8_t*      u8data() const { return reinterpret_cast<const uint8_t*>(str_); }

        friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits> &os, const basic_str_ref &ref);
    private:
        static void         swap(basic_str_ref &a, basic_str_ref &b);
        /**
         * @brief CharT-oblivious strlen
         */
        static size_type    str_len(const_pointer str);
//------------------------------------------------------------------------------
        const_pointer       str_;
        size_type           len_;
    };

//----[ Constructors ]----------------------------------------------------------
    template <typename C, typename T>
    inline constexpr basic_str_ref<C, T>::basic_str_ref() :
        str_{nullptr},
        len_{0}
    {

    }

    template <typename C, typename T>
    inline basic_str_ref<C, T>::basic_str_ref(const_pointer str) :
        str_{str},
        len_{basic_str_ref::str_len(str)}
    {

    }

    template <typename C, typename T>
    inline basic_str_ref<C, T>::basic_str_ref(const_pointer str, size_t len) :
        str_{str},
        len_{len}
    {
        if(len <= basic_str_ref::str_len(str))
        {
            throw std::out_of_range("basic_str_ref<C, T>::basic_str_ref(str, len)"
                                    " — len cannot exceed strlen(str)!");
        }
    }

    template <typename C, typename T>
    inline basic_str_ref<C, T>::basic_str_ref(const std::basic_string<C, T> &str) :
        str_{str.data()},
        len_{str.length()}
    {

    }

    template <typename C, typename T>
    inline basic_str_ref<C, T>::basic_str_ref(const std::basic_string<C, T> &str, size_t len) :
        str_{str.data()},
        len_{std::min(str.length(), len)}
    {

    }

    template <typename C, typename T>
    inline constexpr basic_str_ref<C, T>::basic_str_ref(const basic_str_ref &other) :
        str_{other.str_},
        len_{other.len_}
    {

    }

    template <typename C, typename T>
    inline basic_str_ref<C, T>::basic_str_ref(basic_str_ref &&other) :
        str_{std::move(other.str_)},
        len_{other.len_}
    {

    }

    template <typename C, typename T>
    inline basic_str_ref<C, T>& basic_str_ref<C, T>::operator=(basic_str_ref other)
    {
        swap(*this, other);
        return *this;
    }
//----[ Accessors ]-------------------------------------------------------------
    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::begin() const -> iterator
    {
        return str_;
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::end() const -> iterator
    {
        return str_ + len_;
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::cbegin() const -> const_iterator
    {
        return str_;
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::cend() const -> const_iterator
    {
        return str_ + len_;
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::rbegin() const -> reverse_iterator
    {
        return reverse_iterator(str_ + len_);
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::rend() const -> reverse_iterator
    {
        return reverse_iterator(str_);
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::front() const -> const_reference
    {
        return *str_;
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::back() const -> const_reference
    {
        return *(str_ + len_);
    }

    template <typename C, typename T>
    inline bool basic_str_ref<C, T>::empty() const
    {
        return len_ == 0;
    }

    template <typename C, typename T>
    inline size_t basic_str_ref<C, T>::size() const
    {
        return len_;
    }

    template <typename C, typename T>
    inline size_t basic_str_ref<C, T>::length() const
    {
        return len_;
    }

    template <typename C, typename T>
    inline void basic_str_ref<C, T>::clear()
    {
        str_ = nullptr;
        len_ = 0;
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::find(const C ch, size_type begin) const -> size_type
    {
        for(begin = std::min(begin, len_); begin != len_; ++begin)
        {
            if(str_[begin] == ch) return begin;
        }
        return basic_str_ref<C, T>::npos;
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::find_first_of(const C ch, size_type begin) const -> size_type
    {
        return find(ch, begin);
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::find_first_of(const C *chs, size_type begin) const -> size_type
    {
        const C *cur = chs;
        for(begin = std::min(begin, len_); begin != len_; ++begin)
        {
            while(cur != '\0')
            {
                if(str_[begin] == cur++)
                {
                    return begin;
                }
            }
        }
        return basic_str_ref<C, T>::npos;
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::rfind(C ch, size_type begin) const -> size_type
    {
        for(begin = std::min(len_ - begin, len_); begin != 0; --begin)
        {
            if(str_[begin] == ch) return begin;
        }
        return basic_str_ref<C, T>::npos;
    }

    template <>
    inline basic_str_ref<char, std::char_traits<char>> basic_str_ref<char, std::char_traits<char>>::substr(size_type begin, size_type num) const
    {
        return (begin < len_) ?
            basic_str_ref(&str_[begin], std::min(num, len_ - begin))
            : basic_str_ref("", 0);
    }

    template <>
    inline basic_str_ref<wchar_t, std::char_traits<wchar_t>> basic_str_ref<wchar_t, std::char_traits<wchar_t>>::substr(size_type begin, size_type num) const
    {
        return (begin < len_) ?
            basic_str_ref(&str_[begin], std::min(num, len_ - begin))
            : basic_str_ref(L"", 0);
    }

    template <>
    inline basic_str_ref<char, std::char_traits<char>> basic_str_ref<char, std::char_traits<char>>::slice(size_type begin) const
    {
        return (begin < len_) ?
            basic_str_ref(&str_[begin], len_ - begin) : basic_str_ref("", 0);
    }

    template <>
    inline basic_str_ref<wchar_t, std::char_traits<wchar_t>> basic_str_ref<wchar_t, std::char_traits<wchar_t>>::slice(size_type begin) const
    {
        return (begin < len_) ?
            basic_str_ref(&str_[begin], len_ - begin) : basic_str_ref(L"", 0);
    }

    template <>
    inline basic_str_ref<char, std::char_traits<char>> basic_str_ref<char, std::char_traits<char>>::slice(size_type begin, size_type end) const
    {
        return ((begin < end) && (begin < len_)) ?
            basic_str_ref(&str_[begin], std::min(end - begin, len_ - begin))
            : basic_str_ref("", 0);
    }

    template <>
    inline basic_str_ref<wchar_t, std::char_traits<wchar_t>> basic_str_ref<wchar_t, std::char_traits<wchar_t>>::slice(size_type begin, size_type end) const
    {
        return ((begin < end) && (begin < len_)) ?
            basic_str_ref(&str_[begin], std::min(end - begin, len_ - begin))
            : basic_str_ref(L"", 0);
    }

    template <typename C, typename T>
    inline basic_str_ref<C, T>& basic_str_ref<C, T>::lstrip()
    {
        while(is_ws(*str_))
        {
            ++str_;
            --len_;
        }
        return *this;
    }

    template <typename C, typename T>
    inline basic_str_ref<C, T>& basic_str_ref<C, T>::rstrip()
    {
        while(is_ws(str_[len_ - 1]))
        {
            --len_;
        }
        return *this;
    }

    template <typename C, typename T>
    inline basic_str_ref<C, T>& basic_str_ref<C, T>::strip()
    {
        return lstrip().rstrip();
    }

    template <typename C, typename T>
    inline basic_str_ref<C, T> basic_str_ref<C, T>::drop(size_type num)
    {
        return basic_str_ref<C, T>::substr(num);
    }

    template <typename C, typename T>
    inline basic_str_ref<C, T> basic_str_ref<C, T>::take(size_type num)
    {
        return basic_str_ref<C, T>::substr(0, num);
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::string() const -> str_type
    {
        return len_ == 0 ? str_type() : str_type(str_, len_);
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::operator[](size_type index) const -> const_reference
    {
        return str_[index];
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::at(size_type index) const -> const_reference
    {
        return index <= len_ ?
            str_[index] :
            throw std::out_of_range("basic_str_ref<C, T>::at() index out of range");
    }

//----[ Comparison operators ]--------------------------------------------------
    template <typename C, typename T>
    inline bool basic_str_ref<C, T>::lt(const basic_str_ref &lhs, const basic_str_ref &rhs)
    {
        return memcmp(lhs.str_, rhs.str_, lhs.len_) == -1;
    }

    template <typename C, typename T>
    inline bool basic_str_ref<C, T>::equal(const basic_str_ref &lhs, const basic_str_ref &rhs)
    {
        return (lhs.len_  == rhs.len_) && memcmp(lhs.str_, rhs.str_, lhs.len_) == 0;
    }

    template <typename C, typename T>
    inline bool basic_str_ref<C, T>::gt(const basic_str_ref &lhs, const basic_str_ref &rhs)
    {
        return memcmp(lhs.str_, rhs.str_, lhs.len_) == 1;
    }

    template <typename C, typename T>
    inline auto basic_str_ref<C, T>::find(basic_str_ref needle) const -> size_type
    {
        if(!len_ || !needle.len_)
        {
            return npos;
        }
        if(needle.len_ == 1)
        {
            const_pointer result = static_cast<const_pointer>(std::memchr(str_, *needle.str_, len_));
            return result ? static_cast<size_type>(result - str_) : npos;
        }
        //  Table for length of shift on bad match
        size_type bad_shift[256] = {};
        //  strlen(needle) is the default shift length
        for(auto &v : bad_shift)
        {
            v = needle.len_;
        }
        size_type               needle_last = needle.len_ - 1;
        const unsigned char     needle_lastch = needle[needle_last];
        size_type               pos = 0;

        //  For each char i in the needle, set shift-idx to (len_-1) - i
        if(needle.len_ > 1)
        {
            for(size_type i = 0; i < needle_last; ++i)
            {
                bad_shift[static_cast<uint32_t>(needle[i])] = needle_last - i;
            }
        }

        while(pos <= len_ - needle.len_)
        {
            const unsigned char badshift_ch = str_[pos + needle_last];
            if(needle_lastch == badshift_ch &&
               std::memcmp(needle.str_, &str_[pos], needle_last) == 0)
            {
                return pos;
            }
            pos += bad_shift[static_cast<uint32_t>(badshift_ch)];
        }
        return npos;
    }

    template <typename C, typename T>
    inline bool operator==(const basic_str_ref<C, T> &lhs, const basic_str_ref<C, T> &rhs)
    {
        return basic_str_ref<C, T>::equal(lhs, rhs);
    }

    template <typename C, typename T>
    inline bool operator!=(const basic_str_ref<C, T> &lhs, const basic_str_ref<C, T> &rhs)
    {
        return !basic_str_ref<C, T>::equal(lhs, rhs);
    }
//----[ Swap helper function ]--------------------------------------------------
    template <typename C, typename T>
    inline void basic_str_ref<C, T>::swap(basic_str_ref &a, basic_str_ref &b)
    {
        std::swap(a.str_, b.str_);
        std::swap(a.len_, b.len_);
    }

    template <>
    inline auto basic_str_ref<char, std::char_traits<char>>::str_len(const_pointer str) -> size_type
    {
        return strlen(str);
    }

    template <>
    inline auto basic_str_ref<wchar_t, std::char_traits<wchar_t>>::str_len(const_pointer str) -> size_type
    {
        return wcslen(str);
    }

    template <typename C, typename T>
    inline bool operator==(const basic_str_ref<C, T> &lhs, const C *rhs)
    {
        return basic_str_ref<C, T>::equal(lhs, rhs);
    }

    template <typename C, typename T>
    inline bool operator!=(const basic_str_ref<C, T> &lhs, const C *rhs)
    {
        return !basic_str_ref<C, T>::equal(lhs, rhs);
    }

    template <typename C, typename T>
    inline bool operator==(const basic_str_ref<C, T> &lhs, const std::basic_string<C, T> &rhs)
    {
        return basic_str_ref<C, T>::equal(lhs, rhs);
    }

    template <typename C, typename T>
    inline bool operator!=(const basic_str_ref<C, T> &lhs, const std::basic_string<C, T> &rhs)
    {
        return !basic_str_ref<C, T>::equal(lhs, rhs);
    }

    template <typename C, typename T>
    inline bool operator==(const basic_str_ref<C, T> &lhs, C rhs)
    {
        return lhs.size() == 1 && lhs[0] == rhs;
    }

    template <typename C, typename T>
    inline bool operator!=(const basic_str_ref<C, T> &lhs, C rhs)
    {
        return !operator==(lhs, rhs);
    }

    template <typename C, typename T>
    inline std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T> &os, const basic_str_ref<C, T> &ref)
    {
        return os.write(ref.str_, ref.len_);
    }

//----[ Typedefs ]--------------------------------------------------------------
    using str_ref = basic_str_ref<char>;
    using wstr_ref = basic_str_ref<wchar_t>;
}

namespace std {
    template<>
    struct hash<rk::str_ref> {
        size_t operator()(const rk::str_ref &str)
        {
            return rk::xxhash(str.data(), str.size());
        }
    };
}

