#include "test.h"
#include "shoc/hash/md2.h"
#include "shoc/hash/md4.h"
#include "shoc/hash/md5.h"
#include "shoc/hash/sha1.h"
#include "shoc/hash/sha2.h"
// #include "shoc/hash/sha3.h"
#include "shoc/hash/gimli.h"

namespace shoc {
namespace {

struct pair {
    std::string_view msg;
    std::string_view exp;
};

template<class Hash, size_t N>
void check(const pair (&test)[N])
{
    Hash hash;
    byte bin[Hash::hash_size] = {};
    char str[Hash::hash_size * 2 + 1] = {};

    for (auto it : test) {
        hash(it.msg, bin);
        utl::bin_to_str_null(bin, sizeof(bin), str, sizeof(str));
        ASSERT_STREQ(it.exp.data(), str);
    }
}

template<class H>
constexpr auto ce_gen(span_i<> msg)
{
    H hash;
    std::array<byte, H::hash_size> arr{};
    hash(msg, arr);
    return arr;
}

template<size_t N>
void ce_check(std::array<byte, N> bin, std::string_view exp)
{
    char str[N * 2 + 1] = {};
    utl::bin_to_str_null(bin.data(), bin.size(), str, sizeof(str));
    ASSERT_STREQ(exp.data(), str);
}

}

TEST(Hash, Md2) 
{
    const pair test[] = {
        { "",
            "8350e5a3e24c153df2275c9f80692773" },
        { "a",
            "32ec01ec4a6dac72c0ab96fb34c0b5d1" },
        { "abc",
            "da853b0d3f88d99b30283a69e6ded6bb" },
        { "message digest",
            "ab4f496bfb2a530b219ff33031fe06b0" },
        { "abcdefghijklmnopqrstuvwxyz",
            "4e8ddff3650292ab5a4108c3aa47940b" },
        { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
            "da33def2a42df13975352846c30338cd" },
        { "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
            "d5976f79d83d3a0dc9806c3c66f3efd8" },
    };
    check<md2>(test);
}

TEST(Hash, Md4) 
{
    const pair test[] = {
        { "",
            "31d6cfe0d16ae931b73c59d7e0c089c0" },
        { "a",
            "bde52cb31de33e46245e05fbdbd6fb24" },
        { "abc",
            "a448017aaf21d8525fc10ae87aa6729d" },
        { "message digest",
            "d9130a8164549fe818874806e1c7014b" },
        { "abcdefghijklmnopqrstuvwxyz",
            "d79e1c308aa5bbcdeea8ed63df412da9" },
        { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
            "043f8582f241db351ce627e153e7f0e4" },
        { "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
            "e33b4ddc9c38f2199c3e7b164fcc0536" },
    };
    check<md4>(test);
}

TEST(Hash, Md5) 
{
    const pair test[] = {
        { "",
            "d41d8cd98f00b204e9800998ecf8427e" },
        { "a",
            "0cc175b9c0f1b6a831c399e269772661" },
        { "abc",
            "900150983cd24fb0d6963f7d28e17f72" },
        { "message digest",
            "f96b697d7cb7938d525a2f31aaf161d0" },
        { "abcdefghijklmnopqrstuvwxyz",
            "c3fcd3d76192e4007dfb496cca67e13b" },
        { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
            "d174ab98d277d9f5a5611c2c9f419d9f" },
        { "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
            "57edf4a22be3c955ac49da2e2107b67a" },
    };
    check<md5>(test);
}

TEST(Hash, Sha1)
{
    const pair test[] = {
        { "",
            "da39a3ee5e6b4b0d3255bfef95601890afd80709" },
        { "abc",
            "a9993e364706816aba3e25717850c26c9cd0d89d" },
        { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "84983e441c3bd26ebaae4aa1f95129e5e54670f1" },
        { "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
            "a49b2446a02c645bf419f995b67091253a04a259" },
        { "The quick brown fox jumps over the lazy dog",
            "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12" },
        { "The quick brown fox jumps over the lazy cog", 
            "de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3" },
    };
    check<sha1>(test);
}

TEST(Hash, Sha224)
{
    const pair test[] = {
        { "abc",
            "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7" },
        { "",
            "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f" },
        { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525" },
        { "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
            "c97ca9a559850ce97a04a96def6d99a9e0e0e2ab14e6b8df265fc0b3" },
    };
    check<sha224>(test);
}

TEST(Hash, Sha256)
{
    const pair test[] = {
        { "abc",
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" },
        { "",
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" },
        { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1" },
        { "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
            "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1" },
    };
    check<sha256>(test);
}

TEST(Hash, Sha384)
{
    const pair test[] = {
        { "abc",
            "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7" },
        { "",
            "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b" },
        { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "3391fdddfc8dc7393707a65b1b4709397cf8b1d162af05abfe8f450de5f36bc6b0455a8520bc4e6f5fe95b1fe3c8452b" },
        { "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
            "09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712fcc7c71a557e2db966c3e9fa91746039" },
    };
    check<sha384>(test);
}

TEST(Hash, Sha512)
{
    const pair test[] = {
        { "abc",
            "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f" },
        { "",
            "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e" },
        { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c33596fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445" },
        { "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
            "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909" },
    };
    check<sha512>(test);
}

TEST(Hash, Sha512_224)
{
    const pair test[] = {
        { "abc",
            "4634270f707b6a54daae7530460842e20e37ed265ceee9a43e8924aa" },
        { "",
            "6ed0dd02806fa89e25de060c19d3ac86cabb87d6a0ddd05c333b84f4" },
        { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "e5302d6d54bb242275d1e7622d68df6eb02dedd13f564c13dbda2174" },
        { "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
            "23fec5bb94d60b23308192640b0c453335d664734fe40e7268674af9" },
    };
    check<sha512_224>(test);
}

TEST(Hash, Sha512_256)
{
    const pair test[] = {
        { "abc",
            "53048e2681941ef99b2e29b76b4c7dabe4c2d0c634fc6d46e0e2f13107e7af23" },
        { "",
            "c672b8d1ef56ed28ab87c3622c5114069bdd3ad7b8f9737498d0c01ecef0967a" },
        { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "bde8e1f9f19bb9fd3406c90ec6bc47bd36d8ada9f11880dbc8a22a7078b6a461" },
        { "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
            "3928e184fb8690f840da3988121d31be65cb9d3ef83ee6146feac861e19b563a" },
    };
    check<sha512_256>(test);
}

TEST(Hash, Gimli)
{
    const pair test[] = {
        { "There's plenty for the both of us, may the best Dwarf win.",
            "4afb3ff784c7ad6943d49cf5da79facfa7c4434e1ce44f5dd4b28f91a84d22c8" },
        { "If anyone was to ask for my opinion, which I note they're not, I'd say we were taking the long way around.",
            "ba82a16a7b224c15bed8e8bdc88903a4006bc7beda78297d96029203ef08e07c" },
        { "Speak words we can all understand!",
            "8dd4d132059b72f8e8493f9afb86c6d86263e7439fc64cbb361fcbccf8b01267" },
        { "It's true you don't see many Dwarf-women. And in fact, they are so alike in voice and appearance, that they are often mistaken for Dwarf-men. And this in turn has given rise to the belief that there are no Dwarf-women, and that Dwarves just spring out of holes in the ground! Which is, of course, ridiculous.", 
            "8887a5367d961d6734ee1a0d4aee09caca7fd6b606096ff69d8ce7b9a496cd2f" },
        { "",
            "b0634b2c0b082aedc5c0a2fe4ee3adcfc989ec05de6f00addb04b3aaac271f67" },
        { "abc",
            "7748eb235c37a43a02b8dc64faaa4bd23f054f94e52a3d8dcbb4ee85d7840ef3" },
    };
    check<gimli>(test);
}

TEST(Hash, Constexpr)
{
    static constexpr byte msg[]     = "abc";
    static constexpr auto msg_view  = span_i{msg, sizeof(msg) - 1};

    static constexpr auto ret_md2           = ce_gen<md2>(msg_view);
    static constexpr auto ret_md4           = ce_gen<md4>(msg_view);
    static constexpr auto ret_md5           = ce_gen<md5>(msg_view);
    static constexpr auto ret_sha1          = ce_gen<sha1>(msg_view);
    static constexpr auto ret_sha224        = ce_gen<sha224>(msg_view);
    static constexpr auto ret_sha256        = ce_gen<sha256>(msg_view);
    static constexpr auto ret_sha384        = ce_gen<sha384>(msg_view);
    static constexpr auto ret_sha512        = ce_gen<sha512>(msg_view);
    static constexpr auto ret_sha512_224    = ce_gen<sha512_224>(msg_view);
    static constexpr auto ret_sha512_256    = ce_gen<sha512_256>(msg_view);
    static constexpr auto ret_gimli         = ce_gen<gimli>(msg_view);

    ce_check(ret_md2,           "da853b0d3f88d99b30283a69e6ded6bb");
    ce_check(ret_md4,           "a448017aaf21d8525fc10ae87aa6729d");
    ce_check(ret_md5,           "900150983cd24fb0d6963f7d28e17f72");
    ce_check(ret_sha1,          "a9993e364706816aba3e25717850c26c9cd0d89d");
    ce_check(ret_sha224,        "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7");
    ce_check(ret_sha256,        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    ce_check(ret_sha384,        "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7");
    ce_check(ret_sha512,        "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
    ce_check(ret_sha512_224,    "4634270f707b6a54daae7530460842e20e37ed265ceee9a43e8924aa");
    ce_check(ret_sha512_256,    "53048e2681941ef99b2e29b76b4c7dabe4c2d0c634fc6d46e0e2f13107e7af23");
    ce_check(ret_gimli,         "7748eb235c37a43a02b8dc64faaa4bd23f054f94e52a3d8dcbb4ee85d7840ef3");
}

}