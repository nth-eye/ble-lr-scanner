#ifndef NTH_IO_FORMAT_RT_H
#define NTH_IO_FORMAT_RT_H

#include "nth/io/formatter.h"

#define NTH_IO_FORMAT_RT_FORBID_STR_WITHOUT_ARGS    false
#define NTH_IO_FORMAT_RT_ENABLE_STD_BLOAT           false
#define NTH_IO_FORMAT_RT_CHATGPT_OPTIMIZATION       true

namespace nth::v2 {

constexpr void fmt_parse_stub(char) noexcept {}

constexpr auto fmt_parse_base(std::string_view str, 
    auto handle_char, 
    auto handle_args) noexcept
{
    auto state = fmt_state::text;
    auto posit = 0u;
    auto index = 0u;

    for (size_t i = 0; i < str.size(); ++i) {

        auto event = fmt_event_next(str[i]);

        switch (state) 
        {
#if (NTH_IO_FORMAT_RT_CHATGPT_OPTIMIZATION)
#if !(NTH_IO_FORMAT_BASE_ALLOW_NON_ESCAPED_CLOSING_BRACKET)
        default: break;
#endif
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
            if (event == fmt_event::num) {
                index = str[i] - '0';  // Assuming single digit index
                state = fmt_state::index;
            } else if (event == fmt_event::tail) {
                handle_args(std::string_view{}, index++);
                state = fmt_state::text;
            } else {
                posit = i + 1u;  // Start position of specifier
                state = fmt_state::specifier;
            }
        break;
#else
        case fmt_state::text:
            switch (event)
            {
            case fmt_event::head:
                state = fmt_state::text_head;
            break;
            case fmt_event::tail:
                state = fmt_state::text_tail;
            break;
            default:
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
            case fmt_event::other:
                posit = i + 1u;
                state = fmt_state::specifier;
            break;
            case fmt_event::head:
                handle_char('{');
                state = fmt_state::text;
            break;
            case fmt_event::tail:
                handle_args(std::string_view{}, index++);
                state = fmt_state::text;
            break;
            }
        break;

        case fmt_state::text_tail:
            if consteval {
                if (event != fmt_event::tail)
                    fmt_error_handler(fmt_error::unmatched_bracket_tail);
            }
            handle_char('}');
            state = fmt_state::text;
        break;
#endif
        case fmt_state::index:
            switch (event)
            {
            case fmt_event::num:
                index = index * 10 + str[i] - '0';
            break;
            case fmt_event::semi:
                posit = i + 1u;
                state = fmt_state::specifier;
            break;
            case fmt_event::tail:
                handle_args(std::string_view{}, index++);
                state = fmt_state::text;
            break;
            default:
                if consteval {
                    fmt_error_handler(fmt_error::index_invalid);
                }
            }
        break;

        case fmt_state::specifier:
            switch (event)
            {
            case fmt_event::tail:
#if (NTH_IO_FORMAT_RT_ENABLE_STD_BLOAT)
                handle_args(str.substr(posit, i - posit), index++);
#else
                handle_args(std::string_view{
                    str.data() + posit, 
                    str.data() + i}, index++);
#endif
                state = fmt_state::text;
            break;
            case fmt_event::head:
                if consteval {
                    fmt_error_handler(fmt_error::nested_bracket);
                }
            default:;
            }
        break;
        }
    }
    if consteval {
        if (state != fmt_state::text)
            fmt_error_handler(fmt_error::unmatched_bracket_head);
    }
}

template<class... Args>
struct fmt_string_literal {

    using write_fn = void(*)(const void *arg, std::string_view fmt);
    using check_fn = bool(*)(std::string_view fmt);

    static constexpr std::array<write_fn, sizeof...(Args)> fptr_write = {fmt_write<Args>...};
    static constexpr std::array<check_fn, sizeof...(Args)> fptr_check = {formatter<Args>::check...};
    static consteval void handle_args(std::string_view specifier, size_t idx)
    {
        if (idx >= sizeof...(Args))
            fmt_error_handler(fmt_error::index_oob);
        if constexpr (sizeof...(Args)) {
            if (fptr_check[idx](specifier) == false)
                fmt_error_handler(fmt_error::argument_specifier_failure);
#if (NTH_IO_FORMAT_RT_FORBID_STR_WITHOUT_ARGS)
        } else {
            fmt_error_handler(fmt_error::argument_absent);
#endif
        }
    }
    template<size_t N>
    consteval fmt_string_literal(const char (&str)[N]) : sv{str, N - 1}
    {      
        fmt_parse_base(sv, fmt_parse_stub, handle_args);
    }
    const std::string_view sv = {};
};

template<class... T>
void print(fmt_string_literal<std::decay_t<T>...> fmt, T&&... args) noexcept 
{
    if constexpr (sizeof...(args) == 0) {
        fmt_write_view(fmt.sv);
    } else {
        const void* arr[] = { std::addressof(args)... };
        fmt_parse_base(fmt.sv, fmt_write_char, [&] (auto specifier, auto idx) 
        {
            fmt.fptr_write[idx](arr[idx], specifier);
        });
    }
}

}

#endif