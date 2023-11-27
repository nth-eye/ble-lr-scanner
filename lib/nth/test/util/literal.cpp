#include "test.h"
#include "nth/util/literal.h"

namespace nth {
namespace {

TEST(UtilMeta, StringLiteral)
{
    static constexpr const char example[] = "example";
    static constexpr string_literal str = example;

    static_assert(str.size()    == sizeof(example) - 1);
    static_assert(str.front()   == 'e');
    static_assert(str.back()    == 'e');
    static_assert(str.data()    == &str.front());
    static_assert(str.data()    == str.begin());
    static_assert(str.end()     == str.begin() + str.size());
    static_assert(str           == string_literal{"example"});
    static_assert(str           != string_literal{"exumple"});
    static_assert(str           != string_literal{"not example"});
    static_assert(str.view()    == "example");
    for (size_t i = 0; i < str.size(); ++i)
        ASSERT_EQ(str[i], example[i]);
}

TEST(UtilMeta, ByteLiteral)
{
    static constexpr const char example[] = "\x42\x00\x13\x37";
    static constexpr byte_literal bin = example;

    static_assert(bin.size()    == sizeof(example) - 1);
    static_assert(bin.front()   == 0x42);
    static_assert(bin.back()    == 0x37);
    static_assert(bin.data()    == &bin.front());
    static_assert(bin.data()    == bin.begin());
    static_assert(bin.end()     == bin.begin() + bin.size());
    static_assert(bin           == std::span<const byte>{{0x42, 0x00, 0x13, 0x37}});
    static_assert(bin           == byte_literal{"\x42\x00\x13\x37"});
    static_assert(bin           != byte_literal{"\x41\x00\x13\x37"});
    static_assert(bin           != byte_literal{"\x42\x00\x13\x37\x69"});
    for (size_t i = 0; i < bin.size(); ++i)
        ASSERT_EQ(bin[i], example[i]);
}

}
}