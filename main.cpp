#include "nth/nth.h"
// #include <format>

using namespace nth::literals;
using namespace std::literals;

// using nvs_access_func = size_t(nth::tid, void*, size_t);

// /**
//  * @brief Fixed-size NVS accessor. It's a wrapper which writes underlying 
//  * element to flash with unique entry ID on assignment. Stores element copy 
//  * in RAM and compares to it, so that excessive writes are discarded if 
//  * values equal. Implicitly converts to underlying type. The value itself 
//  * can be accessed through operator->(), but it's read-only!
//  * 
//  * @tparam T Underlying stored type
//  * @tparam L Lambda trick, don't touch
//  */
// template<class T, 
//     nvs_access_func Set, 
//     nvs_access_func Get, auto L = []{}>
// struct nvs_accessor {

//     using value_type = T;
//     const value_type* operator->() const    { return &value; }
//     const value_type& operator*() const     { return value; }
//     value_type get() const                  { return value; }

//     struct nvs_guard {

//         ~nvs_guard()
//         {
            
//         }
//     };

//     nvs_accessor& operator=(const value_type& other)
//     {
//         if (value != other) {
//             value = other;
//             send();
//         }
//         return *this;
//     }

//     bool init()
//     {
//         return Get(nth::type_id<value_type>, std::addressof(value), sizeof(value_type)) == sizeof(value_type);
//     }
// private:
//     bool send() const
//     {
//         return Set(nth::type_id<value_type>, std::addressof(value), sizeof(value_type)) == sizeof(value_type);
//     }
//     value_type value {};
// };

// template<typename T>
// concept IntegralConstantType = requires {
//     typename T::value_type;                            // Check if value_type is defined
//     { T::value } -> std::same_as<const typename T::value_type>; // Check if T::value is a constant of type value_type
// };

// template<class Fmt>
// void test_fn(Fmt str)
// {
//     // static_assert(nth::is_string_constant<Fmt>::value, "only string constants are allowed");

//     printf("%s\n", nth::type_name<Fmt>.data());
// }

// template<nth::string_literal S>
// void test_fn(nth::string_constant<S>) 
// {
//     // std::cout << str.view() << std::endl;

//     // printf("%s\n", nth::type_name<decltype(str)>.data());
// }

#include <vector>

constexpr auto fmt_ct_parse_base(std::string_view str, auto handle_char, auto handle_meta)
{
    using namespace nth;

    auto state = fmt_state::text;
    auto posit = 0u;
    auto index = 0u;

    for (size_t i = 0, j = 0; i < str.size(); ++i) {

        auto event = fmt_event_next(str[i]);

        switch (state) 
        {
#if (NTH_IO_FORMAT_CT_CHATGPT_OPTIMIZATION)
        default: break;
        case fmt_state::text:
            if (event == fmt_event::head) {
                if (i + 1 < str.size() && str[i + 1] == '{') { // Check for escaped bracket
                    handle_char('{');
                    i++; // Skip the next bracket
                } else {
                    state = fmt_state::text_head;
                }
            } else {
                handle_char(str[i]);
            }
        break;

        case fmt_state::text_head:
            switch (event)
            {
            case fmt_event::num:
                index = char_to_bin(str[i]);
                state = fmt_state::index;
            break;
            case fmt_event::semi:
                posit = j;
                state = fmt_state::specifier;
            break;
            case fmt_event::other:
                handle_char(str[i]);
                posit = ++j;
                state = fmt_state::specifier;
            break;
            case fmt_event::head:
                handle_char(str[i]);
                ++j;
                state = fmt_state::text;
            break;
            case fmt_event::tail:
                handle_meta(j, 0u, index++);
                state = fmt_state::text;
            break;
            }
        break;
#else
#if !(NTH_IO_FORMAT_BASE_ALLOW_NON_ESCAPED_CLOSING_BRACKET)
        default: break;
#endif
        case fmt_state::text:
            if (event == fmt_event::head) {
                state = fmt_state::text_head;
            } else {
                handle_char(str[i]);
                ++j;
            }
        break;

        case fmt_state::text_head:
            switch (event)
            {
            case fmt_event::num:
                index = str[i] - '0';
                state = fmt_state::index;
            break;
            case fmt_event::semi:
                posit = j;
                state = fmt_state::specifier;
            break;
            case fmt_event::other:
                handle_char(str[i]);
                posit = ++j;
                state = fmt_state::specifier;
            break;
            case fmt_event::head:
                handle_char(str[i]);
                ++j;
                state = fmt_state::text;
            break;
            case fmt_event::tail:
                handle_meta(j, 0u, index++);
                state = fmt_state::text;
            break;
            }
        break;
#endif
        case fmt_state::index:
            switch (event)
            {
            case fmt_event::num:
                index = index * 10 + char_to_bin(str[i]);
            break;
            case fmt_event::semi:
                posit = j;
                state = fmt_state::specifier;
            break;
            case fmt_event::tail:
                handle_meta(j, 0u, index++);
                state = fmt_state::text;
            break;
            default:
                fmt_error_handler(fmt_error::index_invalid);
            }
        break;

        case fmt_state::specifier:
            switch (event)
            {
            case fmt_event::head:
                fmt_error_handler(fmt_error::nested_bracket);
            break;
            case fmt_event::tail:
                handle_meta(posit, j - posit, index++);
                state = fmt_state::text;
            break;
            default:
                handle_char(str[i]);
                ++j;
            }
        break;
        }
    }
    if (state != fmt_state::text)
        fmt_error_handler(fmt_error::unmatched_bracket_head);
}

