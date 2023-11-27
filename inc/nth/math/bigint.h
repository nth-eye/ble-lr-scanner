#ifndef NTH_MATH_BIGINT_H
#define NTH_MATH_BIGINT_H

#define NTH_MATH_BIGINT_OPTIMIZATION_MUL_ZERO   false

#include "nth/util/bit.h"
#include "nth/util/string.h"

namespace nth {
namespace imp {

template<size_t Bits> 
struct int_by_bits;

template<> struct int_by_bits<8>    { using uint_t = uint8_t; };
template<> struct int_by_bits<16>   { using uint_t = uint16_t; };
template<> struct int_by_bits<32>   { using uint_t = uint32_t; };
template<> struct int_by_bits<64>   { using uint_t = uint64_t; };
#ifdef __SIZEOF_INT128__
template<> struct int_by_bits<128>  { using uint_t = __uint128_t; };
#endif

template<std::integral T>
using next_uint_t = int_by_bits<bit_size<T> * 2>::uint_t;

}

template <std::unsigned_integral T>
constexpr bool overflow_sum(T a, T b, bool carry) 
{
    T sum = a + b + carry;
    return (sum < a) || (sum < b);
}

template <std::unsigned_integral T>
constexpr bool overflow_sub(T a, T b, bool borrow) 
{
    return (a < b) || ((a - b) < borrow);
}

// template<std::unsigned_integral T, size_t N, size_t M>
// constexpr auto multiprecision_sum(std::span<const T, N> rhs, std::span<const T, M> lhs)
// {
//     std::array<T, N + 1> res = {};


// }

// template<std::unsigned_integral T, size_t N, size_t M>
// constexpr auto multiprecision_div(std::span<const T, N> rhs, std::span<const T, M> lhs)
// {
//     static_assert(N >= M && M);

//     constexpr quo_size = N - M;
//     constexpr rem_size = M;

//     std::array<T, quo_size> quo = {};
//     std::array<T, rem_size> rem;

//     for (int i = N - 1; i >= M; --i) {
//         if (rhs[i] == lhs.back()) {
//             quo[i - M - 2] = 
//         } else {
//             quo[i - M - 2] = 
//         }
//     }
// }

/**
 * @brief Big integer implementation intended for numbers up 
 * to ~500 digits. Uses naive long multiplication algorithm.
 * Number is stored in preallocated array of words of type T
 * on which all arithmetic is done. 
 * 
 * @tparam T Word type
 * @tparam N Number of bits
 */
template<std::unsigned_integral T, size_t N> 
struct bigint {

    static_assert(N % bit_size<T> == 0);

    // ANCHOR Member types

    using wider_type = imp::next_uint_t<T>;
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using universal_reference = value_type&&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr auto num_bits = bit_size<value_type>;
    static constexpr auto overflow_bit = bit<wider_type>(num_bits); 

    // ANCHOR Constructors

    constexpr bigint() noexcept = default;
    constexpr bigint(value_type x, size_type n = 1) noexcept
    {
        std::fill_n(buf, std::min(words, n), x);
    }
    constexpr bigint(std::span<const value_type> x) noexcept
    {
        std::copy_n(x.data(), std::min(words, x.size()), buf);
    }
    template<size_t Size>
    constexpr bigint(value_type(&&x)[Size]) noexcept
    {
        static_assert(Size <= words);
        std::copy_n(x, Size, buf);
    }
    constexpr bigint(std::string_view str) noexcept
    {
        if (str.starts_with("0x"))
            init_from_hex(str.substr(2));
        else
            init_from_dec(str);
    }

    // ANCHOR Capacity

    constexpr static size_type capacity()       { return words; }
    constexpr static size_type max_size()       { return words; }

    // ANCHOR Iterators

    constexpr iterator begin() noexcept                 { return buf; }
    constexpr const_iterator begin() const noexcept     { return buf; }
    constexpr const_iterator cbegin() const noexcept    { return begin(); }
    constexpr iterator end() noexcept                   { return buf + words; }
    constexpr const_iterator end() const noexcept       { return buf + words; }
    constexpr const_iterator cend() const noexcept      { return end(); }

