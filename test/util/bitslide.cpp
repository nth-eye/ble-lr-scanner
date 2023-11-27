#include "test.h"
#include "nth/util/bitslide.h"

namespace nth {
namespace {

using bitslide_t = nth::bitslide<uint32_t, uint32_t>;

TEST(UtilBitslide, Latest)
{
    ASSERT_EQ(bitslide_t{0}.latest(), 0);
    ASSERT_EQ(bitslide_t{1}.latest(), 1);
    ASSERT_EQ(bitslide_t{31}.latest(), 31);
    ASSERT_EQ(bitslide_t{32}.latest(), 32);
    ASSERT_EQ(bitslide_t{827280431}.latest(), 827280431);
}

TEST(UtilBitslide, CheckDefault)
{
    bitslide_t b;

    ASSERT_EQ(b.latest(), 0);
    ASSERT_EQ(b.check(0), false);
    ASSERT_EQ(b.check(-1), true);
    for (uint32_t i = 1; i <= 32; ++i)
        ASSERT_EQ(b.check(i), true);
}

TEST(UtilBitslide, Check)
{
    bitslide_t b = 35;

    ASSERT_EQ(b.latest(), 35);
    ASSERT_EQ(b.check(36), true);
    ASSERT_EQ(b.check(-1), true);
    for (uint32_t i = 0; i <= b.latest(); ++i)
        ASSERT_EQ(b.check(i), false);
}

TEST(UtilBitslide, Update)
{
    bitslide_t b = 33;

    b.update(32);

    ASSERT_EQ(b.latest(), 33);
    ASSERT_EQ(b.check(33), false);
    for (uint32_t i = 0; i <= b.latest(); ++i)
        ASSERT_EQ(b.check(i), false);

    b = 0;

    ASSERT_EQ(b.check_and_update(0), false);
    ASSERT_EQ(b.check_and_update(0), false);
    ASSERT_EQ(b.check_and_update(1), true);
    ASSERT_EQ(b.check_and_update(31), true);
    ASSERT_EQ(b.check_and_update(31), false);
    ASSERT_EQ(b.check_and_update(30), true);
    ASSERT_EQ(b.check_and_update(32), true);
    ASSERT_EQ(b.check_and_update(0), false);
    ASSERT_EQ(b.check_and_update(1), false);
    ASSERT_EQ(b.check_and_update(2), true);

    b = 0;

    ASSERT_EQ(b.check_and_update(33), true);
    ASSERT_EQ(b.check_and_update(33), false);
    ASSERT_EQ(b.check_and_update(0), false);
    ASSERT_EQ(b.check_and_update(1), false);
    ASSERT_EQ(b.check_and_update(2), true);
}

}
}