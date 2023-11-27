#ifndef NTH_UTIL_META_H
#define NTH_UTIL_META_H

#include <cstdint>
#include <cstddef>
#include <climits>
#include <cassert>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <algorithm>
#include <concepts>
#include <span>

namespace nth {

/**
 * @brief Macro to prevent function inlining across different compilers. 
 * Preventing inlining can be useful for debugging, reducing binary size, 
 * or meeting specific runtime requirements.
 * 
 */
#ifdef __clang__
#define NTH_NOINLINE    __attribute__((noinline))
#elif defined(__GNUC__)
#define NTH_NOINLINE    __attribute__((noinline))
#elif defined(_MSC_VER)
#define NTH_NOINLINE    __declspec(noinline)
#endif

/**
 * @brief Typedef representing 8-bit object in some library parts.  
 * 
 */
using byte = uint8_t;

/**
 * @brief Concept for strong boolean.
 * 
 * @tparam T Type to check
 */
template<class T>
concept boolean = std::is_same<T, bool>::value;

/**
 * @brief Concept for strong integral (fails for boolean).
 * 
 * @tparam T Type to check
 */
template<class T>
concept integer = std::integral<T> && !boolean<T>;

/**
 * @brief Concept for arithmetic types (integral and floating).
 * 
 * @tparam T Type to check
 */
template<class T> 
concept arithmetic = std::is_arithmetic_v<T>;

/**
 * @brief Concept for convertible to std::string_view.
 * 
 * @tparam T Type to check
 */
template<class T> 
concept stringable = std::convertible_to<T, std::string_view>;

/**
 * @brief Concept for enumeration types.
 * 
 * @tparam T Type to check
 */
template<class T>
concept enumeration = std::is_enum_v<T>;

/**
 * @brief Shortcut for converting strong enum to underlying 
 * integer type. 
 * 
 * @tparam T Enum class type
 * @param e Enumeration value
 * @return Value of underlying integer type 
 */
template<enumeration T>
constexpr auto operator+(T e)
{
    return static_cast<std::underlying_type_t<T>>(e);
}

/**
 * @brief Helper to get number of elements in array. 
 * 
 * @tparam T Auto-deduced element type
 * @tparam N Auto-deduced number of elements
 * @return Array size
 */
template<class T, size_t N>
constexpr size_t countof(T(&)[N]) 
{ 
    return N; 
}

/**
 * @brief Stub to fail consteval conditionally.
 * 
 */
void consteval_failure(const char*);

}

#endif
