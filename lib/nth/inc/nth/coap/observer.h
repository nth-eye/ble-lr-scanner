#ifndef NTH_COAP_OBSERVER_H
#define NTH_COAP_OBSERVER_H

#include "nth/coap/address.h"

namespace nth::coap {

struct observer {
    observer* next = nullptr;
    address addr;
    uint8_t tok[8];
    uint8_t tkl;
    uint16_t id;
    bool reliable;
};

struct observer_iter {
    constexpr observer_iter(observer *p) : p{p} {}
    constexpr bool operator==(const observer_iter &rhs) const   { return p == rhs.p }
    constexpr const observer* operator->() const                { return p; }
    constexpr const observer& operator*() const                 { return *p; }
    constexpr observer* operator->()                            { return p; }
    constexpr observer& operator*()                             { return *p; }
    constexpr observer_iter& operator++()
    {
        p = p->next;
        return *this;
    }
private:
    observer* p;
};

struct observer_list {
    constexpr bool append(observer* obs)
    {
        if (obs == nullptr)
            return false;
        for (const auto& it : *this)
            if (&it == obs)
                return false; // Already in list

        if (head == nullptr)
            head = obs;
        if (tail == nullptr)
            tail = obs;
        obs->next = nullptr;
        return true;
    }
    constexpr bool remove(observer* obs)
    {
        observer* prev = nullptr;
        bool found = false;;

        for (auto &it : *this) {
            if (obs == &it) {
                found = true;
                break;
            }
            prev = &it;
        }
        if (!found)
            return false;

        prev->next = obs->next;
        obs->next = nullptr;

        return true;
    }
    constexpr const observer_iter begin() const { return head; }
    constexpr const observer_iter end()   const { return nullptr; }
private:
    observer* head;
    observer* tail;
};

}

#endif