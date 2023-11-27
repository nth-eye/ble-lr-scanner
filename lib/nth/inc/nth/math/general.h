#ifndef NTH_MATH_GENERAL_H
#define NTH_MATH_GENERAL_H

#include "nth/util/bit.h"

namespace nth {
namespace imp {

/**
 * @brief Calculate number of digits necessary to represent 
 * unsigned integer in a given arbitrary base.
 * 
 * @param x Value
 * @param b Base
 * @return Length 
 */
constexpr size_t uint_len_general(std::unsigned_integral auto x, unsigned b)
{
    size_t l = 1;
    const auto b2 = b  * b;
    const auto b3 = b2 * b;
    const auto b4 = b3 * b;
    while (1) {
        if (x < b)  return l;
        if (x < b2) return l + 1;
        if (x < b3) return l + 2;
        if (x < b4) return l + 3;
        x /= b4;
        l += 4;
	}
}

/**
 * @brief Calculate number of digits necessary to represent 
 * unsigned integer in a power of 2 base.
 * 
 * @tparam B Base
 * @param x Value
 * @return Length
 */
template<unsigned B>
constexpr size_t uint_len_optimal(std::unsigned_integral auto x)
{
    static_assert(is_pow2(B));
    constexpr auto bits = cnttz(B);
    return (std::bit_width(x | 1) + bits - 1) / bits;
}

}

/**
 * @brief Unsigned integer division with round up.
 * 
 * @tparam T Integer type
 * @param dividend Dividend
 * @param divisor Divisor, can be 0
 * @return Quotient
 */
template<std::unsigned_integral T = unsigned>
constexpr T uint_div_ceil(T dividend, T divisor)
{
    return divisor ? (dividend + (divisor - 1)) / divisor : 0;
}

/**
 * @brief Calculate factorial for unsigned integer.
 * 
 * @tparam T Integer type
 * @param x Argument
 * @return Result
 */
template<std::unsigned_integral T = unsigned>
constexpr T uint_fact(T x)
{
    T res = 1;
    for (T i = 2; i <= x; ++i)
        res *= i;
    return res;
}

/**
 * @brief Integer power function.
 * 
 * @tparam T Integer type
 * @param b Base
 * @param e Exponent
 * @return 'b' to the power 'e'
 */
template<std::integral T>
constexpr T int_pow(T b, unsigned e)
{
    T r = 1;
    while (e) {
        if (e & 1)
            r *= b;
        b *= b;
        e >>= 1;
    }
    return r;
}

/**
 * @brief Get integer length in digits for a given base, 
 * for example in base 10:
 * ilen(0)      = 1;
 * ilen(10)     = 2;
 * ilen(100)    = 3;
 * ilen(-777)   = 4;
 * 
 * @tparam T Integer type
 * @param x Value
 * @param b Base
 * @return Length
 */
template<std::integral T>
constexpr size_t int_len(T x, int b = 10)
{
    auto dispatch = [&] (auto v) {
        switch (b) {
            case 2:  return imp::uint_len_optimal<2>(v);
            case 4:  return imp::uint_len_optimal<4>(v);
            case 8:  return imp::uint_len_optimal<8>(v);
            case 16: return imp::uint_len_optimal<16>(v);
            case 32: return imp::uint_len_optimal<32>(v);
            default: return imp::uint_len_general(v, b);
        }
    };
    if constexpr (std::is_signed_v<T>) {
        if (x < 0)
            return dispatch(to_unsigned(-x)) + 1;
        return dispatch(to_unsigned(x));
    } else {
        return dispatch(x);
    }
}

/**
 * @brief Map number from one range to another. Input range 
 * difference shouldn't be zero.
 * 
 * @tparam T Number type
 * @param val Input value
 * @param in_min Input minimum
 * @param in_max Input maximum
 * @param out_min Output range minimum 
 * @param out_max Output range maximum
 * @return Result
 */
template<arithmetic T>
constexpr T num_map(T val, T in_min, T in_max, T out_min, T out_max)
{
    double slope = 1.0 * (out_max - out_min) / (in_max - in_min);
    return out_min + slope * (val - in_min);
}

}

#endif