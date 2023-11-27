#include "test.h"
#include "nth/util/scope.h"

namespace nth {
namespace {

TEST(UtilMeta, ScopeGuardNoArgs)
{
    int counter = 0;
    {
    scope_guard guard = [&] {++counter;};
    ASSERT_EQ(counter, 0);
    }
    ASSERT_EQ(counter, 1);
}

TEST(UtilMeta, ScopeGuardWithArgs)
{
    int counter = 25;
    {
    scope_guard guard = { [&] (int x) {counter += x;}, 17 };
    ASSERT_EQ(counter, 25);
    }
    ASSERT_EQ(counter, 42);
}

TEST(UtilMeta, FusedGuard)
{
    int counter = 25;
    {
    fused_guard guard = { [&] (int x) {counter += x;}, 17 };
    ASSERT_EQ(guard.active(), true);
    ASSERT_EQ(counter, 25);
    }
    ASSERT_EQ(counter, 42);

    {
    fused_guard guard = { [&] (int x) {counter -= x;}, 99 };
    guard.defuse();
    ASSERT_EQ(guard.active(), false);
    ASSERT_EQ(counter, 42);
    }
    ASSERT_EQ(counter, 42);

    {
    fused_guard guard = { [&] (int x) {counter *= x;}, 2 };
    guard.invoke();
    ASSERT_EQ(guard.active(), false);
    ASSERT_EQ(counter, 84);
    }
    ASSERT_EQ(counter, 84);
}

}
}