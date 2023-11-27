#ifndef COAP_OSCORE_H
#define COAP_OSCORE_H

#include "coap/cose.h"
#include "coap/packet.h"
#include <string_view>

namespace coap {

enum CoseAlgorithm {
    cose_es256              = -7,   // ECDSA w/ SHA-256
    cose_es384              = -35,  // ECDSA w/ SHA-384
    cose_es512              = -36,  // ECDSA w/ SHA-512
    cose_eddsa              = -8,   // EdDSA
    cose_hmac_256_64        = 4,    // HMAC w/ SHA-256 truncated to 64 bits 
    cose_hmac_256_256       = 5,    // HMAC w/ SHA-256 
    cose_hmac_384_384       = 6,    // HMAC w/ SHA-384 
    cose_hmac_512_512       = 7,    // HMAC w/ SHA-512 
    cose_aes_mac_128_64     = 14,   // AES-MAC 128-bit key, 64-bit tag 
    cose_aes_mac_256_64     = 15,   // AES-MAC 256-bit key, 64-bit tag 
    cose_aes_mac_128_128    = 25,   // AES-MAC 128-bit key, 128-bit tag
    cose_aes_mac_256_128    = 26,   // AES-MAC 256-bit key, 128-bit tag
    cose_a128gcm            = 1,    // AES-GCM mode w/ 128-bit key, 128-bit tag
    cose_a192gcm            = 2,    // AES-GCM mode w/ 192-bit key, 128-bit tag
    cose_a256gcm            = 3,    // AES-GCM mode w/ 256-bit key, 128-bit tag
    cose_aes_ccm_16_64_128  = 10,   // AES-CCM mode 128-bit key, 64-bit tag, 13-byte nonce
    cose_aes_ccm_16_64_256  = 11,   // AES-CCM mode 256-bit key, 64-bit tag, 13-byte nonce
    cose_aes_ccm_64_64_128  = 12,   // AES-CCM mode 128-bit key, 64-bit tag, 7-byte nonce 
    cose_aes_ccm_64_64_256  = 13,   // AES-CCM mode 256-bit key, 64-bit tag, 7-byte nonce 
    cose_aes_ccm_16_128_128 = 30,   // AES-CCM mode 128-bit key, 128-bit tag, 13-byte nonce  
    cose_aes_ccm_16_128_256 = 31,   // AES-CCM mode 256-bit key, 128-bit tag, 13-byte nonce 
    cose_aes_ccm_64_128_128 = 32,   // AES-CCM mode 128-bit key, 128-bit tag, 7-byte nonce
    cose_aes_ccm_64_128_256 = 33,   // AES-CCM mode 256-bit key, 128-bit tag, 7-byte nonce
    cose_chacha20_poly1305  = 24,   // ChaCha20/Poly1305 w/ 256-bit key, 128-bit tag
};

constexpr bool is_class_e(word num)
{
    switch (num) {
        case option_num_uri_host: 
        case option_num_uri_port: 
        case option_num_oscore:
        case option_num_proxy_uri:
        case option_num_proxy_scheme: return false;
        default: return true;
    }
}

constexpr bool is_class_i([[maybe_unused]] word num)
{
    return false;
}

constexpr bool is_class_u(word num)
{
    switch (num) {
        case option_num_uri_host:
        case option_num_observe:
        case option_num_uri_port:
        case option_num_oscore:
        case option_num_max_age:
        case option_num_block2:
        case option_num_block1:
        case option_num_size2:
        case option_num_proxy_uri: 
        case option_num_proxy_scheme: 
        case option_num_size1:
        case option_num_no_response: return true;
        default: return false;
    }
}

enum OscoreErr {
    oscore_err_ok,
    oscore_err_no_option,
    oscore_err_reserved_bits,
    oscore_err_replay,
    oscore_err_illegal_kid_length,
    oscore_err_illegal_piv_length,
    oscore_err_illegal_tag_length,
    oscore_err_out_of_memory,
    oscore_err_crypto_fail,
};

template<class T = uint32_t>
struct ReplayWindow {

    ReplayWindow() = default;
    ReplayWindow(T seqn) : high{seqn} {}

    constexpr T latest() const
    {
        return high;
    }

    constexpr bool check(T seqn) const
    {
        if (seqn > high)
            return true;
        if (high - seqn >= bits)
            return false;
        return !utl::get_bit(mask, high - seqn);
    }

