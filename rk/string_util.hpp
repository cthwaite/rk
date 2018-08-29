#pragma once
#include <string>
#include <vector>

namespace rk {
        //! 'Whitespace' is ' ', '\t', '\r' and '\n'.
    static constexpr uint8_t _WS_LUT[256] = {
        1, 0, 0, 0, 0, 0, 0, 0,     0, 1, 1, 0, 0, 1, 0, 0,     // 16
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 32
        1, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 48
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 64
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 80
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 96
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 112
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 128
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 144
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 160
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 176
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 192
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 208
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 224
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 240
        0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,     // 256
    };

    //! Check if a character is whitespace (or '\0').
    inline bool is_ws(char c)
    {
        return _WS_LUT[static_cast<uint8_t>(c)];
    }

    //! Increment a char pointer until end of whitespace or '\0'.
    inline const char* skip_ws(const char *ch)
    {
        while(_WS_LUT[static_cast<uint8_t>(*ch)])
        {
            ++ch;
        }
        return ch;
    }

    /**
     * @brief Split a string on a character (whitespace by default).
     * @param str std::string String to split
     * @param delimiter char Character to split on.
     * @return Vector of tokens.
     */
    template <typename TokenType>
    inline std::vector<TokenType> split(const std::string &str, const char delimiter = ' ')
    {
        std::string::size_type pos = 0, last = 0;
        const std::string::size_type len = str.length();

        while((last = str.find(delimiter, last)) != std::string::npos)
        {
            ++pos;
            ++last;
        }

        std::vector<TokenType> toks;
        toks.reserve(pos);
        last = pos = 0;
        while(last < len + 1)
        {
            pos = str.find(delimiter, last);
            if(pos == std::string::npos)
            {
                pos = len;
            }

            if(pos != last)
            {
                // to allow both std::string and str_ref.
                toks.emplace_back(str.data() + last, pos - last);
            }
            last = pos + 1;
        }
        return toks;
    }

    /**
     * @brief Split a string on an arbitrary string delimiter.
     * @param str
     * @param delimiter
     * @return
     */
    template <typename RT>
    std::vector<RT> split(const std::string &str, const std::string &delimiter)
    {
        std::string::size_type pos = 0, last = 0;
        const std::string::size_type len = str.length();
        const std::string::size_type delimiter_len = delimiter.length();

        while((last = str.find(delimiter, last)) != std::string::npos)
        {
            ++pos;
            ++last;
        }

        std::vector<RT> toks;
        toks.reserve(pos);
        last = pos = 0;
        while(last < len + 1)
        {
            pos = str.find(delimiter, last);
            if(pos == std::string::npos)
            {
                pos = len;
            }

            if(pos != last)
            {
                toks.emplace_back(str.data() + last, pos - last);
            }
            last = pos + delimiter_len;
        }
        return toks;
    }
}

