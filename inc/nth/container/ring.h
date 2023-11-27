#ifndef NTH_CONTAINER_RING_H
#define NTH_CONTAINER_RING_H

#include "nth/util/storage.h"

namespace nth {

/**
 * @brief Ring buffer implementation with unmasked indices logic.
 * Provides storage capacity of N, which must be power of 2.
 * 
 * @tparam T Type of elements
 * @tparam N Maximum number of elements, must be power of 2
 */
template<class T, size_t N>
struct ring {

    template<class U>
    struct iterator_base {
        
        using self              = iterator_base;
        using value_type        = U;
        using reference         = value_type&;
        using pointer           = value_type*;
        using size_type         = size_t;
        using difference_type   = ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        constexpr iterator_base() = default;
        constexpr iterator_base(const iterator_base<value_type>& other) = default;
        constexpr iterator_base(const iterator_base<std::remove_const_t<value_type>>& other) requires std::is_const_v<value_type>
            : buf{other.buf}, idx{other.idx}
        {}
        constexpr iterator_base(pointer buf, size_type idx) 
            : buf{buf}, idx{idx} 
        {}
        constexpr self&     operator+=(size_type rhs)           { idx += rhs; return *this; }
        constexpr self&     operator-=(size_type rhs)           { idx -= rhs; return *this; }
        constexpr self&     operator++()                        { ++idx; return *this; }
        constexpr self&     operator--()                        { --idx; return *this; }
        constexpr self      operator++(int)                     { auto tmp = *this; ++(*this); return tmp; }
        constexpr self      operator--(int)                     { auto tmp = *this; --(*this); return tmp; }
        constexpr self      operator+(const self& rhs)          { return {buf, idx + rhs.idx}; }
        constexpr auto      operator-(const self& rhs)          { return difference_type(idx - rhs.idx); }
        constexpr reference operator[](size_type n) const       { return  buf(mask(idx + n)); }
        constexpr reference operator*() const                   { return  buf[mask(idx)]; }
        constexpr pointer   operator->() const                  { return &buf[mask(idx)]; }

        friend constexpr auto operator<=>(const self& lhs, const self& rhs) { return lhs.idx <=> rhs.idx; }
        friend constexpr bool operator==(const self& lhs, const self& rhs)  { return lhs.idx == rhs.idx; }
        friend constexpr auto operator-(const self& lhs, const self& rhs)   { return difference_type(lhs.idx - rhs.idx); }
        friend constexpr self operator+(const self& lhs, size_type rhs)     { return {lhs.buf, lhs.idx + rhs}; }
        friend constexpr self operator-(const self& lhs, size_type rhs)     { return {lhs.buf, lhs.idx - rhs}; }
        friend constexpr self operator+(size_type lhs, const self& rhs)     { return {rhs.buf, rhs.idx + lhs}; }
        friend constexpr self operator-(size_type lhs, const self& rhs)     { return {rhs.buf, rhs.idx - lhs}; }
        friend class iterator_base<std::add_const_t<value_type>>;
        friend ring;
    private:
        pointer buf = nullptr;
        size_type idx = 0;
    };

    // ANCHOR Member types

    using value_type                = T;
    using size_type                 = size_t;
    using difference_type           = ptrdiff_t;
    using pointer                   = value_type*;
    using const_pointer             = const value_type*;
    using reference                 = value_type&;
    using const_reference           = const value_type&;
    using universal_reference       = value_type&&;
    using array_range               = std::span<value_type>;
    using const_array_range         = std::span<const value_type>; 
    using iterator                  = iterator_base<value_type>;
    using const_iterator            = iterator_base<const value_type>;
    using reverse_iterator          = std::reverse_iterator<iterator>;
    using const_reverse_iterator    = std::reverse_iterator<const_iterator>;

    static_assert(std::random_access_iterator<iterator>);

    // ANCHOR Constructors

    constexpr ring() noexcept = default;
    constexpr ring(const ring& other) noexcept(std::is_nothrow_copy_constructible_v<value_type>) : tail{other.size()}
    {
        copy_create_forward(other.begin(), other.end(), pbegin());
    }
    constexpr ring(ring&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>) : tail{other.size()}
    {
        move_create_forward(other.begin(), other.end(), pbegin());
    }
    constexpr explicit ring(size_type n) : tail{n}
    {
        assert(capacity() >= n);
        ctor_n(pbegin(), n);
    }
    constexpr explicit ring(size_type n, const_reference x) : tail{n}
    {
        assert(capacity() >= n);
        ctor_n(pbegin(), n, x);
    }
    template <std::input_or_output_iterator It>
    constexpr ring(It first, It last) : tail{size_type(last - first)}
    {
        assert(capacity() >= tail);
        copy_create_forward(first, last, pbegin());
    }
    constexpr ring(std::span<const value_type> array) : tail{array.size()}
    {
        assert(capacity() >= tail);
        copy_create_forward(array.begin(), array.end(), pbegin());
    }
    template<size_type Size>
    constexpr ring(value_type(&&array)[Size]) : tail{Size}
    {
        static_assert(N >= Size);
        move_create_forward(array, array + Size, pbegin());
    }