    constexpr void update(T seqn)
    {
        if (seqn > high)
            update_latest(seqn);
        else if (high - seqn >= bits)
            return;
        else 
            utl::set_bit(mask, high - seqn);
    }

    constexpr bool check_and_update(T seqn)
    {
        if (seqn > high) {
            update_latest(seqn);
            return true;
        }
        auto idx = high - seqn;
        if (idx >= bits)
            return false;
        if (utl::get_bit(mask, idx))
            return false;
        utl::set_bit(mask, idx);
        return true;
    }
private:
    constexpr void update_latest(T seqn)
    {
        auto diff = seqn - high;
        mask = 1 | (diff < bits ? mask << diff : 0);
        high = seqn;
    }
private:
    T mask = -2;
    T high = 0;
    static constexpr auto bits = sizeof(T) * 8;
};

/**
 * @brief OSCORE security context.
 * 
 * Algorithms:
 *  1. AEAD: AES-CCM-16-64-128 
 *  2. HKDF: HKDF-SHA-256
 * 
 * Length requirements:
 *  1. Sender ID: [0..7]
 *  2. Recipient ID: [0..7]
 *  3. ID Context: variable-length
 *  4. Master Salt: variable-length
 *  5. Master Secret: variable-length
 */
struct Security {
public:
    bool derive(m_span out, span id, std::string_view type, size_t length) const;
    bool derive_key(m_span out, span id) const;
    bool derive_common_iv(m_span common_iv) const;
    bool derive_sender_key(m_span sender_key) const;
    bool derive_recipient_key(m_span recipient_key) const;
    OscoreErr protect_req(const Packet &packet, Packet &oscore);
    OscoreErr protect_rsp(const Packet &packet, Packet &oscore, bool use_piv, bool use_kid);
    OscoreErr verify_req(const Packet &oscore, Packet &packet);
    OscoreErr verify_rsp(const Packet &oscore, Packet &packet);
private:
    OscoreErr protect(
        const Packet &packet, 
              Packet &oscore, 
        bool use_piv, 
        bool use_kid, 
        bool use_ctx, 
        span piv, 
        span kid, 
        span req_piv, 
        span req_kid);
    OscoreErr verify(
        const Packet &oscore, 
              Packet &packet,
        bool use_piv,
        span piv,
        span kid,
        span req_piv,
        span req_kid);
    auto construct_key(span id) const;
    auto construct_nonce(span kid, span piv) const;
private:
    static constexpr size_t NONCE_SIZE  = 13;
    static constexpr size_t KEY_SIZE    = 16;
    static constexpr size_t TAG_SIZE    = 8;
    static constexpr size_t PIV_SIZE    = 5;
    static constexpr size_t KID_SIZE    = NONCE_SIZE - 6;
public:
    const span id_context           = {};
    const span master_salt          = {};
    const span master_secret        = {};
    const span sender_id            = {};
    const span recipient_id         = {};
    ReplayWindow<> replay_window    = {};
    uint32_t sequence_number        = 0;
};

struct OscoreOption {

    OscoreOption() = delete;
    OscoreOption(const byte* buf, size_t len) : buf{buf}, len{len}
    {}

    bool get_kid_flag() const           { return buf[0] & 0x08; }
    bool get_ctx_flag() const           { return buf[0] & 0x10; }
    auto get_reserved() const           { return buf[0] & 0xe0; }
    size_t get_piv_len() const          { return buf[0] & 0x07; }
    size_t get_ctx_len() const          { return buf[1 + get_piv_len()]; }
    size_t get_kid_len() const          { return buf + len - kid_begin(); }
    span get_piv() const                 { return {piv_begin(), get_piv_len()}; }
    span get_ctx() const                 { return {ctx_begin(), get_ctx_len()}; } 
    span get_kid() const                 { return {kid_begin(), get_kid_len()}; }
private:
    const byte* piv_begin() const    { return buf + 1; }
    const byte* ctx_begin() const    { return piv_begin() + get_piv_len() + get_ctx_flag(); }
    const byte* kid_begin() const    { return ctx_begin() + get_ctx_len() * get_ctx_flag(); }
    const byte* buf;
    size_t len;
};

}

#endif

