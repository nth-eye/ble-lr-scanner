#include "test.h"
#include "nth/coap/packet.h"

namespace nth::coap {

TEST(CoapPacket, DeleteOptionBetween)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x11, 0x01,
        0x21, 0x03,
        0x11, 0x04,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    ASSERT_EQ(pkt.opt_delete(option_num::uri_host), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), false);
    ASSERT_EQ(pkt.opt_is_set(option_num::etag), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option opt = pkt.opt_get(option_num::if_match);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::if_match);
    ASSERT_EQ(opt.len, 1);
    ASSERT_EQ(opt.dat[0], 0x01);

    opt = pkt.opt_get(option_num::uri_host);

    ASSERT_EQ(opt_valid(opt), false);

    opt = pkt.opt_get(option_num::etag);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::etag);
    ASSERT_EQ(opt.len, 1);
    ASSERT_EQ(opt.dat[0], 0x04);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x11, 0x01, 0x31, 0x04,  
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, DeleteOptionFirst)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x31, 0x03, 
        0x11, 0x04,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    ASSERT_EQ(pkt.opt_delete(option_num::uri_host), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), false);
    ASSERT_EQ(pkt.opt_is_set(option_num::etag), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option opt = pkt.opt_get(option_num::uri_host);

    ASSERT_EQ(opt_valid(opt), false);

    opt = pkt.opt_get(option_num::etag);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::etag);
    ASSERT_EQ(opt.len, 1);
    ASSERT_EQ(opt.dat[0], 0x04);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x41, 0x04,   
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, DeleteOptionLast)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x31, 0x03, 
        0x11, 0x04,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    ASSERT_EQ(pkt.opt_delete(option_num::etag), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::etag), false);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option opt = pkt.opt_get(option_num::uri_host);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::uri_host);
    ASSERT_EQ(opt.len, 1);
    ASSERT_EQ(opt.dat[0], 0x03);

    opt = pkt.opt_get(option_num::etag);

    ASSERT_EQ(opt_valid(opt), false);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x31, 0x03,   
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, DeleteOptionEmpty)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    ASSERT_EQ(pkt.opt_delete(option_num::if_match), false);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), false);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option opt = pkt.opt_get(option_num::if_match);

    ASSERT_EQ(opt_valid(opt), false);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00,    
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, DeleteTheOnlyOption)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x11, 0x01,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    ASSERT_EQ(pkt.opt_delete(option_num::if_match), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), false);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option opt = pkt.opt_get(option_num::if_match);

    ASSERT_EQ(opt_valid(opt), false);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00,    
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, DeleteOptionWithDeltaResize)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x44, 0x04, 0x04, 0x04, 0x04, 
        0xa1, 0x0e,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    ASSERT_EQ(pkt.opt_delete(option_num::etag), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::etag), false);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option opt = pkt.opt_get(option_num::etag);

    ASSERT_EQ(opt_valid(opt), false);

    opt = pkt.opt_get(option_num::max_age);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::max_age);
    ASSERT_EQ(opt.len, 1);
    ASSERT_EQ(opt.dat[0], 0x0e);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00,    
        0xd1, 0x01, 0x0e,
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, DeleteOptionWithDeltaResizeDouble)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x33, 0x03, 0x03, 0x03, 
        0xd0, 0xfd,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    ASSERT_EQ(pkt.opt_delete(option_num::uri_host), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), false);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option opt = pkt.opt_get(option_num::uri_host);

    ASSERT_EQ(opt_valid(opt), false);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00,    
        0xe0, 0x00, 0x00,
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, DeleteOptionRepeatable)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x10, 
        0x01, 0x01, 
        0x02, 0x01, 0x02,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    ASSERT_EQ(pkt.opt_delete(option_num::if_match), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), false);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option opt = pkt.opt_get(option_num::if_match);

    ASSERT_EQ(opt_valid(opt), false);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00,    
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

}