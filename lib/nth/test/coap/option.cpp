#include "test.h"
#include "nth/coap/option.h"

namespace nth::coap {

TEST(CoapOption, HeadSize)
{
    ASSERT_EQ(opt_head_size(0, 0), 1);
    ASSERT_EQ(opt_head_size(12, 12), 1);
    ASSERT_EQ(opt_head_size(13, 12), 2);
    ASSERT_EQ(opt_head_size(13, 13), 3);
    ASSERT_EQ(opt_head_size(269, 13), 4);
    ASSERT_EQ(opt_head_size(269, 269), 5);
}

TEST(CoapOption, TotalSize)
{
    ASSERT_EQ(opt_total_size(0, 0), 1);
    ASSERT_EQ(opt_total_size(12, 12), 13);
    ASSERT_EQ(opt_total_size(13, 12), 14);
    ASSERT_EQ(opt_total_size(13, 13), 16);
    ASSERT_EQ(opt_total_size(269, 13), 17);
    ASSERT_EQ(opt_total_size(269, 269), 274);
}

TEST(CoapOption, EncodeHead)
{
    byte buf[5] = {};

    ASSERT_EQ(opt_encode_head(buf, 1, 1), buf + 1);
    ASSERT_EQ(buf[0], 0x11);

    ASSERT_EQ(opt_encode_head(buf, 13, 1), buf + 2);
    ASSERT_EQ(buf[0], 0xd1);
    ASSERT_EQ(buf[1], 0x00);

    ASSERT_EQ(opt_encode_head(buf, 13, 13), buf + 3);
    ASSERT_EQ(buf[0], 0xdd);
    ASSERT_EQ(buf[1], 0x00);
    ASSERT_EQ(buf[2], 0x00);

    ASSERT_EQ(opt_encode_head(buf, 269, 13), buf + 4);
    ASSERT_EQ(buf[0], 0xed);
    ASSERT_EQ(buf[1], 0x00);
    ASSERT_EQ(buf[2], 0x00);
    ASSERT_EQ(buf[3], 0x00);

    ASSERT_EQ(opt_encode_head(buf, 269, 269), buf + 5);
    ASSERT_EQ(buf[0], 0xee);
    ASSERT_EQ(buf[1], 0x00);
    ASSERT_EQ(buf[2], 0x00);
    ASSERT_EQ(buf[3], 0x00);
    ASSERT_EQ(buf[4], 0x00);
}

TEST(CoapOption, Encode)
{
    byte data[] = {0x13, 0x37};
    byte buf[5] = {};

    ASSERT_EQ(opt_encode(buf, 1, 1, data), buf + 2);
    ASSERT_EQ(buf[0], 0x11);
    ASSERT_EQ(buf[1], 0x13);

    ASSERT_EQ(opt_encode(buf, 13, 1, data), buf + 3);
    ASSERT_EQ(buf[0], 0xd1);
    ASSERT_EQ(buf[1], 0x00);
    ASSERT_EQ(buf[2], 0x13);

    ASSERT_EQ(opt_encode(buf, 13, 2, data), buf + 4);
    ASSERT_EQ(buf[0], 0xd2);
    ASSERT_EQ(buf[1], 0x00);
    ASSERT_EQ(buf[2], 0x13);
    ASSERT_EQ(buf[3], 0x37);

    ASSERT_EQ(opt_encode(buf, 269, 2, data), buf + 5);
    ASSERT_EQ(buf[0], 0xe2);
    ASSERT_EQ(buf[1], 0x00);
    ASSERT_EQ(buf[2], 0x00);
    ASSERT_EQ(buf[3], 0x13);
    ASSERT_EQ(buf[4], 0x37);
}

TEST(CoapOption, Decode)
{
    const byte raw[] = {0xde, 0xad, 0xbe, 0xef};
    
    auto opt = opt_decode(raw, raw + sizeof(raw), 42);

    ASSERT_EQ(opt_valid(opt), true);
    ASSERT_EQ(opt.num, 0xad + 42 + 13);
    ASSERT_EQ(opt.len, ((0xbe << 8) | 0xef) + 269);
    ASSERT_EQ(opt.dat, raw + 4);
}

}
