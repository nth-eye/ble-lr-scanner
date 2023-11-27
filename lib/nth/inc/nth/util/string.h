#ifndef NTH_UTIL_STRING_H
#define NTH_UTIL_STRING_H

#define NTH_UTIL_STRING_CONVERSION_CHECK_BASE   false
#define NTH_UTIL_STRING_TO_CHARS_RETURN_PTR     true

#include "nth/util/bit.h"
#include "nth/util/literal.h"
#include "nth/math/general.h"
#include "nth/container/vector.h"
#include <limits>

namespace nth {
template<bool U = false> constexpr char bin_to_char(byte b); // Forward declaration
namespace imp {

template<std::integral T>
inline constexpr auto max_digits        = 1 + std::numeric_limits<T>::digits + std::is_signed_v<T>;
inline constexpr char digits_base64[]   = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
inline constexpr char digits_upper[]    = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
inline constexpr char digits_lower[]    = "0123456789abcdefghijklmnopqrstuvwxyz";
inline constexpr char digits_100[]      =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";
inline constexpr uint8_t ascii_map[256] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ................
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ................
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //  !"#$%&'()*+,-./
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 0123456789:;<=>?
    0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, // @ABCDEFGHIJKLMNO
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0xff, 0xff, 0xff, 0xff, 0xff, // PQRSTUVWXYZ[\]^_
    0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, // `abcdefghijklmno
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0xff, 0xff, 0xff, 0xff, 0xff, // pqrstuvwxyz{|}~.
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ................
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ................
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ................
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ................
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ................
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ................
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ................
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // ................
};

// ANCHOR Integer -> string

template<int B, bool U = false>
constexpr auto uint_str_optimal(char* p, std::unsigned_integral auto val)
{
    static_assert(B  > 10 || !U);
    static_assert(B == 10 || is_pow2(B));
    if constexpr (B == 10) {
        while (val >= 100) {
            auto i = (val % 100) * 2;
            val /= 100;
            *--p = digits_100[i + 1];
            *--p = digits_100[i];
        }
        if (val < 10) {
            *--p = '0' + val;
        } else {
            auto i = val * 2;
            *--p = digits_100[i + 1];
            *--p = digits_100[i];
        }
    } else {
        constexpr auto bits = cnttz(B);
        constexpr auto mask = bit_mask(bits);
        do {
            if constexpr (bits < 4)
                *--p = '0' + (val & mask);
            else
                *--p =  U ? digits_upper[val & mask]:
                            digits_lower[val & mask];
        } while (val >>= bits);
    }
    return p;
}

template<bool U = false>
constexpr auto uint_str_general(char* p, std::unsigned_integral auto val, int base)
{
    do {
        *--p =  U ? digits_upper[val % base]:
                    digits_lower[val % base];
    } while (val /= base);
    return p;
}

template<bool U = false>
constexpr auto uint_str_select(char* p, std::unsigned_integral auto val, int base)
{
    switch (base) {
        case 2:  return uint_str_optimal<2>(p, val);
        case 4:  return uint_str_optimal<4>(p, val);
        case 8:  return uint_str_optimal<8>(p, val);
        case 10: return uint_str_optimal<10>(p, val);
        case 16: return uint_str_optimal<16, U>(p, val);
        case 32: return uint_str_optimal<32, U>(p, val);
        default: return uint_str_general<U>(p, val, base);
    }
}

template<bool U = false>
constexpr auto to_chars_unsafe(char* p, std::integral auto val, int base)
{
#if (NTH_UTIL_STRING_TO_CHARS_RETURN_PTR)
    if constexpr (std::is_signed_v<decltype(val)>) {
        if (val < 0) {
            p = uint_str_select<U>(p, to_unsigned(-val), base) - 1;
            *p = '-';
        } else {
            p = uint_str_select<U>(p, to_unsigned(val), base);
        }
        return p;
    } else {
        return uint_str_select<U>(p, val, base);
    }
#else
    if constexpr (std::is_signed_v<decltype(val)>) {
        if (val < 0)
            uint_str_select<U>(p, to_unsigned(-val), base)[-1] = '-';
        else
            uint_str_select<U>(p, to_unsigned(val), base);
    } else {
        uint_str_select<U>(p, val, base);
    }
#endif
}

template<bool U = false>
constexpr auto to_chars(char* head, char* tail, std::integral auto val, int base)
{
#if (NTH_UTIL_STRING_CONVERSION_CHECK_BASE)
    if (base < 2 || base >= int(sizeof(digits_upper)))
        return 0;
#else
    assert(base > 1 && base < int(sizeof(digits_upper)));
#endif
    int len = int_len(val, base);
    if (len > tail - head)
        return 0;
    to_chars_unsafe<U>(head + len, val, base);
    return len;
}

// ANCHOR Float -> string

// TODO

// ANCHOR Binary -> string

/**
 * @brief Convert byte array to hexadecimal string, NOT null-terminated. If input 
 * is too large for output, as much bytes as possible will be processed.
 * 
 * @tparam U Uppercase
 * @param bin Input array
 * @param bin_len Input array length
 * @param str Output string
 * @param max_str_len Output string maximum length
 * @return Resulting string length, 0 on failure
 */
template<bool U = false>
constexpr size_t to_chars(const byte* bin, size_t bin_len, char* str, size_t max_str_len)
{
    if (!str || !bin || !bin_len || max_str_len < 2)
        return 0;

    size_t str_len = bin_len << 1;

    if (str_len >= max_str_len) {
        str_len = max_str_len & ~1;
        bin_len = str_len >> 1;
    }
    for (size_t i = 0; i < bin_len; ++i) {
        *str++ = bin_to_char<U>(bin[i] >> 4);
        *str++ = bin_to_char<U>(bin[i] & 0xf);
    }
    return str_len;
}

// ANCHOR Binary -> base64

/**
 * @brief Convert array to base64 string, NOT null-terminated. If input 
 * is too large for output, no processing occurs and result is 0.
 * 
 * @param bin Input array
 * @param bin_len Input array length
 * @param str Output string
 * @param max_str_len Output string maximum length
 * @return Resulting string length, 0 on failure
 */
constexpr size_t to_base64(const byte* bin, size_t bin_len, char* str, size_t max_str_len)
{
    auto rem = bin_len % 3;
    auto len = bin_len - rem;
    auto str_len = uint_div_ceil(bin_len, size_t(3)) * 4;

    if (!str || !bin || !bin_len || max_str_len < str_len)
        return 0;

    auto encode = [] (char* p, byte x, byte y, byte z) {
        uint32_t w = (x << 16) | (y << 8) | z;
        p[0] = digits_base64[(w >> 18)];
        p[1] = digits_base64[(w >> 12) & 63];
        p[2] = digits_base64[(w >> 6 ) & 63];
        p[3] = digits_base64[(w      ) & 63];
    };
    for (size_t i = 0; i < len; i += 3) {
        encode(str, bin[i], bin[i + 1], bin[i + 2]);
        str += 4;
    }
    switch (rem) {
        case 1: 
            encode(str, bin[len], 0, 0); 
            str[2] = '=';
            str[3] = '=';
        break;
        case 2: 
            encode(str, bin[len], bin[len + 1], 0); 
            str[3] = '=';
        break;
    }
    return str_len;
}

// ANCHOR String -> binary

/**
 * @brief Convert string with hexadecimal characters ('0'...'F') to array of bytes.
 * On non-hex chars function will stop and return number of already converted bytes. 
 * String with odd length will be interpeted as with prepended '0', e.g. "fff" --> 
 * "0fff". Works with both upper and lower cases. If output array is too small, as 
 * much as possible will be processed.
 * 
 * @param str Input string
 * @param str_len Input string length
 * @param bin Output array
 * @param max_bin_len Output array max size 
 * @return Length of resulting array, 0 on failure
 */
constexpr size_t to_binary(const char* str, size_t str_len, byte* bin, size_t max_bin_len)
{
#if __GNUC__ == 12 && __GNUC_MINOR__ == 1
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

    if (!str || !bin)
        return 0;

    size_t i = 0; 
    size_t j = 0;
    size_t bin_len = (str_len + 1) >> 1; // The output array size is half the str length (rounded up)

    if (bin_len > max_bin_len) {
        bin_len = max_bin_len;
        str_len = max_bin_len << 1; // return 0;
    }
    if (str_len & 1) {
        i = j = 1;
        auto b = ascii_map[int(str[0])];
        if (b >= 16)
            return 0;
        bin[0] = b;
    }
    for (; i < str_len; i += 2, ++j) {
        auto b0 = ascii_map[int(str[i])];
        auto b1 = ascii_map[int(str[i + 1])];
        if ((b1 | b0) >= 16)
            return j;
        bin[j] = (b0 << 4) | b1;
    }
    return bin_len;

#if __GNUC__ == 12 && __GNUC_MINOR__ == 1
#pragma GCC diagnostic pop
#endif
}

// ANCHOR String -> integer

template<std::unsigned_integral T, T B>
constexpr auto str_uint_optimal(const char* head, const char* tail)
{
    static_assert(B >= 2 && B <= 36);
    static_assert(B == 10 || is_pow2(B));
    T res = 0;
    if constexpr (B == 10) {
        for (; head != tail; ++head) {
            byte c = *head - '0';
            if (c >= 10)
                break;
            res = (res * 10) + c;
        }
    } else {
        constexpr auto shft = cnttz(B);
        for (; head != tail; ++head) {
            auto c = ascii_map[int(*head)];
            if (c >= B)
                break;
            res = (res << shft) + c;
        }
    }
    return res;
}

template<std::unsigned_integral T>
constexpr auto str_uint_general(const char* head, const char* tail, T base)
{
    T res = 0;
    for (; head != tail; ++head) {
        auto c = ascii_map[int(*head)];
        if (c >= base)
            break;
        res = res * base + c;
    }
    return res;
}

template<std::unsigned_integral T>
constexpr auto str_uint_select(const char* head, const char* tail, int base)
{
    switch (base) {
        case 2:  return str_uint_optimal<T, 2>(head, tail);
        case 4:  return str_uint_optimal<T, 4>(head, tail);
        case 8:  return str_uint_optimal<T, 8>(head, tail);
        case 10: return str_uint_optimal<T, 10>(head, tail);
        case 16: return str_uint_optimal<T, 16>(head, tail);
        case 32: return str_uint_optimal<T, 32>(head, tail);
        default: return str_uint_general<T>(head, tail, base);
    }
}

template<std::integral T = long>
constexpr auto to_integer_unsafe(const char* head, const char* tail, int base)
{
    if constexpr (std::is_signed_v<T>) {
        T neg = 1;
        switch (*head) {
            case '-': neg = -1; [[fallthrough]];
            case '+': ++head;
        }
        return T(str_uint_select<std::make_unsigned_t<T>>(head, tail, base)) * neg;
    } else {
        return str_uint_select<T>(head, tail, base);
    }
}

template<std::integral T = long>
constexpr auto to_integer(const char* head, const char* tail, int base)
{
#if (NTH_UTIL_STRING_CONVERSION_CHECK_BASE)
    if (base < 2 || base >= int(sizeof(digits_upper)))
        return 0;
#else
    assert(base > 1 && base < int(sizeof(digits_upper)));
#endif
    int len = tail - head;
    if (len <= 0)
        return T(0);
    return to_integer_unsafe<T>(head, tail, base);
}

// ANCHOR String -> float

template<std::floating_point T = double>
constexpr auto to_floating_unsafe(const char* head, const char* tail)
{
    T neg = 1;
    T res = 0;
    int dot = 0;

    switch (*head) {
        case '-': neg = -1; [[fallthrough]];
        case '+': ++head;
        break;
    }
    for (; head != tail; ++head) {
        if (*head >= '0' && *head <= '9') {
            res = res * 10.0 + (*head - '0');
        } else {
            if (*head != '.' || dot)
                return T(0);
            dot = tail - head - 1;
        }
    }
    return res * neg / int_pow(10, dot);
}

template<std::floating_point T = double>
constexpr auto to_floating(const char* head, const char* tail)
{
    int len = tail - head;
    if (len <= 0)
        return T(0);
    return to_floating_unsafe<T>(head, tail);
}

}

