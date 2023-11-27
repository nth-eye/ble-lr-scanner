#ifndef NTH_IO_FORMATTER_H
#define NTH_IO_FORMATTER_H

#define NTH_IO_FORMATTER_WRITE_HANDLER_NOBLOAT  false
#define NTH_IO_FORMATTER_NOINLINE               !NTH_IO_FORMATTER_WRITE_HANDLER_NOBLOAT

#include "nth/util/string.h"
#include "nth/io/format_base.h"
#include "nth/io/format_write.h"

namespace nth {

// ANCHOR: Aux types

enum class fmt_spec_align : char {
    left    = '<',
    right   = '>',
    center  = '^',
};

enum class fmt_spec_sign : char {
    minus   = '-',
    plus    = '+',
    space   = ' ',
};

enum class fmt_spec_type : char {
    non     = 0,
    dec     = 'd',
    oct     = 'o',
    hex_lo  = 'x',
    hex_up  = 'X',
    bin_lo  = 'b',
    bin_up  = 'B',
    hef_lo  = 'a',
    hef_up  = 'A',
    exp_lo  = 'e',
    exp_up  = 'E',
    fix_lo  = 'f',
    fix_up  = 'F',
    gen_lo  = 'g',
    gen_up  = 'G',
    chr     = 'c',
    str     = 's',
    ptr     = 'p',
    dbg     = '?',
};

enum class fmt_spec_state {
    fill,
    align,
    sign,
    hash,
    zero,
    width,
    precision,
    type,
    finish,
};

constexpr static bool is_align(char c)  { return c == '<' || c == '>' || c == '^'; }
constexpr static bool is_sign(char c)   { return c == ' ' || c == '+' || c == '-'; }
constexpr static bool is_hash(char c)   { return c == '#'; }
constexpr static bool is_zero(char c)   { return c == '0'; }
constexpr static bool is_dot(char c)    { return c == '.'; }
constexpr static bool is_type(char c)
{
    switch (fmt_spec_type(c))
    {
    case fmt_spec_type::chr:
    case fmt_spec_type::dec:
    case fmt_spec_type::oct:
    case fmt_spec_type::hex_lo:
    case fmt_spec_type::hex_up:
    case fmt_spec_type::bin_lo:
    case fmt_spec_type::bin_up: return true;
    default: return false;
    }
}

using fmt_widest_uint = unsigned long long;

struct fmt_integer {
    fmt_widest_uint val;
    bool negative = false;
};

template <class T>
using fmt_decay_t = std::conditional_t<integer<T>, fmt_integer, std::decay_t<T>>;

template<class T>
concept integer_only = std::integral<T> && !std::is_same_v<T, char> && !std::is_same_v<T, bool>;

// ANCHOR: Formatters

struct formatter_integer_general {

