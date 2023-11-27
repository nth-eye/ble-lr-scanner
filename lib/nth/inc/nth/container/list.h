#ifndef NTH_CONTAINER_LIST_H
#define NTH_CONTAINER_LIST_H

#include "nth/container/pool.h"

namespace nth {
namespace imp {

struct node_dummy {

    constexpr void push_back(node_dummy* n) 
    {
        n->next = this;
        n->prev = prev;
        prev->next = n;
        prev = n;
    }

    constexpr void unlink() 
    {
        auto next_tmp = next;
        auto prev_tmp = prev;
        next->prev = prev_tmp;
        prev->next = next_tmp;
        next = this;
        prev = this;
    }

    node_dummy* next = this; 
    node_dummy* prev = this;
};

template<class T>
struct node : node_dummy {

    using value_type = T;

    template<class... Args>
    constexpr node(Args&&... args) : val(std::forward<Args>(args)...) {}
    constexpr node() = default;

    T val;
};

template<class T>
struct node_iter {

    using node_type = T;
    using value_type = std::remove_pointer_t<node_type>::value_type;

    constexpr node_iter(node_type np) : np{np} {}
    constexpr node_iter(const node_iter<node_type>& other) = default;
    constexpr node_iter(const node_iter<std::remove_const_t<node_type>>& other) requires std::is_const_v<node_type> : np{other.np} {}

    constexpr bool operator==(const node_iter& rhs) const   { return np == rhs.np; }
    constexpr value_type& operator*() const                 { return np->val; }
    constexpr value_type* operator->() const                { return &np->val; }
    constexpr node_iter operator++(int)
    {
        auto tmp = *this; 
        ++(*this); 
        return tmp;
    }
    constexpr node_iter& operator++()
    {
        np = static_cast<node_type>(np->next);
        return *this;
    }
    constexpr node_iter operator--(int)
    {
        auto tmp = *this; 
        --(*this); 
        return tmp;
    }
    constexpr node_iter& operator--()
    {
        np = np->prev;
        return *this;
    }
    friend class node_iter<std::add_const_t<node_type>>;
public:
    node_type np;
};

}

template<class T, size_t N>
struct list {

    // ANCHOR Member types

    using size_type             = size_t;
    using value_type            = T;
    using pointer               = value_type*;
    using const_pointer         = const value_type*;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using universal_reference   = value_type&&; 
    using node_type             = imp::node<value_type>;
    using node_pointer          = node_type*;
    using iterator              = imp::node_iter<node_pointer>;
    using const_iterator        = imp::node_iter<const node_pointer>;

    // ANCHOR Constructors

    constexpr list() = default;

    // ANCHOR Destructor

    constexpr ~list() 
    { 
        clear(); 
    }

    // ANCHOR Capacity

    constexpr static size_type capacity()   { return N; }
    constexpr static size_type max_size()   { return N; }
    constexpr size_type size() const        { return buf.capacity() - buf.left(); }
    constexpr bool full() const             { return available == 0; }
    constexpr bool empty() const            { return available == N; } // { return root.next == &root; }

    // ANCHOR Iterators

    constexpr iterator begin()              { return static_cast<node_pointer>(root.next); }
    constexpr const_iterator begin() const  { return static_cast<node_pointer>(root.next); }
    constexpr const_iterator cbegin() const { return begin(); }
    constexpr iterator end()                { return static_cast<node_pointer>(&root); }
    constexpr const_iterator end() const    { return static_cast<node_pointer>(&root); }
    constexpr const_iterator cend() const   { return end(); }

    // ANCHOR Modifiers

    constexpr void push_back(const_reference x)
    {
        emplace_back(x);
    }
    
    constexpr void push_back(universal_reference x)
    {
        emplace_back(std::move(x));
    }

    template<class... Args>
    constexpr reference emplace_back(Args&&... args)
    {
        assert(full() == false);
        auto ret = ctor(&buf[bits.acquire_any()], std::forward<Args>(args)...);
        --available;
        root.push_back(ret);
        return ret->val;
    }

    constexpr void erase(const_iterator i) 
    {
        i.np->unlink();
        dtor(i.np);
        bits.clr(i.np - buf);
        available++;
    }

    constexpr void clear() 
    {
        while (!empty())
            erase(cbegin());
    }
private:
    storage<node_type, N> buf;
    size_type available = N;
    bitset<uint8_t, N, 7> bits;
    imp::node_dummy root;
};

}

#endif