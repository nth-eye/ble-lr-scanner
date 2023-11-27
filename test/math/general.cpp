#include "test.h"
#include "nth/math/general.h"

namespace nth {
namespace {

TEST(MathGeneral, UnsignedDivCeil)
{
    ASSERT_EQ(uint_div_ceil(0u, 0u), 0);
    ASSERT_EQ(uint_div_ceil(0u, 1u), 0);
    ASSERT_EQ(uint_div_ceil(4u, 2u), 2);
    ASSERT_EQ(uint_div_ceil(5u, 2u), 3);
    ASSERT_EQ(uint_div_ceil(42u, 11u), 4); 
}

TEST(MathGeneral, UnsignedFactorial)
{
    ASSERT_EQ(uint_fact(0u), 1);
    ASSERT_EQ(uint_fact(1u), 1);
    ASSERT_EQ(uint_fact(2u), 2);
    ASSERT_EQ(uint_fact(3u), 6);
    ASSERT_EQ(uint_fact(4u), 24);
    ASSERT_EQ(uint_fact(13ul), 6227020800); 
}

TEST(MathGeneral, IntegerPower)
{
    ASSERT_EQ(int_pow(1, 0), 1);
    ASSERT_EQ(int_pow(1, 1), 1);
    ASSERT_EQ(int_pow(1, 99), 1);
    ASSERT_EQ(int_pow(2, 0), 1);
    ASSERT_EQ(int_pow(2, 1), 2);
    ASSERT_EQ(int_pow(2, 2), 4);
    ASSERT_EQ(int_pow(2, 10), 1024);
    ASSERT_EQ(int_pow(3ull, 20), 3486784401);
    ASSERT_EQ(int_pow(2ull, 63), 9223372036854775808ull);
}

TEST(MathGeneral, IntegerLength)
{
    ASSERT_EQ(int_len(0), 1);
    ASSERT_EQ(int_len(1), 1);
    ASSERT_EQ(int_len(9), 1);
    ASSERT_EQ(int_len(10), 2);
    ASSERT_EQ(int_len(99), 2);
    ASSERT_EQ(int_len(100), 3);
    ASSERT_EQ(int_len(999), 3);
    ASSERT_EQ(int_len(20220805), 8);
    ASSERT_EQ(int_len(-1), 2);
    ASSERT_EQ(int_len(-777), 4);
    ASSERT_EQ(int_len(-20220805), 9);
    ASSERT_EQ(int_len(0b011011, 2), 5);
    ASSERT_EQ(int_len(0x1fff, 16), 4);
    ASSERT_EQ(int_len(0x10ff, 17), 3);
}

TEST(MathGeneral, NumberMap)
{
    ASSERT_EQ(num_map(0, 0, 1, 0, 0), 0);
    ASSERT_EQ(num_map(1, 0, 1, 0, 0), 0);
    ASSERT_EQ(num_map(0, 0, 1, 0, 1), 0);
    ASSERT_EQ(num_map(1, 0, 1, 0, 1), 1);
    ASSERT_EQ(num_map(0xff, 0x00, 0xff, 0, 100), 100);
    ASSERT_EQ(num_map(0x80, 0x00, 0xff, 0, 100), 50);
    ASSERT_EQ(num_map(0x40, 0x00, 0xff, 0, 100), 25);
    ASSERT_EQ(num_map(0x20, 0x00, 0xff, 0, 100), 12);
    ASSERT_EQ(num_map(0x42, 0x00, 0xff, 0, 1000), 258);
    ASSERT_EQ(num_map(0x69, 0x00, 0xff, 0, 1000), 411);
}

}
}