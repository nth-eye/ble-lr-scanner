#ifndef NTH_UTIL_LITERAL_H
#define NTH_UTIL_LITERAL_H

#include "nth/util/meta.h"

namespace nth {
namespace imp {

/**
 * @brief Basic string literal wrapper to use it as template parameter.
 * 
 * @tparam T Buffer character type
 * @tparam N Buffer size to store literal
 * @tparam S Size of literal string, returned by size() function
 */
template<class T, size_t N, size_t S>
struct literal {
    constexpr static auto size()                            { return S; }
    constexpr auto operator==(const literal& rhs) const     { return std::equal(begin(), end(), rhs.begin()); }
    constexpr auto operator==(std::span<const T> rhs) const { return size() == rhs.size() && std::equal(begin(), end(), rhs.begin()); }
    constexpr auto data() noexcept                          { return buf; }
    constexpr auto data() const noexcept                    { return buf; }
    constexpr auto begin() noexcept                         { return buf; }
    constexpr auto begin() const noexcept                   { return buf; }
    constexpr auto end() noexcept                           { return buf + size(); }
    constexpr auto end() const noexcept                     { return buf + size(); }
    constexpr auto& operator[](size_t i) noexcept           { assert(i < N); return buf[i]; }
    constexpr auto& operator[](size_t i) const noexcept     { assert(i < N); return buf[i]; }
    constexpr auto& front() noexcept                        { return buf[0]; }
    constexpr auto& front() const noexcept                  { return buf[0]; }
    constexpr auto& back() noexcept                         { return buf[size() - 1]; }
    constexpr auto& back() const noexcept                   { return buf[size() - 1]; }
    T buf[N]{};
};

}

/**
 * @brief String literal wrapper to use it as template parameter.
 * Stores null-terminator, but doesn't count it in size().
 * 
 * @tparam N Deduced string literal length with null-terminator
 */
template<size_t N>
struct string_literal : imp::literal<char, N, N - 1> {
    constexpr string_literal() = default;
    constexpr string_literal(const char (&str)[N])  { std::copy_n(str, N, this->buf); }
    constexpr auto view() & noexcept                { return std::string_view{this->buf, this->size()}; }
    constexpr auto view() && = delete;
    constexpr auto view() const & noexcept          { return std::string_view{this->buf, this->size()}; }
};

/**
 * @brief String literal wrapper to convert it to nth::byte array 
 * at compile time and use it in constexpr context. Doesn't store 
 * null-terminator!
 * 
 * @tparam N Deduced string literal length with null-terminator
 */
template<size_t N>
struct byte_literal : imp::literal<byte, N - 1, N - 1> {
    constexpr byte_literal() = default;
    constexpr byte_literal(const char (&str)[N])    { std::copy_n(str, N - 1, this->buf); }
};

template<string_literal S>
struct string_constant {
    static constexpr auto cstr()        { return literal.data(); }
    static constexpr auto view()        { return literal.view(); }
    static constexpr auto literal = S;
};

/**
 * @brief Operator to concatenate two string_literal instances into a new 
 * string_literal with a combined size. The resulting string_literal's size 
 * is the sum of the sizes of the two operands, minus one for the overlapping 
 * null-terminator.
 * 
 * @tparam N Size of the first string_literal including the null-terminator.
 * @tparam M Size of the second string_literal including the null-terminator.
 * @param lhs The first string_literal operand.
 * @param rhs The second string_literal operand.
 * @return A new string_literal instance representing the concatenated string.
 */
template <size_t N, size_t M>
constexpr auto operator+(const string_literal<N>& lhs, const string_literal<M>& rhs) 
{
    string_literal<N + M - 1> ret;
    std::copy_n(lhs.data(), lhs.size(), ret.data());
    std::copy_n(rhs.data(), rhs.size(), ret.data() + lhs.size());
    return ret;
}

template<size_t... Sizes>
constexpr auto join(const string_literal<Sizes>&... strings) 
{
    string_literal<(strings.size() + ... + 0) + 1> ret;
    auto append = [i = 0, &ret] (const auto& s) mutable {
        for (auto c : s) 
            ret[i++] = c;
    };
    (append(strings), ...);
    return ret;
}

template<size_t... Lengths>
constexpr auto join(const char (&...strings)[Lengths])
{
    return join<Lengths...>(string_literal(strings)...);
}

namespace literals {

/**
 * @brief User-defined literal for creating string_literal instances.
 * 
 * @tparam S The string_literal to be created from the string literal.
 * @return A string_literal instance representing the given string.
 */
template<string_literal S>
constexpr auto operator"" _sl() 
{
    return S;
}

template <string_literal S>
constexpr auto operator""_sc()
{
    return string_constant<S>{};
}

}

}

#endif