#ifndef NTH_UTIL_BITSLIDE_H
#define NTH_UTIL_BITSLIDE_H

#include "nth/util/bit.h"

#define NTH_UTIL_BITSLIDE_EXT   false
#define NTH_UTIL_BITSLIDE_ARR   false

namespace nth {

/**
 * @brief Bit window which stores: 1) value of last received sequence 
 * number and 2) bitmask which holds indexes of previously received 
 * numbers within window of sizeof(Word) * 8 bits. New number which 
 * is greater than the latest shifts the window. Numbers outside the 
 * window (too old) and duplicates of already present numbers can be 
 * checked. Basicaly an implementation of anti-replay window. Initial
 * value and everything smaller is considered as already passed, so 0 
 * cannot be checked as a valid sequence number. The latest value 
 * itself takes a bit within bitmask, so the actual number of valid 
 * values smaller than latest is bit_size<Word> - 1, e.g. 31 if Word 
 * is uint32_t. 
 * 
 * @tparam Seqn Sequence number type
 * @tparam Word Bit window type, determines window size
 */
template<std::unsigned_integral Seqn, std::unsigned_integral Word>
struct bitslide {
    static constexpr Seqn bits = bit_size<Word>;
public:
    constexpr bitslide() = default;
    constexpr bitslide(Seqn seqn) : high{seqn} {}
public:
    constexpr Seqn latest() const
    {
        return high;
    }
    constexpr bool check(Seqn seqn) const
    {
        if (seqn > high)
            return true;
#if (NTH_UTIL_BITSLIDE_EXT)
        if (high - seqn - 1 >= bits)
            return false;
        return !get_bit(mask, high - seqn - 1);
#else
        if (high - seqn >= bits)
            return false;
        return !get_bit(mask, high - seqn);
#endif
    }
    constexpr void update(Seqn seqn)
    {
        if (seqn > high)
            update_latest(seqn);
#if (NTH_UTIL_BITSLIDE_EXT)
        else if (high - seqn - 1 < bits)
            set_bit(mask, high - seqn - 1);
#else
        else if (high - seqn < bits)
            set_bit(mask, high - seqn);
#endif
    }
    constexpr bool check_and_update(Seqn seqn)
    {
        if (seqn > high) {
            update_latest(seqn);
            return true;
        }
#if (NTH_UTIL_BITSLIDE_EXT)
        auto idx = high - seqn - 1;
#else
        auto idx = high - seqn;
#endif
        if ((idx < bits) &&
            get_bit(mask, idx) == false) {
            set_bit(mask, idx);
            return true;
        }
        return false;
    }
private:
    constexpr void update_latest(Seqn seqn)
    {
#if (NTH_UTIL_BITSLIDE_EXT)
        auto diff = seqn - high;
        mask = diff < bits ? bit<Seqn>(diff - 1) | (mask << diff) : 0;
#else
        auto diff = seqn - high;
        mask = 1 | (diff < bits ? mask << diff : 0);
#endif
        high = seqn;
    }
private:
    Seqn high = 0;
#if (NTH_UTIL_BITSLIDE_EXT)
    Word mask = -1;
#else
    Word mask = -1;
#endif
};

#if (NTH_UTIL_BITSLIDE_ARR)
/**
 * @brief Same as regular bitslide, but with an array of 
 * words as underlying bitmask storage another update logic.
 * Implementation of RFC-6479 anti-replay window without bit shifting: https://datatracker.ietf.org/doc/html/rfc6479
 * 
 * @tparam Seqn Sequence number type
 * @tparam Word Storage word type
 * @tparam Bits Window size, power of 2
 */
template<std::unsigned_integral Seqn, std::unsigned_integral Word, size_t Bits>
struct bitslide {
    static_assert(is_pow2(bit_size<Word>));
    static_assert(is_pow2(Bits));
    static constexpr auto slide = Bits - bit_size<Word>;
    static constexpr auto words = Bits / bit_size<Word>;
    static constexpr auto imask = words - 1;
    static constexpr auto shift = int_bits_log2<Word>;
    static constexpr auto bmask = bit_mask(shift);
public:
    constexpr bitslide() = default;
    constexpr bitslide(Seqn seqn) : high{seqn} {}
public:
    constexpr Seqn latest() const
    {
        return high;
    }
    constexpr bool check(Seqn seqn) const
    {
        if (seqn > high)
            return true;
        if (seqn == high)
            return false;
        if (high - seqn > slide) // NOTE: 32 bits window BEFORE latest (not including)
            return false;
        auto idx = (seqn >> shift) & imask;
        auto bit =  seqn           & bmask;
        return !get_bit(buff[idx], bit);
    }
    constexpr void update(Seqn seqn)
    {
        // TODO
    }
    constexpr bool check_and_update(Seqn seqn)
    {
        // TODO
    }
private:
    Seqn high = 0;
    Word buff[words] = {};
};
#endif

}

#undef NTH_UTIL_BITSLIDE_EXT
#undef NTH_UTIL_BITSLIDE_ARR

#endif
