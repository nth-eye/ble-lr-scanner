#include "test.h"
#include "nth/coap/packet.h"

namespace nth::coap {

TEST(CoapPacket, OptionIsSet)
{
    const byte request[] = {
        (1 << 6), +msg_code::empty, 0x00, 0x00,
        0x11, 0x01,
        0x21, 0x03,
    };
    packet pkt = {request};

    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), true);
    ASSERT_EQ(pkt.opt_is_set(option_num::uri_host), true);
}

TEST(CoapPacket, OptionGet)
{
    const byte request[] = {
        (1 << 6), +msg_code::empty, 0x00, 0x00,
        0x11, 0x11,
    };
    packet pkt = {request};

    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), true);

    option opt = pkt.opt_get(option_num::if_match);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::if_match);
    ASSERT_EQ(opt.len, 1);
    ASSERT_EQ(opt.dat[0], 0x11);
}

TEST(CoapPacket, OptionNext)
{
    const byte request[] = {
        (1 << 6), +msg_code::empty, 0x00, 0x00,
        0x11, 0x11,
        0x02, 0x22, 0x22,
        0x03, 0x33, 0x33, 0x33,
        0x20,
    };
    packet pkt = {request};
    option opt = pkt.opt_get(option_num::if_match);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::if_match);
    ASSERT_EQ(opt.len, 1);
    ASSERT_EQ(opt.dat[0], 0x11);

    opt = pkt.opt_next(opt);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::if_match);
    ASSERT_EQ(opt.len, 2);
    ASSERT_EQ(opt.dat[0], 0x22);
    ASSERT_EQ(opt.dat[1], 0x22);

    opt = pkt.opt_next(opt);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::if_match);
    ASSERT_EQ(opt.len, 3);
    ASSERT_EQ(opt.dat[0], 0x33);
    ASSERT_EQ(opt.dat[1], 0x33);
    ASSERT_EQ(opt.dat[2], 0x33);

    opt = pkt.opt_next(opt);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, +option_num::uri_host);
    ASSERT_EQ(opt.len, 0);

    opt = pkt.opt_next(opt);

    ASSERT_EQ(opt_valid(opt), false);
}

TEST(CoapPacket, OptionIterators)
{
    const byte request[] = {
        (1 << 6), +msg_code::empty, 0x00, 0x00,
        0x11, 0x11,
        0x22, 0x22, 0x22,
        0x03, 0x33, 0x33, 0x33,
    };
    packet pkt = {request};

    auto range = pkt.get_options();
    auto iter = range.begin();

    ASSERT_NE(iter, range.end());
    ASSERT_EQ(iter->num, +option_num::if_match);
    ASSERT_EQ(iter->len, 1);
    ASSERT_EQ(iter->dat[0], 0x11);

    ASSERT_NE(++iter, range.end());
    ASSERT_EQ(iter->num, +option_num::uri_host);
    ASSERT_EQ(iter->len, 2);
    ASSERT_EQ(iter->dat[0], 0x22);
    ASSERT_EQ(iter->dat[1], 0x22);

    ASSERT_NE(++iter, range.end());
    ASSERT_EQ(iter->num, +option_num::uri_host);
    ASSERT_EQ(iter->len, 3);
    ASSERT_EQ(iter->dat[0], 0x33);
    ASSERT_EQ(iter->dat[1], 0x33);
    ASSERT_EQ(iter->dat[2], 0x33);

    ASSERT_EQ(++iter, range.end());

    auto same_range = pkt.get_options(option_num::uri_host);
    auto same_iter = same_range.begin();

    ASSERT_NE(same_iter, same_range.end());
    ASSERT_EQ(same_iter->num, +option_num::uri_host);
    ASSERT_EQ(same_iter->len, 2);
    ASSERT_EQ(same_iter->dat[0], 0x22);
    ASSERT_EQ(same_iter->dat[1], 0x22);

    ASSERT_NE(++same_iter, same_range.end());
    ASSERT_EQ(same_iter->num, +option_num::uri_host);
    ASSERT_EQ(same_iter->len, 3);
    ASSERT_EQ(same_iter->dat[0], 0x33);
    ASSERT_EQ(same_iter->dat[1], 0x33);
    ASSERT_EQ(same_iter->dat[2], 0x33);

    ASSERT_EQ(++same_iter, same_range.end());
}

}