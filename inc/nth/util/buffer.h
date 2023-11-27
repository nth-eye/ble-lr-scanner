#ifndef NTH_UTIL_BUFFER_H
#define NTH_UTIL_BUFFER_H

#include "nth/util/meta.h"

#define NTH_UTIL_BUFFER_SAFE_HANDLER    false
#define NTH_UTIL_BUFFER_CONST_INTERFACE true

namespace nth {

/**
 * @brief Special constant representing a buffer with dynamic extent size.
 * 
 */
inline constexpr size_t dynamic_extent = -1;

/**
 * @brief Special constant representing an interface-wrapper of a buffer.
 * 
 */
inline constexpr size_t wrapper_extent = 0;

#if (NTH_UTIL_BUFFER_CONST_INTERFACE)

/**
 * @brief Special constant representing a constant interface-wrapper of a buffer.
 * 
 */
inline constexpr size_t const_wrapper_extent = -2;

#endif

/**
 * @brief Concept to check if size extent is not special value.
 * 
 * @tparam N Size extent to check
 */
template<size_t N>
#if (NTH_UTIL_BUFFER_CONST_INTERFACE)
concept static_extent = (N != dynamic_extent && N != wrapper_extent && N != const_wrapper_extent);
#else
concept static_extent = (N != dynamic_extent && N != wrapper_extent);
#endif

template<template<size_t> class Codec>
struct buffer_interface : public Codec<wrapper_extent> {
    template<size_t N>
    constexpr buffer_interface(Codec<N>& codec) noexcept requires(N != wrapper_extent)
        : Codec<wrapper_extent>(codec.buf, codec.len)
    {}
};

#if (NTH_UTIL_BUFFER_CONST_INTERFACE)

template<template<size_t> class Codec>
struct const_buffer_interface : public Codec<const_wrapper_extent> {
    template<size_t N>
    constexpr const_buffer_interface(const Codec<N>& codec) noexcept requires(N != const_wrapper_extent)
        : Codec<const_wrapper_extent>(codec.buf, codec.len)
    {}
};

#endif

template <size_t N = dynamic_extent>
struct buffer_handler {

    template<template<size_t> class Encoder>
    friend struct buffer_interface;
#if (NTH_UTIL_BUFFER_CONST_INTERFACE)
    template<template<size_t> class Encoder>
    friend struct const_buffer_interface;
#endif
#if (NTH_UTIL_BUFFER_CONST_INTERFACE)
    using size_type         = std::conditional_t<N == const_wrapper_extent, const size_t&, 
                              std::conditional_t<N == wrapper_extent, size_t&, size_t>>;
    using data_element_type = std::conditional_t<N == const_wrapper_extent, const byte, byte>;
    using input_type        = std::span<data_element_type>;
    using data_type         = std::conditional_t<N == wrapper_extent || N == const_wrapper_extent || N == dynamic_extent, 
                                std::span<data_element_type>, 
                                std::array<data_element_type, N>>;
#else
    using size_type = std::conditional_t<N == wrapper_extent, size_t&, size_t>;
    using data_type = std::conditional_t<N == wrapper_extent || N == dynamic_extent, std::span<byte>, std::array<byte, N>>;
#endif

#if (NTH_UTIL_BUFFER_SAFE_HANDLER)
    constexpr buffer_handler(std::span<byte> buffer, size_t& ref_size) 
        noexcept requires(N == wrapper_extent) : buf(buffer), len(ref_size)
    {
        // TODO
    }
    constexpr buffer_handler(std::span<byte> buffer, size_t initial_size = 0) 
        noexcept requires(N == dynamic_extent) : buf(buffer), len(std::min(initial_size, capacity()))
    {

    }
    constexpr buffer_handler(std::span<byte> buffer = {}) 
        noexcept requires(static_extent<N>) 
    {
        if (resize(buffer.size()) == buffer.size())
            std::copy_n(buffer.data(), size(), data());
    }
#else
    constexpr buffer_handler(input_type buffer, size_type ref_size)
#if (NTH_UTIL_BUFFER_CONST_INTERFACE)
        noexcept requires(N == wrapper_extent || N == const_wrapper_extent) : buf(buffer), len(ref_size)
#else
        noexcept requires(N == wrapper_extent) : buf(buffer), len(ref_size)
#endif
    {
        assert(capacity() >= len);
    }
    constexpr buffer_handler(input_type buffer, size_type initial_size = 0) 
        noexcept requires(N == dynamic_extent) : buf(buffer), len(initial_size)
    {
        assert(capacity() >= len);
    }
    constexpr buffer_handler(input_type buffer = {}) 
        noexcept requires(static_extent<N>) : len(buffer.size())
    {
        assert(capacity() >= len);
        std::copy_n(buffer.data(), buffer.size(), data());
    }
#endif
    constexpr auto capacity() const requires(static_extent<N> == false) 
    { 
        return buf.size(); 
    }
    constexpr static auto capacity() requires(static_extent<N> == true)
    { 
        return N; 
    }
    constexpr void clear()                      { len = 0; }
    constexpr auto resize(size_t new_size)      { return new_size <= capacity() ? len = new_size : len; }
    constexpr auto size() const                 { return len; }
    constexpr auto data()                       { return buf.data(); }
    constexpr auto data() const                 { return buf.data(); }
    constexpr auto begin()                      { return buf.data(); }
    constexpr auto begin() const                { return buf.data(); }
    constexpr auto end()                        { return buf.data() + size(); }
    constexpr auto end() const                  { return buf.data() + size(); }
    constexpr auto& back() const                { return buf[len]; }
    constexpr auto& back()                      { return buf[len]; }
    constexpr auto& operator[](size_t i) const  { return buf[i]; }
    constexpr auto& operator[](size_t i)        { return buf[i]; }
    constexpr void push_back(byte b)            { buf[len++] = b; }
    constexpr void push_back(std::span<const byte> b)
    { 
        std::copy_n(b.data(), b.size(), end());
        len += b.size(); 
    }
    constexpr void push_back(std::span<const char> b)
    { 
        std::copy_n(b.data(), b.size(), end());
        len += b.size(); 
    }
private:
    data_type buf;
    size_type len;
};

}

#endif