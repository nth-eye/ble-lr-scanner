#include "test.h"
#include "nth/util/typeid.h"

namespace nth {
namespace {

TEST(UtilTypeid, TypeId)
{
    static_assert(type_id<int> != type_id<bool>);
    static_assert(type_id<int> != type_id<float>);

    ASSERT_EQ(type_id<int>, type_id<int>);
    ASSERT_NE(type_id<int>, type_id<bool>);
    ASSERT_NE(type_id<int>, type_id<float>);
}

TEST(UtilTypeid, TypeName)
{
    static_assert(type_name<int> == "int");
    static_assert(type_name<bool> == "bool");
    static_assert(type_name<float> == "float");

    ASSERT_EQ(type_name<int>, "int");
    ASSERT_EQ(type_name<bool>, "bool");
    ASSERT_EQ(type_name<float>, "float");
}

}
}
