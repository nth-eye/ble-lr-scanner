#include "test.h"
#include "nth/util/string.h"

namespace nth {
namespace {

using namespace std::literals;

TEST(UtilString, IsDigit)
{
    for (char c = -128; c < '0'; ++c)
        ASSERT_EQ(is_digit(c), false);
    for (char c = '0'; c <= '9'; ++c)
        ASSERT_EQ(is_digit(c), true);
     for (char c = '9' + 1; c != -128; ++c)
        ASSERT_EQ(is_digit(c), false);   
}

TEST(UtilString, IsXdigit)
{
    for (char c = '0'; c <= '9'; ++c)
        ASSERT_EQ(is_xdigit(c), true);
    for (char c = 'a'; c <= 'f'; ++c)
        ASSERT_EQ(is_xdigit(c), true);
    for (char c = 'A'; c <= 'F'; ++c)
        ASSERT_EQ(is_xdigit(c), true);
    for (char c = -128; c != '0'; ++c)
        ASSERT_EQ(is_xdigit(c), false);
    for (char c = '9' + 1; c != 'A'; ++c)
        ASSERT_EQ(is_xdigit(c), false);
    for (char c = 'F' + 1; c != 'a'; ++c)
        ASSERT_EQ(is_xdigit(c), false);
    for (char c = 'f' + 1; c != -128; ++c)
        ASSERT_EQ(is_xdigit(c), false); 
}

TEST(UtilString, CharToBin)
{
    for (char c = '0'; c <= '9'; ++c)
        ASSERT_EQ(char_to_bin(c), c - '0');
    for (char c = 'a'; c <= 'f'; ++c)
        ASSERT_EQ(char_to_bin(c), c - 'a' + 10);
    for (char c = 'A'; c <= 'F'; ++c)
        ASSERT_EQ(char_to_bin(c), c - 'A' + 10);
}

TEST(UtilString, BinToChar)
{
    for (int i = 0; i < 10; ++i)
        ASSERT_EQ(bin_to_char(i), '0' + i);
    for (int i = 10; i < 16; ++i)
        ASSERT_EQ(bin_to_char(i), 'a' + i - 10);
    for (int i = 16; i < 26; ++i)
        ASSERT_EQ(bin_to_char(i), '0' + i - 16);
}

TEST(UtilString, StringLength)
{
    static_assert(str_len("") == 0);
    static_assert(str_len("a") == 1);
    static_assert(str_len("hello") == 5);
    static_assert(str_len("world") == 5);
    static_assert(str_len("hello world") == 11);
    static_assert(str_len("zero\0zero") == 4);
}

TEST(UtilString, StringNullify)
{
    char str[32];    
    std::fill(str, str + sizeof(str), 0xff);
    str_nullify(str, to_chars({{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}}, str));
    ASSERT_STREQ(str, "0123456789abcdef");
}

TEST(UtilString, StringSplit)
{
    auto str_1 = "hello,world";
    auto spl_1 = str_split<2>(str_1, ",");

    ASSERT_EQ(spl_1.size(), 2);
    ASSERT_EQ(spl_1[0], "hello");
    ASSERT_EQ(spl_1[1], "world");

    auto str_2 = "hello";
    auto spl_2 = str_split<2>(str_2, ",");

    ASSERT_EQ(spl_2.size(), 1);
    ASSERT_EQ(spl_2[0], "hello");

    auto str_3 = "hello,";
    auto spl_3 = str_split<2>(str_3, ",");

    ASSERT_EQ(spl_3.size(), 2);
    ASSERT_EQ(spl_3[0], "hello");
    ASSERT_EQ(spl_3[1], "");

    auto str_4 = ",hello";
    auto spl_4 = str_split<2>(str_4, ",");

    ASSERT_EQ(spl_4.size(), 2);
    ASSERT_EQ(spl_4[0], "");
    ASSERT_EQ(spl_4[1], "hello");

    auto str_5 = "hello,world,overflow";
    auto spl_5 = str_split<2>(str_5, ",");

    ASSERT_EQ(spl_5.size(), 2);
    ASSERT_EQ(spl_5[0], "hello");
    ASSERT_EQ(spl_5[1], "world");

    auto str_6 = "hello,.+world/";
    auto spl_6 = str_split<4>(str_6, ",.+");

    ASSERT_EQ(spl_6.size(), 4);
    ASSERT_EQ(spl_6[0], "hello");
    ASSERT_EQ(spl_6[1], "");
    ASSERT_EQ(spl_6[2], "");
    ASSERT_EQ(spl_6[3], "world/");
}

TEST(UtilString, ToCharsBinary)
{
    char str[16] = {};

    const char str_2[] = "0123456789abcdef";
    const char str_3[] = "0123456789abcdef";
    const char str_4[] = "0123456789abcd";

    const byte bin_1[] = {0x11};
    const byte bin_2[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
    const byte bin_3[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    const byte bin_4[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};

    ASSERT_EQ(to_chars({}, str), 0);
    ASSERT_EQ(to_chars(bin_1, {}), 0);

    ASSERT_EQ(to_chars(bin_2, str), strlen(str_2));
    assert_arreq(str, str_2, strlen(str_2));

    ASSERT_EQ(to_chars(bin_3, str), strlen(str_3));
    assert_arreq(str, str_3, strlen(str_3));

    ASSERT_EQ(to_chars(bin_4, {str, sizeof(str) - 1}), strlen(str_4));
    assert_arreq(str, str_4, strlen(str_4));
}

TEST(UtilString, ToCharsInteger)
{
    char str[8] = {};

    const char str_0[] = "";
    const char str_1[] = "";
    const char str_2[] = "0";
    const char str_3[] = "999";
    const char str_4[] = "-1337";
    const char str_5[] = "deadbeef";
    const char str_6[] = "DEADC0DE";
    const char str_7[] = "11010001";
    const char str_8[] = "4567";
    const char str_9[] = "xyz";

    ASSERT_EQ(to_chars(str, 214748364), 0);
    assert_arreq(str, str_0, strlen(str_0));

    ASSERT_EQ(to_chars({}, 13), 0);
    assert_arreq(str, str_1, strlen(str_1));

    ASSERT_EQ(to_chars(str, 0), 1);
    assert_arreq(str, str_2, strlen(str_2));

    ASSERT_EQ(to_chars(str, 999), 3);
    assert_arreq(str, str_3, strlen(str_3));

    ASSERT_EQ(to_chars(str, -1337), 5);
    assert_arreq(str, str_4, strlen(str_4));

    ASSERT_EQ(to_chars(str, 0xdeadbeef, 16), 8);
    assert_arreq(str, str_5, strlen(str_5));

    ASSERT_EQ(to_chars<true>(str, 0xdeadc0de, 16), 8);
    assert_arreq(str, str_6, strlen(str_6));

    ASSERT_EQ(to_chars(str, 0b11010001, 2), 8);
    assert_arreq(str, str_7, strlen(str_7));

    ASSERT_EQ(to_chars(str, 04567, 8), 4);
    assert_arreq(str, str_8, strlen(str_8));

    ASSERT_EQ(to_chars(str, 44027, 36), 3);
    assert_arreq(str, str_9, strlen(str_9));
}

TEST(UtilString, ToCharsFloating)
{
    // TODO
}

TEST(UtilString, ToBase64)
{
    char str[8] = {};

    const char str_1[] = "";
    const char str_2[] = "Zg==";
    const char str_3[] = "Zm8=";
    const char str_4[] = "Zm9v";
    const char str_5[] = "Zm9vYg==";
    const char str_6[] = "Zm9vYmE=";
    const char str_7[] = "Zm9vYmFy";

    const byte b64_1[] = "";
    const byte b64_2[] = "f";
    const byte b64_3[] = "fo";
    const byte b64_4[] = "foo";
    const byte b64_5[] = "foob";
    const byte b64_6[] = "fooba";
    const byte b64_7[] = "foobar";

    ASSERT_EQ(to_base64({}, str), 0);
    ASSERT_EQ(to_base64(b64_1, {}), 0);
    assert_arreq(str, str_1, strlen(str_1));

    ASSERT_EQ(to_base64({b64_2, sizeof(b64_2) - 1}, str), strlen(str_2));
    assert_arreq(str, str_2, strlen(str_2));

    ASSERT_EQ(to_base64({b64_3, sizeof(b64_3) - 1}, str), strlen(str_3));
    assert_arreq(str, str_3, strlen(str_3));

    ASSERT_EQ(to_base64({b64_4, sizeof(b64_4) - 1}, str), strlen(str_4));
    assert_arreq(str, str_4, strlen(str_4));

    ASSERT_EQ(to_base64({b64_5, sizeof(b64_5) - 1}, str), strlen(str_5));
    assert_arreq(str, str_5, strlen(str_5));

    ASSERT_EQ(to_base64({b64_6, sizeof(b64_6) - 1}, str), strlen(str_6));
    assert_arreq(str, str_6, strlen(str_6));

    ASSERT_EQ(to_base64({b64_7, sizeof(b64_7) - 1}, str), strlen(str_7));
    assert_arreq(str, str_7, strlen(str_7));
}

TEST(UtilString, ToBinary)
{
    byte bin[8];

    const char str_1[] = "11";

    ASSERT_EQ(to_binary({}, bin), 0);
    ASSERT_EQ(to_binary(str_1, {}), 0);

    const char str_2[] = "0123456789abcdef";
    const byte bin_2[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};

    ASSERT_EQ(to_binary(str_2, bin), 8);

    assert_arreq(bin, bin_2, sizeof(bin_2));

    const char str_3[] = "0123456789abcdef0011223344556677";
    const byte bin_3[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};

    ASSERT_EQ(to_binary(str_3, bin), 8);

    assert_arreq(bin, bin_3, sizeof(bin_3));

    const char str_4[] = "4";
    const byte bin_4[] = {0x04};

    ASSERT_EQ(to_binary(str_4, bin), 1);

    assert_arreq(bin, bin_4, sizeof(bin_4));

    const char str_5[] = "67b";
    const byte bin_5[] = {0x06, 0x7b};

    ASSERT_EQ(to_binary(str_5, bin), 2);

    assert_arreq(bin, bin_5, sizeof(bin_5));

    const char str_6[] = "426g";
    const byte bin_6[] = {0x42};

    ASSERT_EQ(to_binary(str_6, bin), 1);

    assert_arreq(bin, bin_6, sizeof(bin_6));
}

TEST(UtilString, ToInteger)
{
    ASSERT_EQ(to_integer("0"), 0);
    ASSERT_EQ(to_integer("1"), 1);
    ASSERT_EQ(to_integer("-1"), -1);
    ASSERT_EQ(to_integer("12"), 12);
    ASSERT_EQ(to_integer("+12"), 12);
    ASSERT_EQ(to_integer("-12"), -12);
    ASSERT_EQ(to_integer("+1234"), 1234);
    ASSERT_EQ(to_integer("-1234"), -1234);
    ASSERT_EQ(to_integer("2147483647"), 2147483647);
    ASSERT_EQ(to_integer("-2147483648"), -2147483648);
    ASSERT_EQ(to_integer("deadbeef", 16), 0xdeadbeef);
    ASSERT_EQ(to_integer("DEADC0DE", 16), 0xdeadc0de);
    ASSERT_EQ(to_integer("11010001", 2), 0b11010001);
    ASSERT_EQ(to_integer("4567", 8), 04567);
    ASSERT_EQ(to_integer("xyz", 36), 44027);
    ASSERT_EQ(to_integer("10 still_10"), 10);
    ASSERT_EQ(to_integer("10again_10"), 10);

    ASSERT_EQ(to_integer({}), 0);
    ASSERT_EQ(to_integer(""), 0);
    ASSERT_EQ(to_integer("hello"), 0);
    ASSERT_EQ(to_integer("--1"), 0);
    ASSERT_EQ(to_integer(" 42"), 0);
}

TEST(UtilString, ToFloating)
{
    ASSERT_EQ(to_floating("0"), 0);
    ASSERT_EQ(to_floating("4"), 4);
    ASSERT_EQ(to_floating("5."), 5);
    ASSERT_EQ(to_floating(".9"), 0.9);
    ASSERT_EQ(to_floating("+5."), 5);
    ASSERT_EQ(to_floating("-.9"), -0.9);
    ASSERT_EQ(to_floating("0.0"), 0);
    ASSERT_EQ(to_floating("0.1"), 0.1);
    ASSERT_EQ(to_floating("+0.2"), 0.2);
    ASSERT_EQ(to_floating("-0.3"), -0.3);
    ASSERT_EQ(to_floating("666.777"), 666.777);
    ASSERT_EQ(to_floating("+666.777"), 666.777);
    ASSERT_EQ(to_floating("-888.999"), -888.999);

    ASSERT_EQ(to_floating({}), 0);
    ASSERT_EQ(to_floating(""), 0);
    ASSERT_EQ(to_floating("hello"), 0);
    ASSERT_EQ(to_floating("1.0.0"), 0);
    ASSERT_EQ(to_floating("--1"), 0);
    ASSERT_EQ(to_floating(" 42"), 0);
    ASSERT_EQ(to_floating(" 69.0"), 0);
}

// TEST(UtilString, Join)
// {
//     static constexpr auto hello_world_ltrl = join_obj<"hello, ", "world">::literal();
//     static constexpr auto hello_world_view = join_obj<"hello, ", "world">::view();
//     static constexpr auto hello_world_cstr = join_obj<"hello, ", "world">::cstr();

//     static_assert(join_v<hello_world_ltrl, "!"> == string_literal{"hello, world!"});
//     static_assert(hello_world_view == "hello, world");
//     static_assert(std::equal(hello_world_cstr, hello_world_cstr + str_len(hello_world_cstr), "hello, world"));

//     ASSERT_EQ(hello_world_ltrl, string_literal{"hello, world"});
//     ASSERT_EQ(hello_world_view, "hello, world");
//     ASSERT_STREQ(hello_world_cstr, "hello, world");
// }

// TEST(UtilString, ToStringInteger)
// {
//     ASSERT_EQ(to_string<0>::view(), "0");
//     ASSERT_EQ(to_string<1337>::view(), "1337");
//     ASSERT_EQ(to_string<12345>::view(), "12345");
//     ASSERT_EQ(to_string<-10000>::view(), "-10000");
//     ASSERT_EQ((to_string<0xdeadbeef, 16>::view()), "deadbeef");
//     ASSERT_EQ((to_string<0xdeadc0de, 16, true>::view()), "DEADC0DE");
//     ASSERT_EQ((to_string<0b11010001, 2>::view()), "11010001");
//     ASSERT_EQ((to_string<04567, 8>::view()), "4567");
//     ASSERT_EQ((to_string<44027, 36>::view()), "xyz");
// }

TEST(UtilString, ToStringFloating)
{
    // TODO ASSERT_EQ(to_string<0.0, 2>::view, "0.00");
}

}
}