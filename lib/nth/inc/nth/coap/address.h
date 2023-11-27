#ifndef NTH_COAP_ADDRESS_H
#define NTH_COAP_ADDRESS_H

#include "nth/coap/base.h"
#include <array>

namespace nth::coap {

/**
 * @brief Possible address types for network/transport layers.
 * 
 */
enum class addr_type : byte {
    unspec,
    ipv4,
    ipv6,
};

/**
 * @brief Necessary data to store for any address type. 
 * 
 */
union addr_data {
    std::array<byte, 4>  ipv4;
    std::array<byte, 16> ipv6;
    uint32_t u32;
};

/**
 * @brief General port type for network/transport layers.
 * 
 */
using addr_port = uint16_t;

/**
 * @brief CoAP address with all necessary data stored.
 * 
 */
struct address {

    constexpr bool operator==(const address& rhs) const
    {
        if (type != rhs.type)
            return false;
        switch (type) 
        {
        case addr_type::ipv4:
            return port == rhs.port && data.ipv4 == rhs.data.ipv4;
        case addr_type::ipv6:
            return port == rhs.port && data.ipv6 == rhs.data.ipv6;
        default:
            return false;
        }
    }

    addr_data data;
    addr_port port;
    addr_type type;
};

}

#endif