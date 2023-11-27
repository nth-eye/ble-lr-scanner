#include "test.h"
#include "nth/coap/packet.h"

namespace nth::coap {

TEST(CoapPacket, Capacity)
{
    byte storage[16];
    packet_view pkt = {storage};
    ASSERT_EQ(pkt.capacity(), 16);

    packet<12> pkt_n = {};
    ASSERT_EQ(pkt_n.capacity(), 12);
}

TEST(CoapPacket, Size)
{
    packet<16> pkt;

    ASSERT_EQ(pkt.size(), 0);

    const byte data_short[3] = {imp::version << 6};

    ASSERT_EQ(pkt.parse(data_short), error::too_short_header);
    ASSERT_EQ(pkt.size(), 3);
}

TEST(CoapPacket, Version)
{
    packet pkt;
    pkt.set_ver(1);
    ASSERT_EQ(pkt.get_ver(), 1 & 0x3);
    pkt.set_ver(3);
    ASSERT_EQ(pkt.get_ver(), 3 & 0x3);
    pkt.set_ver(4);
    ASSERT_EQ(pkt.get_ver(), 4 & 0x3);
}

TEST(CoapPacket, Type)
{
    packet pkt;
    pkt.set_type(msg_type::con);
    ASSERT_EQ(pkt.get_type(), +msg_type::con);
    pkt.set_type(msg_type::non);
    ASSERT_EQ(pkt.get_type(), +msg_type::non);
    pkt.set_type(msg_type::ack);
    ASSERT_EQ(pkt.get_type(), +msg_type::ack);
    pkt.set_type(msg_type::rst);
    ASSERT_EQ(pkt.get_type(), +msg_type::rst);
}

TEST(CoapPacket, Code)
{
    packet pkt;
    pkt.set_code(msg_code::bad_gateway);
    ASSERT_EQ(pkt.get_code(), +msg_code::bad_gateway);
    ASSERT_EQ(pkt.get_code_class(), +msg_class::server_err);
    ASSERT_EQ(pkt.get_code_detail(), 2);
}

TEST(CoapPacket, Id)
{
    packet pkt;

    pkt.set_id(0);
    ASSERT_EQ(pkt.get_id(), 0);
    pkt.set_id(0x6969);
    ASSERT_EQ(pkt.get_id(), 0x6969);
}

TEST(CoapPacket, PayloadNull)
{
    packet pkt;

    pkt.set_payload({});

    ASSERT_EQ(pkt.size(), 4);
    ASSERT_EQ(pkt.get_payload().size(), 0);
}

TEST(CoapPacket, PayloadNormal)
{
    static constexpr size_t bytes = 16; 
    packet<bytes> pkt;
    const byte data[bytes - 4 - 1] = {};

    pkt.clear();
    pkt.set_payload(data);

    ASSERT_EQ(pkt.size(), bytes);
    ASSERT_EQ(pkt.get_payload().size(), sizeof(data));

    const byte exp[bytes] = {
        0x00, 0x00, 0x00, 0x00,
        0xff,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, PayloadReplace)
{
    packet pkt;
    const byte init_data[] = {
        0x40, 0x00, 0x00, 0x00, 
        0xff, 0xde, 0xad,
    };

    pkt.parse(init_data);

    ASSERT_EQ(pkt.size(), sizeof(init_data));
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);

    const byte data_replace[] = {0xba, 0xbe};

    pkt.set_payload(data_replace);

    ASSERT_EQ(pkt.size(), sizeof(data_replace) + 5);
    ASSERT_EQ(pkt.get_payload().size(), 2);
    ASSERT_EQ(pkt.get_payload()[0], 0xba);
    ASSERT_EQ(pkt.get_payload()[1], 0xbe);

    const byte exp[] = {
        0x40, 0x00, 0x00, 0x00,
        0xff, 0xba, 0xbe,
    };
    assert_arreq(pkt, exp);
}

TEST(CoapPacket, TokenLength)
{
    packet pkt;

    const byte tok_ok[] = {0x44, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44};
    
    ASSERT_EQ(pkt.parse(tok_ok), error::ok);
    ASSERT_EQ(pkt.get_tkl(), 4);

    const byte tok_nok[] = {0x49, 0x00, 0x00, 0x00};

    ASSERT_EQ(pkt.parse(tok_nok), error::invalid_token_length);

    const byte tok_short[] = {0x44, 0x00, 0x00, 0x00};

    ASSERT_EQ(pkt.parse(tok_short), error::too_short_token);
}

TEST(CoapPacket, Setup)
{
    packet pkt;

    pkt.clear();

    ASSERT_EQ(pkt.size(), 0);
    ASSERT_EQ(pkt.get_ver(), 0);
    ASSERT_EQ(pkt.get_type(), 0);
    ASSERT_EQ(pkt.get_code(), 0);
    ASSERT_EQ(pkt.get_id(), 0);
    ASSERT_EQ(pkt.get_tkl(), 0);
    ASSERT_EQ(pkt.get_token().size(), 0);
    ASSERT_EQ(pkt.get_payload().size(), 0);   

    ASSERT_EQ(pkt.setup(msg_type::con, msg_code::get, 42, {}), true);
    ASSERT_EQ(pkt.size(), 4);
    ASSERT_EQ(pkt.get_ver(), imp::version);
    ASSERT_EQ(pkt.get_type(), +msg_type::con);
    ASSERT_EQ(pkt.get_code(), +msg_code::get);
    ASSERT_EQ(pkt.get_id(), 42);
    ASSERT_EQ(pkt.get_tkl(), 0);
    ASSERT_EQ(pkt.get_token().size(), 0);
    ASSERT_EQ(pkt.get_payload().size(), 0);

    const byte token[] = {0x69};

    ASSERT_EQ(pkt.setup(msg_type::ack, msg_code::put, 44, token), true);
    ASSERT_EQ(pkt.size(), 5);
    ASSERT_EQ(pkt.get_ver(), imp::version);
    ASSERT_EQ(pkt.get_type(), +msg_type::ack);
    ASSERT_EQ(pkt.get_code(), +msg_code::put);
    ASSERT_EQ(pkt.get_id(), 44);
    ASSERT_EQ(pkt.get_tkl(), 1);
    ASSERT_EQ(pkt.get_token().size(), 1);
    ASSERT_EQ(pkt.get_token()[0], token[0]);
    ASSERT_EQ(pkt.get_payload().size(), 0);

    const byte token_huge[9] = {};

    ASSERT_EQ(pkt.setup(msg_type::ack, msg_code::put, 44, token_huge), false);
    ASSERT_EQ(pkt.size(), 5);
    ASSERT_EQ(pkt.get_ver(), imp::version);
    ASSERT_EQ(pkt.get_type(), +msg_type::ack);
    ASSERT_EQ(pkt.get_code(), +msg_code::put);
    ASSERT_EQ(pkt.get_id(), 44);
    ASSERT_EQ(pkt.get_tkl(), 1);
    ASSERT_EQ(pkt.get_token().size(), 1);
    ASSERT_EQ(pkt.get_token()[0], token[0]);
    ASSERT_EQ(pkt.get_payload().size(), 0);
}

TEST(CoapPacket, Parse)
{
    packet pkt;

    const byte data[] = {
        0x44, 0x00, 0x13, 0x37,
        0x01, 0x02, 0x03, 0x04,
        0x13, 0x11, 0x11, 0x11,
        0xff, 0xde, 0xad, 0x20,
    };

    ASSERT_EQ(pkt.parse(data), error::ok);
    ASSERT_EQ(pkt.size(), sizeof(data));
    ASSERT_EQ(pkt.get_ver(), 1);
    ASSERT_EQ(pkt.get_type(), +msg_type::con);
    ASSERT_EQ(pkt.get_code(), +msg_code::empty);
    ASSERT_EQ(pkt.get_id(), 0x1337);
    ASSERT_EQ(pkt.get_tkl(), 4);
    ASSERT_EQ(pkt.get_token().size(), 4);
    ASSERT_EQ(pkt.get_token()[0], 0x01);
    ASSERT_EQ(pkt.get_token()[1], 0x02);
    ASSERT_EQ(pkt.get_token()[2], 0x03);
    ASSERT_EQ(pkt.get_token()[3], 0x04);
    ASSERT_EQ(pkt.opt_is_set(option_num::if_match), true);
    ASSERT_EQ(opt_valid(pkt.opt_get(option_num::if_match)), true);
    ASSERT_EQ(pkt.opt_get(option_num::if_match).num, 1);
    ASSERT_EQ(pkt.opt_get(option_num::if_match).len, 3);
    ASSERT_EQ(pkt.opt_get(option_num::if_match).dat[0], 0x11);
    ASSERT_EQ(pkt.opt_get(option_num::if_match).dat[1], 0x11);
    ASSERT_EQ(pkt.opt_get(option_num::if_match).dat[2], 0x11);
    ASSERT_EQ(pkt.get_options().begin()->num, 1);
    ASSERT_EQ(pkt.get_options().begin()->len, 3);
    ASSERT_EQ(pkt.get_options().begin()->dat[0], 0x11);
    ASSERT_EQ(pkt.get_options().begin()->dat[1], 0x11);
    ASSERT_EQ(pkt.get_options().begin()->dat[2], 0x11);
    ASSERT_EQ(pkt.get_payload().size(), 3);
    ASSERT_EQ(pkt.get_payload()[0], 0xde);
    ASSERT_EQ(pkt.get_payload()[1], 0xad);
    ASSERT_EQ(pkt.get_payload()[2], 0x20);
}

TEST(CoapPacket, ClearBody)
{
    const byte with_opt_and_pld[] = {
        0x44, 0x00, 0x13, 0x37,
        0x01, 0x02, 0x03, 0x04,
        0x13, 0x11, 0x11, 0x11,
        0xff, 0xde, 0xad, 0x20,
    };
    const byte already_empty[] = {
        0x44, 0x00, 0x13, 0x37,
        0x01, 0x02, 0x03, 0x04,
    };
    packet pkt;

    ASSERT_EQ(pkt.parse(with_opt_and_pld), error::ok);
    pkt.clear_body();
    assert_arreq(pkt, already_empty);

    ASSERT_EQ(pkt.parse(already_empty), error::ok);
    pkt.clear_body();
    assert_arreq(pkt, already_empty);
}

}