// #include <algorithm>
// #include "coap/oscore.h"
// #include "coap/config.h"
// #include "coap/log.h"
// #include "shoc/kdf/hkdf.h"
// #include "shoc/hash/sha2.h"
// #include "shoc/mode/ccm.h"
// #include "shoc/cipher/aes.h"
// #include "zbor/codec.h"
// #include "zbor/log.h"

// #define LOG_ENABLED_DBG false
// #define LOG_ENABLED_WRN true
// #define LOG_ENABLED_ERR true

// namespace coap {
// namespace {

// auto construct_piv_buffer(auto seqn)
// {
//     std::array<uint8_t, sizeof(seqn)> seqn_buf = {};

//     int seqn_idx = sizeof(seqn);
//     while (seqn && seqn_idx) {
//         seqn_buf[--seqn_idx] = seqn;
//         seqn >>= 8;
//     }
//     return std::pair{seqn_buf, seqn_idx};
// }

// auto construct_piv(auto &buf, size_t idx)
// {
//     return idx == sizeof(buf) ?
//         span{buf.begin(), 1} :
//         span{buf.begin() + idx, buf.end()};
// }

// auto construct_aad(span kid, span piv)
// {
//     zbor::Codec<imp::oscore_cbor_size> aad;

//     auto aad_array = [&]()
//     {
//         aad.encode_arr(5);
//         aad.encode(COAP_OSCORE_VER);
//         aad.encode_arr(1);
//         aad.encode(cose_aes_ccm_16_64_128);
//         aad.encode(kid);
//         aad.encode(piv);
//         aad.encode(span{}); // NOTE: No class I options currently exist, so it's simplified here a lot.
//     };
//     aad_array();
//     size_t aad_array_size = aad.size();

//     aad.clear();
//     aad.encode_arr(3);
//     aad.encode("Encrypt0");
//     aad.encode(span{});
//     aad.encode_head(zbor::mt_data, aad_array_size);
//     aad_array();

// #if (LOG_ENABLED_DBG)
//     logd() << "aad";
//     log_hex(aad.data(), aad.size());
// #endif
//     return aad;
// }

// template<class T>
// auto construct_seqn(span buf)
// {
//     T val = 0;
//     for (size_t i = 0; i < buf.size(); ++i) {
//         val <<= 8;
//         val |= buf[i];
//     }
//     return val;
// }

// auto copy_options(const Packet &src, Packet &dst)
// {
//     for (auto opt : src.options()) {
//         if (!is_class_e(OptionNum(opt.num)) && !dst.opt_insert(opt)) {
//             return false;
//         }
//     }
//     return true;
// }

// }

// auto Security::construct_key(span id) const
// {
//     std::array<uint8_t, KEY_SIZE> key;
//     derive_key(key, id);
// #if (LOG_ENABLED_DBG)
//     logd() << "key";
//     log_hex(key.data(), key.size());
// #endif
//     return key;
// }

// auto Security::construct_nonce(span kid, span piv) const
// {
//     std::array<uint8_t, NONCE_SIZE> iv;
//     std::array<uint8_t, NONCE_SIZE> nonce = { uint8_t(kid.size()) };

//     derive_common_iv(iv);

//     shoc::copy(&nonce[1 + KID_SIZE - kid.size()], kid.data(), kid.size());
//     shoc::copy(&nonce[NONCE_SIZE - piv.size()], piv.data(), piv.size());
//     shoc::xorb(nonce.data(), iv.data(), NONCE_SIZE);
    
// #if (LOG_ENABLED_DBG)
//     logd() << "common_iv";
//     log_hex(iv.data(), iv.size());
//     logd() << "nonce";
//     log_hex(nonce.data(), nonce.size());
// #endif
//     return nonce;
// }

// bool Security::derive(m_span out, span id, std::string_view type, size_t length) const
// {
//     if (out.size() < length) {
//         loge(LOG_ENABLED_ERR) << "too small buffer, can't derive";
//         return false;
//     }
//     zbor::Codec<imp::oscore_cbor_size> info;

//     info.encode_arr(5);
//     info.encode(id);
//     if (id_context.empty())
//         info.encode(zbor::prim_null);
//     else
//         info.encode(id_context);
//     info.encode(cose_aes_ccm_16_64_128);
//     info.encode(type);
//     info.encode(length);

