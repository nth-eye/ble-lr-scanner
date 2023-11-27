#ifndef DRAGONBOX_FMT_H
#define DRAGONBOX_FMT_H

#include <cstdint>
#include <cassert>

namespace dragonboxfmt {

template<class T>
struct ieee754_metadata {};

template<>
struct ieee754_metadata<float> {
    using uint = uint32_t;
    static constexpr int capacity_size = 32;
    static constexpr int mantissa_size = 23;
    static constexpr int exponent_size = 8;
    static constexpr int decimal_digits = 9;
    static constexpr int shorter_interval_tie_lower_threshold = -35;
    static constexpr int shorter_interval_tie_upper_threshold = -35;
    static constexpr int kappa = 1;
    static constexpr int big_divisor = 100;
    static constexpr int small_divisor = 10;
};

template<>
struct ieee754_metadata<double> {
    using uint = uint64_t;
    static constexpr int capacity_size = 64;
    static constexpr int mantissa_size = 52;
    static constexpr int exponent_size = 11;
    static constexpr int decimal_digits = 17;
    static constexpr int shorter_interval_tie_lower_threshold = -77;
    static constexpr int shorter_interval_tie_upper_threshold = -77;
    static constexpr int kappa = 2;
    static constexpr int big_divisor = 1000;
    static constexpr int small_divisor = 100;
};

template<std::floating_point T>
struct ieee754_traits {

    using meta = ieee754_metadata<T>;
    using uint = meta::uint;
    using type = T;

    static_assert(
        std::numeric_limits<T>::is_iec559   && 
        std::numeric_limits<T>::radix == 2  &&
        nth::bit_size<uint> == meta::capacity_size);

    static constexpr auto sign_mask     = nth::bit<uint>(meta::capacity_size - 1);
    static constexpr auto mantissa_mask = nth::bit_mask<uint>(meta::mantissa_size);
    static constexpr auto exponent_mask = nth::bit_mask<uint>(meta::exponent_size) << meta::mantissa_size;
    static constexpr auto exponent_bias = 1 - nth::bit<int>(meta::capacity_size - meta::mantissa_size - 2);
    static constexpr auto exponent_min  =  exponent_bias + 1;
    static constexpr auto exponent_max  = -exponent_bias;

    static constexpr auto to_integral(type f)       { return std::bit_cast<uint>(f); }
    static constexpr auto to_floating(uint u)       { return std::bit_cast<type>(u); }
    static constexpr bool is_negative(uint u)       { return u & sign_mask; }
    static constexpr bool is_infinite(uint u)       { return get_exponent_bits(u) == exponent_mask; }
    static constexpr bool has_even_mantissa(uint u) { return !(u & 1); }
    static constexpr uint get_mantissa_bits(uint u) { return u &  mantissa_mask; }
    static constexpr uint get_exponent_bits(uint u) { return u &  exponent_mask; }
    static constexpr uint clr_exponent_bits(uint u) { return u & ~exponent_mask; }