    // ANCHOR Desctructor

    constexpr ~ring() noexcept
    {
        clear();
    }

    // ANCHOR Assingment operator

    constexpr ring& operator=(const ring& other) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
    {
        if (&other != this) {
            impl_assign_prepare(other.size());
            copy_create_forward(other.begin(), other.end(), pbegin());
        }
        return *this;
    }
    constexpr ring& operator=(ring&& other) noexcept(std::is_nothrow_move_assignable_v<value_type>)
    {
        if (&other != this) {
            impl_assign_prepare(other.size());
            move_create_forward(other.begin(), other.end(), pbegin());
        }
        return *this;
    }
    constexpr ring& operator=(std::span<const value_type> array)
    {
        assign(array);
        return *this;
    }
    template<size_type Size>
    constexpr ring& operator=(value_type(&&array)[Size])
    {
        assign(std::move(array));
        return *this;
    }

    // ANCHOR Assign

    constexpr void assign(size_type n, const_reference x)
    {
        impl_assign_prepare(n);
        ctor_n(pbegin(), n, x);
    }
    template <std::input_iterator It>
    constexpr void assign(It first, It last)
    {
        impl_assign_prepare(last - first);
        copy_create_forward(first, last, pbegin());
    }
    constexpr void assign(std::span<const value_type> array)
    {
        impl_assign_prepare(array.size());
        copy_create_forward(array.begin(), array.end(), pbegin());
    }
    template<size_type Size>
    constexpr void assign(value_type(&&array)[Size])
    {
        static_assert(N >= Size);
        impl_assign_prepare(Size);
        move_create_forward(array, array + Size, pbegin());
    }

    // ANCHOR Capacity

    constexpr static size_type capacity()       { return N; }
    constexpr static size_type max_size()       { return N; }
    constexpr size_type size() const noexcept   { return tail - head; }
    constexpr bool empty() const noexcept       { return tail == head; }
    constexpr bool full() const noexcept        { return size() == capacity(); }
    constexpr bool linear() const noexcept      { return plain() || empty(); }

    // ANCHOR Iterators

    constexpr iterator begin() noexcept                 { return {buf, head}; }
    constexpr const_iterator begin() const noexcept     { return {buf, head}; }
    constexpr const_iterator cbegin() const noexcept    { return begin(); }
    constexpr iterator end() noexcept                   { return {buf, tail}; }
    constexpr const_iterator end() const noexcept       { return {buf, tail}; }
    constexpr const_iterator cend() const noexcept      { return end(); }