//     return shoc::hkdf<shoc::Sha2<shoc::SHA_256>>(
//         out.data(),
//         length,
//         master_secret.data(),
//         master_secret.size(),
//         master_salt.data(),
//         master_salt.size(),
//         info.data(),
//         info.size()
//     );
// }

// bool Security::derive_common_iv(m_span out) const
// {
//     return derive(out, {}, "IV", NONCE_SIZE);
// }

// bool Security::derive_key(m_span out, span id) const
// {
//     return derive(out, id, "Key", KEY_SIZE);
// }

// bool Security::derive_sender_key(m_span out) const
// {
//     return derive_key(out, sender_id);
// }

// bool Security::derive_recipient_key(m_span out) const
// {
//     return derive_key(out, recipient_id);
// }

// OscoreErr Security::protect(
//     const Packet &packet, 
//           Packet &oscore, 
//     bool use_piv,
//     bool use_kid,
//     bool use_ctx,
//     span piv,
//     span kid,
//     span req_piv,
//     span req_kid)
// {
//     if (kid.size() > KID_SIZE) {
//         logw() << "kid.size() is greater than maximum: " << kid.size() << " > " << KID_SIZE;
//         return oscore_err_illegal_kid_length;
//     }
//     if (piv.size() > PIV_SIZE) {
//         logw() << "piv.size() is greater than maximum: " << piv.size() << " > " << PIV_SIZE;
//         return oscore_err_illegal_piv_length;
//     }
//     if (req_kid.size() > KID_SIZE) {
//         logw() << "req_kid.size() is greater than maximum: " << req_kid.size() << " > " << KID_SIZE;
//         return oscore_err_illegal_kid_length;
//     }
//     if (req_piv.size() > PIV_SIZE) {
//         logw() << "req_piv.size() is greater than maximum: " << req_piv.size() << " > " << PIV_SIZE;
//         return oscore_err_illegal_piv_length;
//     }
    
//     // 1. Mirror header
//     // 2. Add OSCORE option
//     // 3. Sort all options: either direct copy or store to plaintext section
//     // 4. Construct common_iv, sender_key and nonce
//     // 5. Construct aad_array and AAD
//     // 6. Encrypt plaintext and add to OSCORE packet

//     oscore.clear();
//     oscore.init(
//         packet.get_ver(), 
//         packet.get_type(),
//         packet.get_code(), 
//         packet.get_id(), 
//         packet.get_token().data(), 
//         packet.get_tkl());

//     bool observe = packet.opt_is_set(option_num_observe);

//     if (packet.get_code_class() == cc_request)
//         oscore.set_code(observe ? code_fetch : code_post);
//     else
//         oscore.set_code(observe ? code_content : code_changed);

//     //////////////////////////////////
// #if (COAP_OSCORE_NO_SCRATCH)

//     uint16_t scratch_size = 0;

//     if (use_piv || use_kid || use_ctx) {
//         scratch_size += 1;
//     }
//     if (use_piv) {
//         scratch_size += piv.size();
//     }
//     if (use_ctx) {
//         scratch_size += 1;
//         scratch_size += id_context.size();
//     }
//     if (use_kid) {
//         scratch_size += kid.size();
//     }
//     auto oscore_opt_write_idx = oscore.size();
//     auto oscore_opt_size = opt_total_size(option_num_oscore, scratch_size);
//     if (oscore_opt_write_idx + oscore_opt_size > oscore.capacity()) {
//         loge(LOG_ENABLED_ERR) << "too huge option for OSCORE packet";
//         return oscore_err_out_of_memory;
//     }
//     opt_encode_head(oscore.end(), option_num_oscore, scratch_size);
//     oscore_opt_write_idx += opt_head_size(option_num_oscore, scratch_size);

//     if (use_piv || use_kid || use_ctx) {
//         oscore[oscore_opt_write_idx] = (piv.size() * use_piv) | (use_kid << 3) | (use_ctx << 4);
//         oscore_opt_write_idx++;
//     }
//     if (use_piv) {
//         memcpy(&oscore[oscore_opt_write_idx], piv.data(), piv.size());
//         oscore_opt_write_idx += piv.size();
//     }
//     if (use_ctx) {
//         oscore[oscore_opt_write_idx] = id_context.size();
//         oscore_opt_write_idx += 1;
//         memcpy(&oscore[oscore_opt_write_idx], id_context.data(), id_context.size());
//         oscore_opt_write_idx += id_context.size();
//     }
//     if (use_kid) {
//         memcpy(&oscore[oscore_opt_write_idx], kid.data(), kid.size());
//         oscore_opt_write_idx += kid.size();
//     }
//     oscore.resize(oscore_opt_write_idx);
//     oscore.parse();
// #else
//     uint8_t scratch[imp::oscore_scratch_size];
//     uint16_t scratch_size = 0;