    static constexpr auto get_exponent(uint e) 
    {
        return e ? int(e >> meta::mantissa_size) + exponent_bias : exponent_min;
    }
};

template<class T> 
struct decimal_fp {
    ieee754_traits<T>::uint significand;
    int exponent;
};

struct uint128_t {
    constexpr uint128_t() noexcept {}
    constexpr uint128_t(uint64_t hi, uint64_t lo) noexcept : upper{hi}, lower{lo} {}
#if __SIZEOF_INT128__
    constexpr uint128_t(__uint128_t full) noexcept : full{full} {}
#endif
    constexpr auto hi() const noexcept { return upper; }
    constexpr auto lo() const noexcept { return lower; }
    constexpr static uint128_t mul(uint64_t x, uint64_t y) noexcept
    {
#if __SIZEOF_INT128__
        return __uint128_t(x) * __uint128_t(y);
#else
        constexpr uint64_t mask = nth::int_bits_full<uint32_t>;

        uint64_t a = x >> 32;
        uint64_t b = x & mask;
        uint64_t c = y >> 32;
        uint64_t d = y & mask;

        uint64_t ac = a * c;
        uint64_t bc = b * c;
        uint64_t ad = a * d;
        uint64_t bd = b * d;

        uint64_t intermediate = (bd >> 32) + (ad & mask) + (bc & mask);

        return {ac + (intermediate >> 32) + (ad >> 32) + (bc >> 32), (intermediate << 32) + (bd & mask)};
#endif  
    }
    constexpr auto& operator+=(uint64_t n) noexcept 
    {
#ifdef __SIZEOF_INT128__
        full += __uint128_t(n);
#else
        uint64_t sum = lower + n;
        upper += sum < lower ? 1 : 0;
        lower  = sum;
#endif
        return *this;
    }
private:
    union {
#ifdef __SIZEOF_INT128__
        __uint128_t full;
#endif
        struct {
            uint64_t lower;
            uint64_t upper;
        };
    };
};

constexpr uint64_t umul128_upper64(uint64_t x, uint64_t y) noexcept 
{
    return uint128_t::mul(x, y).hi();
}

constexpr uint64_t umul96_upper64(uint32_t x, uint64_t y) noexcept 
{
    return umul128_upper64(uint64_t(x) << 32, y);
}

constexpr uint128_t umul192_lower128(uint64_t x, uint128_t y) noexcept 
{
    const auto high = x * y.hi();
    const auto high_low = uint128_t::mul(x, y.lo());
    return {high + high_low.hi(), high_low.lo()};
}

constexpr uint128_t umul192_upper128(uint64_t x, uint128_t y) noexcept 
{
    uint128_t r = uint128_t::mul(x, y.hi());
    r += umul128_upper64(x, y.lo());
    return r;
}

constexpr uint64_t umul96_lower64(uint32_t x, uint64_t y) noexcept 
{
    return x * y;
}

constexpr int floor_log10_pow2(int e) noexcept 
{
    assert(e <=  2620);
    assert(e >= -2620);
    return (e * 315653) >> 20;
}

constexpr int floor_log2_pow10(int e) noexcept 
{
    assert(e <=  1233);
    assert(e >= -1233);
    return (e * 1741647) >> 19;
}

constexpr int floor_log10_pow2_minus_log10_4_over_3(int e) noexcept 
{
    assert(e <=  2936);
    assert(e >= -2985);
    return (e * 631305 - 261663) >> 21;
}

inline constexpr struct {
    uint32_t divisor;
    int shift_amount;
} div_small_pow10_infos[] = {{10, 16}, {100, 16}};

template<class T> 
struct cache_accessor;

template <> 
struct cache_accessor<float> {

    using uint = ieee754_traits<float>::uint;
    using cache_entry_type = uint64_t;

    static constexpr int mantissa_bits = ieee754_metadata<float>::mantissa_size;
    static constexpr int min_k = -31;
    static constexpr int max_k = 46;
    static constexpr uint64_t pow10_significands[] = {
        0x81ceb32c4b43fcf5, 0xa2425ff75e14fc32, 0xcad2f7f5359a3b3f,
        0xfd87b5f28300ca0e, 0x9e74d1b791e07e49, 0xc612062576589ddb,
        0xf79687aed3eec552, 0x9abe14cd44753b53, 0xc16d9a0095928a28,
        0xf1c90080baf72cb2, 0x971da05074da7bef, 0xbce5086492111aeb,
        0xec1e4a7db69561a6, 0x9392ee8e921d5d08, 0xb877aa3236a4b44a,
        0xe69594bec44de15c, 0x901d7cf73ab0acda, 0xb424dc35095cd810,
        0xe12e13424bb40e14, 0x8cbccc096f5088cc, 0xafebff0bcb24aaff,
        0xdbe6fecebdedd5bf, 0x89705f4136b4a598, 0xabcc77118461cefd,
        0xd6bf94d5e57a42bd, 0x8637bd05af6c69b6, 0xa7c5ac471b478424,
        0xd1b71758e219652c, 0x83126e978d4fdf3c, 0xa3d70a3d70a3d70b,
        0xcccccccccccccccd, 0x8000000000000000, 0xa000000000000000,
        0xc800000000000000, 0xfa00000000000000, 0x9c40000000000000,
        0xc350000000000000, 0xf424000000000000, 0x9896800000000000,
        0xbebc200000000000, 0xee6b280000000000, 0x9502f90000000000,
        0xba43b74000000000, 0xe8d4a51000000000, 0x9184e72a00000000,
        0xb5e620f480000000, 0xe35fa931a0000000, 0x8e1bc9bf04000000,
        0xb1a2bc2ec5000000, 0xde0b6b3a76400000, 0x8ac7230489e80000,
        0xad78ebc5ac620000, 0xd8d726b7177a8000, 0x878678326eac9000,
        0xa968163f0a57b400, 0xd3c21bcecceda100, 0x84595161401484a0,
        0xa56fa5b99019a5c8, 0xcecb8f27f4200f3a, 0x813f3978f8940985,
        0xa18f07d736b90be6, 0xc9f2c9cd04674edf, 0xfc6f7c4045812297,
        0x9dc5ada82b70b59e, 0xc5371912364ce306, 0xf684df56c3e01bc7,
        0x9a130b963a6c115d, 0xc097ce7bc90715b4, 0xf0bdc21abb48db21,
        0x96769950b50d88f5, 0xbc143fa4e250eb32, 0xeb194f8e1ae525fe,
        0x92efd1b8d0cf37bf, 0xb7abc627050305ae, 0xe596b7b0c643c71a,
        0x8f7e32ce7bea5c70, 0xb35dbf821ae4f38c, 0xe0352f62a19e306f
    };

