#ifndef NTH_CRYPTO_CIPHER_CHACHA20_H
#define NTH_CRYPTO_CIPHER_CHACHA20_H

#define NTH_CRYPTO_CIPHER_CHACHA20_STORE_POSITION true

#include "nth/crypto/util.h"

namespace nth {
namespace imp {

constexpr void chacha_quarter_round(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d)
{
    a += b; d ^= a; d = rol(d, 16);
    c += d; b ^= c; b = rol(b, 12);
    a += b; d ^= a; d = rol(d, 8);
    c += d; b ^= c; b = rol(b, 7);
}

struct chacha20_context {

    using word = uint32_t;

    static constexpr size_t block_size_bytes    = 64;
    static constexpr size_t block_size          = 16;
    static constexpr size_t key_size            = 32;
    static constexpr size_t iv_size             = 12;
public:
    constexpr chacha20_context() = default;
    constexpr chacha20_context(const byte* key, const byte* iv, word count) 
    { 
        init(key, iv, count); 
    }
    constexpr ~chacha20_context()
    { 
        deinit(); 
    }
public:
    constexpr void init(const byte* key, const byte* iv, word count);
    constexpr void deinit();
    constexpr void encrypt(const byte* in, byte* out, size_t len);
    constexpr void decrypt(const byte* in, byte* out, size_t len);
private:
    constexpr void quarter(size_t a, size_t b, size_t c, size_t d);
    constexpr void round();
    constexpr void block();
private:
    std::array<word, block_size> initial {};
    union {
        std::array<word, block_size> state {};
        std::array<byte, block_size_bytes> stream;
    };
#if (NTH_CRYPTO_CIPHER_CHACHA20_STORE_POSITION)
    size_t position = 0;
#endif
};

constexpr void chacha20_context::init(const byte* key, const byte* iv, word count)
{
#if (NTH_CRYPTO_CIPHER_CHACHA20_STORE_POSITION)
    position = 0;
#endif
    initial[0] = 0x61707865;
    initial[1] = 0x3320646e;
    initial[2] = 0x79622d32;
    initial[3] = 0x6b206574;

    for (size_t i = 4; i < 12; ++i, key += sizeof(word))
        initial[i] = getle<word>(key);

    initial[12] = count;

    for (size_t i = 13; i < 16; ++i, iv += sizeof(word))
        initial[i] = getle<word>(iv);

    block();
}

constexpr void chacha20_context::deinit()
{
    zero(state.data(), state.size());
    zero(initial.data(), initial.size());
#if (NTH_CRYPTO_CIPHER_CHACHA20_STORE_POSITION)
    position = 0;
#endif
}

constexpr void chacha20_context::quarter(size_t a, size_t b, size_t c, size_t d)
{
    chacha_quarter_round(state[a], state[b], state[c], state[d]);
}

constexpr void chacha20_context::round()
{
    for (int i = 0; i < 10; ++i) {
        quarter(0, 4, 8,  12);
        quarter(1, 5, 9,  13);
        quarter(2, 6, 10, 14);
        quarter(3, 7, 11, 15);
        quarter(0, 5, 10, 15);
        quarter(1, 6, 11, 12);
        quarter(2, 7, 8,  13);
        quarter(3, 4, 9,  14);
    }
}

constexpr void chacha20_context::block()
{
    state = initial;
    round();
    for (size_t i = 0; i < block_size; ++i)
        state[i] += initial[i];
    ++initial[12];
}

constexpr void chacha20_context::encrypt(const byte* in, byte* out, size_t len)
{
#if !(NTH_CRYPTO_CIPHER_CHACHA20_STORE_POSITION)
    size_t position = 0;
#endif
    for (size_t i = 0; i < len; ++i) {
        if (position >= block_size_bytes) {
            position = 0;
            block();
        }
        out[i] = in[i] ^ stream[position++];
    }
}

constexpr void chacha20_context::decrypt(const byte* in, byte* out, size_t len)
{
    encrypt(in, out, len);
}

}

using chacha20 = imp::chacha20_context;

}

#undef NTH_CRYPTO_CIPHER_CHACHA20_STORE_POSITION

#endif