#ifndef NTH_CONTAINER_STACK_H
#define NTH_CONTAINER_STACK_H

#include "nth/util/storage.h"

namespace nth {

/**
 * @brief Simple LIFO stack structure with pre-allocated memory.
 * Uses storage, which doesn't default construct elements 
 * and can be used in constexpr context.
 * 
 * @tparam T Type of elements
 * @tparam N Maximum number of elements
 */
template<class T, size_t N>
struct stack {

    // ANCHOR Member types

    using value_type            = T;
    using size_type             = size_t;
    using difference_type       = ptrdiff_t;
    using pointer               = value_type*;
    using const_pointer         = const value_type*;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using universal_reference   = value_type&&;

    // ANCHOR Constructors

    constexpr stack() noexcept = default;
    constexpr stack(const stack& other) noexcept(std::is_nothrow_copy_constructible_v<value_type>) : len{other.size()}
    {
        copy_create_forward(other.begin(), other.end(), begin());
    }
    constexpr stack(stack&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>) : len{other.size()}
    {
        move_create_forward(other.begin(), other.end(), begin());
    }
    template <std::input_iterator It>
    constexpr stack(It first, It last) : len{size_type(last - first)}
    {
        assert(capacity() >= size());
        copy_create_forward(first, last, begin());
    }
    constexpr stack(std::span<const value_type> array) : len{array.size()}
    {
        assert(capacity() >= size());
        copy_create_forward(array.begin(), array.end(), begin());
    }
    template<size_type Size>
    constexpr stack(value_type(&&array)[Size]) : len{Size}
    {
        static_assert(N >= Size);
        move_create_forward(array, array + Size, begin());
    }

    // ANCHOR Destructor 
    
    constexpr ~stack() noexcept
    {
        clear();
    }

    // ANCHOR Assingment operator

    constexpr stack& operator=(const stack& other) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
    {
        if (&other != this) {
            impl_assign_prepare(other.size());
            copy_create_forward(other.begin(), other.end(), begin());
        }
        return *this;
    }
    constexpr stack& operator=(stack&& other) noexcept(std::is_nothrow_move_assignable_v<value_type>)
    {
        if (&other != this) {
            impl_assign_prepare(other.size());
            move_create_forward(other.begin(), other.end(), begin());
        }
        return *this;
    }

    // ANCHOR Capacity

    constexpr static size_type capacity() noexcept  { return N; }
    constexpr static size_type max_size() noexcept  { return N; }
    constexpr size_type size() const noexcept       { return len; }
    constexpr bool empty() const noexcept           { return len == 0; }
    constexpr bool full() const noexcept            { return len == capacity(); }

    // ANCHOR Access

    constexpr reference top()               { assert(!empty()); return buf[len - 1]; }
    constexpr const_reference top() const   { assert(!empty()); return buf[len - 1]; }

    // ANCHOR Modifiers

    constexpr void clear() noexcept
    {
        impl_assign_prepare(0);
    }

    constexpr void push(const_reference x)
    {
        emplace(x);
    }

    constexpr void push(universal_reference x)
    {
        emplace(std::move(x));
    }

    template<class... Args>
    constexpr reference emplace(Args&&... args)
    {
        assert(full() == false);
        return *ctor(begin() + len++, std::forward<Args>(args)...);
    }

    constexpr void pop()
    {
        assert(empty() == false);
        dtor(begin() + --len);
    }

    constexpr value_type pop_get()
    {
        auto ret = top();
        pop();
        return ret;
    }

    constexpr void swap(stack& other) noexcept(std::is_nothrow_swappable_v<value_type>)
    {
        auto tmp    = std::move(other);
        other       = std::move(*this);
        *this       = std::move(tmp);
    }
private:
    constexpr pointer begin() noexcept              { return buf; }
    constexpr const_pointer begin() const noexcept  { return buf; }
    constexpr pointer end() noexcept                { return buf + len; }
    constexpr const_pointer end() const noexcept    { return buf + len; }
    constexpr void impl_assign_prepare(size_type n)
    {
        assert(capacity() >= n);
        dtor(begin(), end());
        len = n;
    }
private:
    storage<T, N> buf;
    size_type len = 0;
};

}

#endif