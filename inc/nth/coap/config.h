#ifndef NTH_COAP_CONFIG_H
#define NTH_COAP_CONFIG_H

#include "nth/coap/base.h"

namespace nth::coap {
namespace imp {

// FIXME make an inline constexpr flags

#define NTH_COAP_OPTION_CACHE       true
#define NTH_COAP_OSCORE_NO_SCRATCH  false
// #define NTH_COAP_NSTART             1
// #define NTH_COAP_DEFAULT_LEISURE    5 // seconds
// #define NTH_COAP_PROBING_RATE       1 // byte/second

// Some configs explained here: https://datatracker.ietf.org/doc/html/rfc7252#section-4.8

inline constexpr auto version               = 1;
inline constexpr auto default_packet_size   = 256;  // 1280
inline constexpr auto max_transition_buf    = 2;
inline constexpr auto max_retransmit_cnt    = 4;
inline constexpr auto ack_timeout           = timestamp(2000); 
inline constexpr auto ack_random_factor     = 1.5;  // Multiplier for `ack_timeout`
inline constexpr auto log_packet_raw_hex    = true;
inline constexpr auto oscore_version        = 1;
inline constexpr auto oscore_cbor_size      = 128;
inline constexpr auto oscore_scratch_size   = 128;

}

}

#endif