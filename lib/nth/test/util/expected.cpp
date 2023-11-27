#include "test.h"
#include "nth/util/expected.h"

namespace nth {
namespace {

TEST(UtilExpected, HasValue)
{
    ASSERT_EQ((expected<double, int>{0.0}).has_value(), true);
    ASSERT_EQ((expected<double, int>{0}).has_value(), false);
    ASSERT_EQ((expected<double, int>{}).has_value(), false);

    ASSERT_EQ((expected<double, int>{0.0}), true);
    ASSERT_EQ((expected<double, int>{0}), false);
    ASSERT_EQ((expected<double, int>{}), false); 
}

TEST(UtilExpected, Value)
{
    ASSERT_EQ((expected<bool, int>{true}).value(), true);
    ASSERT_EQ((expected<bool, int>{false}).value(), false);
    ASSERT_EQ((expected<int, bool>{424242}).value(), 424242);
}

TEST(UtilExpected, Error)
{
    ASSERT_EQ((expected<bool, int>{0}).error(), 0);
    ASSERT_EQ((expected<bool, int>{-55}).error(), -55);
    ASSERT_EQ((expected<int, bool>{true}).error(), true);
}

}
}