#include "test.h"
#include "nth/coap/address.h"

namespace nth::coap {

TEST(CoapAddress, Equal)
{
    address addr_1 = {
        .data = {
            .u32 = 0x7f000001,
        },
        .port = 666,
        .type = addr_type::ipv4,
    };
    address addr_2 = {
        .data = {
            .ipv4 = {1, 0, 0, 127},
        },
        .port = 666,
        .type = addr_type::ipv4,
    };
    address addr_3 = {
        .data = {
            .ipv4 = {1, 0, 0, 127},
        },
        .port = 667,
        .type = addr_type::ipv4,
    };
    ASSERT_EQ(addr_1, addr_2);
    ASSERT_NE(addr_1, addr_3);
}

}