/**
 * @brief Check if character is digit. C style function, 
 * but with constexpr support. ASCII not necessary.
 * 
 * @param c Character
 * @return true if digit
 */
constexpr bool is_digit(char c) 
{ 
    return unsigned(c - '0') < 10;
}

/**
 * @brief Check if character is hexadecimal digit. C style 
 * function, but with constexpr support. ASCII encoding is
 * expected!
 * 
 * @param c Character
 * @return true if hexadecimal
 */
constexpr bool is_xdigit(char c)
{
    char x = c | 0x20;
    return is_digit(c) || (x >= 'a' && x <= 'f');
}

/**
 * @brief Convert hexadecimal character to ASCII nibble 
 * value. Non-hex character input gives unexpected output. 
 * 
 * @param c Hexadecimal character 
 * @return Nibble value
 */
constexpr byte char_to_bin(char c)
{
    return 9 * (c >> 6) + (c & 0xf);
}

/**
 * @brief Convert integer to hexadecimal character. Value
 * greater than representable by nibble trimmed to 4 bits.
 * 
 * @tparam U Uppercase
 * @param bin Nibble value
 * @return Hexadecimal character 
 */
template<bool U>
constexpr char bin_to_char(byte b)
{
    return U ?
        imp::digits_upper[b & 0xf]:
        imp::digits_lower[b & 0xf];
}

