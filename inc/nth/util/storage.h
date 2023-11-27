#ifndef NTH_UTIL_STORAGE_H
#define NTH_UTIL_STORAGE_H

#include "nth/util/meta.h"
#include <memory>

namespace nth {

/**
 * @brief Uninitialized union container for any type. Uses regular C 
 * array as underlying storage. Can be used in constexpr context and
 * converts implicitly to pointer of underlying type.
 * 
 * @tparam T Type of elements
 * @tparam N Number of elements
 */
template<class T, size_t N>
union storage {
    using type = T;
    static constexpr size_t size = N;
    static_assert(N);
    constexpr storage() noexcept    {}
    constexpr ~storage() noexcept   {}
    constexpr operator auto()       { return buf; }
    constexpr operator auto() const { return buf; }
private:
    T buf[N];
    // std::byte bytes[sizeof(T) * N];
};

/**
 * @brief Properly destoy object of any given type inplace.
 * 
 * @param it Object iterator
 */
constexpr void dtor(std::forward_iterator auto it) 
{
    std::destroy_at(std::addressof(*it));
}

/**
 * @brief Properly destroy range of objects.
 * 
 * @tparam It Iterator type
 * @param head Iterator of first element
 * @param tail Iterator after last element
 */
template<std::forward_iterator It>
constexpr void dtor(It head, It tail)
{
    std::destroy(head, tail);
}

/**
 * @brief Properly destroy range of objects.
 * 
 * @tparam T Object type
 * @param span Range of elements
 */
template<class T>
constexpr void dtor(std::span<T> span)
{
    std::destroy(span.begin(), span.end());
}

/**
 * @brief Properly destroy N objects.
 * 
 * @param p Iterator of first element
 * @param n Number of elements
 */
constexpr void dtor_n(std::forward_iterator auto it, size_t n)
{
    std::destroy_n(it, n);
}

/**
 * @brief Create a T object initialized with arguments at address.
 * 
 * @tparam T Object type
 * @tparam Args Arguments used for initialization
 * @param p Pointer to the uninitialized storage
 * @param args Arguments used for initialization
 * @return Pointer to constructed object
 */
template<class T, class... Args>
constexpr T* ctor(T* p, Args&&... args)
{
    return std::construct_at(p, std::forward<Args>(args)...);
}

/**
 * @brief Create multiple T objects in address range, all 
 * initialized with given arguments.
 * 
 * @tparam T Object type
 * @tparam Args Arguments used for initialization
 * @param head Pointer to first element
 * @param tail Pointer past last element
 * @param args Arguments used for initialization
 */
template<class T, class... Args>
constexpr void ctor(T* head, T* tail, Args&&... args)
{
    while (head != tail)
        std::construct_at(head++, std::forward<Args>(args)...);
}

/**
 * @brief Create multiple T objects in address range, all 
 * initialized with given arguments.
 * 
 * @tparam T Object type
 * @tparam Args Arguments used for initialization
 * @param span Range of elements
 * @param args Arguments used for initialization
 */
template<class T, class... Args>
constexpr void ctor(std::span<T> span, Args&&... args)
{
    return ctor(span.data(), span.data() + span.size(), std::forward<Args>(args)...);
}

/**
 * @brief Create N number of T objects all initialized with arguments.
 * 
 * @tparam T Object type
 * @tparam Args Arguments used for initialization
 * @param p Pointer to the uninitialized storage for first object
 * @param n Number of objects to construct
 * @param args Arguments used for initialization
 */
template<class T, class... Args>
constexpr void ctor_n(T* p, size_t n, Args&&... args)
{
    for (size_t i = 0; i < n; ++i)
        ctor(p + i, std::forward<Args>(args)...);
}

// ANCHOR Move/Copy helpers

/**
 * Move objects from the range [first, last) in reverse order, and construct them
 * starting from the iterator before d_last.
 * 
 * @tparam InIt Input iterator type for the source range.
 * @tparam OutIt Output iterator type for the destination range.
 * @param first The beginning of the source range.
 * @param last The end of the source range.
 * @param d_last The end of the destination range.
 * @return The iterator pointing to the beginning of the constructed range.
 */
template<std::bidirectional_iterator InIt, std::bidirectional_iterator OutIt>
constexpr OutIt move_create_backward(InIt first, InIt last, OutIt d_last)
{
    while (first != last)
        ctor(std::addressof(*--d_last), std::move(*--last));
    return d_last;
}

/**
 * Move objects from the range [first, last) in forward order, and construct them
 * starting from d_first.
 * 
 * @tparam InIt Input iterator type for the source range.
 * @tparam OutIt Output iterator type for the destination range.
 * @param first The beginning of the source range.
 * @param last The end of the source range.
 * @param d_first The beginning of the destination range.
 * @return The iterator pointing to the position past the end of the constructed range.
 */
template<std::forward_iterator InIt, std::forward_iterator OutIt>
constexpr OutIt move_create_forward(InIt first, InIt last, OutIt d_first)
{
    while (first != last)
        ctor(d_first++, std::move(*first++));
    return d_first;
}

/**
 * Move objects from the range [first, last) and assign to the range of already existing 
 * objects, ending at the iterator before d_last.
 * 
 * @tparam InIt Input iterator type for the source range.
 * @tparam OutIt Output iterator type for the destination range.
 * @param first The beginning of the source range.
 * @param last The end of the source range.
 * @param d_last The end of the destination range.
 * @return The iterator pointing to the beginning of the assigned range.
 */
template<std::bidirectional_iterator InIt, std::bidirectional_iterator OutIt>
constexpr OutIt move_assign_backward(InIt first, InIt last, OutIt d_last)
{
    while (first != last)
        *--d_last = std::move(*--last);
    return d_last;
}

/**
 * Move objects from the range [first, last) and assign to the range of already existing 
 * objects, starting at d_first.
 * 
 * @tparam InIt Input iterator type for the source range.
 * @tparam OutIt Output iterator type for the destination range.
 * @param first The beginning of the source range.
 * @param last The end of the source range.
 * @param d_first The beginning of the destination range.
 * @return The iterator pointing to the position past the end of the assigned range.
 */
template<std::forward_iterator InIt, std::forward_iterator OutIt>
constexpr OutIt move_assign_forward(InIt first, InIt last, OutIt d_first)
{
    while (first != last)
        *d_first++ = std::move(*first++);
    return d_first;
}

/**
 * @brief Copy a range of objects to a new range of memory and construct new objects from the copied ones.
 * 
 * @tparam InIt Iterator type of the input range
 * @tparam OutIt Iterator type of the output range
 * @param first Iterator to the first element in the input range
 * @param last Iterator to one past the last element in the input range
 * @param d_first Iterator to the first element in the output range
 * @return OutIt Iterator to one past the last element in the output range
 */
template<std::forward_iterator InIt, std::forward_iterator OutIt>
constexpr OutIt copy_create_forward(InIt first, InIt last, OutIt d_first)
{
    while (first != last)
        ctor(d_first++, *first++);
    return d_first;
}

/**
 * @brief Copy a range of objects to an existing range of objects.
 * 
 * @tparam InIt Iterator type of the input range
 * @tparam OutIt Iterator type of the output range
 * @param first Iterator to the first element in the input range
 * @param last Iterator to one past the last element in the input range
 * @param d_first Iterator to the first element in the output range
 * @return OutIt Iterator to one past the last element in the output range
 */
template<std::forward_iterator InIt, std::forward_iterator OutIt>
constexpr OutIt copy_assign_forward(InIt first, InIt last, OutIt d_first)
{
    while (first != last)
        *d_first++ = *first++;
    return d_first;
}

}

#endif