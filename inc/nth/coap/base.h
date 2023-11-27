#ifndef NTH_COAP_BASE_H
#define NTH_COAP_BASE_H

#include "nth/util/meta.h"

namespace nth::coap {

using byte = nth::byte;
using word = uint16_t;
using timestamp = int64_t; // platform timestamp in milliseconds

/**
 * @brief Input span of const bytes.
 * 
 */
using ispan = std::span<const byte>;

/**
 * @brief Output span of mutable bytes.
 * 
 */
using ospan = std::span<byte>;

/**
 * @brief Generic CoAP error enumeration.
 * 
 */
enum class error {
    ok,
    invalid_version,
    invalid_token_length,
    invalid_option,
    too_short_header,
    too_short_token,
    too_short_option,
    too_short_payload,
    too_small_buffer,
    unexpected_event,
    unreachable,
    packet_parsing_error,
};

/**
 * @brief Message type.
 * 
 */
enum class msg_type {
    con,
    non,
    ack,
    rst,
};

/**
 * @brief Message code class.
 * 
 */
enum class msg_class {
    request    = 0,
    success    = 2,
    client_err = 4,
    server_err = 5,
};

/**
 * @brief Construct message code (used in 'msg_code' enum).
 * 
 * @param c Code class
 * @param d Code detail
 * @return Message code
 */
constexpr int make_msg_code(msg_class c, int d)
{
    return (int(c) << 5) | d;
}

/**
 * @brief Constructed message code (class + detail).
 * 
 */
enum class msg_code {
    empty                      = make_msg_code(msg_class::request, 0),
    get                        = make_msg_code(msg_class::request, 1),
    post                       = make_msg_code(msg_class::request, 2),
    put                        = make_msg_code(msg_class::request, 3),
    delete_                    = make_msg_code(msg_class::request, 4),
    fetch                      = make_msg_code(msg_class::request, 5),
    patch                      = make_msg_code(msg_class::request, 6),
    ipatch                     = make_msg_code(msg_class::request, 7),
    created                    = make_msg_code(msg_class::success, 1),
    deleted                    = make_msg_code(msg_class::success, 2),
    valid                      = make_msg_code(msg_class::success, 3),
    changed                    = make_msg_code(msg_class::success, 4),
    content                    = make_msg_code(msg_class::success, 5),
    bad_request                = make_msg_code(msg_class::client_err, 0),
    unauthorized               = make_msg_code(msg_class::client_err, 1),
    bad_option                 = make_msg_code(msg_class::client_err, 2),
    forbidden                  = make_msg_code(msg_class::client_err, 3),
    not_found                  = make_msg_code(msg_class::client_err, 4),
    method_not_allowed         = make_msg_code(msg_class::client_err, 5),
    not_acceptable             = make_msg_code(msg_class::client_err, 6),
    precondition_failed        = make_msg_code(msg_class::client_err, 12),
    request_entity_too_large   = make_msg_code(msg_class::client_err, 13),
    unsupported_content_format = make_msg_code(msg_class::client_err, 15),
    internal_server_error      = make_msg_code(msg_class::server_err, 0),
    not_implemented            = make_msg_code(msg_class::server_err, 1),
    bad_gateway                = make_msg_code(msg_class::server_err, 2),
    service_unavailable        = make_msg_code(msg_class::server_err, 3),
    gateway_timeout            = make_msg_code(msg_class::server_err, 4),
    proxying_not_supported     = make_msg_code(msg_class::server_err, 5),
};

/**
 * @brief Representation format of message payload.
 * 
 */
enum class msg_format {
    text_plain   = 0,
    link_format  = 40,
    xml          = 41,
    octet_stream = 42,
    exi          = 47,
    json         = 50,
};

/**
 * @brief Message layer state: 
 * https://datatracker.ietf.org/doc/html/draft-ietf-lwig-coap-06#section-2.5.2
 * 
 */
enum class msg_state {
    closed,
    ack_pending,
    reliable_tx,
};

/**
 * @brief Message layer event:
 * https://datatracker.ietf.org/doc/html/draft-ietf-lwig-coap-06#section-2.5.2
 * 
 */
enum class msg_event {
    rx_con,
    rx_non,
    rx_ack,
    rx_rst,
    retx_timeout,
    retx_window,
    cmd_reliable_send,
    cmd_unreliable_send,
    cmd_accept,
    cmd_cancel,
};

/**
 * @brief Request/response layer event:
 * https://datatracker.ietf.org/doc/html/draft-ietf-lwig-coap-06#section-2.5.1
 * 
 */
enum class rr_event {
    rx_con,
    rx_non,
    rx_ack,
    rx_rst,
    fail,
    idle,
};

/**
 * @brief Request/response layer server state:
 * https://datatracker.ietf.org/doc/html/draft-ietf-lwig-coap-06#section-2.5.1
 * 
 */
enum class rr_server_state {
    idle,
    serving,
    separate,
};

enum class rr_server_event {
    rx_con,
    rx_non,
    rx_ack,
    rx_rst,
    fail,
    idle,
    emptyack,
    separate,
    piggybacked,
};

/**
 * @brief Request/response layer client state:
 * https://datatracker.ietf.org/doc/html/draft-ietf-lwig-coap-06#section-2.5.1
 * 
 */
enum class rr_client_state {
    idle,
    waiting
};

}

#endif