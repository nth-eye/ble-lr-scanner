#include "test.h"
#include "nth/math/finite.h"

namespace nth {
namespace {

using base_int_t = bigint<uint32_t, 256>;
using base_fint_t = ff_int<base_int_t, 13>; 

TEST(MathFinite, Equality)
{
    ASSERT_EQ(base_fint_t{}, base_fint_t{});
    ASSERT_EQ(base_fint_t{1337}, base_fint_t{1337});
    ASSERT_EQ(base_fint_t{}, base_fint_t{});
    // ASSERT_EQ
}

}
}