//     if (use_piv || use_kid || use_ctx) {
//         scratch_size += 1;
//     }
//     if (use_piv) {
//         scratch_size += piv.size();
//     }
//     if (use_ctx) {
//         scratch_size += 1;
//         scratch_size += id_context.size();
//     }
//     if (use_kid) {
//         scratch_size += kid.size();
//     }

//     if (scratch_size > sizeof(scratch)) {
//         loge(LOG_ENABLED_ERR) << "not enough memory in scratch section";
//         return oscore_err_out_of_memory;
//     }
//     scratch_size = 0;

//     if (use_piv || use_kid || use_ctx) {
//         scratch[0] = (piv.size() * use_piv) | (use_kid << 3) | (use_ctx << 4);
//         scratch_size += 1;
//     }
//     if (use_piv) {
//         memcpy(scratch + scratch_size, piv.data(), piv.size());
//         scratch_size += piv.size();
//     }
//     if (use_ctx) {
//         scratch[scratch_size] = id_context.size();
//         scratch_size += 1;
//         memcpy(scratch + scratch_size, id_context.data(), id_context.size());
//         scratch_size += id_context.size();
//     }
//     if (use_kid) {
//         memcpy(scratch + scratch_size, kid.data(), kid.size());
//         scratch_size += kid.size();
//     }
//     oscore.opt_insert({
//         .dat = scratch,
//         .len = scratch_size,
//         .num = option_num_oscore,
//     });
// #if (LOG_ENABLED_DBG)
//     logd() << "option";
//     log_hex(scratch, scratch_size);
// #endif
// #endif
//     //////////////////////////////////

// #if (COAP_OSCORE_NO_SCRATCH)
//     if (!copy_options(packet, oscore)) {
//         loge(LOG_ENABLED_ERR) << "failed to add non-E option to OSCORE packet";
//         return oscore_err_out_of_memory;
//     }
//     auto payload = oscore.end() + 1;
//     auto payload_size = oscore.size() + 1;
//     auto write_idx = oscore.size();

//     if (write_idx + 2 > oscore.capacity()) { // 0xff + CODE
//         loge(LOG_ENABLED_ERR) << "not enough memory for OSCORE payload";
//         return oscore_err_out_of_memory;
//     }
//     oscore[write_idx++] = 0xff;
//     oscore[write_idx++] = packet.get_code();

//     uint16_t prev_num = 0;

//     for (auto opt : packet.options()) {

//         if (is_class_e(OptionNum(opt.num))) {

//             uint16_t delta = opt.num - prev_num;
//             uint16_t opt_size = opt_total_size(delta, opt.len);

//             if (write_idx + opt_size> oscore.capacity()) {
//                 loge(LOG_ENABLED_ERR) << "too huge option for OSCORE packet";
//                 return oscore_err_out_of_memory;
//             }
//             opt_encode(&oscore[write_idx], delta, opt.len, opt.dat);
//             write_idx += opt_size;
//             delta = 0;
//             prev_num = opt.num;
//         }
//     }
//     auto payload_plain = packet.get_payload();
//     if (!payload_plain.empty()) {
//         if (write_idx + payload_plain.size() + 1 > oscore.capacity()) {
//             loge(LOG_ENABLED_ERR) << "too huge payload for OSCORE packet";
//             return oscore_err_out_of_memory;
//         }
//         oscore[write_idx++] = 0xff;
//         memcpy(&oscore[write_idx], payload_plain.data(), payload_plain.size());
//         write_idx += payload_plain.size();
//     }

//     if (write_idx + TAG_SIZE >= oscore.capacity()) {
//         loge(LOG_ENABLED_ERR) << "too huge payload for OSCORE packet, can't include tag";
//         return oscore_err_out_of_memory;
//     }
//     payload_size = write_idx - payload_size;
// #else
//     scratch[0] = packet.get_code();
//     scratch_size = 1;

//     uint16_t prev_num = 0;

//     for (auto opt : packet.options()) {

