#ifndef NTH_COAP_RESOURCE_H
#define NTH_COAP_RESOURCE_H

#include "nth/coap/observer.h"
#include "nth/coap/packet.h"
#include "nth/coap/hal.h"

namespace nth::coap {

struct resource;
struct server;

using method_handler = void(*)(server&);
using resource_list = std::span<resource>;

struct resource {
    const std::string_view path;
    const method_handler get    = nullptr;
    const method_handler post   = nullptr;
    const method_handler put    = nullptr;
    const method_handler del    = nullptr;
    const resource_list sub     = {};
    observer_list obs           = {};

    void notify(ispan payload)
    {
        // Packet pkt;

        // for (auto &o : obs) {

        //     logi() << "notifying " << o.addr;

        //     pkt.init(imp::version, o.reliable ? type_ack : type_non, code_content, o.id += 1, o.tok, o.tkl);
        //     pkt.set_payload(data, len);

        //     log() << K_GRN << 
        //         "+---------------------------->\n" << pkt <<
        //         "+---------------------------->\n" << K_NRM;

        //     sock.sendto(pkt.data(), pkt.size(), o.addr);
        // }
    }
};

}

#endif
