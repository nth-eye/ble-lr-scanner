#ifndef NTH_IO_WRITER_H
#define NTH_IO_WRITER_H

#include <cstdio>

namespace nth {

inline void fmt_write_char(char c)
{
    // static char buf[1];
    // buf[0] = c;
    putchar(c);
}

inline void fmt_write_chars(char c, size_t len)
{
    for (size_t i = 0; i < len; ++i)
        fmt_write_char(c);
}

inline void fmt_write_view(std::string_view str)
{
    std::ranges::for_each(str, fmt_write_char);
}

inline void fmt_write_view(const char* str, size_t len)
{
    for (size_t i = 0; i < len; ++i)
        fmt_write_char(str[i]);
}

}

#endif