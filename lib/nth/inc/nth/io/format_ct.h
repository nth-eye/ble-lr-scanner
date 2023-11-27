#ifndef NTH_IO_FORMAT_CT_H
#define NTH_IO_FORMAT_CT_H

#include "nth/io/formatter.h"

#define NTH_IO_FORMAT_CT_CHATGPT_OPTIMIZATION   false
#define NTH_IO_FORMAT_CT_OFFSET                 false
#define NTH_IO_FORMAT_CT_NOINLINE_PRINT         true
#define NTH_IO_FORMAT_CT_STRING_CONSTANT        true

namespace nth::v1 {

struct fmt_ct_argdata {
#if (NTH_IO_FORMAT_CT_OFFSET)
    uint16_t off;
    uint8_t len;
#else
    uint16_t pos;
    uint8_t cnt;
#endif
    uint8_t idx;
};

consteval auto fmt_ct_parse_base(std::string_view str, auto handle_char, auto handle_meta)
{
    auto state = fmt_state::text;
    auto posit = 0u;
    auto index = 0u;

    for (size_t i = 0, j = 0; i < str.size(); ++i) {

        auto event = fmt_event_next(str[i]);

        switch (state) 
        {
#if (NTH_IO_FORMAT_CT_CHATGPT_OPTIMIZATION)
        default: break;
        case fmt_state::text:
            if (event == fmt_event::head) {
                if (i + 1 < str.size() && str[i + 1] == '{') { // Check for escaped bracket
                    handle_char('{');
                    i++; // Skip the next bracket
                } else {
                    state = fmt_state::text_head;
                }
            } else {
                handle_char(str[i]);
            }
        break;

        case fmt_state::text_head:
            switch (event)
            {
            case fmt_event::num:
                index = char_to_bin(str[i]);
                state = fmt_state::index;
            break;
            case fmt_event::semi:
                posit = j;
                state = fmt_state::specifier;
            break;
            case fmt_event::other:
                handle_char(str[i]);
                posit = ++j;
                state = fmt_state::specifier;
            break;
            case fmt_event::head:
                handle_char(str[i]);
                ++j;
                state = fmt_state::text;
            break;
            case fmt_event::tail:
                handle_meta(j, 0u, index++);
                state = fmt_state::text;
            break;
            }
        break;
#else
#if !(NTH_IO_FORMAT_BASE_ALLOW_NON_ESCAPED_CLOSING_BRACKET)
        default: break;
#endif
        case fmt_state::text:
            if (event == fmt_event::head) {
                state = fmt_state::text_head;
            } else {
                handle_char(str[i]);
                ++j;
            }
        break;

        case fmt_state::text_head:
            switch (event)
            {
            case fmt_event::num:
                index = str[i] - '0';
                state = fmt_state::index;
            break;
            case fmt_event::semi:
                posit = j;
                state = fmt_state::specifier;
            break;
            case fmt_event::other:
                handle_char(str[i]);
                posit = ++j;
                state = fmt_state::specifier;
            break;
            case fmt_event::head:
                handle_char(str[i]);
                ++j;
                state = fmt_state::text;
            break;
            case fmt_event::tail:
                handle_meta(j, 0u, index++);
                state = fmt_state::text;
            break;
            }
        break;
#endif
        case fmt_state::index:
            switch (event)
            {
            case fmt_event::num:
                index = index * 10 + char_to_bin(str[i]);
            break;
            case fmt_event::semi:
                posit = j;
                state = fmt_state::specifier;
            break;
            case fmt_event::tail:
                handle_meta(j, 0u, index++);
                state = fmt_state::text;
            break;
            default:
                fmt_error_handler(fmt_error::index_invalid);
            }
        break;

        case fmt_state::specifier:
            switch (event)
            {
            case fmt_event::head:
                fmt_error_handler(fmt_error::nested_bracket);
            break;
            case fmt_event::tail:
                handle_meta(posit, j - posit, index++);
                state = fmt_state::text;
            break;
            default:
                handle_char(str[i]);
                ++j;
            }
        break;
        }
    }
    if (state != fmt_state::text)
        fmt_error_handler(fmt_error::unmatched_bracket_head);
}

consteval auto fmt_ct_parse_size(std::string_view str)
{
    size_t size = 0;
    size_t args = 0;

    fmt_ct_parse_base(str, 
        [&] (...) { ++size; }, 
        [&] (...) { ++args; }
    );
    return std::pair{size, args};
}

template<auto Pair>
consteval auto fmt_ct_parse_data(std::string_view str)
{
    auto liter = std::array<char, Pair.first>{};
    auto cache = std::array<fmt_ct_argdata, Pair.second>{};

    size_t size = 0;
    size_t args = 0;
#if (NTH_IO_FORMAT_CT_OFFSET)
    size_t prev = 0;
#endif
    auto handler_char = [&] (auto c) {
        liter[size++] = c;
    };
    auto handler_meta = [&] (auto a, auto b, auto c) {
#if (NTH_IO_FORMAT_CT_OFFSET)
        cache[args++] = {uint16_t(a - prev), uint8_t(b), uint8_t(c)};
        prev = a + b;
#else
        cache[args++] = {uint16_t(a), uint8_t(b), uint8_t(c)};
#endif
    };
    fmt_ct_parse_base(str, handler_char, handler_meta);

    struct fmt_ct_cache {
        decltype(liter) literal;
        decltype(cache) arguments; 
    };
    return fmt_ct_cache {liter, cache};
}

#if (NTH_IO_FORMAT_CT_NOINLINE_PRINT)
using write_handler_fn = void(*)(const void*, std::string_view);
#endif

template<string_literal S, class... Args>
struct fmt_string_literal {
private:
    static constexpr auto view = S.view();
    static constexpr auto size = fmt_ct_parse_size(view);
    static constexpr auto data = fmt_ct_parse_data<size>(view);
#if (NTH_IO_FORMAT_CT_NOINLINE_PRINT)
    static constexpr std::array<write_handler_fn, sizeof...(Args)> fptr = { fmt_write<Args>... };
#else
    static constexpr auto fptr = std::array { fmt_write<Args>... };
#endif
    static constexpr auto chck = std::array { formatter<Args>::check... };
public:
    constexpr auto& arguments() const   { return data.arguments; }
    constexpr auto begin() const        { return data.literal.begin(); }
    constexpr auto end() const          { return data.literal.end(); }
    constexpr void operator()(size_t idx, const void* arg, std::string_view fmt) const
    {
        fptr[idx](arg, fmt);
    }
#if (NTH_IO_FORMAT_CT_NOINLINE_PRINT)
    constexpr const write_handler_fn* get_handlers() const
    {
        return fptr.data();
    }
#endif
    consteval fmt_string_literal()
    {
#if (NTH_IO_FORMAT_CT_OFFSET)
        auto ptr = data.literal.begin();
        for (const auto& it : arguments()) {
            if (it.idx >= sizeof...(Args))
                fmt_error_handler(fmt_error::index_oob);
            if (chck[it.idx]({ptr + it.off, it.len}) == false)
                fmt_error_handler(fmt_error::argument_specifier_failure);
            ptr += it.off + it.len;
        }
#else
        for (const auto& it : arguments()) {
            if (it.idx >= sizeof...(Args))
                fmt_error_handler(fmt_error::index_oob);
            if (chck[it.idx]({&data.literal[it.pos], it.cnt}) == false)
                fmt_error_handler(fmt_error::argument_specifier_failure);
        }
#endif
    }
};

#if (NTH_IO_FORMAT_CT_NOINLINE_PRINT)

NTH_NOINLINE inline void print_impl(const void* arr[], std::string_view fmt, std::span<const fmt_ct_argdata> arguments, const write_handler_fn* handlers)
{
#if (NTH_IO_FORMAT_CT_OFFSET)
        auto i = fmt.begin();
        for (const auto& it : fmt.arguments()) {
            fmt_write_view({i, it.off});
            i += it.off;
            fmt(it.idx, arr[it.idx], {i, it.len});
            i += it.len;
        }
        fmt_write_view({i, fmt.end()});
#else
        auto i = fmt.begin();
        for (const auto& it : arguments) {
            fmt_write_view({i, fmt.begin() + it.pos});
            handlers[it.idx](arr[it.idx], {fmt.begin() + it.pos, it.cnt});
            i = fmt.begin() + it.pos + it.cnt;
        }
        fmt_write_view({i, fmt.end()});
#endif
}

template<string_literal S, class... T>
void print(T&&... args)
{
    if constexpr (sizeof...(args) == 0) {
        fmt_write_view(S.view());
    } else {
        static constexpr auto fmt = fmt_string_literal<S, std::decay_t<T>...>();
        const void* arr[] = { std::addressof(args)... };
        print_impl(arr, {fmt.begin(), fmt.end()}, fmt.arguments(), fmt.get_handlers());
    }
}

#else

template<string_literal S, class... T>
void print(T&&... args)
{
    if constexpr (sizeof...(args) == 0) {
        fmt_write_view(S.view());
    } else {
        static constexpr auto fmt = fmt_string_literal<S, std::decay_t<T>...>();
        const void* arr[] = { std::addressof(args)... };
//         fmt_write_view(S.view());
//         fmt_write_view({fmt.begin(), fmt.end()});
//         for (auto it : fmt.arguments())
//             printf("[%lu : %u-%u ] \n", it.idx, it.off, it.len);
#if (NTH_IO_FORMAT_CT_OFFSET)
        auto i = fmt.begin();
        for (const auto& it : fmt.arguments()) {
            fmt_write_view({i, it.off});
            i += it.off;
            fmt(it.idx, arr[it.idx], {i, it.len});
            i += it.len;
        }
        fmt_write_view({i, fmt.end()});
#else
        auto i = fmt.begin();
        for (const auto& it : fmt.arguments()) {
            fmt_write_view({i, fmt.begin() + it.pos});
            fmt(it.idx, arr[it.idx], {fmt.begin() + it.pos, it.cnt});
            i = fmt.begin() + it.pos + it.cnt;
        }
        fmt_write_view({i, fmt.end()});
#endif
    }
}

#endif

template <string_literal S>
struct fmt_string_helper_callable {
    template<class... T>
    void operator()(T&&... args) const
    {
        print<S>(std::forward<T>(args)...);
    }
};

template<string_literal S, class... T>
void print(fmt_string_helper_callable<S>, T&&... args) 
{
    print<S>(std::forward<T>(args)...);
}

#if (NTH_IO_FORMAT_CT_STRING_CONSTANT)

#endif

}

namespace nth::literals {

template<string_literal S>
constexpr auto operator"" _f() 
{
    return nth::v1::fmt_string_helper_callable<S>();
}

template<string_literal S>
constexpr auto operator"" _print() 
{
    return nth::v1::fmt_string_helper_callable<S>();
}

}

#endif
