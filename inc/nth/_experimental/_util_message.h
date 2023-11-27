#ifndef NTH_MESSAGE_H
#define NTH_MESSAGE_H

#include "nth/base.h"

namespace nth {

struct message_base {
    message_base(int id) : id{id} {}
    const int id;
};

template<int Id>
struct message : message_base {
    message() : message_base(Id) {}
    static constexpr auto id = Id; 
};

struct message_router {
    virtual void receive(const message_base& msg) = 0;
};

}

#endif