#include "test.h"
#include "nth/util/bit.h"

namespace nth {
namespace {

TEST(UtilBit, LittleEndian)
{
    uint16_t word = 0x1337;

    if constexpr (little_endian())
        ASSERT_EQ(reinterpret_cast<uint8_t*>(&word)[0], 0x37);
    else
        ASSERT_EQ(reinterpret_cast<uint8_t*>(&word)[0], 0x13);
}

TEST(UtilBit, ToUnsigned)
{
    static_assert(std::is_same_v<decltype(to_unsigned(1)), unsigned>);
    static_assert(std::is_same_v<decltype(to_unsigned(1l)), unsigned long>);
    static_assert(std::is_same_v<decltype(to_unsigned(1ll)), unsigned long long>);
    static_assert(std::is_same_v<decltype(to_unsigned((short) 1)), unsigned short>);
    static_assert(std::is_same_v<decltype(to_unsigned((signed char) 1)), unsigned char>);

    static_assert(std::is_same_v<decltype(to_unsigned(int8_t(1))), uint8_t>);
    static_assert(std::is_same_v<decltype(to_unsigned(int16_t(1))), uint16_t>);
    static_assert(std::is_same_v<decltype(to_unsigned(int32_t(1))), uint32_t>);
    static_assert(std::is_same_v<decltype(to_unsigned(int64_t(1))), uint64_t>);

    ASSERT_EQ(to_unsigned(int8_t(-128)), uint8_t(128));
    ASSERT_EQ(to_unsigned(int8_t(-1)), uint8_t(255));
    ASSERT_EQ(to_unsigned(0xb16b00b5), 0xb16b00b5u);
}

TEST(UtilBit, IsPow2)
{
    ASSERT_EQ(is_pow2(0), false);
    ASSERT_EQ(is_pow2(1), true);
    ASSERT_EQ(is_pow2(2), true);
    ASSERT_EQ(is_pow2(3), false);
    ASSERT_EQ(is_pow2(4), true);
    ASSERT_EQ(is_pow2(5), false);
    ASSERT_EQ(is_pow2(6), false);
    ASSERT_EQ(is_pow2(7), false);
    ASSERT_EQ(is_pow2(8), true);
    ASSERT_EQ(is_pow2(1337), false);
    ASSERT_EQ(is_pow2(4096), true);
}

TEST(UtilBit, BitSize)
{
    ASSERT_EQ(bit_size<uint8_t>, 8);
    ASSERT_EQ(bit_size<uint16_t>, 16);
    ASSERT_EQ(bit_size<uint32_t>, 32);
    ASSERT_EQ(bit_size<uint64_t>, 64);
}

TEST(UtilBit, IntBitsFull)
{
    ASSERT_EQ(int_bits_full<uint8_t>, 0xff);
    ASSERT_EQ(int_bits_full<uint16_t>, 0xffff);
    ASSERT_EQ(int_bits_full<uint32_t>, 0xffffffff);
    ASSERT_EQ(int_bits_full<uint64_t>, 0xffffffffffffffff);
}

TEST(UtilBit, IntBitsWrap)
{
    ASSERT_EQ(int_bits_wrap<uint8_t>, 0b111);
    ASSERT_EQ(int_bits_wrap<uint16_t>, 0b1111);
    ASSERT_EQ(int_bits_wrap<uint32_t>, 0b11111);
    ASSERT_EQ(int_bits_wrap<uint64_t>, 0b111111);
}

TEST(UtilBit, IntBitsLog2)
{
    ASSERT_EQ(int_bits_log2<uint8_t>, 3);
    ASSERT_EQ(int_bits_log2<uint16_t>, 4);
    ASSERT_EQ(int_bits_log2<uint32_t>, 5);
    ASSERT_EQ(int_bits_log2<uint64_t>, 6);
}

TEST(UtilBit, IntBitCeilDiv)
{
    auto ceil8 = [](int x) {return int_bits_ceil_div<uint8_t>(x);};
    ASSERT_EQ(ceil8(0), 0);
    ASSERT_EQ(ceil8(1), 1);
    ASSERT_EQ(ceil8(7), 1);
    ASSERT_EQ(ceil8(8), 1);
    ASSERT_EQ(ceil8(42), 6);
    ASSERT_EQ(ceil8(63), 8);
    ASSERT_EQ(ceil8(64), 8);
    ASSERT_EQ(ceil8(69), 9);
    ASSERT_EQ(ceil8(1337), 168);

    auto ceil16 = [](int x) {return int_bits_ceil_div<uint16_t>(x);};
    ASSERT_EQ(ceil16(0), 0);
    ASSERT_EQ(ceil16(1), 1);
    ASSERT_EQ(ceil16(15), 1);
    ASSERT_EQ(ceil16(16), 1);
    ASSERT_EQ(ceil16(42), 3);
    ASSERT_EQ(ceil16(63), 4);
    ASSERT_EQ(ceil16(64), 4);
    ASSERT_EQ(ceil16(69), 5);
    ASSERT_EQ(ceil16(1337), 84);

    auto ceil32 = [](int x) {return int_bits_ceil_div<uint32_t>(x);};
    ASSERT_EQ(ceil32(0), 0);
    ASSERT_EQ(ceil32(1), 1);
    ASSERT_EQ(ceil32(31), 1);
    ASSERT_EQ(ceil32(32), 1);
    ASSERT_EQ(ceil32(42), 2);
    ASSERT_EQ(ceil32(63), 2);
    ASSERT_EQ(ceil32(64), 2);
    ASSERT_EQ(ceil32(69), 3);
    ASSERT_EQ(ceil32(1337), 42);

    auto ceil64 = [](int x) {return int_bits_ceil_div<uint64_t>(x);};
    ASSERT_EQ(ceil64(0), 0);
    ASSERT_EQ(ceil64(1), 1);
    ASSERT_EQ(ceil64(42), 1);
    ASSERT_EQ(ceil64(63), 1);
    ASSERT_EQ(ceil64(64), 1);
    ASSERT_EQ(ceil64(69), 2);
    ASSERT_EQ(ceil64(1337), 21);
    ASSERT_EQ(ceil64(4096), 64);
}

TEST(UtilBit, WordsIntBits)
{
    auto wib8 = [](size_t x) {return words_in_bits<uint8_t>(x);};
    ASSERT_EQ(wib8(0), 0);
    ASSERT_EQ(wib8(1), 1);
    ASSERT_EQ(wib8(7), 1);
    ASSERT_EQ(wib8(8), 1);
    ASSERT_EQ(wib8(9), 2);
    ASSERT_EQ(wib8(42), 6);
    ASSERT_EQ(wib8(64), 8);
    ASSERT_EQ(wib8(69), 9);
    ASSERT_EQ(wib8(228), 29);
    ASSERT_EQ(wib8(9999), 1250);

    auto wib16 = [](size_t x) {return words_in_bits<uint16_t>(x);};
    ASSERT_EQ(wib16(0), 0);
    ASSERT_EQ(wib16(1), 1);
    ASSERT_EQ(wib16(15), 1);
    ASSERT_EQ(wib16(16), 1);
    ASSERT_EQ(wib16(17), 2);
    ASSERT_EQ(wib16(42), 3);
    ASSERT_EQ(wib16(64), 4);
    ASSERT_EQ(wib16(69), 5);
    ASSERT_EQ(wib16(228), 15);
    ASSERT_EQ(wib16(9999), 625);

    auto wib32 = [](size_t x) {return words_in_bits<uint32_t>(x);};
    ASSERT_EQ(wib32(0), 0);
    ASSERT_EQ(wib32(1), 1);
    ASSERT_EQ(wib32(31), 1);
    ASSERT_EQ(wib32(32), 1);
    ASSERT_EQ(wib32(33), 2);
    ASSERT_EQ(wib32(42), 2);
    ASSERT_EQ(wib32(64), 2);
    ASSERT_EQ(wib32(69), 3);
    ASSERT_EQ(wib32(228), 8);
    ASSERT_EQ(wib32(9999), 313);

    auto wib64 = [](size_t x) {return words_in_bits<uint64_t>(x);};
    ASSERT_EQ(wib64(0), 0);
    ASSERT_EQ(wib64(1), 1);
    ASSERT_EQ(wib64(63), 1);
    ASSERT_EQ(wib64(64), 1);
    ASSERT_EQ(wib64(65), 2);
    ASSERT_EQ(wib64(42), 1);
    ASSERT_EQ(wib64(64), 1);
    ASSERT_EQ(wib64(69), 2);
    ASSERT_EQ(wib64(228), 4);
    ASSERT_EQ(wib64(9999), 157);
}

TEST(UtilBit, BytesInBits)
{
    ASSERT_EQ(bytes_in_bits(0), 0);
    ASSERT_EQ(bytes_in_bits(1), 1);
    ASSERT_EQ(bytes_in_bits(7), 1);
    ASSERT_EQ(bytes_in_bits(8), 1);
    ASSERT_EQ(bytes_in_bits(9), 2);
    ASSERT_EQ(bytes_in_bits(42), 6);
    ASSERT_EQ(bytes_in_bits(64), 8);
    ASSERT_EQ(bytes_in_bits(69), 9);
    ASSERT_EQ(bytes_in_bits(228), 29);
    ASSERT_EQ(bytes_in_bits(9999), 1250);
}

TEST(UtilBit, Bit)
{
    ASSERT_EQ(bit(0), 1);
    ASSERT_EQ(bit(1), 2);
    ASSERT_EQ(bit(7), 128);
    ASSERT_EQ(bit(31), 2147483648);
    ASSERT_EQ(bit<uint64_t>(63), uint64_t(9223372036854775808ull));
}

TEST(UtilBit, BitMask)
{
    ASSERT_EQ(bit_mask(0), 0b0);
    ASSERT_EQ(bit_mask(1), 0b1);
    ASSERT_EQ(bit_mask(2), 0b11);
    ASSERT_EQ(bit_mask(3), 0b111);
    ASSERT_EQ(bit_mask(7), 0b1111111);
    ASSERT_EQ(bit_mask(31), 0x7fffffff);
}

TEST(UtilBit, GetBit)
{
    ASSERT_EQ(get_bit(0, 0), 0);
    ASSERT_EQ(get_bit(0, 1), 0);
    ASSERT_EQ(get_bit(0, 2), 0);
    ASSERT_EQ(get_bit(0, 3), 0);
    ASSERT_EQ(get_bit(0, 4), 0);
    ASSERT_EQ(get_bit(0, 5), 0);
    ASSERT_EQ(get_bit(0, 6), 0);
    ASSERT_EQ(get_bit(0, 7), 0);
    ASSERT_EQ(get_bit(42, 0), 0);
    ASSERT_EQ(get_bit(42, 1), 1);
    ASSERT_EQ(get_bit(42, 2), 0);
    ASSERT_EQ(get_bit(42, 3), 1);
    ASSERT_EQ(get_bit(42, 4), 0);
    ASSERT_EQ(get_bit(42, 5), 1);
    ASSERT_EQ(get_bit(42, 6), 0);
    ASSERT_EQ(get_bit(42, 7), 0);
    ASSERT_EQ(get_bit(255, 0), 1);
    ASSERT_EQ(get_bit(255, 1), 1);
    ASSERT_EQ(get_bit(255, 2), 1);
    ASSERT_EQ(get_bit(255, 3), 1);
    ASSERT_EQ(get_bit(255, 4), 1);
    ASSERT_EQ(get_bit(255, 5), 1);
    ASSERT_EQ(get_bit(255, 6), 1);
    ASSERT_EQ(get_bit(255, 7), 1);
}

TEST(UtilBit, SetBit)
{
    uint8_t b = 0;

    set_bit(b, 0);
    set_bit(b, 1);
    set_bit(b, 5);

    ASSERT_EQ(b, 0x23);
}

TEST(UtilBit, ClearBit)
{
    uint8_t b = 0x23;

    clr_bit(b, 0);
    clr_bit(b, 1);
    clr_bit(b, 6);

    ASSERT_EQ(b, 0x20);

    clr_bit(b, 5);

    ASSERT_EQ(b, 0x00);
}

TEST(UtilBit, GetArrayBit)
{
    const uint8_t arr[] = { 0x02, 0x78 };

    ASSERT_EQ(get_arr_bit(arr, 0), 0);
    ASSERT_EQ(get_arr_bit(arr, 1), 1);
    ASSERT_EQ(get_arr_bit(arr, 2), 0);
    ASSERT_EQ(get_arr_bit(arr, 3), 0);
    ASSERT_EQ(get_arr_bit(arr, 4), 0);
    ASSERT_EQ(get_arr_bit(arr, 5), 0);
    ASSERT_EQ(get_arr_bit(arr, 6), 0);
    ASSERT_EQ(get_arr_bit(arr, 7), 0);
    ASSERT_EQ(get_arr_bit(arr, 8), 0);
    ASSERT_EQ(get_arr_bit(arr, 9), 0);
    ASSERT_EQ(get_arr_bit(arr, 10), 0);
    ASSERT_EQ(get_arr_bit(arr, 11), 1);
    ASSERT_EQ(get_arr_bit(arr, 12), 1);
    ASSERT_EQ(get_arr_bit(arr, 13), 1);
    ASSERT_EQ(get_arr_bit(arr, 14), 1);
    ASSERT_EQ(get_arr_bit(arr, 15), 0);
}

TEST(UtilBit, SetArrayBit)
{
    uint8_t arr[] = { 0x02, 0x15 };

    set_arr_bit(arr, 0);
    set_arr_bit(arr, 1);
    set_arr_bit(arr, 5);
    set_arr_bit(arr, 14);

    ASSERT_EQ(arr[0], 0x23);
    ASSERT_EQ(arr[1], 0x55);
}

TEST(UtilBit, ClearArrayBit)
{
    uint8_t arr[] = { 0x23, 0x55 };

    clr_arr_bit(arr, 0);
    clr_arr_bit(arr, 1);
    clr_arr_bit(arr, 6);
    clr_arr_bit(arr, 12);
    clr_arr_bit(arr, 14);

    ASSERT_EQ(arr[0], 0x20);
    ASSERT_EQ(arr[1], 0x05);
}

TEST(UtilBit, ShiftArrayLeft)
{
    uint8_t arr_1[] = { 0x11, 0x88 };

    shift_arr_left(arr_1);

    ASSERT_EQ(arr_1[0], 0x22);
    ASSERT_EQ(arr_1[1], 0x10);

    uint16_t arr_2[] = { 0x8811, 0x2233 };

    shift_arr_left(arr_2);
    shift_arr_left(arr_2);

    ASSERT_EQ(arr_2[0], 0x2044);
    ASSERT_EQ(arr_2[1], 0x88ce);
}

TEST(UtilBit, ShiftArrayRight)
{
    uint8_t arr_1[] = { 0x11, 0x88 };

    shift_arr_right(arr_1);

    ASSERT_EQ(arr_1[0], 0x08);
    ASSERT_EQ(arr_1[1], 0x44);

    uint16_t arr_2[] = { 0x8811, 0x2233 };

    shift_arr_right(arr_2);
    shift_arr_right(arr_2);

    ASSERT_EQ(arr_2[0], 0b1110'0010'0000'0100);
    ASSERT_EQ(arr_2[1], 0b0000'1000'1000'1100);
}

TEST(UtilBit, Bitswap)
{
    ASSERT_EQ(bitswap<uint8_t>(0xf0), 0b00001111);
    ASSERT_EQ(bitswap<uint16_t>(0xb307), 0b11100000'11001101);
    ASSERT_EQ(bitswap<uint32_t>(0x9e9cf800), 0b00000000'00011111'00111001'01111001);
    ASSERT_EQ(bitswap<uint64_t>(0x0123456789abcdef), 0b11110111'10110011'11010101'10010001'11100110'10100010'11000100'10000000);
}

TEST(UtilBit, Byteswap)
{
    ASSERT_EQ(byteswap<uint16_t>(0xdead), 0xadde);
    ASSERT_EQ(byteswap<uint32_t>(0x01234567), 0x67452301);
    ASSERT_EQ(byteswap<uint64_t>(0xefcdab8967452301), 0x0123456789abcdef);
}

TEST(UtilBit, Wordswap)
{
    ASSERT_EQ((wordswap<uint8_t, uint16_t>(0xdead)), 0xadde);
    ASSERT_EQ((wordswap<uint16_t, uint32_t>(0x01234567)), 0x45670123);
    ASSERT_EQ((wordswap<uint32_t, uint64_t>(0xefcdab8967452301)), 0x67452301efcdab89);
}

TEST(UtilBit, CountLeadingZeros)
{
    ASSERT_EQ(cntlz(0x80000000), 0);
    ASSERT_EQ(cntlz(0x01000000), 7);
    ASSERT_EQ(cntlz(0x00800000), 8);
    ASSERT_EQ(cntlz(0x00010000), 15);
    ASSERT_EQ(cntlz(0x00008000), 16);
    ASSERT_EQ(cntlz(0x00000100), 23);
    ASSERT_EQ(cntlz(0x00000080), 24);
    ASSERT_EQ(cntlz(0x00000001), 31);
    ASSERT_EQ(cntlz(0x00000000), 32);

    ASSERT_EQ(cntlz(0xfb5f2e2c), 0);
    ASSERT_EQ(cntlz(0x58835269), 1);
    ASSERT_EQ(cntlz(0xf9a63895), 0);
    ASSERT_EQ(cntlz(0xeb157265), 0);
    ASSERT_EQ(cntlz(0x25d613b2), 2);
    ASSERT_EQ(cntlz(0x7a0aa4ee), 1);
    ASSERT_EQ(cntlz(0x2e744e63), 2);
    ASSERT_EQ(cntlz(0x8bb413d2), 0);
    ASSERT_EQ(cntlz(0x54fe413f), 1);
}

TEST(UtilBit, CountTrailingZeros)
{
    ASSERT_EQ(cnttz(0x00000000), 32);
    ASSERT_EQ(cnttz(0x80000000), 31);
    ASSERT_EQ(cnttz(0x01000000), 24);
    ASSERT_EQ(cnttz(0x00800000), 23);
    ASSERT_EQ(cnttz(0x00010000), 16);
    ASSERT_EQ(cnttz(0x00008000), 15);
    ASSERT_EQ(cnttz(0x00000100), 8);
    ASSERT_EQ(cnttz(0x00000080), 7);
    ASSERT_EQ(cnttz(0x00000001), 0);

    ASSERT_EQ(cnttz(0xfb5f2e2c), 2);
    ASSERT_EQ(cnttz(0x58835269), 0);
    ASSERT_EQ(cnttz(0xf9a63895), 0);
    ASSERT_EQ(cnttz(0xeb157265), 0);
    ASSERT_EQ(cnttz(0x25d613b2), 1);
    ASSERT_EQ(cnttz(0x7a0aa4ee), 1);
    ASSERT_EQ(cnttz(0x2e744e63), 0);
    ASSERT_EQ(cnttz(0x8bb413d2), 1);
    ASSERT_EQ(cnttz(0x54fe413f), 0);
}

TEST(UtilBit, RotateLeft)
{
    ASSERT_EQ(rol(uint8_t(0b0011'0001), 4), 0b0001'0011);
    ASSERT_EQ(rol(uint16_t(0b1111'0000'0011'0001), 3), 0b1000'0001'1000'1111);
}

TEST(UtilBit, RotateRight)
{
    ASSERT_EQ(ror(uint8_t(0b0011'0001), 3), 0b0010'0110);
    ASSERT_EQ(ror(uint16_t(0b1111'0000'0011'0001), 1), 0b1111'1000'0001'1000);
}

TEST(UtilBit, MakeIntLsb)
{
    ASSERT_EQ(make_int_lsb<uint16_t>(0x37, 0x13), 0x1337);
    ASSERT_EQ(make_int_lsb<uint32_t>(0x44, 0x33, 0x22, 0x11), 0x11223344);
    ASSERT_EQ(make_int_lsb<uint64_t>(0xde, 0xc0, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde), 0xdeadbeefdeadc0de);
}

TEST(UtilBit, MakeIntMsb)
{
    ASSERT_EQ(make_int_msb<uint16_t>(0x13, 0x37), 0x1337);
    ASSERT_EQ(make_int_msb<uint32_t>(0x11, 0x22, 0x33, 0x44), 0x11223344);
    ASSERT_EQ(make_int_msb<uint64_t>(0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xc0, 0xde), 0xdeadbeefdeadc0de);
}

}
}