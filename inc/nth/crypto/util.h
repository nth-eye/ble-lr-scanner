#ifndef NTH_CRYPTO_UTIL_H
#define NTH_CRYPTO_UTIL_H

#include "nth/util/string.h"

namespace nth {

using std::swap;

/**
 * @brief Input (const) span of bytes.
 * 
 * @tparam N Size
 */
template<size_t N = std::dynamic_extent>
struct ispan : std::span<const byte, N> {
    using base = std::span<const byte, N>;
    using base::base;
    ispan(const void* str, size_t len) : base{reinterpret_cast<const byte*>(str), len} {}
    ispan(std::string_view str) : base{reinterpret_cast<const byte*>(str.data()), str.size()} {}
};

/**
 * @brief Output (non-const) span of bytes.
 * 
 * @tparam N Size
 */
template<size_t N = std::dynamic_extent>
using ospan = std::span<byte, N>;


/**
 * @brief Copy from one memory region to another.
 * 
 * @param dst Pointer to destination objects
 * @param src Pointer to source objects
 * @param cnt Number of objects
 */
template<class T>
constexpr void copy(T* dst, const T* src, size_t cnt)
{
    std::copy(src, src + cnt, dst);
}

/**
 * @brief Fill memory with given object value.
 * 
 * @tparam T Object type
 * @param dst Pointer to objects to fill
 * @param val Object value to fill with
 * @param cnt Number of objects
 */
template<class T>
constexpr void fill(T* dst, const T& val, size_t cnt)
{
    std::fill_n(dst, cnt, val);
}

/**
 * @brief Reliably zero out memory region.
 * 
 * @tparam T Object type
 * @param dst Pointer to objects to zero out
 * @param cnt Number of objects
 */
template<class T>
constexpr void zero(T* dst, size_t cnt)
{
    std::fill_n(static_cast<volatile T*>(dst), cnt, T(0));
}

/**
 * @brief XOR block of bytes with given byte.
 * 
 * @param dst Destination array
 * @param val Byte to XOR with
 */
template<size_t N>
constexpr void xorb(byte (&dst)[N], byte val)
{
    for (size_t i = 0; i < N; ++i)
        dst[i] ^= val;
}

/**
 * @brief XOR block of bytes with another. Arrays must be of equal 
 * length and valid pointers!
 * 
 * @param x Destination array
 * @param y Another array
 * @param len Length of a block in bytes
 */
constexpr void xorb(byte* x, const byte* y, size_t len)
{
    for (size_t i = 0; i < len; ++i)
        x[i] ^= y[i];
}

/**
 * @brief CRTP base class for general consumer API, e.g. hash, mac, etc.
 * 
 * @tparam H Hash class
 * @tparam N Hash size
 */
template<class H, size_t N>
struct consumer {
    static constexpr auto hash_size = N;
public:
    constexpr void operator()(ispan<> key, ispan<> msg, ospan<hash_size> out)
    {
        impl()->init(key);
        impl()->feed(msg);
        impl()->stop(out);
    }
    constexpr void operator()(ispan<> in, ospan<hash_size> out)
    {
        impl()->init();
        impl()->feed(in);
        impl()->stop(out);
    }
    constexpr ~consumer()   { impl()->wipe(); }
private:
    constexpr auto impl()   { return static_cast<H*>(this); }
    friend H;
};

}

#endif