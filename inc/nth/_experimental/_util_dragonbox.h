#ifndef NTH_DRAGONBOX_H
#define NTH_DRAGONBOX_H

#include "nth/util/bit.h"
#include "nth/math/general.h"
#include <limits>

namespace nth {

struct ieee754_binary32 {
    using uint = uint32_t;
    static constexpr int capacity_size = 32;
    static constexpr int mantissa_size = 23;
    static constexpr int exponent_size = 8;
    static constexpr int decimal_digits = 9;
};

struct ieee754_binary64 {
    using uint = uint64_t;
    static constexpr int capacity_size = 64;
    static constexpr int mantissa_size = 52;
    static constexpr int exponent_size = 11;
    static constexpr int decimal_digits = 17;
};

template<std::floating_point T, class D>
struct ieee754_traits {

    using uint = D::uint;
    using type = T;

    static_assert(
        std::numeric_limits<T>::is_iec559   && 
        std::numeric_limits<T>::radix == 2  &&
        bit_size<uint> == D::capacity_size);

    static constexpr uint sign_mask     = bit(D::capacity_size - 1);
    static constexpr uint mantissa_mask = bit_mask(D::mantissa_size);
    static constexpr uint exponent_mask = bit_mask(D::exponent_size) << D::mantissa_size;
    static constexpr auto exponent_bias = 1 - bit<int>(D::capacity_size - D::mantissa_size - 2);
    static constexpr auto exponent_min  =  exponent_bias + 1;
    static constexpr auto exponent_max  = -exponent_bias;

    static constexpr auto to_integral(type f)       { return std::bit_cast<uint>(f); }
    static constexpr auto to_floating(uint u)       { return std::bit_cast<type>(u); }
    static constexpr bool is_negative(uint u)       { return u & sign_mask; }
    static constexpr bool is_infinite(uint u)       { return get_exponent_bits(u) == exponent_mask; }
    static constexpr bool has_even_mantissa(uint u) { return !(u & 1); }
    static constexpr uint get_mantissa_bits(uint u) { return u &  mantissa_mask; }
    static constexpr uint get_exponent_bits(uint u) { return u &  exponent_mask; }
    static constexpr uint clr_exponent_bits(uint u) { return u & ~exponent_mask; }