    struct compute_mul_result {
        uint result;
        bool is_integer;
    };

    struct compute_mul_parity_result {
        bool parity;
        bool is_integer;
    };

    static constexpr uint64_t get_cached_power(int k) noexcept 
    {
        assert(k >= min_k);
        assert(k <= max_k);
        return pow10_significands[k - min_k];
    }

    static constexpr compute_mul_result compute_mul(uint u, const cache_entry_type& cache) noexcept 
    {
        auto r = umul96_upper64(u, cache);
        return {uint(r >> 32),
                uint(r) == 0};
    }

    static constexpr uint32_t compute_delta(const cache_entry_type& cache, int beta) noexcept 
    {
        return uint32_t(cache >> (64 - 1 - beta));
    }

    static constexpr compute_mul_parity_result compute_mul_parity(uint two_f, const cache_entry_type& cache, int beta) noexcept 
    {
        assert(beta >= 1);
        assert(beta < 64);

        auto r = umul96_lower64(two_f, cache);
        return {((r >> (64 - beta)) & 1) != 0, static_cast<uint32_t>(r >> (32 - beta)) == 0};
    }

    static constexpr uint compute_left_endpoint_for_shorter_interval_case(const cache_entry_type& cache, int beta) noexcept 
    {
        return uint((cache - (cache >> (mantissa_bits + 2))) >> (64 - mantissa_bits - 1 - beta));
    }

    static constexpr uint compute_right_endpoint_for_shorter_interval_case(const cache_entry_type& cache, int beta) noexcept 
    {
        return uint((cache + (cache >> (mantissa_bits + 1))) >> (64 - mantissa_bits - 1 - beta));
    }

    static constexpr uint compute_round_up_for_shorter_interval_case(const cache_entry_type& cache, int beta) noexcept 
    {
        return (uint(cache >> (64 - mantissa_bits - 2 - beta)) + 1) / 2;
    }
};

template <> 
struct cache_accessor<double> 
{
    using uint = ieee754_traits<double>::uint;
    using cache_entry_type = uint128_t;