//         uint16_t delta = opt.num - prev_num;

//         if (is_class_e(opt.num)) {
//             if (scratch_size + opt_total_size(delta, opt.len) > sizeof(scratch)) {
//                 loge(LOG_ENABLED_ERR) << "too huge option for scratch section";
//                 return oscore_err_out_of_memory;
//             }
//             opt_encode(&scratch[scratch_size], delta, opt.len, opt.dat);
//             scratch_size += opt_total_size(delta, opt.len);
//             delta = 0;
//             prev_num = opt.num;
//         } else {
//             if (!oscore.opt_insert(opt)) {
//                 loge(LOG_ENABLED_ERR) << "failed to add non-E option to OSCORE packet";
//                 return oscore_err_out_of_memory;
//             }
//         }
//     }

//     auto payload = packet.get_payload();
//     if (!payload.empty()) {
//         if (scratch_size + payload.size() + 1 > sizeof(scratch)) {
//             loge(LOG_ENABLED_ERR) << "too huge payload for scratch section";
//             return oscore_err_out_of_memory;
//         }
//         scratch[scratch_size++] = 0xff;
//         memcpy(scratch + scratch_size, payload.data(), payload.size());
//         scratch_size += payload.size();
//     }

//     if (scratch_size + TAG_SIZE >= sizeof(scratch)) {
//         loge(LOG_ENABLED_ERR) << "too huge payload, can't include tag";
//         return oscore_err_out_of_memory;
//     }
// #if (LOG_ENABLED_DBG)
//     logd() << "plaintext";
//     log_hex(scratch, scratch_size);
// #endif
// #endif
//     //////////////////////////////////

//     auto nonce = use_piv ? 
//         construct_nonce(kid, piv) :
//         construct_nonce(req_kid, req_piv);
//     auto aad = construct_aad(req_kid, req_piv);
//     auto key = construct_key(sender_id);
    
//     //////////////////////////////////

// #if (COAP_OSCORE_NO_SCRATCH)
//     if (!shoc::ccm_encrypt<shoc::Aes>(
//         key.data(),
//         nonce.data(),
//         aad.data(), aad.size(),
//         payload + payload_size, TAG_SIZE,
//         payload,
//         payload, payload_size))
//     {
//         loge(LOG_ENABLED_ERR) << "CCM encryption failed";
//         return oscore_err_crypto_fail;
//     };
//     write_idx += TAG_SIZE;
    
//     oscore.resize(write_idx);
//     oscore.parse();
// #else
//     if (!shoc::ccm_encrypt<shoc::Aes128>(
//         key.data(),
//         nonce.data(),
//         aad.data(), aad.size(),
//         scratch + scratch_size, TAG_SIZE,
//         scratch,
//         scratch, scratch_size))
//     {
//         loge(LOG_ENABLED_ERR) << "CCM encryption failed";
//         return oscore_err_crypto_fail;
//     };
//     scratch_size += TAG_SIZE;
//     oscore.set_payload(scratch, scratch_size);
// #endif

// #if (LOG_ENABLED_DBG)
//     logd() << "ciphertext";
//     log_hex(scratch, scratch_size);
// #endif

//     if (use_piv)
//         ++sequence_number;

//     return oscore_err_ok;
// }

// OscoreErr Security::protect_req(const Packet &packet, Packet &oscore)
// {
//     auto [piv_buf, piv_len] = construct_piv_buffer(sequence_number);
//     auto piv = construct_piv(piv_buf, piv_len);

//     return protect(packet, oscore, true, true, !id_context.empty(), piv, sender_id, piv, sender_id);
// }

// OscoreErr Security::protect_rsp(const Packet &packet, Packet &oscore, bool use_piv, bool use_kid)
// {
//     auto [piv_buf, piv_len] = construct_piv_buffer(sequence_number);
//     auto piv = construct_piv(piv_buf, piv_len);

//     auto [req_piv_buf, req_piv_len] = construct_piv_buffer(replay_window.latest());
//     auto req_piv = construct_piv(req_piv_buf, req_piv_len);

//     return protect(packet, oscore, use_piv, use_kid, false, piv, sender_id, req_piv, recipient_id);
// }