    constexpr reverse_iterator rbegin() noexcept                { return std::reverse_iterator(buf + words); }
    constexpr const_reverse_iterator rbegin() const noexcept    { return std::reverse_iterator(buf + words); }
    constexpr const_reverse_iterator crbegin() const noexcept   { return rbegin(); }
    constexpr reverse_iterator rend() noexcept                  { return std::reverse_iterator(buf); }
    constexpr const_reverse_iterator rend() const noexcept      { return std::reverse_iterator(buf); }
    constexpr const_reverse_iterator crend() const noexcept     { return rend(); }

    // ANCHOR Access

    constexpr reference operator[](size_type i)             { assert(capacity() > i); return buf[i]; }
    constexpr const_reference operator[](size_type i) const { assert(capacity() > i); return buf[i]; }
    constexpr reference front()                             { return buf[0]; }
    constexpr const_reference front() const                 { return buf[0]; }
    constexpr reference back()                              { return buf[words - 1]; }
    constexpr const_reference back() const                  { return buf[words - 1]; }
    constexpr pointer data() noexcept                       { return buf; }
    constexpr const_pointer data() const noexcept           { return buf; }

    // ANCHOR Modifiers

    constexpr bool operator==(const bigint& rhs) const
    {
        return std::equal(begin(), end(), rhs.begin());
    }

    constexpr bool operator<(const bigint& rhs) const 
    {
        for (int i = capacity() - 1; i >= 0; --i) {
            if (buf[i] != rhs.buf[i])
                return buf[i] < rhs.buf[i];
        }
        return false;
    }

    constexpr bool operator>(const bigint& rhs) const 
    {
        return rhs < *this;
    }

    constexpr bool operator<=(const bigint& rhs) const
    {
        return !(*this > rhs);
    }

    constexpr bool operator>=(const bigint& rhs) const
    {
        return !(*this < rhs);
    }

    constexpr bigint operator+(const bigint& rhs) const
    {
        bigint res;
        add(*this, rhs, res);
        return res;
    }

    constexpr bigint operator-(const bigint& rhs) const
    {
        bigint res;
        sub(*this, rhs, res);
        return res;
    }

    constexpr bigint operator*(const bigint& rhs) const
    {
        bigint res;
        mul(*this, rhs, res);
        return res;
    }

    constexpr bigint operator/(const bigint& rhs) const
    {
        // TODO
        // auto ptr = std::find_if(rhs.begin(), rhs.end(), [](auto x) { return x != 0; })
        bigint ret;
        return ret;
    }

    constexpr bigint operator^(int) const
    {
        bigint res;
        square(*this, res);
        return res;
    }

    // ANCHOR Operations

    static constexpr void add(const bigint& lhs, const bigint& rhs, bigint& res)
    {
        wider_type carry = 0;
        for (size_type i = 0; i < capacity() - 1; ++i) {
            auto x = wider_type(lhs[i]) + 
                     wider_type(rhs[i]) + carry;
            res[i] = x;
            carry  = x >> num_bits;
        }
        res.back() = lhs.back() + rhs.back() + carry;
    }

    static constexpr void sub(const bigint& lhs, const bigint& rhs, bigint& res)
    {
        wider_type carry = 0;
        for (size_type i = 0; i < capacity() - 1; ++i) {
            auto x = wider_type(lhs[i]) - 
                     wider_type(rhs[i]) - carry;
            res[i] = x;
            carry  = x >> num_bits;
        }
        res.back() = lhs.back() - rhs.back() - carry;
    }

    static constexpr void mul(const bigint& lhs, const bigint& rhs, bigint& res)
    {
        std::array<value_type, capacity() * 2> tmp {};
        for (size_type i = 0; i < capacity(); ++i) {
#if (NTH_MATH_BIGINT_OPTIMIZATION_MUL_ZERO)
            if (lhs[i]) {
#endif
                wider_type carry = 0;
                for (size_type j = 0; j < capacity(); ++j) {
                    auto x = wider_type(lhs[i]) * 
                             wider_type(rhs[j]) + 
                             wider_type(tmp[i + j]) + carry;
                    carry = x >> num_bits;
                    tmp[i + j] = x;
                }
                tmp[i + capacity()] = carry;
#if (NTH_MATH_BIGINT_OPTIMIZATION_MUL_ZERO)
            }
#endif
        }
        std::copy(tmp.begin(), tmp.begin() + capacity(), res.data());
    }

