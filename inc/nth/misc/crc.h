#ifndef NTH_MISC_CRC_H
#define NTH_MISC_CRC_H

#include "nth/util/bit.h"

namespace nth {
namespace imp {

/**
 * @brief Lookup table with 256 elements for reciprocal CRC. Polynomial is 
 * reflected and shift changed from MSB-to-LSB to LSB-to-MSB, resulting in 
 * right instead of left shift. For more info:
 * http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html#ch73
 * 
 * @param poly Polynomial
 */
template<std::integral auto poly>
inline constexpr auto crc_table = [] () 
{
    using T = decltype(poly);
    std::array<T, 256> table = {};
    for (int dividend = 0; dividend < 256; ++dividend) {
        T remainder = dividend;
        for (int i = 8; i; --i) {
            if (remainder & 1)
                remainder = (remainder >> 1) ^ bitswap(poly);
            else
                remainder = (remainder >> 1);
        }
        table[dividend] = remainder;
    }
    return table;
}();

}

// ANCHOR Slow

/**
 * @brief Get initial value. Actually unnecesarry for slow version.
 * 
 * @tparam T Integer type
 * @tparam init Initial value
 * @return Initial value
 */
template<std::integral T, T init>
constexpr T crc_slow_init()
{
    return init;
}

/**
 * @brief Calculate running CRC without lookup table, 
 * starting with a given value.
 * 
 * @tparam T Integer type
 * @tparam poly Polynomial
 * @tparam refin Reflect input bytes
 * @param val Running CRC value
 * @param data Data to calculate CRC on
 * @return CRC value
 */
template<std::integral T, T poly, bool refin>
constexpr T crc_slow_feed(T val, std::span<const byte> data)
{
    constexpr int bits = sizeof(T) * 8;
    constexpr T topbit = T(1) << (bits - 1);

    for (auto b : data) {
        if constexpr (refin)
            b = bitswap(b);
        val ^= T(b) << (bits - 8);
        for (int i = 8; i; --i)
            val = val & topbit ? (val << 1) ^ poly : (val << 1);
    }
    return val;
}

/**
 * @brief Finalize CRC result by optional reflection and xoring. 
 * 
 * @tparam T Integer type
 * @tparam xorout Value to XOR with
 * @tparam refout Reflection flag
 * @param val Current value
 * @return Final CRC value
 */
template<std::integral T, T xorout, bool refout>
constexpr T crc_slow_stop(T val)
{
    if constexpr (refout)
        val = bitswap(val);
    return val ^ xorout;
}

/**
 * @brief Calculate CRC without lookup table. This is 
 * obviously slower, but saves 256 x sizeof(T) bytes,
 * which can be crucial in embedded systems.
 * 
 * @tparam T Integer type
 * @tparam poly Polynomial
 * @tparam init Initial value
 * @tparam xorout Value to XOR with final result
 * @tparam refin Reflect input bytes
 * @tparam refout Reflect output value
 * @param data Data to calculate CRC on
 * @return CRC value
 */
template<std::integral T, T poly, T init, T xorout, bool refin, bool refout>
constexpr T crc_slow(std::span<const byte> data)
{
    auto val = crc_slow_feed<T, poly, refin>(init, data);
    return crc_slow_stop<T, xorout, refout>(val);
}

// ANCHOR Fast

/**
 * @brief Get prepared (reflected) initial value.
 * 
 * @tparam T Integer type
 * @tparam init Initial value
 * @return Reflected initial value
 */
template<std::integral T, T init>
constexpr T crc_fast_init()
{
    return bitswap(init);
}

/**
 * @brief Calculate running CRC using lookup table, 
 * starting with a given value.
 * 
 * @tparam T Integer type
 * @tparam poly Polynomial
 * @tparam refin Reflect input bytes
 * @param val Running CRC value
 * @param data Data to calculate CRC on
 * @return CRC value
 */
template<std::integral T, T poly, bool refin>
constexpr T crc_fast_feed(T val, std::span<const byte> data)
{
    for (auto b : data) {
        if constexpr (!refin)
            b = bitswap(b);
        val = imp::crc_table<poly>[(val ^ b) & 0xff] ^ (val >> 8);
    }
    return val;
}

/**
 * @brief Finalize CRC result by optional reflection and xoring. 
 * 
 * @tparam T Integer type
 * @tparam xorout Value to XOR with
 * @tparam refout Reflection flag
 * @param val Current value
 * @return Final CRC value
 */
template<std::integral T, T xorout, bool refout>
constexpr T crc_fast_stop(T val)
{
    if constexpr (!refout)
        val = bitswap(val);
    return val ^ xorout;
}

/**
 * @brief Calculate CRC using lookup table. This is faster, 
 * but requires 256 x sizeof(T) bytes, which isn't always
 * possible in embedded systems.
 * 
 * @tparam T Integer type
 * @tparam poly Polynomial
 * @tparam init Initial value
 * @tparam xorout Value to XOR with final result
 * @tparam refin Reflect input bytes
 * @tparam refout Reflect output value
 * @param data Data to calculate CRC on
 * @return CRC value
 */
template<std::integral T, T poly, T init, T xorout, bool refin, bool refout>
constexpr T crc_fast(std::span<const byte> data)
{
    auto val = crc_fast_feed<T, poly, refin>(bitswap(init), data);
    return crc_fast_stop<T, xorout, refout>(val);
}

}

#endif