    constexpr reverse_iterator rbegin() noexcept                { return std::reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept    { return std::reverse_iterator(end()); }
    constexpr const_reverse_iterator crbegin() const noexcept   { return rbegin(); }
    constexpr reverse_iterator rend() noexcept                  { return std::reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept      { return std::reverse_iterator(begin()); }
    constexpr const_reverse_iterator crend() const noexcept     { return rend(); }

    // ANCHOR Access

    constexpr reference operator[](size_type i)             { return at(head + i); }
    constexpr const_reference operator[](size_type i) const { return at(head + i); }
    constexpr reference front()                             { return at(head); }
    constexpr const_reference front() const                 { return at(head); }
    constexpr reference back()                              { return at(tail - 1); }
    constexpr const_reference back() const                  { return at(tail - 1); }
    constexpr array_range array_one()                       { return {pfront(), hless() || empty() ?  ptail() :  pend()}; }
    constexpr array_range array_two()                       { return {pbegin(), hless() || empty() ? pbegin() : ptail()}; }
    constexpr const_array_range array_one() const           { return {pfront(), hless() || empty() ?  ptail() :  pend()}; }
    constexpr const_array_range array_two() const           { return {pbegin(), hless() || empty() ? pbegin() : ptail()}; }
    
    // ANCHOR Modifiers

    constexpr void clear() noexcept
    {
        dtor(array_one());
        dtor(array_two());
        head = tail = 0;
    }

    constexpr void erase_begin(size_type n)
    {
        assert(size() >= n);

        auto rem = mask(head + n);

        if (mask(head) <= rem) {
            dtor(pfront(), pfront() + n);
        } else {
            dtor(pfront(), pend());
            dtor(pbegin(), pbegin() + rem);
        }
        head += n;
    }

    constexpr void erase_end(size_type n)
    {
        assert(size() >= n);

        auto rem = mask(tail - n);

        if (mask(tail - 1) >= rem) {
            dtor(pback() - n + 1, pback() + 1);
        } else {
            dtor(pend() - rem, pend());
            dtor(pbegin(), ptail());
        }
        tail -= n;
    }

    constexpr void pop_back()
    {
        assert(empty() == false);
        dtor(on(--tail));
    }

    constexpr void pop_front()
    {
        assert(empty() == false);
        dtor(on(head++));
    }

    constexpr void push_back(const_reference x)
    {
        emplace_back(x);
    }
    
    constexpr void push_back(universal_reference x)
    {
        emplace_back(std::move(x));
    }

    constexpr void push_front(const_reference x)
    {
        emplace_front(x);
    }
    
    constexpr void push_front(universal_reference x)
    {
        emplace_front(std::move(x));
    }

    template<class... Args>
    constexpr reference emplace_back(Args&&... args)
    {
        assert(full() == false);
        return *ctor(on(tail++), std::forward<Args>(args)...);
    }

    template<class... Args>
    constexpr reference emplace_front(Args&&... args)
    {
        assert(full() == false);
        return *ctor(on(--head), std::forward<Args>(args)...);
    }

    template<class... Args>
    constexpr reference put_back(Args&&... args)
    {
        if (full())
            pop_front();
        return emplace_back(std::forward<Args>(args)...);
    }

    template<class... Args>
    constexpr reference put_front(Args&&... args)
    {
        if (full())
            pop_back();
        return emplace_front(std::forward<Args>(args)...);
    }

    template<class... Args>
    constexpr void yeet_back(Args&&... args)
    {
        if (full() == false)
            emplace_back(std::forward<Args>(args)...);
    }

    template<class... Args>
    constexpr void yeet_front(Args&&... args)
    {
        if (full() == false)
            emplace_front(std::forward<Args>(args)...);
    }

    constexpr bool get_back(reference& item)
    {
        if (empty()) 
            return false;
        item = std::move(back());
        pop_back(); 
        return true; 
    }

    constexpr bool get_front(reference& item)
    { 
        if (empty()) 
            return false;
        item = std::move(front());
        pop_front(); 
        return true; 
    }

    constexpr void rotate(const_iterator new_begin)
    {
        assert(valid(new_begin));

        if (full()) {
            head = new_begin.idx;
            tail = new_begin.idx + N;
        } else {
            auto m = tail - new_begin.idx;
            auto n = new_begin.idx - head;
            if (m < n) {
                while (m--) {
                    push_front(std::move(back()));
                    pop_back();
                }
            } else {
                while (n--) {
                    push_back(std::move(front()));
                    pop_front();
                }
            }
        }
    }

    constexpr pointer linearize()
    {
        if (empty())
            return nullptr;

        if (linear())
            return pfront();

        auto src = pfront();
        auto dst = pbegin();
        auto end = pbegin() + size() - full();

        while (dst != end) {
            if (src == pend())
                src = pfront();
            std::swap(*src++, *dst++);
        }
        tail = size();
        head = 0;
        
        return pbegin();
    }

    constexpr void swap(ring& other) noexcept(std::is_nothrow_swappable_v<value_type>)
    {
        auto tmp    = std::move(other);
        other       = std::move(*this);
        *this       = std::move(tmp);
    }
public:
    static constexpr auto M = N - 1;
    static_assert(N > 1 && !(M & N), "ring size must be > 1 and power of 2");   
    static constexpr auto mask(size_type val)   { return val & M; }
    constexpr auto& at(size_type idx)           { return  buf[mask(idx)]; }
    constexpr auto& at(size_type idx) const     { return  buf[mask(idx)]; }
    constexpr auto on(size_type idx)            { return &buf[mask(idx)]; }
    constexpr auto on(size_type idx) const      { return &buf[mask(idx)]; }
    constexpr auto pbegin()                     { return &buf[0]; }
    constexpr auto pbegin() const               { return &buf[0]; }
    constexpr auto pend()                       { return &buf[N]; }
    constexpr auto pend() const                 { return &buf[N]; }
    constexpr auto pfront()                     { return on(head); }
    constexpr auto pfront() const               { return on(head); }
    constexpr auto pback()                      { return on(tail - 1); }
    constexpr auto pback() const                { return on(tail - 1); }
    constexpr auto ptail()                      { return on(tail); }
    constexpr auto ptail() const                { return on(tail); }
    constexpr bool hless() const                { return mask(head) < mask(tail); } // head is less than tail
    constexpr bool plain() const                { return mask(head) <= mask(tail - 1); }
    constexpr bool valid(const_iterator pos) const
    { 
        return  pos.buf == buf && 
                pos.idx < tail &&
                pos.idx >= head;
    }
    constexpr void impl_assign_prepare(size_type n)
    {
        assert(capacity() >= n);
        dtor(array_one());
        dtor(array_two());
        head = 0;
        tail = n;
    }
public:
    storage<T, N> buf;
    size_type head = 0; // First item index / beginning of the buffer
    size_type tail = 0; // Last item index
};

}

#endif