    static constexpr auto get_exponent(uint e) 
    {
        return e ? int(e >> D::mantissa_size) + exponent_bias : exponent_min;
    }
};

#ifdef __SIZEOF_INT128__
using builtin_uint128_t = __uint128_t;
#endif

struct uint128_t {
    constexpr uint128_t() = default;
    constexpr uint128_t(uint64_t hi, uint64_t lo) noexcept : hi_{hi}, lo_{lo} {}
    constexpr auto hi() const noexcept { return hi_; }
    constexpr auto lo() const noexcept { return lo_; }
    constexpr auto& operator+=(uint64_t n) noexcept 
    {
        uint64_t sum = lo_ + n;
        hi_ += sum < lo_ ? 1 : 0;
        lo_ = sum;
        return *this;
    }
private:
    uint64_t hi_;
    uint64_t lo_;
};

constexpr uint64_t umul64(uint32_t x, uint32_t y) noexcept 
{
    return x * uint64_t(y);
}

constexpr uint128_t umul128(uint64_t x, uint64_t y) noexcept 
{
#ifdef __SIZEOF_INT128__
    auto result = builtin_uint128_t(x) * builtin_uint128_t(y);
    return {uint64_t(result >> 64), 
            uint64_t(result)};
#else
    auto a = uint32_t(x >> 32);
    auto b = uint32_t(x);
    auto c = uint32_t(y >> 32);
    auto d = uint32_t(y);

    auto ac = umul64(a, c);
    auto bc = umul64(b, c);
    auto ad = umul64(a, d);
    auto bd = umul64(b, d);

    auto intermediate = (bd >> 32) + uint32_t(ad) + uint32_t(bc);

    return {ac + (intermediate >> 32) + (ad >> 32) + (bc >> 32),
                    (intermediate << 32) + uint32_t(bd)};
#endif
}

constexpr uint64_t umul128_upper64(uint64_t x, uint64_t y) noexcept 
{
#ifdef __SIZEOF_INT128__
    auto result = builtin_uint128_t(x) * builtin_uint128_t(y);
    return uint64_t(result >> 64);
#else
    auto a = uint32_t(x >> 32);
    auto b = uint32_t(x);
    auto c = uint32_t(y >> 32);
    auto d = uint32_t(y);

    auto ac = umul64(a, c);
    auto bc = umul64(b, c);
    auto ad = umul64(a, d);
    auto bd = umul64(b, d);

    auto intermediate = (bd >> 32) + uint32_t(ad) + uint32_t(bc);

    return ac + (intermediate >> 32) + (ad >> 32) + (bc >> 32);
#endif
}

constexpr uint128_t umul192_upper128(uint64_t x, uint128_t y) noexcept 
{
    auto r = umul128(x, y.hi());
    r += umul128_upper64(x, y.lo());
    return r;
}

constexpr uint64_t umul96_upper64(uint32_t x, uint64_t y) noexcept 
{
#ifdef __SIZEOF_INT128__ ||
    return umul128_upper64(uint64_t(x) << 32, y);
#else
    auto yh = uint32_t(y >> 32);
    auto yl = uint32_t(y);
    auto xyh = umul64(x, yh);
    auto xyl = umul64(x, yl);
    return xyh + (xyl >> 32);
#endif
}

constexpr uint128_t umul192_lower128(uint64_t x, uint128_t y) noexcept 
{
    auto high = x * y.hi();
    auto high_low = umul128(x, y.lo());
    return {high + high_low.hi(), high_low.lo()};
}

constexpr uint64_t umul96_lower64(uint32_t x, uint64_t y) noexcept 
{
    return x * y;
}

// constexpr char* format_digits(char* buffer, uint64_t digits, int32_t decimal_exponent)
// {
//     static constexpr int32_t MinFixedDecimalPoint = -6;
//     static constexpr int32_t MaxFixedDecimalPoint =  17;
//     static_assert(MinFixedDecimalPoint <= -1, "internal error");
//     static_assert(MaxFixedDecimalPoint >= 17, "internal error");

//     assert(digits >= 1);
//     assert(digits <= 99999999999999999ull);
//     assert(decimal_exponent >= -999);
//     assert(decimal_exponent <=  999);

//     int32_t num_digits = imp::uint_len_general(digits, 10);
//     const int32_t decimal_point = num_digits + decimal_exponent;

//     const bool use_fixed = MinFixedDecimalPoint <= decimal_point && decimal_point <= MaxFixedDecimalPoint;

//     // Prepare the buffer.
//     // Avoid calling memset/memcpy with variable arguments below...

//     std::memset(buffer +  0, '0', 16);
//     std::memset(buffer + 16, '0', 16);
//     static_assert(MinFixedDecimalPoint >= -30, "internal error");
//     static_assert(MaxFixedDecimalPoint <=  32, "internal error");

//     int32_t decimal_digits_position;
//     if (use_fixed)
//     {
//         if (decimal_point <= 0)
//         {
//             // 0.[000]digits
//             decimal_digits_position = 2 - decimal_point;
//         }
//         else
//         {
//             // dig.its
//             // digits[000]
//             decimal_digits_position = 0;
//         }
//     }
//     else
//     {
//         // dE+123 or d.igitsE+123
//         decimal_digits_position = 1;
//     }

//     char* digits_end = buffer + decimal_digits_position + num_digits;

//     const int32_t tz = PrintDecimalDigitsBackwards(digits_end, digits);
//     digits_end -= tz;
//     num_digits -= tz;
// //  decimal_exponent += tz; // => decimal_point unchanged.

//     if (use_fixed)
//     {
//         if (decimal_point <= 0)
//         {
//             // 0.[000]digits
//             buffer[1] = '.';
//             buffer = digits_end;
//         }
//         else if (decimal_point < num_digits)
//         {
//             // dig.its
// #if defined(_MSC_VER) && !defined(__clang__)
//             // VC does not inline the memmove call below. (Even if compiled with /arch:AVX2.)
//             // However, memcpy will be inlined.
//             uint8_t tmp[16];
//             char* const src = buffer + decimal_point;
//             char* const dst = src + 1;
//             std::memcpy(tmp, src, 16);
//             std::memcpy(dst, tmp, 16);
// #else
//             std::memmove(buffer + decimal_point + 1, buffer + decimal_point, 16);
// #endif
//             buffer[decimal_point] = '.';
//             buffer = digits_end + 1;
//         }
//         else
//         {
//             // digits[000]
//             buffer += decimal_point;
//             std::memcpy(buffer, ".0", 2);
//             buffer += 2;
//         }
//     }
//     else
//     {
//         // Copy the first digit one place to the left.
//         buffer[0] = buffer[1];
//         if (num_digits == 1)
//         {
//             // dE+123
//             ++buffer;
//         }
//         else
//         {
//             // d.igitsE+123
//             buffer[1] = '.';
//             buffer = digits_end;
//         }

//         const int32_t scientific_exponent = decimal_point - 1;
// //      SF_ASSERT(scientific_exponent != 0);

//         std::memcpy(buffer, scientific_exponent < 0 ? "e-" : "e+", 2);
//         buffer += 2;

//         const uint32_t k = static_cast<uint32_t>(scientific_exponent < 0 ? -scientific_exponent : scientific_exponent);
//         if (k < 10)
//         {
//             *buffer++ = static_cast<char>('0' + k);
//         }
//         else if (k < 100)
//         {
//             Utoa_2Digits(buffer, k);
//             buffer += 2;
//         }
//         else
//         {
//             const uint32_t q = k / 100;
//             const uint32_t r = k % 100;
//             *buffer++ = static_cast<char>('0' + q);
//             Utoa_2Digits(buffer, r);
//             buffer += 2;
//         }
//     }

//     return buffer;
// }

// constexpr auto to_chars(char* buf, double val)
// {
//     using traits = ieee754_traits<decltype(val), ieee754_binary64>;

//     auto integral = traits::to_integral(val);
//     auto mantissa = traits::get_mantissa_bits(integral); 
//     auto exponent = v.PhysicalExponent();

//     // ANCHOR OLD

//     if (exponent != traits::exponent_max) { // [[likely]]
//         // Finite
//         if (traits::is_negative(integral))
//             *buf++ = '-';

//         if (exponent || mantissa ) { // [[likely]] 
//             auto dec = ToDecimal64(mantissa, exponent);
//             return format_digits(buf, dec.mantissa, dec.exponent);
//         } else {
//             std::copy_n("0.0", 3, buf);
//             return buf + 3;
//         }
//     }

//     if (mantissa == 0) {
//         if (traits::is_negative(integral))
//             *buf++ = '-';
//         std::copy_n("inf", 3, buf);
//     } else {
//         std::copy_n("nan", 3, buf);
//     }
//     return buf + 3;
// }

}

#endif