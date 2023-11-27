#include "test.h"
#include "nth/util/half.h"
#if !(NTH_UTIL_HALF_NATIVE)
#include <cmath>

namespace nth {
namespace {

constexpr auto f_bits(float fp)
{
    return std::bit_cast<uint32_t>(fp);
}

constexpr auto d_bits(double fp)
{
    return std::bit_cast<uint64_t>(fp);
}

constexpr auto f_to_half(float fp)
{
    return float_to_half(f_bits(fp));
}

constexpr auto d_to_half(double fp)
{
    return double_to_half(d_bits(fp));
}

TEST(UtilHalf, FloatToHalf)
{
    ASSERT_EQ(0x0000, f_to_half(0.0));
    ASSERT_EQ(0x8000, f_to_half(-0.0));
    ASSERT_EQ(0x3c00, f_to_half(1.0));
    ASSERT_EQ(0x3e00, f_to_half(1.5));
    ASSERT_EQ(0x7bff, f_to_half(65504.0));
    ASSERT_EQ(0x0001, f_to_half(5.960464477539063e-8));
    ASSERT_EQ(0x0400, f_to_half(0.00006103515625));
    ASSERT_EQ(0xc400, f_to_half(-4.0));
    ASSERT_EQ(0x7c00, f_to_half(INFINITY));
    ASSERT_EQ(0x7e00, f_to_half(NAN));
    ASSERT_EQ(0xfc00, f_to_half(-INFINITY));
}

TEST(UtilHalf, HalfToFloat)
{
    ASSERT_EQ(half_to_float(0x0000), f_bits(0.0));
    ASSERT_EQ(half_to_float(0x8000), f_bits(-0.0));
    ASSERT_EQ(half_to_float(0x3c00), f_bits(1.0));
    ASSERT_EQ(half_to_float(0x3e00), f_bits(1.5));
    ASSERT_EQ(half_to_float(0x7bff), f_bits(65504.0));
    ASSERT_EQ(half_to_float(0x0001), f_bits(5.960464477539063e-8));
    ASSERT_EQ(half_to_float(0x0400), f_bits(0.00006103515625));
    ASSERT_EQ(half_to_float(0xc400), f_bits(-4.0));
    ASSERT_EQ(half_to_float(0x7c00), f_bits(INFINITY));
    ASSERT_EQ(half_to_float(0x7e00), f_bits(NAN));
    ASSERT_EQ(half_to_float(0xfc00), f_bits(-INFINITY));
}

TEST(UtilHalf, DoubleToHalf)
{
    ASSERT_EQ(0x0000, d_to_half(0.0));
    ASSERT_EQ(0x8000, d_to_half(-0.0));
    ASSERT_EQ(0x3c00, d_to_half(1.0));
    ASSERT_EQ(0x3e00, d_to_half(1.5));
    ASSERT_EQ(0x7bff, d_to_half(65504.0));
    ASSERT_EQ(0x0001, d_to_half(5.960464477539063e-8));
    ASSERT_EQ(0x0400, d_to_half(0.00006103515625));
    ASSERT_EQ(0xc400, d_to_half(-4.0));
    ASSERT_EQ(0x7c00, d_to_half(INFINITY));
    ASSERT_EQ(0x7e00, d_to_half(NAN));
    ASSERT_EQ(0xfc00, d_to_half(-INFINITY));
}

TEST(UtilHalf, HalfToDouble)
{
    ASSERT_EQ(half_to_double(0x0000), d_bits(0.0));
    ASSERT_EQ(half_to_double(0x8000), d_bits(-0.0));
    ASSERT_EQ(half_to_double(0x3c00), d_bits(1.0));
    ASSERT_EQ(half_to_double(0x3e00), d_bits(1.5));
    ASSERT_EQ(half_to_double(0x7bff), d_bits(65504.0));
    ASSERT_EQ(half_to_double(0x0001), d_bits(5.960464477539063e-8));
    ASSERT_EQ(half_to_double(0x0400), d_bits(0.00006103515625));
    ASSERT_EQ(half_to_double(0xc400), d_bits(-4.0));
    ASSERT_EQ(half_to_double(0x7c00), d_bits(INFINITY));
    ASSERT_EQ(half_to_double(0x7e00), d_bits(NAN));
    ASSERT_EQ(half_to_double(0xfc00), d_bits(-INFINITY));
}

}
}

#endif