/**
 * @brief Calculate length of null-terminated string.
 * C style function, but with constexpr support.
 * 
 * @param str Pointer to null-terminated string
 * @return String length
 */
constexpr auto str_len(const char* str)
{
    size_t len = 0;
    while (*str++) ++len;
    return len;
}

/**
 * @brief Append null at the end of not null-terminated string.
 * 
 * @param str String buffer, includes space for null-terminator 
 * @param len String length
 */
constexpr void str_nullify(std::span<char> str, size_t len)
{
    if (str.size() > len)
        str[len] = 0;
}

/**
 * @brief Split string into tokens by given delimiter. Returns 
 * nth::vector filled with std::string_view tokens.
 * 
 * @tparam N Maximum number of tokens
 * @param str String to split
 * @param del All delimiter characters
 * @return Static vector with split string
 */
template<size_t N>
constexpr auto str_split(std::string_view str, std::string_view del)
{
    vector<std::string_view, N> tokens;

    size_t head = 0; 
    size_t tail = 0; 
 
    for (size_t i = 0; i < N && tail != std::string_view::npos; ++i) { 
        tail = str.find_first_of(del, head); 
        tokens.push_back(str.substr(head, tail - head)); 
        head = tail + 1; 
    } 
    return tokens;
}

/**
 * @brief Convert integer to string in a given buffer. Head of 
 * string is placed at the beginning of a buffer. NOTE: result 
 * is not null-terminated! 
 * 
 * @tparam U 
 * @param dst Destination buffer
 * @param val Integer to convert
 * @param base Integer base
 * @return Number of characters in result
 */
