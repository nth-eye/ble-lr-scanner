#ifndef NTH_IO_FORMAT_BASE_H
#define NTH_IO_FORMAT_BASE_H

#include "nth/util/string.h"

#define NTH_IO_FORMAT_BASE_ALLOW_NON_ESCAPED_CLOSING_BRACKET    true

namespace nth {

enum class fmt_event {
    num,     // numerical digit
    semi,    // semicolon
    head,    // opening bracket '{'
    tail,    // closing bracket '}'
    other,   // other character
};

enum class fmt_state {
    text,
    text_head,
#if !(NTH_IO_FORMAT_BASE_ALLOW_NON_ESCAPED_CLOSING_BRACKET)
    text_tail,
#endif
    index,
    specifier,
};

enum class fmt_error {
    ok,
    unmatched_bracket_head,
    unmatched_bracket_tail,
    nested_bracket,
    index_invalid,
    index_oob,
    argument_specifier_failure,
    argument_absent,
};

constexpr auto fmt_event_next(char c)
{
    if (c == '{')
        return fmt_event::head;
    if (c == '}')
        return fmt_event::tail;
    if (c == ':')
        return fmt_event::semi;
    if (is_digit(c))
        return fmt_event::num;
    return fmt_event::other;
}

constexpr auto fmt_error_handler(fmt_error e) noexcept
{
    if consteval {
        switch (e) 
        {
        case fmt_error::ok:                             break;
        case fmt_error::unmatched_bracket_head:         consteval_failure("unmatched '{' bracket");
        case fmt_error::unmatched_bracket_tail:         consteval_failure("unmatched '}' bracket");
        case fmt_error::nested_bracket:                 consteval_failure("nested '{' not allowed");
        case fmt_error::index_invalid:                  consteval_failure("argument index is invalid");
        case fmt_error::index_oob:                      consteval_failure("argument index is out of bounds");
        case fmt_error::argument_absent:                consteval_failure("argument not present");
        case fmt_error::argument_specifier_failure:     consteval_failure("invalid argument specifier");
        default:                                        consteval_failure("unknown error");
        }
    } else {
        assert(e == fmt_error::ok);
    }
}

}

#endif