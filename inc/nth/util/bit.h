#ifndef NTH_UTIL_BIT_H
#define NTH_UTIL_BIT_H

#include "nth/util/meta.h"
#include <bit>

namespace nth {

/**
 * @brief Check if platform is little endian.
 * 
 * @return true if little endian
 */
constexpr bool little_endian()
{ 
    return std::endian::native == std::endian::little; 
}

/**
 * @brief Change argument type to its unsigned counterpart. 
 * Doesn't negate sign of argument, i.e. preserves bit pattern.
 * 
 * @param x Signed integer
 * @return Same value as unsigned type
 */
constexpr auto to_unsigned(std::integral auto x)
{
    if constexpr (std::is_signed_v<decltype(x)>)
        return std::make_unsigned_t<decltype(x)>(x);
    else
        return x;
}

/**
 * @brief Check if integer is power of 2.
 * 
 * @param n Integer
 * @return true if power of 2
 */
constexpr bool is_pow2(std::integral auto n)
{
    return n && !(n & (n - 1));
}

/**
 * @brief Size of object in bits.
 * 
 * @tparam T Object type
 */
template<class T>
inline constexpr auto bit_size = sizeof(T) * CHAR_BIT;

/**
 * @brief Integer with all bits set to 1.
 * 
 * @tparam T Integer type
 */
template<std::integral T>
inline constexpr auto int_bits_full = T(-1);

/**
 * @brief Wrap around mask for power of two number of bits
 * within given integer type. For example: 
 * [ int_bits_wrap<uint8_t>  = 8  - 1 = 0b111   ]
 * [ int_bits_wrap<uint16_t> = 16 - 1 = 0b1111  ]
 * [ int_bits_wrap<uint32_t> = 32 - 1 = 0b11111 ]
 * 
 * @tparam T Integer type
 */
template<std::integral T>
inline constexpr auto int_bits_wrap = T(bit_size<T> - 1);

/**
 * @brief Number of bits to shift to divide by bit_size<T>, 
 * bit width of (bit_size<T> - 1)). For example: 
 * int_bits_log2<uint8_t>  = bit_width(0b111)      = 3;
 * int_bits_log2<uint16_t> = bit_width(0b1111)     = 4;
 * int_bits_log2<uint32_t> = bit_width(0b11111)    = 5;
 * 
 * @tparam T Integer type
 */
template<std::integral T>
inline constexpr auto int_bits_log2 = std::bit_width(int_bits_wrap<T>);

namespace imp {

/**
 * @brief Reverse bits within integer with given granularity.
 * 
 * @param x Integer to swap
 * @param swap_degree Size of reversable chunk in bits, must be power of 2
 * @return Result
 */
constexpr auto intswap(std::integral auto x, unsigned swap_degree)
{
    auto bits = bit_size<decltype(x)>;
    auto mask = int_bits_full<decltype(x)>;
    while ((bits >>= 1) >= swap_degree) {
        mask ^= mask << bits;
        x = (x & ~mask) >> bits | (x & mask) << bits;
    }
    return x;
}

}

/**
 * @brief Fast round up division by number of bits within given integer type.
 * 
 * @tparam T Integer type
 * @param x Dividend
 * @return Quotient
 */
template<std::integral T>
constexpr auto int_bits_ceil_div(auto x)
{
    return (x + int_bits_wrap<T>) >> int_bits_log2<T>;
}

/**
 * @brief Get number of words (integers) required to store N bits.
 * 
 * @tparam T Word integer type
 * @param n Number of bits to store
 * @return Number of words
 */
template<std::integral T>
constexpr size_t words_in_bits(size_t n)
{
    return (n >> int_bits_log2<T>) + !!(n & int_bits_wrap<T>); 
}

/**
 * @brief Get number of bytes required to store N bits.
 * 
 * @param n Number of bits to store
 * @return Number of bytes
 */
constexpr size_t bytes_in_bits(size_t n)
{ 
    return words_in_bits<uint8_t>(n);
}

/**
 * @brief Make integer with bit at given position.
 * 
 * @tparam T Integer type
 * @param n Bit position
 * @return Integer with set bit
 */
template<std::integral T = unsigned> 
constexpr T bit(unsigned n)  
{
    assert(n < bit_size<T>);
    return T(1) << n;  
}

/**
 * @brief Get mask with given number of LSB bits set, e.g. 
 * bit_mask(0) = 0;
 * bit_mask(1) = 1;
 * bit_mask(2) = 3;
 * bit_mask(3) = 7;
 * @param x Number of bits to set
 * @return Mask for number of bits
 */
template<std::integral T = unsigned> 
constexpr auto bit_mask(unsigned x)
{
    return T(bit<T>(x) - 1);
}

/**
 * @brief Get n-th bit of an integer.
 * 
 * @param x Integer
 * @param n Bit position from LSB
 * @return true if set
 */
template<std::integral T>
constexpr bool get_bit(T x, unsigned n)
{
    assert(n < bit_size<T>);
    return (x >> n) & 1; 
}

/**
 * @brief Set n-th bit of an integer.
 * 
 * @param x Integer
 * @param n Bit position from LSB
 */
template<std::integral T>
constexpr void set_bit(T& x, unsigned n)
{
    assert(n < bit_size<T>);
    x |= T(1) << n; 
}

/**
 * @brief Clear n-th bit of an integer.
 * 
 * @param x Integer
 * @param n Bit position from LSB
 */
template<std::integral T>
constexpr void clr_bit(T& x, unsigned n)
{
    assert(n < bit_size<T>);
    x &= ~(T(1) << n); 
}

/**
 * @brief Get n-th bit in array of words (starting from LSB).
 * 
 * @tparam T Integer type
 * @param p Array of words
 * @param n Index of bit to get
 * @return true if set
 */
template<std::integral T>
constexpr bool get_arr_bit(const T* p, unsigned n)
{
    return get_bit(p[n >> int_bits_log2<T>], n & int_bits_wrap<T>);
}

/**
 * @brief Set n-th bit in array of words (starting from LSB).
 * 
 * @tparam T Integer type
 * @param p Array of words
 * @param n Index of bit to set
 */
template<std::integral T>
constexpr void set_arr_bit(T* p, unsigned n)
{
    set_bit(p[n >> int_bits_log2<T>], n & int_bits_wrap<T>);
}

/**
 * @brief Clear n-th bit in array of words (starting from LSB).
 * 
 * @tparam T Integer type
 * @param p Array of words
 * @param n Index of bit to clear
 */
template<std::integral T>
constexpr void clr_arr_bit(T* p, unsigned n)
{
    clr_bit(p[n >> int_bits_log2<T>], n & int_bits_wrap<T>);
}

/**
 * @brief Shift bits left in array of integers from least significant 
 * bit, considering 0-th byte as the right most.
 * uint16_t example: 0b10000000'11100001 ==> 0b00000001'11000010. 
 * 
 * @tparam T Integer type
 * @tparam L Length of array
 * @param x Array of integers
 */
template<std::integral T, size_t L>
constexpr void shift_arr_left(T (&x)[L])
{
    for (size_t i = L - 1; i > 0; --i) {
        x[i] <<= 1;
        x[i] |= x[i - 1] >> int_bits_wrap<T>;
    }
    x[0] <<= 1;
}

/**
 * @brief Shift bits right in array of integers from least significant 
 * bit, considering 0-th byte as the right most.
 * uint16_t example: 0b10000000'11100001 ==> 0b01000000'01110000. 
 * 
 * @tparam T Integer type
 * @tparam L Length of array
 * @param x Array of integers
 */
template<std::integral T, size_t L>
constexpr void shift_arr_right(T (&x)[L])
{
    for (size_t i = 0; i < L - 1; ++i) {
        x[i] >>= 1;
        x[i] |= x[i + 1] << int_bits_wrap<T>;
    }
    x[L - 1] >>= 1;
}

/**
 * @brief Reverse integral words in the given integer.
 * 
 * @tparam Swap Type of word which corresponds to chunk of reversion
 * @tparam T Type of integer to reverse
 * @param x Integer to reverse
 * @return Result
 */
template<std::integral Swap, std::integral T>
constexpr auto wordswap(T x)
{
    static_assert(sizeof(T) > 1);
    return imp::intswap(x, bit_size<Swap>);
}

/**
 * @brief Reverse bytes in the given integer.
 * 
 * @param x Integer to reverse
 * @return Result
 */
constexpr auto byteswap(std::integral auto x)
{
    return wordswap<byte>(x);
}

/**
 * @brief Reverse bits in the given integer.
 * 
 * @param x Integer to reverse
 * @return Result
 */
constexpr auto bitswap(std::integral auto x)
{
    return imp::intswap(x, 1);
}

/**
 * @brief Count leading zeros. Signed integer will be
 * casted to unsigned without changing bit pattern.
 * 
 * @param x Integer argument
 * @return Number of leading zeros
 */
constexpr auto cntlz(std::integral auto x)
{
    return std::countl_zero(to_unsigned(x));
}

/**
 * @brief Count trailing zeros. Signed integer will be
 * casted to unsigned without changing bit pattern.
 * 
 * @param x Integer argument
 * @return Number of trailing zeros
 */
constexpr auto cnttz(std::integral auto x)
{
    return std::countr_zero(to_unsigned(x));
}

/**
 * @brief Circular bit rotation of an integer to the left.
 * 
 * @param x Integer to rotate
 * @param s Shift
 * @return Result
 */
constexpr auto rol(std::integral auto x, int s)
{
    return std::rotl(to_unsigned(x), s);
}

/**
 * @brief Circular bit rotation of an integer to the right.
 * 
 * @param x Integer to rotate
 * @param s Shift
 * @return Result
 */
constexpr auto ror(std::integral auto x, int s)
{
    return std::rotr(to_unsigned(x), s);
}

/**
 * @brief Convert bytes to integer starting from least significant byte.
 * 
 * @tparam T Integer type
 * @param x Least significant byte
 * @param args Other bytes
 * @return Integer of type T
 */
template<std::integral T>
constexpr auto make_int_lsb(auto x, auto... args)
{
    static_assert(sizeof...(args) == sizeof(T) - 1);
    static_assert(sizeof(T) > 1);
    // static_assert((std::is_same_v<decltype(args), byte>, ...));
    int i = 0;
    T ret = x;
    ((ret |= T(args) << (i += 8)), ...);
    return ret;
}

/**
 * @brief Convert bytes to integer starting from most significant byte.
 * 
 * @tparam T Integer type
 * @param x Most significant byte
 * @param args Other bytes
 * @return Integer of type T
 */
template<std::integral T>
constexpr auto make_int_msb(auto x, auto... args)
{
    static_assert(sizeof...(args) == sizeof(T) - 1);
    static_assert(sizeof(T) > 1);
    // static_assert((std::is_same_v<decltype(args), byte>, ...));
    T ret = x;
    ((ret = T(args) | (ret << 8)), ...);
    return ret;
}

/**
 * @brief Put integer into array in little endian order.
 * 
 * @tparam T Integer type
 * @param val Input integer
 * @param out Output array
 */
template<std::integral T>
constexpr void putle(T val, byte* out)
{
    for (size_t i = 0; i < bit_size<T>; i += 8)
        *out++ = val >> i;
}

/**
 * @brief Put integer into array in big endian order.
 * 
 * @tparam T Integer type
 * @param val Input integer
 * @param out Output array
 */
template<std::integral T>
constexpr void putbe(T val, byte* out)
{
    for (int i = bit_size<T> - 8; i >= 0; i -= 8)
        *out++ = val >> i;
}

/**
 * @brief Get integer from array in little endian order.
 * 
 * @tparam T Integer type
 * @param in Input array
 * @return Result integer
 */
template<std::integral T>
constexpr T getle(const byte* in)
{
    T val = 0;
    for (size_t i = 0; i < bit_size<T>; i += 8)
        val |= T(*in++) << i;
    return val;
}

/**
 * @brief Get integer from array in big endian order.
 * 
 * @tparam T Integer type
 * @param in Input array
 * @return Result integer
 */
template<std::integral T>
constexpr T getbe(const byte* in)
{
    T val = 0;
    for (int i = bit_size<T> - 8; i >= 0; i -= 8)
        val |= T(*in++) << i;
    return val;
}

}

#endif
