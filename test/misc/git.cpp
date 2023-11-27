#include "test.h"
#include "nth/misc/git.h"
#include "nth/util/string.h"

namespace nth {
namespace {

TEST(UtilGit, UtcAndBranch)
{
    ASSERT_NE(git_utc, 0);
    ASSERT_EQ(git_branch, "main");
    // ASSERT_EQ(std::string_view{git_rev}.ends_with(to_string<git_utc>::view()), !git_clean);
    // ASSERT_EQ(std::string_view{git_rev}.ends_with(git_commit), git_clean);
    // ASSERT_EQ(std::string_view{git_rev}.starts_with(git_branch), true);
}

}
}