    static constexpr void square(const bigint& lhs, bigint& res)
    {
        std::array<value_type, capacity() * 2> tmp {};
        for (size_type i = 0; i < capacity(); ++i) {
#if (NTH_MATH_BIGINT_OPTIMIZATION_MUL_ZERO)
            if (lhs[i]) {
#endif
                wider_type x = wider_type(tmp[i * 2]) +
                            wider_type(lhs[i]) * 
                            wider_type(lhs[i]);
                wider_type carry = x >> num_bits;
                tmp[i * 2] = x;
                for (size_type j = i + 1; j < capacity(); ++j) {
                    x = wider_type(tmp[i + j]) + carry +
                        wider_type(lhs[i]) *
                        wider_type(lhs[j]) * 2;
                    carry = x >> num_bits;
                    tmp[i + j] = x;
                }
                tmp[i + capacity()] = carry;
#if (NTH_MATH_BIGINT_OPTIMIZATION_MUL_ZERO)
            }
#endif
        }
        std::copy(tmp.begin(), tmp.begin() + capacity(), res.data());
    }

    static constexpr auto div(const bigint& lhs, const bigint& rhs)
    {
        // TODO
    }

    // ANCHOR Print

    void print() const
    {
        printf("0x");
        for (size_type i = capacity(); i;)
            printf("%0*x'", int(chars), buf[--i]);
        printf("\b \n");
    }
private:
    void init_from_hex(std::string_view str) 
    {
        if (str.empty() || str.size() > capacity() * chars)
            return;
        auto to_int = [this] (const char* head, const char* tail, size_type i) {
            T res = 0;
            for (; head != tail; ++head) {
                auto c = imp::ascii_map[int(*head)];
                if (c >= 16) {
                    std::fill(begin(), end(), T(0));
                    return false;
                }
                res = (res << 4) + c;
            }
            buf[i] = res;
            return true;
        };
        auto chunks = str.size() / chars;
        auto remain = str.size() % chars;
        auto ptr = str.end();

        for (size_t i = 0; i < chunks; ++i, ptr -= chars)
            if (to_int(ptr - chars, ptr, i) == false)
                return;

        if (remain)
            to_int(ptr - remain, ptr, chunks);
    }
    void init_from_dec(std::string_view str)
    {
        // *this = bigint{};
        // bigint digit;
        // bigint ten = 10;

        // for (size_t j = 0; j < str.size(); ++j) {
        //     if (is_digit(str[j])) {
        //         digit = str[j] - '0';
        //         *this = *this * ten + digit;
        //     }
        // }
    }
public:
    static constexpr auto words = words_in_bits<T>(N);
    static constexpr auto chars = sizeof(value_type) * 2;
public:
    T buf[words]{};
};

template<class T>
concept any_integral = std::integral<T> || requires(T x) {
    { bigint{x} } -> std::same_as<T>;
};

}

#endif

        // auto sum = T(0);
        // auto mul = T(1);
        // auto dst = 0u;
        // auto src = str.rbegin();
        // bool carry = false;
        // bool mul_overflow = false;

        // while (dst < capacity() && src != str.rend()) {
        //     int num = *src++ - '0';
        //     if (num < 0 || num > 10) {
        //         std::fill_n(buf, capacity(), value_type{0});
        //         break;
        //     }
        //     if (mul_overflow) {
        //         mul_overflow = false;
        //         buf[dst++] += num * mul * 10;
        //         mul = T(mul) / (int_bits_full<T> / 10);
        //     }
        //     printf("%d x %d \n", num, mul);

        //     auto sum = buf[dst] + num * mul + carry;
        //     carry = overflow_sum(buf[dst], value_type(num * mul), carry);
        //     buf[dst] = sum;
        //     mul_overflow = overflow_mul(T(mul), T(10));

        //     if (mul_overflow) {
        //         printf("mul_overflow \n");
        //     } else {
        //         mul *= 10;
        //     }
        // }