#ifndef NTH_UTIL_EXPECTED_H
#define NTH_UTIL_EXPECTED_H

namespace nth {

/**
 * @brief Simplified version of C++23 std::expected.
 * Intentionally made non-modifiable and noexcept.
 * 
 * @tparam T Value type
 * @tparam E Error type
 */
template<class T, class E>
struct expected {

    // ANCHOR Member types

    using value_type = T;
    using error_type = E;
    using pointer = value_type*;
    using reference = value_type&;
    using const_pointer = const value_type*;
    using const_reference = const value_type&;

    // ANCHOR Constructors and access

    constexpr expected() = default;
    constexpr expected(value_type v) : v{v}, ok{true}       {}
    constexpr expected(error_type e) : e{e}, ok{false}      {}
    constexpr auto& value() const noexcept                  { return v; }
    constexpr auto& error() const noexcept                  { return e; }
    constexpr bool has_value() const noexcept               { return ok; }
    constexpr operator bool() const noexcept                { return ok; }
    constexpr const_reference operator*() const noexcept    { return v; }
    constexpr const_pointer operator->() const noexcept     { return &v; }
private:
    union {
        value_type v;
        error_type e;
    };
    bool ok = false;
};

}

#endif