    static constexpr int compression_ratio = 27;
    static constexpr int mantissa_bits = ieee754_metadata<double>::mantissa_size;
    static constexpr int min_k = -292;
    static constexpr int max_k = 326;
    static constexpr uint128_t pow10_significands[] = {
        {0xff77b1fcbebcdc4f, 0x25e8e89c13bb0f7b},
        {0xce5d73ff402d98e3, 0xfb0a3d212dc81290},
        {0xa6b34ad8c9dfc06f, 0xf42faa48c0ea481f},
        {0x86a8d39ef77164bc, 0xae5dff9c02033198},
        {0xd98ddaee19068c76, 0x3badd624dd9b0958},
        {0xafbd2350644eeacf, 0xe5d1929ef90898fb},
        {0x8df5efabc5979c8f, 0xca8d3ffa1ef463c2},
        {0xe55990879ddcaabd, 0xcc420a6a101d0516},
        {0xb94470938fa89bce, 0xf808e40e8d5b3e6a},
        {0x95a8637627989aad, 0xdde7001379a44aa9},
        {0xf1c90080baf72cb1, 0x5324c68b12dd6339},
        {0xc350000000000000, 0x0000000000000000},
        {0x9dc5ada82b70b59d, 0xf020000000000000},
        {0xfee50b7025c36a08, 0x02f236d04753d5b5},
        {0xcde6fd5e09abcf26, 0xed4c0226b55e6f87},
        {0xa6539930bf6bff45, 0x84db8346b786151d},
        {0x865b86925b9bc5c2, 0x0b8a2392ba45a9b3},
        {0xd910f7ff28069da4, 0x1b2ba1518094da05},
        {0xaf58416654a6babb, 0x387ac8d1970027b3},
        {0x8da471a9de737e24, 0x5ceaecfed289e5d3},
        {0xe4d5e82392a40515, 0x0fabaf3feaa5334b},
        {0xb8da1662e7b00a17, 0x3d6a751f3b936244},
        {0x95527a5202df0ccb, 0x0f37801e0c43ebc9},
        {0xf13e34aabb430a15, 0x647726b9e7c68ff0},
    };
    static constexpr uint64_t powers_of_5_64[] = {
        0x0000000000000001, 0x0000000000000005, 0x0000000000000019,
        0x000000000000007d, 0x0000000000000271, 0x0000000000000c35,
        0x0000000000003d09, 0x000000000001312d, 0x000000000005f5e1,
        0x00000000001dcd65, 0x00000000009502f9, 0x0000000002e90edd,
        0x000000000e8d4a51, 0x0000000048c27395, 0x000000016bcc41e9,
        0x000000071afd498d, 0x0000002386f26fc1, 0x000000b1a2bc2ec5,
        0x000003782dace9d9, 0x00001158e460913d, 0x000056bc75e2d631,
        0x0001b1ae4d6e2ef5, 0x000878678326eac9, 0x002a5a058fc295ed,
        0x00d3c21bcecceda1, 0x0422ca8b0a00a425, 0x14adf4b7320334b9
    };

    static constexpr uint128_t get_cached_power(int k) noexcept 
    {
        assert(k >= min_k);
        assert(k <= max_k);

        int cache_index = (k - min_k) / compression_ratio;
        int kb          = cache_index * compression_ratio + min_k;
        int offset      = k - kb;
        auto base_cache = pow10_significands[cache_index];

        if (offset == 0) 
            return base_cache;

        int alpha = floor_log2_pow10(kb + offset) - floor_log2_pow10(kb) - offset;

        assert(alpha > 0);
        assert(alpha < 64);

        uint64_t pow5               = powers_of_5_64[offset];
        uint128_t recovered_cache   = uint128_t::mul(base_cache.hi(), pow5);
        uint128_t middle_low        = uint128_t::mul(base_cache.lo(), pow5);

        recovered_cache += middle_low.hi();

        uint64_t high_to_middle = recovered_cache.hi() << (64 - alpha);
        uint64_t middle_to_low  = recovered_cache.lo() << (64 - alpha);

        recovered_cache = uint128_t{
            (recovered_cache.lo() >> alpha) | high_to_middle,
            (     middle_low.lo() >> alpha) | middle_to_low };

        assert(recovered_cache.lo() + 1 != 0);
        return {
            recovered_cache.hi(), 
            recovered_cache.lo() + 1};
    }

    struct compute_mul_result {
        uint result;
        bool is_integer;
    };

    struct compute_mul_parity_result {
        bool parity;
        bool is_integer;
    };

    static constexpr compute_mul_result compute_mul(uint u, const cache_entry_type& cache) noexcept 
    {
        auto r = umul192_upper128(u, cache);
        return {r.hi(), 
                r.lo() == 0};
    }

    static constexpr uint32_t compute_delta(cache_entry_type const& cache, int beta) noexcept 
    {
        return uint32_t(cache.hi() >> (64 - 1 - beta));
    }

    static constexpr compute_mul_parity_result compute_mul_parity(uint two_f, const cache_entry_type& cache, int beta) noexcept 
    {
        assert(beta >= 1);
        assert(beta < 64);

        auto r = umul192_lower128(two_f, cache);
        return {((r.hi() >> (64 - beta)) & 1) != 0,
                ((r.hi() << beta) | (r.lo() >> (64 - beta))) == 0};
    }

    static constexpr uint compute_left_endpoint_for_shorter_interval_case(const cache_entry_type& cache, int beta) noexcept 
    {
        return uint(cache.hi() - (cache.hi() >> (mantissa_bits + 2))) >> (64 - mantissa_bits - 1 - beta);
    }