template<class _>
struct literal_dummy {
    template<size_t N>
    consteval literal_dummy(const char (&str)[N]) : sv{str, N - 1}
    {      
        // fmt_parse_base(sv, fmt_parse_stub, handle_args);
        // std::vector<char> tmp;
        // fmt_ct_parse_base(sv, [&] (char c ) { tmp.push_back(c); }, [] (...) {});
    }
    const std::string_view sv;
    const std::vector<char> new_string;
};

template<auto _ = []{}>
constexpr auto test_fn(literal_dummy<decltype(_)> str) 
{
    return str;
}

constexpr auto ret_1 = test_fn("test");
constexpr auto ret_2 = test_fn("test_2");
constexpr auto ret_3 = test_fn("test_3");

static_assert(typeid (ret_1) != typeid (ret_2));

int main()
{
    // test_fn("test");
    // test_fn("test_2");
    // test_fn("test_3");
    // auto test_1 = [] {
    //     static char buf[512];
    //     std::sprintf(buf, "| my view is: %d, %d, %s |\n", 42, 69, "true");
    // };
    // auto test_2 = [] {
    //     nth::v1::print<"| my view is: {}, {}, {} |\n">(42, 69, true);
    // };
    // auto test_3 = [] {
    //     nth::v2::print("| my view is: {}, {}, {} |\n", 42, 69, true);
    // };
    // static constexpr auto count = 50000000;

    // auto ret11 = nth::exec_time<count>(test_1);
    // auto ret21 = nth::exec_time<count>(test_2);
    // auto ret31 = nth::exec_time<count>(test_3);
    // auto ret12 = nth::exec_time<count>(test_1);
    // auto ret22 = nth::exec_time<count>(test_2);
    // auto ret32 = nth::exec_time<count>(test_3);
    // auto ret33 = nth::exec_time<count>(test_3);
    // auto ret23 = nth::exec_time<count>(test_2);
    // auto ret13 = nth::exec_time<count>(test_1);

    // printf("1) %ld\n", ret11);
    // printf("2) %ld\n", ret21);
    // printf("3) %ld\n", ret31);
    // printf("1) %ld\n", ret12);
    // printf("2) %ld\n", ret22);
    // printf("3) %ld\n", ret32);
    // printf("1) %ld\n", ret13);
    // printf("2) %ld\n", ret23);
    // printf("3) %ld\n", ret33);

    // std::string_view fmt = "+"; // +^+#08x
    // volatile bool ret = nth::formatter<int>::check(fmt);
    // printf("%d\n", ret);

    // auto str1 = std::format("| my view is: {}, {}, {}, {}, {}, {}, {} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // auto str2 = std::format("| my view is: {}, {}, {}, {}, {}, {}, {} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // auto str3 = std::format("| my view is: {}, {}, {}, {}, {}, {}, {} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // auto str4 = std::format("| my view is: {}, {}, {}, {}, {}, {}, {} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // auto str5 = std::format("| my view is: {}, {}, {}, {}, {}, {}, {} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // auto str6 = std::format("| my view is: {}, {}, {}, {}, {}, {} |\n", 42u, char('Z'), true, -1, -2l, "123");

    // printf("%s", str1.data());
    // printf("%s", str2.data());
    // printf("%s", str3.data());
    // printf("%s", str4.data());
    // printf("%s", str5.data());
    // printf("%s", str6.data());
    // std::cout << 
    //     str1 << 
    //     str2 << 
    //     str3 << 
    //     str4 << 
    //     str5 << 
    //     str6 << std::endl;
    // nth::v1::testing("1234");


    // "hello world, {}"_print(42);
    // print("hello world, {}"_f, 42);

    // nth::v1::print("| my view is: {1:5c^_}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:3}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');

    // using nth::operator+;

    // auto str1 = "string_literal"_sc;
    // auto str2 = "_test"_sl;

    // auto str3 = str1 + str2;
    // auto str4 = nth::join("str1", "str2");
    // auto str5 = nth::string_constant<"str1"_sl + "str2"_sl + "str3"_sl>::view();
    // auto str6 = "hello"_sl + "_"_sl + "world"_sl;
    // // auto str7 = str5.view();

    // nth::log_hex(str1.data(), str1.size());
    // printf("===============\n");
    // nth::log_hex(str2.data(), str2.size());
    // printf("===============\n");
    // nth::log_hex(str3.data(), str3.size());
    // printf("===============\n");
    // nth::log_hex(str4.data(), str4.size());
    // printf("===============\n");
    // nth::log_hex(str5.data(), str5.size());
    // printf("===============\n");
    // nth::log_hex(str6.data(), str6.size());
    // printf("===============\n");
    // nth::log_hex(str7.data(), str7.size());

    // nth::v1::print_sc("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');

    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n"_f, 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {} |\n"_f, 42u, char('Z'), true, -1, -2l, "123");
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {} |\n"_f, 42u, char('Z'), true, -1, -2l);
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:} |\n"_f, 42u, char('Z'), true, -1);
    // nth::v1::print("| my view is: {1:_^5c}, {0:}, {2:d} |\n"_f, 42u, char('Z'), true);
    // nth::v1::print("| my view is: {1:_^5c}, {0:} |\n"_f, 42u, char('Z'));
    // nth::v1::print("| my view is: {0:_^5c} |\n"_f, 42u);
    // nth::v1::print("| my view is: {0:_^5c} |\n"_f, 42u);
    // nth::v1::print("| my view is: {0:_^5c} |\n"_f, 42u);
    // nth::v1::print("| my view is: {0:_^5c} |\n"_f, 42u);
    // nth::v1::print("| my view is: {0:_^5c} |\n"_f, 42u);
    // nth::v1::print("| my view is: {0:_^5c} |\n"_f, 42u);
    // nth::v1::print("| my view is: {0:_^5c} |\n"_f, 42u);
    // nth::v1::print("| my view is: {0:_^5c} |\n"_f, 42u);

    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {}, {:#02x} |\n", 42u, char('Z'), true, -1, -2l, "123", 'b');
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {}, {} |\n", 42u, char('Z'), true, -1, -2l, "123");
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:}, {} |\n", 42u, char('Z'), true, -1, -2l);
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d}, {3:} |\n", 42u, char('Z'), true, -1);
    // nth::v2::print("| my view is: {1:_^5c}, {0:}, {2:d} |\n", 42u, char('Z'), true);
    // nth::v2::print("| my view is: {1:_^5c}, {0:} |\n", 42u, char('Z'));
    // nth::v2::print("| my view is: {0:_^5c} |\n", 42u);
    // nth::v2::print("| my view is: {0:_^5c} |\n", 42u);
    // nth::v2::print("| my view is: {0:_^5c} |\n", 42u);
    // nth::v2::print("| my view is: {0:_^5c} |\n", 42u);
    // nth::v2::print("| my view is: {0:_^5c} |\n", 42u);
    // nth::v2::print("| my view is: {0:_^5c} |\n", 42u);
    // nth::v2::print("| my view is: {0:_^5c} |\n", 42u);
    // nth::v2::print("| my view is: {0:_^5c} |\n", 42u);

    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s, %02x |\n", 42u, char('Z'), "true", -1, -2l, "123", 'b');
    // printf("| my view is: %c, %d, %s, %d, %d, %s |\n", 42u, char('Z'), "true", -1, -2l, "123");
    // printf("| my view is: %c, %d, %s, %d, %d |\n", 42u, char('Z'), "true", -1, -2l);
    // printf("| my view is: %c, %d, %s, %d |\n", 42u, char('Z'), "true", -1);
    // printf("| my view is: %c, %d, %d s\n", 42u, char('Z'), "true");
    // printf("| my view is: %c, %d |\n", 42u, char('Z'));
    // printf("| my view is: %c |\n", 42u);
    // printf("| my view is: %c |\n", 42u);
    // printf("| my view is: %c |\n", 42u);
    // printf("| my view is: %c |\n", 42u);
    // printf("| my view is: %c |\n", 42u);
    // printf("| my view is: %c |\n", 42u);
    // printf("| my view is: %c |\n", 42u);
    // printf("| my view is: %c |\n", 42u);
}
