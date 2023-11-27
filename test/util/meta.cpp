#include "test.h"
#include "nth/util/meta.h"

namespace nth {
namespace {

TEST(UtilMeta, Boolean)
{
    static_assert(boolean<bool>     == true);
    static_assert(boolean<void*>    == false);
    static_assert(boolean<float>    == false);
    static_assert(boolean<unsigned> == false);
}

TEST(UtilMeta, Arithmetic)
{
    static_assert(arithmetic<bool>      == true);
    static_assert(arithmetic<void*>     == false);
    static_assert(arithmetic<int*>      == false);
    static_assert(arithmetic<float>     == true);
    static_assert(arithmetic<unsigned>  == true);
    static_assert(arithmetic<uint64_t>  == true);
}

TEST(UtilMeta, Stringable)
{
    static_assert(stringable<char*>             == true);
    static_assert(stringable<std::string_view>  == true);
    static_assert(stringable<std::string>       == true);
    static_assert(stringable<void*>             == false);
    static_assert(stringable<int*>              == false);
    static_assert(stringable<float>             == false);
    static_assert(stringable<unsigned>          == false);
    static_assert(stringable<uint64_t>          == false);
}

TEST(UtilMeta, Enumeration)
{
    enum weak {};
    enum class strong {};

    static_assert(enumeration<char*>             == false);
    static_assert(enumeration<std::string_view>  == false);
    static_assert(enumeration<std::string>       == false);
    static_assert(enumeration<void*>             == false);
    static_assert(enumeration<int*>              == false);
    static_assert(enumeration<float>             == false);
    static_assert(enumeration<unsigned>          == false);
    static_assert(enumeration<weak>             == true);
    static_assert(enumeration<strong>           == true);
}

TEST(UtilMeta, Countof)
{
    const uint8_t  arr_1[10] = {};
    const uint16_t arr_2[20] = {};
    const uint32_t arr_3[30] = {};
    const uint64_t arr_4[40] = {};

    static_assert(countof(arr_1) == 10);
    static_assert(countof(arr_2) == 20);
    static_assert(countof(arr_3) == 30);
    static_assert(countof(arr_4) == 40);
}

}
}