    static constexpr uint compute_right_endpoint_for_shorter_interval_case(const cache_entry_type& cache, int beta) noexcept 
    {
        return uint(cache.hi() + (cache.hi() >> (mantissa_bits + 1))) >> (64 - mantissa_bits - 1 - beta);
    }

    static constexpr uint compute_round_up_for_shorter_interval_case(const cache_entry_type& cache, int beta) noexcept 
    {
        return uint((cache.hi() >> (64 - mantissa_bits - 2 - beta)) + 1) / 2;
    }
};

constexpr uint128_t get_cached_power(int k) noexcept 
{
    return cache_accessor<double>::get_cached_power(k);
}

constexpr uint32_t divide_by_10_to_kappa_plus_1(uint32_t n) noexcept 
{
    return uint32_t((uint64_t(n) * 1374389535) >> 37); // 1374389535 = ceil(2^37/100)
}

constexpr uint64_t divide_by_10_to_kappa_plus_1(uint64_t n) noexcept 
{
    return umul128_upper64(n, 2361183241434822607ull) >> 7; // 2361183241434822607 = ceil(2^(64+7)/1000)
}

template <typename T>
constexpr bool is_left_endpoint_integer_shorter_interval(int exponent) noexcept 
{
    constexpr int case_shorter_interval_left_endpoint_lower_threshold = 2;
    constexpr int case_shorter_interval_left_endpoint_upper_threshold = 3;
    return exponent >= case_shorter_interval_left_endpoint_lower_threshold &&
           exponent <= case_shorter_interval_left_endpoint_upper_threshold;
}

// Replaces n by floor(n / pow(10, N)) returning true if and only if n is
// divisible by pow(10, N).
// Precondition: n <= pow(10, N + 1).
template <int N>
constexpr bool check_divisibility_and_divide_by_pow10(uint32_t& n) noexcept 
{
    // The numbers below are chosen such that:
    //   1. floor(n/d) = floor(nm / 2^k) where d=10 or d=100,
    //   2. nm mod 2^k < m if and only if n is divisible by d,
    // where m is magic_number, k is shift_amount
    // and d is divisor.
    //
    // Item 1 is a common technique of replacing division by a constant with
    // multiplication, see e.g. "Division by Invariant Integers Using
    // Multiplication" by Granlund and Montgomery (1994). magic_number (m) is set
    // to ceil(2^k/d) for large enough k.
    // The idea for item 2 originates from Schubfach.
    constexpr auto info = div_small_pow10_infos[N - 1];
    assert(n <= info.divisor * 10);
    constexpr uint32_t magic_number = (1u << info.shift_amount) / info.divisor + 1;
    constexpr uint32_t comparison_mask = (1u << info.shift_amount) - 1;
    n *= magic_number;
    bool result = (n & comparison_mask) < magic_number;
    n >>= info.shift_amount;
    return result;
}

// Remove trailing zeros from n and return the number of zeros removed (float)
constexpr int remove_trailing_zeros(uint32_t& n, int s = 0) noexcept 
{
    assert(n != 0);
    // Modular inverse of 5 (mod 2^32): (mod_inv_5 * 5) mod 2^32 = 1.
    constexpr uint32_t mod_inv_5 = 0xcccccccd;
    constexpr uint32_t mod_inv_25 = 0xc28f5c29; // = mod_inv_5 * mod_inv_5

    while (true) {
        auto q = nth::ror(n * mod_inv_25, 2);
        if (q > nth::int_bits_full<uint32_t> / 100) 
            break;
        n = q;
        s += 2;
    }
    auto q = nth::ror(n * mod_inv_5, 1);
    if (q <= nth::int_bits_full<uint32_t> / 10) {
        n = q;
        s |= 1;
    }
    return s;
}

// Removes trailing zeros and returns the number of zeros removed (double)
constexpr int remove_trailing_zeros(uint64_t& n) noexcept 
{
    assert(n != 0);

    // This magic number is ceil(2^90 / 10^8).
    constexpr uint64_t magic_number = 12379400392853802749ull;
    auto nm = uint128_t::mul(n, magic_number);

    // Is n is divisible by 10^8?
    if ((nm.hi() & ((1ull << (90 - 64)) - 1)) == 0 && nm.lo() < magic_number) {
        // If yes, work with the quotient...
        auto n32 = uint32_t(nm.hi() >> (90 - 64));
        // ... and use the 32 bit variant of the function
        int s = remove_trailing_zeros(n32, 8);
        n = n32;
        return s;
    }
    // If n is not divisible by 10^8, work with n itself.
    constexpr uint64_t mod_inv_5 = 0xcccccccccccccccd;
    constexpr uint64_t mod_inv_25 = 0x8f5c28f5c28f5c29; // = mod_inv_5 * mod_inv_5

    int s = 0;
    while (true) {
        auto q = nth::ror(n * mod_inv_25, 2);
        if (q > nth::int_bits_full<uint64_t> / 100) 
            break;
        n = q;
        s += 2;
    }
    auto q = nth::ror(n * mod_inv_5, 1);
    if (q <= nth::int_bits_full<uint64_t> / 10) {
        n = q;
        s |= 1;
    }
    return s;
}

// The main algorithm for shorter interval case
template<class T>
constexpr decimal_fp<T> shorter_interval_case(int exponent) noexcept 
{
    decimal_fp<T> ret_value;
    // Compute k and beta
    const int minus_k = floor_log10_pow2_minus_log10_4_over_3(exponent);
    const int beta = exponent + floor_log2_pow10(-minus_k);

    // Compute xi and zi
    using cache_entry_type = typename cache_accessor<T>::cache_entry_type;
    const cache_entry_type cache = cache_accessor<T>::get_cached_power(-minus_k);

    auto xi = cache_accessor<T>::compute_left_endpoint_for_shorter_interval_case(cache, beta);
    auto zi = cache_accessor<T>::compute_right_endpoint_for_shorter_interval_case(cache, beta);

    // If the left endpoint is not an integer, increase it
    if (!is_left_endpoint_integer_shorter_interval<T>(exponent)) 
        ++xi;

    // Try bigger divisor
    ret_value.significand = zi / 10;

    // If succeed, remove trailing zeros if necessary and return
    if (ret_value.significand * 10 >= xi) {
        ret_value.exponent = minus_k + 1;
        ret_value.exponent += remove_trailing_zeros(ret_value.significand);
        return ret_value;
    }

    // Otherwise, compute the round-up of y
    ret_value.significand = cache_accessor<T>::compute_round_up_for_shorter_interval_case(cache, beta);
    ret_value.exponent = minus_k;

    // When tie occurs, choose one of them according to the rule
    if (exponent >= ieee754_metadata<T>::shorter_interval_tie_lower_threshold &&
        exponent <= ieee754_metadata<T>::shorter_interval_tie_upper_threshold) 
    {
        ret_value.significand = ret_value.significand % 2 == 0
                                    ? ret_value.significand
                                    : ret_value.significand - 1;
    } else if (ret_value.significand < xi) {
        ++ret_value.significand;
    }
    return ret_value;
}

template<std::floating_point T> 
constexpr decimal_fp<T> to_decimal(T x) noexcept 
{
    // Step 1: integer promotion & Schubfach multiplier calculation.

    using carrier_uint = typename ieee754_traits<T>::uint;
    using cache_entry_type = typename cache_accessor<T>::cache_entry_type;
    auto br = std::bit_cast<carrier_uint>(x);

    // Extract significand bits and exponent bits.
    const carrier_uint significand_mask = (carrier_uint(1) << ieee754_metadata<T>::mantissa_size) - 1;
    carrier_uint significand = (br & significand_mask);

    int exponent = (br & ieee754_traits<T>::exponent_mask) >> ieee754_metadata<T>::mantissa_size;

    if (exponent != 0) {  // Check if normal.
        exponent -= ieee754_traits<T>::exponent_bias + ieee754_metadata<T>::mantissa_size;

        // Shorter interval case; proceed like Schubfach.
        // In fact, when exponent == 1 and significand == 0, the interval is
        // regular. However, it can be shown that the end-results are anyway same.
        if (significand == 0) 
            return shorter_interval_case<T>(exponent);

        significand |= (carrier_uint(1) << ieee754_metadata<T>::mantissa_size);
    } else {
        // Subnormal case; the interval is always regular.
        if (significand == 0) 
            return {0, 0};
        exponent = std::numeric_limits<T>::min_exponent - ieee754_metadata<T>::mantissa_size - 1;
    }

    const bool include_left_endpoint = (significand % 2 == 0);
    const bool include_right_endpoint = include_left_endpoint;

    // Compute k and beta.
    const int minus_k = floor_log10_pow2(exponent) - ieee754_metadata<T>::kappa;
    const int beta = exponent + floor_log2_pow10(-minus_k);
    const cache_entry_type cache = cache_accessor<T>::get_cached_power(-minus_k);

    // Compute zi and deltai.
    // 10^kappa <= deltai < 10^(kappa + 1)
    const uint32_t deltai = cache_accessor<T>::compute_delta(cache, beta);
    const carrier_uint two_fc = significand << 1;

    // For the case of binary32, the result of integer check is not correct for
    // 29711844 * 2^-82
    // = 6.1442653300000000008655037797566933477355632930994033813476... * 10^-18
    // and 29711844 * 2^-81
    // = 1.2288530660000000001731007559513386695471126586198806762695... * 10^-17,
    // and they are the unique counterexamples. However, since 29711844 is even,
    // this does not cause any problem for the endpoints calculations; it can only
    // cause a problem when we need to perform integer check for the center.
    // Fortunately, with these inputs, that branch is never executed, so we are
    // fine.
    const typename cache_accessor<T>::compute_mul_result z_mul =
        cache_accessor<T>::compute_mul((two_fc | 1) << beta, cache);

    // Step 2: Try larger divisor; remove trailing zeros if necessary.

    // Using an upper bound on zi, we might be able to optimize the division
    // better than the compiler; we are computing zi / big_divisor here.
    decimal_fp<T> ret_value = {
        .significand    = divide_by_10_to_kappa_plus_1(z_mul.result),
        .exponent       = 0,
    };
    auto r = uint32_t(z_mul.result - ieee754_metadata<T>::big_divisor * ret_value.significand);

    auto small_divisor_case = [&] () 
    {
        // Step 3: Find the significand with the smaller divisor.

        ret_value.significand *= 10;
        ret_value.exponent = minus_k + ieee754_metadata<T>::kappa;

        uint32_t dist = r - (deltai / 2) + (ieee754_metadata<T>::small_divisor / 2);
        const bool approx_y_parity = ((dist ^ (ieee754_metadata<T>::small_divisor / 2)) & 1) != 0;
        const bool divisible_by_small_divisor = check_divisibility_and_divide_by_pow10<ieee754_metadata<T>::kappa>(dist);

        // Add dist / 10^kappa to the significand.
        ret_value.significand += dist;

        if (!divisible_by_small_divisor) 
            return ret_value;

        // Check z^(f) >= epsilon^(f).
        // We have either yi == zi - epsiloni or yi == (zi - epsiloni) - 1,
        // where yi == zi - epsiloni if and only if z^(f) >= epsilon^(f).
        // Since there are only 2 possibilities, we only need to care about the
        // parity. Also, zi and r should have the same parity since the divisor
        // is an even number.
        const auto y_mul = cache_accessor<T>::compute_mul_parity(two_fc, cache, beta);

        // If z^(f) >= epsilon^(f), we might have a tie when z^(f) == epsilon^(f),
        // or equivalently, when y is an integer.
        if (y_mul.parity != approx_y_parity)
            --ret_value.significand;
        else if (y_mul.is_integer & (ret_value.significand % 2 != 0))
            --ret_value.significand;
        return ret_value;
    };

    if (r < deltai) {
        // Exclude the right endpoint if necessary.
        if (r == 0 && (z_mul.is_integer & !include_right_endpoint)) {
            --ret_value.significand;
            r = ieee754_metadata<T>::big_divisor;
            return small_divisor_case();
        }
    } else if (r > deltai) {
        return small_divisor_case();
    } else {
        // r == deltai; compare fractional parts.
        const typename cache_accessor<T>::compute_mul_parity_result x_mul =
            cache_accessor<T>::compute_mul_parity(two_fc - 1, cache, beta);

        if (!(x_mul.parity | (x_mul.is_integer & include_left_endpoint)))
            return small_divisor_case();
    }
    ret_value.exponent = minus_k + ieee754_metadata<T>::kappa + 1;
    ret_value.exponent += remove_trailing_zeros(ret_value.significand); // We may need to remove trailing zeros.
    return ret_value;
}

}

#endif