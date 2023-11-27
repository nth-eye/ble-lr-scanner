#ifndef NTH_COAP_OPTION_H
#define NTH_COAP_OPTION_H

#include "nth/coap/base.h"
#include <algorithm>

namespace nth::coap {

/**
 * @brief CoAP option handle with pointer to data, length and number.
 * 
 */
struct option {
    const byte* dat = nullptr;
    word len = 0;
    word num = 0;
};

/**
 * @brief Option number known to the library.
 * 
 */
enum class option_num : word {
    if_match         = 1,
    uri_host         = 3,
    etag             = 4,
    if_none_match    = 5,
    observe          = 6,
    uri_port         = 7,
    location_path    = 8,
    oscore           = 9,
    uri_path         = 11,
    content_format   = 12,
    max_age          = 14,
    uri_query        = 15,
    accept           = 17,
    location_query   = 20,
    block2           = 23,
    block1           = 27,
    size2            = 28,
    proxy_uri        = 35,
    proxy_scheme     = 39,
    size1            = 60,
    no_response      = 258,
    unknown          = 65535,
};

/**
 * @brief Bit position in a packet cache.
 * 
 */
enum class option_bit {
    if_match,
    uri_host,
    etag,
    if_none_match,
    observe,
    uri_port,
    location_path,
    oscore,
    uri_path,
    content_format,
    max_age,
    uri_query,
    accept,
    location_query,
    block2,
    block1,
    size2,
    proxy_uri,
    proxy_scheme,
    size1,
    no_response,
    unknown, // NOTE: Leave at the end
};
inline constexpr size_t option_bit_ = int(option_bit::unknown) + 1;

/**
 * @brief Option value format of known CoAP options.
 * 
 */
enum class option_format {
    empty,
    opaque,
    uint,
    string,
    unknown,
};

constexpr auto opt_edge(option opt)
{
    return opt.dat + opt.len;
}

/**
 * @brief Is option valid.
 * 
 * @param opt Option
 * @return true if yes
 */
constexpr bool opt_valid(option opt)
{ 
    return opt.num; 
}

/**
 * @brief Is option critical for processing.
 * 
 * @param num Optiom number
 * @return true if yes
 */
constexpr bool opt_critical(word num)
{ 
    return num & 1; 
}

/**
 * @brief Is option unsafe to forward.
 * 
 * @param num Optiom number
 * @return true if yes
 */
constexpr bool opt_unsafe(word num)
{ 
    return num & 2; 
}

/**
 * @brief Is intended to be part of the Cache-Key in a request: 
 * https://www.rfc-editor.org/rfc/rfc7252#section-5.6
 * 
 * @param num Optiom number
 * @return true if yes
 */
constexpr bool opt_nocachekey(word num)
{ 
    return (num & 0x1e) == 0x1c; 
}

/**
 * @brief Can option be repeated.
 * 
 * @param num Option number
 * @return true if yes
 */
constexpr bool opt_repeatable(word num)
{
    switch (option_num(num)) {
        case option_num::if_match:
        case option_num::etag:
        case option_num::location_path:
        case option_num::uri_path:
        case option_num::uri_query:
        case option_num::location_query: return true;
        default: return false;
    }
}

/**
 * @brief Convert option number to corresponding bit mask.
 * 
 * @param num Option number
 * @return Option bit
 */
constexpr auto opt_num_to_bit(word num)
{
    switch (option_num(num)) {
        case option_num::if_match:          return option_bit::if_match;
        case option_num::uri_host:          return option_bit::uri_host;
        case option_num::etag:              return option_bit::etag;
        case option_num::if_none_match:     return option_bit::if_none_match;
        case option_num::observe:           return option_bit::observe;
        case option_num::uri_port:          return option_bit::uri_port;
        case option_num::location_path:     return option_bit::location_path;
        case option_num::oscore:            return option_bit::oscore;
        case option_num::uri_path:          return option_bit::uri_path;
        case option_num::content_format:    return option_bit::content_format;
        case option_num::max_age:           return option_bit::max_age;
        case option_num::uri_query:         return option_bit::uri_query;
        case option_num::accept:            return option_bit::accept;
        case option_num::location_query:    return option_bit::location_query;
        case option_num::block2:            return option_bit::block2;
        case option_num::block1:            return option_bit::block1;
        case option_num::size2:             return option_bit::size2;
        case option_num::proxy_uri:         return option_bit::proxy_uri;
        case option_num::proxy_scheme:      return option_bit::proxy_scheme;
        case option_num::size1:             return option_bit::size1;
        case option_num::no_response:       return option_bit::no_response;
        default: return option_bit::unknown;
    }
}

/**
 * @brief Get value format corresponding to option.
 * 
 * @param num Option num
 * @return Value format
 */
constexpr auto opt_val_format(word num)
{
    switch (option_num(num)) {
        case option_num::if_match:
        case option_num::etag:
        case option_num::oscore:        return option_format::opaque;
        case option_num::uri_host:
        case option_num::location_path:
        case option_num::uri_path:
        case option_num::uri_query:
        case option_num::location_query:
        case option_num::proxy_uri:
        case option_num::proxy_scheme:  return option_format::string;
        case option_num::if_none_match: return option_format::empty;
        case option_num::observe:
        case option_num::uri_port:
        case option_num::content_format:
        case option_num::max_age:
        case option_num::accept:
        case option_num::block2:
        case option_num::block1:
        case option_num::size2:
        case option_num::size1:
        case option_num::no_response:   return option_format::uint;
        default: return option_format::unknown;
    }
}

/**
 * @brief Calculate size necessary to store given delta 
 * and length fields.
 * 
 * @param delta Delta 
 * @param length Length
 * @return Number of bytes 
 */
constexpr size_t opt_head_size(word delta, word length)
{
    auto fn = [] (word x) -> int {
        if (x >= 269)
            return 2;
        return x >= 13;
    };
    return 1 + fn(delta) + fn(length);
}

/**
 * @brief Calculate total size necessary to store an option
 * with given parameters.
 * 
 * @param delta Delta
 * @param length Length
 * @return Number of bytes 
 */
constexpr size_t opt_total_size(word delta, word length)
{
    return length + opt_head_size(delta, length);
}

/**
 * @brief Encode option header: delta and length.
 * 
 * @param p Pointer to beginning 
 * @param delta Delta
 * @param length Length
 * @return Pointer to byte past header
 */
constexpr auto opt_encode_head(byte* p, word delta, word length)
{
    auto start = p;

    if (delta < 13) {
        *p++ = delta << 4;
    } else if (delta < 269) {
        *p++ = 13 << 4;
        *p++ = delta - 13;
    } else {
        *p++ = 14 << 4;
        *p++ = (delta - 269) >> 8;
        *p++ = (delta - 269) & 0xff;
    }

    if (length < 13) {
        *start |= length;
    } else if (length < 269) {
        *start |= 13;
        *p++ = length - 13;
    } else {
        *start |= 14;
        *p++ = (length - 269) >> 8;
        *p++ = (length - 269) & 0xff;
    }
    return p;
}

/**
 * @brief Encode option, including header. Destination must 
 * have enough space.
 * 
 * @param p Pointer to beginning
 * @param delta Number delta
 * @param length Payload length
 * @param data Payload
 * @return Pointer to byte past encoded option
 */
constexpr auto opt_encode(byte* p, word delta, word length, const byte* data)
{
    p = opt_encode_head(p, delta, length);
    std::copy(data, data + length, p);
    return p + length;
}

/**
 * @brief Decode an option from buffer, checks bounds.
 * 
 * @param ptr Pointer to the start of the option
 * @param end Pointer to the end of the buffer
 * @param opt_delta Running option delta
 * @return The decoded option
 */
constexpr option opt_decode(const byte* ptr, const byte* end, word opt_delta)
{
    if (ptr >= end)
        return {};
    auto decode_field = [] (const byte* p, const byte* end, word& field) -> const byte* {
        if (field == 14) {
            if (p + 1 >= end)
                return nullptr;
            field += *p++ << 8;
            field += *p++ + 255;
        } else if (field == 15 || p > end) {
            return nullptr;
        } else if (field == 13) {
            field += *p++;
        }
        return p;
    };
    word delta  = (*ptr   & 0xf0) >> 4;
    word length =  *ptr++ & 0x0f;

    if ((ptr = decode_field(ptr, end, delta)) == nullptr)
        return {};
    if ((ptr = decode_field(ptr, end, length)) == nullptr)
        return {};
    return {ptr, length, word(opt_delta + delta)};
}

/**
 * @brief An iterator for CoAP options in a serialized buffer. It provides methods
 * for iterating over the options, checking their validity, and accessing
 * their values. Automatically stops before going out of bounds.
 */
struct option_iterator {
    constexpr option_iterator() = default;
    constexpr option_iterator(option o, const byte* tail) 
        : end{tail}, opt{o} 
    {}
    constexpr option_iterator(const byte* head, const byte* tail) : end{tail}, opt{head}
    {
        step();
    }
    constexpr bool operator==(const option_iterator&) const
    { 
        return !opt_valid(opt);
    }
    constexpr auto& operator*() const 
    { 
        return opt; 
    }
    constexpr auto operator->() const
    { 
        return &opt; 
    }
    constexpr auto& operator++()
    {
        step();
        return *this;
    }
    constexpr auto operator++(int) 
    { 
        auto tmp = *this; 
        ++(*this); 
        return tmp; 
    }
protected:
    constexpr void step() 
    {
        opt = opt_decode(opt.dat + opt.len, end, opt.num);
    }
protected:
    const byte* end;
    option opt;
};

/**
 * @brief An iterator for CoAP options with the same option number in a serialized buffer.
 * It provides methods for iterating over the options, checking their validity, and ensuring 
 * that they have the same option number. Automatically stops before going out of bounds.
 */
struct same_option_iterator : option_iterator {
    constexpr same_option_iterator() = default;
    constexpr same_option_iterator(option o, const byte* tail) 
        : option_iterator{o, tail}, num{o.num} 
    {}
    constexpr bool operator==(const same_option_iterator&) const
    { 
        return !opt_valid(opt) || opt.num != num; 
    }
private:
    const word num = 0;
};

/**
 * @brief A range for CoAP options in a serialized buffer. Represents a range 
 * between two byte pointers, `head` and `tail`. Provides methods for getting 
 * the beginning and the end iterators for the range.
 */
struct option_range {
    constexpr option_range(const byte* head, const byte* tail) : head{head}, tail{tail} {}
    constexpr option_iterator begin() const { return {head, tail}; }
    constexpr option_iterator end() const   { return {}; }
private:
    const byte* head;
    const byte* tail;
};


/**
 * @brief A range for CoAP options with the same option number in a serialized buffer.
 * Defined by an `option` object and a byte pointer `tail`, where given option is 
 * already decoded and contains number, length and pointer to payload. Provides 
 * methods for getting the beginning and the end iterators for the range.
 */
struct same_option_range {
    constexpr same_option_range(option o, const byte* tail) : tail{tail}, opt{o} {}
    constexpr same_option_iterator begin() const    { return {opt, tail}; }
    constexpr same_option_iterator end() const      { return {}; }
private:
    const byte* tail;
    const option opt;
};

}

#endif