template<bool U = false>
constexpr auto to_chars(std::span<char> dst, std::integral auto val, int base = 10)
{
    return imp::to_chars<U>(dst.data(), dst.data() + dst.size(), val, base);
}

// TODO
constexpr auto to_chars(std::span<char> dst, std::floating_point auto val)
{
    assert(false);
    return 0; // imp::to_chars(dst.begin(), dst.end(), val);
}

/**
 * @brief Convert byte array to hexadecimal string, NOT null-terminated. If input 
 * is too large for output, as much bytes as possible will be processed.
 * 
 * @tparam U Uppercase
 * @param bin Input array
 * @param str Output string
 * @return Resulting string length, 0 on failure
 */
template<bool U = false>
constexpr auto to_chars(std::span<const byte> bin, std::span<char> str)
{
    return imp::to_chars<U>(bin.data(), bin.size(), str.data(), str.size());
}

/**
 * @brief Convert binary to base64 string, NOT null-terminated. If input 
 * is too large for output, no processing occurs and result is 0.
 * 
 * @param bin Input array
 * @param str Output string
 * @return Resulting string length, 0 on failure
 */
constexpr auto to_base64(std::span<const byte> bin, std::span<char> str)
{
    return imp::to_base64(bin.data(), bin.size(), str.data(), str.size());
}

/**
 * @brief Convert string with hexadecimal characters ('0'...'F') to array of bytes.
 * On non-hex chars function will stop and return number of already converted bytes. 
 * String with odd length will be interpeted as with prepended '0', e.g. "fff" --> 
 * "0fff". Works with both upper and lower cases. If output array is too small, as 
 * much as possible will be processed.
 * 
 * @param str Input string
 * @param bin Output array
 * @return Length of resulting array, 0 on failure
 */