    constexpr static bool check(std::string_view str)
    {
        bool align_present = false;
        bool width_present = false;
        fmt_spec_state state = str.size() > 1 && is_align(str[1]) ? 
            fmt_spec_state::fill:
            fmt_spec_state::align;

        for (auto c : str) {
            switch (state)
            {
            case fmt_spec_state::fill:
                state = fmt_spec_state::align;
            break;
            case fmt_spec_state::align:
                if (is_align(c)) {
                    align_present = true;
                    state = fmt_spec_state::sign;
                    break;
                }
            case fmt_spec_state::sign:
                if (is_sign(c)) {
                    state = fmt_spec_state::hash;
                    break;
                }
            case fmt_spec_state::hash:
                if (is_hash(c)) {
                    state = fmt_spec_state::zero;
                    break;
                }
            case fmt_spec_state::zero:
                if (is_zero(c)) {
                    state = fmt_spec_state::width;
                    break;
                }
            case fmt_spec_state::width:
                if (is_digit(c)) {
                    width_present = true;
                    state = fmt_spec_state::width;
                    break;
                }
            case fmt_spec_state::type:
                if (is_type(c)) {
                    state = fmt_spec_state::finish;
                    break;
                }
            default: return false;
            }
        }
        return !align_present || width_present;
    }

#if (NTH_IO_FORMATTER_NOINLINE)
    NTH_NOINLINE 
#endif
    constexpr formatter_integer_general(fmt_integer input, std::string_view fmt, fmt_spec_type t) : 
        negative{input.negative},
        type{t}
    {
        auto force_pad = false;
        auto base = 10;
        auto end = fmt.data() + fmt.size();
        auto ptr = fmt.data();

        if (fmt.size() >= 2 && is_align(fmt[1])) {
            pad_char = *ptr++;
            force_pad = true;
            align = fmt_spec_align(*ptr++);
        } else if (fmt.size() >= 1 && is_align(fmt[0])) {
            force_pad = true;
            align = fmt_spec_align(*ptr++);
        }
        if (ptr == end) goto formatter_int_ctor_exit; 
        if (is_sign(*ptr)) {
            sign = fmt_spec_sign(*ptr++);
        }
        if (ptr == end) goto formatter_int_ctor_exit; 
        if (is_hash(*ptr)) {
            alternate = true;
            ptr++;
        }
        if (ptr == end) goto formatter_int_ctor_exit; 
        if (is_zero(*ptr)) {
            pad_zero = !force_pad;
            ptr++;
        }
        while (ptr != end && is_digit(*ptr)) {
            pad_size = pad_size * 10 + (*ptr++ - '0');
        }
        if (ptr != end) {
            switch (type = fmt_spec_type(*ptr)) {
                case fmt_spec_type::oct:    base = 0x08; break;
                case fmt_spec_type::hex_up:
                case fmt_spec_type::hex_lo: base = 0x10; break;
                case fmt_spec_type::bin_up:
                case fmt_spec_type::bin_lo: base = 0x02; break;
                default:;
            }
        }
    formatter_int_ctor_exit:
        if (type == fmt_spec_type::chr) {
            negative = false;
            pad_zero = false;
            alternate = false;
            sign = fmt_spec_sign::minus;
            begin = &(buf.back() = char(negative ? -input.val : input.val));
        } else if (type == fmt_spec_type::non) {
            negative = false;
            pad_zero = false;
            alternate = false;
            sign = fmt_spec_sign::minus;
            if (input.val) {
                begin = buf.end() - 4;
                std::copy_n("true", 4, begin);
            } else {
                begin = buf.end() - 5;
                std::copy_n("false", 5, begin);
            }
        } else {
            begin = imp::to_chars_unsafe(buf.end(), input.val, base);
        }
    }
    
#if (NTH_IO_FORMATTER_NOINLINE)
    NTH_NOINLINE 
#endif
    void write() const
    {
        const size_t len = [this] {
            size_t len = negative || sign != fmt_spec_sign::minus;
            if (alternate) {
                switch (type)
                {
                case fmt_spec_type::hex_up:
                case fmt_spec_type::hex_lo:
                case fmt_spec_type::bin_up:
                case fmt_spec_type::bin_lo: ++len;
                case fmt_spec_type::oct:    ++len;
                default:;
                }
            }
            return len + (buf.end() - begin);
        }();
        const size_t pad_bckp = len < pad_size ? pad_size - len : 0;
        size_t pad = pad_bckp;

        if (!pad_zero) {
            switch (align)
            {
            case fmt_spec_align::center:
                pad >>= 1;
            case fmt_spec_align::right:
                fmt_write_chars(pad_char, pad);
            default:;
            }
        }

        if (negative) {
            fmt_write_char('-');
        } else {
            switch (sign)
            {
            case fmt_spec_sign::plus:
            case fmt_spec_sign::space: 
                fmt_write_char(+sign);
            default:;
            }
        }
            
        if (alternate) {
            switch (type)
            {
            case fmt_spec_type::oct:
                fmt_write_char('0');
            break;
            case fmt_spec_type::hex_up:
            case fmt_spec_type::hex_lo:
            case fmt_spec_type::bin_up:
            case fmt_spec_type::bin_lo:
                fmt_write_char('0');
                fmt_write_char(+type);
            default:;
            }
        }

        if (pad_zero) {
            fmt_write_chars('0', pad);
            fmt_write_view({begin, buf.end()});
        } else {
            fmt_write_view({begin, buf.end()});
            switch (align)
            {
            case fmt_spec_align::center:
                pad = pad_bckp - pad;
            case fmt_spec_align::left:
                fmt_write_chars(pad_char, pad);
            default:;
            }
        }
    }
private:
    char* begin;
    size_t pad_size = 0;
    std::array<char, imp::max_digits<fmt_widest_uint>> buf;
    char pad_char = ' ';
    bool pad_zero = false;
    bool negative = false;
    bool alternate = false;
    fmt_spec_sign sign = fmt_spec_sign::minus;
    fmt_spec_type type;
    fmt_spec_align align = fmt_spec_align::right;
};

template<class T>
struct formatter {
    constexpr formatter() = delete;
    constexpr void write() = delete;
};

template<integer_only T>
struct formatter<T> : formatter_integer_general {
    using base = formatter_integer_general;
    using base::write;
    constexpr formatter(fmt_integer input, std::string_view fmt) : base(input, fmt, fmt_spec_type::dec)
    {}
};

template<>
struct formatter<char> : formatter_integer_general {
    using base = formatter_integer_general;
    using base::write;
    constexpr formatter(fmt_integer input, std::string_view fmt) : base(input, fmt, fmt_spec_type::chr)
    {}
};

template<>
struct formatter<bool> : formatter_integer_general {
    using base = formatter_integer_general;
    using base::write;
    constexpr formatter(fmt_integer input, std::string_view fmt) : base(input, fmt, fmt_spec_type::non)
    {}
};

template<nth::stringable T>
struct formatter<T> {
    constexpr static bool check(std::string_view)
    {
        return true;
    }
    constexpr formatter(std::string_view val, std::string_view) : str{val}
    {
        
    }
    void write() const
    {
        fmt_write_view(str);
    }
private:
    std::string_view str;
};

// ANCHOR: Write helper

#if (NTH_IO_FORMATTER_WRITE_HANDLER_NOBLOAT)

template<class T>
NTH_NOINLINE void fmt_write_impl(const T& arg, std::string_view fmt)
{
    formatter<T>(arg, fmt).write();
}

template<class T>
inline void fmt_write(const void* arg, std::string_view fmt) 
{
    if constexpr (integer<T>) {
        auto val = *((const T*) arg);
        auto tmp = val < 0 ? fmt_integer {fmt_widest_uint(-val), true} :
                             fmt_integer {fmt_widest_uint(val)};
        fmt_write_impl(tmp, fmt);
    } else if constexpr (std::is_same_v<T, const char*>) {
        fmt_write_impl(std::string_view{T(arg)}, fmt);
    } else {
        fmt_write_impl(*static_cast<const T*>(arg), fmt);
    }
}

#else

template<class T>
void fmt_write(const void* arg, std::string_view fmt) 
{
    if constexpr (std::integral<T>) {
        auto val = *((const T*) arg);
        if constexpr (boolean<T>) {
            formatter<T>(fmt_integer{ fmt_widest_uint(val) }, fmt).write();
        } else {
            auto tmp = val < 0 ? fmt_integer {fmt_widest_uint(-val), true} :
                                 fmt_integer {fmt_widest_uint(val)};
            formatter<T>(tmp, fmt).write();
        }
    } else if constexpr (std::is_same_v<T, const char*>) {
        formatter<T>(T(arg), fmt).write();
    } else { 
        formatter<T>(*static_cast<const T*>(arg), fmt).write();
    }
}

#endif

}

#endif
