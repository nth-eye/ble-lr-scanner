#ifndef SHOC_HASH_SHA1_H
#define SHOC_HASH_SHA1_H

#include "shoc/hash/md4.h"

namespace shoc {

struct sha1 : consumer<sha1, 20> {
    static constexpr size_t state_size = 5;     // In words
    static constexpr size_t block_size = 64;    // In bytes
private:
    static constexpr size_t pad_start = block_size - 8;
public:
    constexpr void init();
    constexpr void feed(span_i<> in);
    constexpr void stop(span_o<hash_size> out);
    constexpr void wipe();
private:
    constexpr void pad();
    constexpr void step();
private:
    using word = uint32_t;
    template<size_t start, size_t end, word k, decltype(ch<word>) Fn>
    static constexpr void round(word(&w)[80], word (&var)[state_size])
    {
        enum { a, b, c, d, e };
        for (size_t t = start; t < end; ++t) {
            word tmp = rol(var[a], 5) + var[e] + w[t] + k + Fn(var[b], var[c], var[d]);
            var[e] = var[d];
            var[d] = var[c]; 
            var[c] = rol(var[b], 30);
            var[b] = var[a];
            var[a] = tmp;
        }
    }
private:
    word length_low;
    word length_high;
    word state[state_size];
    byte block[block_size];
    byte block_idx;
};

constexpr void sha1::init()
{
    state[0] = 0x67452301;
    state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;
    state[3] = 0x10325476;
    state[4] = 0xc3d2e1f0;

    block_idx = length_high = length_low = 0;
}

constexpr void sha1::feed(span_i<> in)
{
    for (auto it : in) {
        block[block_idx++] = it;
        if ((length_low += 8) == 0)
            length_high += 1;
        if (block_idx == block_size)
            step();
    }
}

constexpr void sha1::stop(span_o<hash_size> out)
{
    pad();
    for (size_t i = 0; i < state_size; ++i)
        putbe<word>(state[i], &out[i * sizeof(word)]);
    wipe();
}

constexpr void sha1::wipe()
{
    length_low = 0;
    length_high = 0;
    zero(state, countof(state));
    zero(block, countof(block));
    block_idx = 0;
}

constexpr void sha1::pad()
{
    block[block_idx++] = 0x80;

    if (block_idx > pad_start) {
        fill(block + block_idx, byte(0), block_size - block_idx);
        step();
    }
    fill(block + block_idx, byte(0), pad_start - block_idx);

    for (size_t i = 0, j = 0; i < 4; ++i, j += 8) {
        block[block_size - 1 - i] = length_low  >> j;
        block[block_size - 5 - i] = length_high >> j;
    }
    step();
}

constexpr void sha1::step()
{
    word buf[80];
    word var[state_size];
    
    copy(var, state, countof(state));

    for (size_t t = 0; t < 16; ++t)
        buf[t] = getbe<word>(block + t * 4);
    for (size_t t = 16; t < 80; ++t)
        buf[t] = rol(buf[t-3] ^ buf[t-8] ^ buf[t-14] ^ buf[t-16], 1);

    round<0,  20, 0x5a827999, ch>(buf, var);
    round<20, 40, 0x6ed9eba1, parity>(buf, var);
    round<40, 60, 0x8f1bbcdc, maj>(buf, var);
    round<60, 80, 0xca62c1d6, parity>(buf, var);

    for (size_t i = 0; i < state_size; ++i)
        state[i] += var[i];

    block_idx = 0;

    zero(buf, countof(buf));
}

}

#endif