constexpr auto to_binary(std::string_view str, std::span<byte> bin)
{
    return imp::to_binary(str.data(), str.size(), bin.data(), bin.size());
}

/**
 * @brief Convert string to integer. Malformed input returns value
 * parsed till point of failure.
 * 
 * @tparam T Integer type
 * @param str String view
 * @return Result
 */
template<std::integral T = long>
constexpr auto to_integer(std::string_view str, int base = 10)
{
    return imp::to_integer<T>(str.begin(), str.end(), base);
}

/**
 * @brief Convert string to floating, malformed input returns 0.
 * 
 * @tparam T Floating type
 * @param str String view
 * @return Result
 */
template<std::floating_point T = double>
constexpr auto to_floating(std::string_view str)
{
    return imp::to_floating<T>(str.begin(), str.end());
}

/**
 * @brief Wrapper to convert either integral or floating number 
 * to nth::string_literal at compile time and save it to static 
 * constexpr storage. Can be copied or accessed as const char* 
 * and std::string_view. For integral types P should be from 2 
 * to 36 inclusive. If base > 10 digits can be in lower- or 
 * uppercase depending on U parameter.
 * 
 * @tparam N Number to convert
 * @tparam P Additional parameter: base for integral, precision for floating
 * @tparam U Use uppercase for integral with base > 10
 */
template<arithmetic auto N, int P = std::integral<decltype(N)> ? 10 : 5, bool U = false>
struct to_string {
    static constexpr auto& literal()    { return arr; }
    static constexpr auto cstr()        { return arr.data(); }
    // static constexpr auto view()        { return arr.view(); }
private:
    static constexpr auto arr = []() 
    {
        if constexpr (std::integral<decltype(N)>) {
            static_assert(P > 1 && P < sizeof(imp::digits_upper));
            string_literal<int_len(N, P) + 1> arr;
            imp::to_chars_unsafe<U>(arr.end(), N, P);
            return arr;
        } else {
            // string_literal<flen(N, P) + 1> arr;
            // arr.back() = 0;

            // auto n = N;
            // auto p = arr.size() - 1;

            // if (N < 0) {
            //     arr.front() = '-';
            //     n = -n;
            //     p = p + 1;
            // }
            // auto append = [&](auto n, int iters) {
            //     if (n) {
            //         for (int i = 0; i < iters; ++i, n /= 10)
            //             arr[--p] = imp::digits_lower[n % 10];
            //     } else {
            //         arr[--p] = '0';
            //     }
            // };
            // long long int whole = n;
            // long long int frac = (n - whole) * ipow(10, P);

            // append(frac, P);
            // arr[--p] = '.';
            // append(whole, int_len(whole));
            string_literal<1> arr = {}; // TODO
            return arr;
        }
    }();
};

/**
 * @brief Helper to access number converted to string literal.
 * 
 * @tparam N Number to convert
 * @tparam P Additional parameter: base for integral, precision for floating
 * @tparam U Use uppercase for integral with base > 10
 */
template<arithmetic auto N, int P = std::integral<decltype(N)> ? 10 : 5, bool U = false>
inline constexpr auto& to_string_v = to_string<N, P, U>::literal();

// /**
//  * @brief Wrapper to join string literals at compile time and
//  * save them in static constexpr storage as nth::string_literal.
//  * Can be copied or accessed as std::string_view and const char*.
//  * 
//  * @tparam String literals
//  */
// template <string_literal... Strings>
// struct join_obj {
//     static constexpr auto& literal()    { return arr; }
//     static constexpr auto cstr()        { return arr.data(); }
//     static constexpr auto view()        { return arr.view(); }
// private:
//     static constexpr auto arr = []() 
//     {
//         string_literal<(Strings.size() + ... + 0) + 1> arr{};
//         auto append = [i = 0, &arr] (const auto& s) mutable {
//             for (auto c : s) 
//                 arr[i++] = c;
//         };
//         (append(Strings), ...);
//         return arr;
//     }();
// };

// /**
//  * @brief Helper to access joined string literal.
//  * 
//  * @tparam Strings Literals
//  */
// template <string_literal... Strings>
// inline constexpr auto& join_v = join_obj<Strings...>::literal();

}

#undef NTH_UTIL_STRING_CONVERSION_CHECK_BASE
#undef NTH_UTIL_STRING_TO_CHARS_RETURN_PTR

#endif