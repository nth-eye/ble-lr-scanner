#include "test.h"
#include "nth/crypto/cipher/chacha20.h"

namespace nth {
namespace {

TEST(Crypto, CipherChaChaQuarterRound)
{
    // imp::chacha::word a = 0x11111111;
    // imp::chacha::word b = 0x01020304;
    // imp::chacha::word c = 0x9b8d6f43;
    // imp::chacha::word d = 0x01234567;

    // imp::chacha::quarter_round(a, b, c, d);

    // ASSERT_EQ(a, 0xea2a92f4);
    // ASSERT_EQ(b, 0xcb1cf8ce);
    // ASSERT_EQ(c, 0x4581472e);
    // ASSERT_EQ(d, 0x5881c4bb);
}

TEST(Crypto, CipherChaChaRound)
{
    
}

}

}