// OscoreErr Security::verify(
//     const Packet &oscore, 
//           Packet &packet, 
//     bool use_piv,
//     span piv,
//     span kid,
//     span req_piv,
//     span req_kid)
// {
//     if (kid.size() > KID_SIZE) {
//         logw() << "kid.size() is greater than maximum: " << kid.size() << " > " << KID_SIZE;
//         return oscore_err_illegal_kid_length;
//     }
//     if (piv.size() > PIV_SIZE) {
//         logw() << "piv.size() is greater than maximum: " << piv.size() << " > " << PIV_SIZE;
//         return oscore_err_illegal_piv_length;
//     }
//     if (req_kid.size() > KID_SIZE) {
//         logw() << "req_kid.size() is greater than maximum: " << req_kid.size() << " > " << KID_SIZE;
//         return oscore_err_illegal_kid_length;
//     }
//     if (req_piv.size() > PIV_SIZE) {
//         logw() << "req_piv.size() is greater than maximum: " << req_piv.size() << " > " << PIV_SIZE;
//         return oscore_err_illegal_piv_length;
//     }

//     //////////////////////////////////

//     auto nonce = use_piv ? 
//         construct_nonce(kid, piv) :
//         construct_nonce(req_kid, req_piv);
//     auto aad = construct_aad(req_kid, req_piv);
//     auto key = construct_key(recipient_id);
    
//     //////////////////////////////////

//     packet.clear();
//     packet.init(
//         oscore.get_ver(), 
//         oscore.get_type(),
//         oscore.get_code(), 
//         oscore.get_id(), 
//         oscore.get_token().data(), 
//         oscore.get_tkl());

//     if (!copy_options(oscore, packet)) {
//         loge(LOG_ENABLED_ERR) << "failed to retrieve non-E options from OSCORE packet";
//         return oscore_err_out_of_memory;
//     }

//     //////////////////////////////////

//     uint8_t scratch[imp::oscore_scratch_size];

//     auto payload = oscore.get_payload();

//     if (payload.size() < TAG_SIZE) {
//         loge(LOG_ENABLED_ERR) << "payload size is smaller than minimum tag size: " << payload.size() << " < " << TAG_SIZE;
//         return oscore_err_illegal_tag_length;
//     }
//     auto payload_size = payload.size() - TAG_SIZE;

//     if (payload_size > sizeof(scratch)) {
//         loge(LOG_ENABLED_ERR) << "not enough scratch memory to decrypt payload: " << payload_size << " > " << sizeof(scratch);
//         return oscore_err_out_of_memory;
//     }

//     if (!shoc::ccm_decrypt<shoc::Aes128>(
//         key.data(),
//         nonce.data(),
//         aad.data(), aad.size(),
//         payload.data() + payload_size, TAG_SIZE,
//         payload.data(),
//         scratch, payload_size))
//     {
//         loge(LOG_ENABLED_ERR) << "CCM decryption failed";
//         return oscore_err_crypto_fail;
//     };

//     //////////////////////////////////

//     packet.opt_delete(option_num_oscore);
//     packet.set_code(Code(scratch[0]));

//     for (auto opt : OptRange<OptIter>(scratch + 1, scratch + payload_size)) {
//         packet.opt_insert(opt);
//     }
//     return oscore_err_ok;
// }

// OscoreErr Security::verify_req(const Packet &oscore, Packet &packet)
// {
//     auto opt = oscore.opt_get(option_num_oscore);
//     auto piv = OscoreOption{opt.dat, opt.len}.get_piv();

//     auto seqn = construct_seqn<uint32_t>(piv);

//     if (!replay_window.check(seqn)) {
//         loge(LOG_ENABLED_ERR) << "sequence number replay: " << seqn;
//         return oscore_err_replay;
//     }
//     auto ret = verify(oscore, packet, false, piv, recipient_id, piv, recipient_id);

//     if (ret == oscore_err_ok)
//         replay_window.update(seqn);

//     return ret;
// }

// OscoreErr Security::verify_rsp(const Packet &oscore, Packet &packet)
// {
//     auto opt = oscore.opt_get(option_num_oscore);
//     auto piv = OscoreOption{opt.dat, opt.len}.get_piv();

//     auto [req_piv_buf, req_piv_len] = construct_piv_buffer(sequence_number - 1);
//     auto req_piv = construct_piv(req_piv_buf, req_piv_len);

//     return verify(oscore, packet, !piv.empty(), piv, recipient_id, req_piv, sender_id);
// }
    
// }