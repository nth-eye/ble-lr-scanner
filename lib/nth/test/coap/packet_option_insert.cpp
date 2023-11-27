#include "test.h"
#include "nth/coap/packet.h"

namespace nth::coap {

TEST(CoapPacket, AddOptionBetween)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x11, 0x01,
        0x31, 0x04,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    const byte opt_dat[] = {0x03}; 
    option opt = {opt_dat, sizeof(opt_dat), +option_num::uri_host};

    ASSERT_EQ(pkt.opt_insert(opt), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::etag), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option tmp = pkt.opt_get(option_num::if_match);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, +option_num::if_match);
    ASSERT_EQ(tmp.len, 1);
    ASSERT_EQ(tmp.dat[0], 0x01);

    tmp = pkt.opt_get(option_num::uri_host);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, opt.num);
    ASSERT_EQ(tmp.len, opt.len);
    for (int i = 0; i < tmp.len; ++i)
        ASSERT_EQ(tmp.dat[i], opt.dat[i]) << "at idx " << i;

    tmp = pkt.opt_get(option_num::etag);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, +option_num::etag);
    ASSERT_EQ(tmp.len, 1);
    ASSERT_EQ(tmp.dat[0], 0x04);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x11, 0x01, 0x21, 0x03, 0x11, 0x04, 
        0xff, 0xde, 0xad 
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, AddOptionFirst)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x41, 0x04,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    const byte opt_dat[] = {0x03}; 
    option opt = {opt_dat, sizeof(opt_dat), +option_num::uri_host};

    ASSERT_EQ(pkt.opt_insert(opt), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::etag), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option tmp = pkt.opt_get(option_num::uri_host);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, opt.num);
    ASSERT_EQ(tmp.len, opt.len);
    for (int i = 0; i < tmp.len; ++i)
        ASSERT_EQ(tmp.dat[i], opt.dat[i]) << "at idx " << i;

    tmp = pkt.opt_get(option_num::etag);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, +option_num::etag);
    ASSERT_EQ(tmp.len, 1);
    ASSERT_EQ(tmp.dat[0], 0x04);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x31, 0x03, 0x11, 0x04,
        0xff, 0xde, 0xad 
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, AddOptionLast)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x31, 0x03,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    const byte opt_dat[] = {0x04}; 
    option opt = {opt_dat, sizeof(opt_dat), +option_num::etag};

    ASSERT_EQ(pkt.opt_insert(opt), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::etag), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option tmp = pkt.opt_get(option_num::uri_host);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, +option_num::uri_host);
    ASSERT_EQ(tmp.len, 1);
    ASSERT_EQ(tmp.dat[0], 0x03);

    tmp = pkt.opt_get(option_num::etag);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, opt.num);
    ASSERT_EQ(tmp.len, opt.len);
    for (int i = 0; i < tmp.len; ++i)
        ASSERT_EQ(tmp.dat[i], opt.dat[i]) << "at idx " << i;

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x31, 0x03, 0x11, 0x04,
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, AddOptionEmpty)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    const byte opt_dat[] = {0x01}; 
    option opt = {opt_dat, sizeof(opt_dat), +option_num::if_match};

    ASSERT_EQ(pkt.opt_insert(opt), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option tmp = pkt.opt_get(option_num::if_match);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, opt.num);
    ASSERT_EQ(tmp.len, opt.len);
    for (int i = 0; i < tmp.len; ++i)
        ASSERT_EQ(tmp.dat[i], opt.dat[i]) << "at idx " << i;

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x11, 0x01,
        0xff, 0xde, 0xad
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, AddOptionWithDeltaResize)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0xd1, 0x01, 0x0e,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    const byte opt_dat[] = {0x04, 0x04, 0x04, 0x04}; 
    option opt = {opt_dat, sizeof(opt_dat), +option_num::etag};

    ASSERT_EQ(pkt.opt_insert(opt), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::etag), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::max_age), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option tmp = pkt.opt_get(option_num::etag);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, opt.num);
    ASSERT_EQ(tmp.len, opt.len);
    for (int i = 0; i < tmp.len; ++i)
        ASSERT_EQ(tmp.dat[i], opt.dat[i]) << "at idx " << i;

    tmp = pkt.opt_get(option_num::max_age);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, +option_num::max_age);
    ASSERT_EQ(tmp.len, 1);
    ASSERT_EQ(tmp.dat[0], 0x0e);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x44, 0x04, 0x04, 0x04, 0x04, 0xa1, 0x0e,
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, AddOptionWithDeltaResizeDouble)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0xe0, 0x00, 0x00,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    const byte opt_dat[] = {0x03, 0x03, 0x03}; 
    option opt = {opt_dat, sizeof(opt_dat), +option_num::uri_host};

    ASSERT_EQ(pkt.opt_insert(opt), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::unknown), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option tmp = pkt.opt_get(option_num::uri_host);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, opt.num);
    ASSERT_EQ(tmp.len, opt.len);
    for (int i = 0; i < tmp.len; ++i)
        ASSERT_EQ(tmp.dat[i], opt.dat[i]) << "at idx " << i;

    // We can't get UKNOWN option, so compare only with hardcoded values if everything is okay.
    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x33, 0x03, 0x03, 0x03, 0xd0, 0xfd,
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, AddOptionAtCapacityLimit)
{
    static constexpr size_t bytes = imp::default_packet_size;

    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x10,
        0xff, 0xde, 0xad,
    };
    packet<bytes> pkt = {request};

    byte opt_dat[bytes - 10] = {}; 
    memset(opt_dat, +option_num::uri_host, sizeof(opt_dat));
    option opt = {opt_dat, sizeof(opt_dat), +option_num::uri_host};

    ASSERT_EQ(pkt.opt_insert(opt), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    option tmp = pkt.opt_get(option_num::uri_host);

    ASSERT_EQ(opt_valid(tmp), true);
    ASSERT_EQ(tmp.num, opt.num);
    ASSERT_EQ(tmp.len, opt.len);
    for (int i = 0; i < tmp.len; ++i)
        ASSERT_EQ(tmp.dat[i], opt.dat[i]) << "at idx " << i;

    byte exp[bytes] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x10, 0x2d, bytes - 10 - 13,
    };
    memset(exp + 7, +option_num::uri_host, sizeof(exp) - 10);
    exp[bytes - 1] = 0xad;
    exp[bytes - 2] = 0xde,
    exp[bytes - 3] = 0xff;
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, AddOptionExceedingCapacity)
{
    static constexpr size_t bytes = imp::default_packet_size;
    
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x10,
        0xff, 0xde, 0xad,
    };
    packet<bytes> pkt = {request};

    const byte opt_dat[bytes] = {}; 
    option opt = {opt_dat, sizeof(opt_dat), +option_num::uri_host};

    ASSERT_EQ(pkt.opt_insert(opt), false);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), false);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);
    ASSERT_EQ(pkt.size(), sizeof(request));

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x10,
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, AddOptionRepeatable)
{
    const byte request[] = {
        0x40, 0x00, 0x00, 0x00,
        0x10,
        0xff, 0xde, 0xad,
    };
    packet pkt = {request};

    const byte opt_dat[] = {0x01, 0x02}; 
    option opt = {opt_dat, 1, +option_num::if_match};

    ASSERT_EQ(pkt.opt_insert(opt), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    opt.len = 2;

    ASSERT_EQ(pkt.opt_insert(opt), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), true);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    opt = pkt.opt_get(option_num::if_match);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::if_match);
    ASSERT_EQ(opt.len, 0);

    opt = pkt.opt_next(opt);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::if_match);
    ASSERT_EQ(opt.len, 1);
    ASSERT_EQ(opt.dat[0], 0x01);

    opt = pkt.opt_next(opt);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::if_match);
    ASSERT_EQ(opt.len, 2);
    ASSERT_EQ(opt.dat[0], 0x01);
    ASSERT_EQ(opt.dat[1], 0x02);

    opt = pkt.opt_next(opt);

    ASSERT_EQ(opt_valid(opt), false);

    const byte exp[] = { 
        0x40, 0x00, 0x00, 0x00, 
        0x10, 0x01, 0x01, 0x02, 0x01, 0x02,
        0xff, 0xde, 0xad,
    };
    assert_arreq(pkt, exp);
}

}