#ifndef NTH_COAP_LOG_H
#define NTH_COAP_LOG_H

#include "nth/io/log.h"
#include "nth/coap/packet.h"
#include "nth/coap/hal.h"
#include <iostream>

namespace nth::coap {
namespace imp {
inline constexpr auto unknown_str = "<unknown>"; // String for any unexpected / unknown value
}

/**
 * @brief Converts an error enumeration value to a string representation.
 * 
 * @param e Error
 * @return String representation
 */
constexpr auto str_err(error e)
{
    switch (e) {
        case error::ok:                   return "ok";
        case error::invalid_version:      return "invalid_version";
        case error::invalid_token_length: return "invalid_token_length";
        case error::invalid_option:       return "invalid_option";
        case error::too_short_header:     return "too_short_header";
        case error::too_short_token:      return "too_short_token";
        case error::too_short_option:     return "too_short_option";
        case error::too_short_payload:    return "too_short_payload";
        case error::too_small_buffer:     return "too_small_buffer";
        default: return imp::unknown_str;
    }
}

/**
 * @brief Converts an option number enumeration value to a string representation.
 * 
 * @param num Option number
 * @return String representation
 */
constexpr auto str_opt(option_num num)
{
    switch (num) {
        case option_num::if_match:          return "If-Match";
        case option_num::uri_host:          return "Uri-Host";
        case option_num::etag:              return "ETag";
        case option_num::if_none_match:     return "If-None-Match";
        case option_num::observe:           return "Observe";
        case option_num::uri_port:          return "Uri-Port";
        case option_num::location_path:     return "Location-Path";
        case option_num::oscore:            return "OSCORE";
        case option_num::uri_path:          return "Uri-Path";
        case option_num::content_format:    return "Content-Format";
        case option_num::max_age:           return "Max-Age";
        case option_num::uri_query:         return "Uri-Query";
        case option_num::accept:            return "Accept";
        case option_num::location_query:    return "Location-Query";
        case option_num::block2:            return "Block2";
        case option_num::block1:            return "Block1";
        case option_num::size2:             return "Size2";
        case option_num::proxy_uri:         return "Proxy-Uri";
        case option_num::proxy_scheme:      return "Proxy-Scheme";
        case option_num::size1:             return "Size1";
        case option_num::no_response:       return "No-Response";
        default: return imp::unknown_str;
    }
}

/**
 * @brief Converts a message type enumeration value to a string representation.
 * 
 * @param type Message type
 * @return Sstring representation
 */
constexpr auto str_msg_type(msg_type type)
{
    switch (type) {
        case msg_type::con: return "CON";
        case msg_type::non: return "NON";
        case msg_type::ack: return "ACK";
        case msg_type::rst: return "RST";
        default: return imp::unknown_str;
    }
}

/**
 * @brief Converts a message code enumeration value to a string representation.
 * 
 * @param code Message code
 * @return String representation
 */
constexpr auto str_msg_code(msg_code code)
{
    switch (code) {
        case msg_code::empty:                        return "Empty";
        case msg_code::get:                          return "GET";
        case msg_code::post:                         return "POST";
        case msg_code::put:                          return "PUT";
        case msg_code::delete_:                      return "DELETE";
        case msg_code::fetch:                        return "FETCH";
        case msg_code::patch:                        return "PATCH";
        case msg_code::ipatch:                       return "iPATCH";
        case msg_code::created:                      return "Created";
        case msg_code::deleted:                      return "Deleted";
        case msg_code::valid:                        return "Valid";
        case msg_code::changed:                      return "Changed";
        case msg_code::content:                      return "Content";
        case msg_code::bad_request:                  return "Bad Request";
        case msg_code::unauthorized:                 return "Unauthorized";
        case msg_code::bad_option:                   return "Bad Option";
        case msg_code::forbidden:                    return "Forbidden";
        case msg_code::not_found:                    return "Not Found";
        case msg_code::method_not_allowed:           return "Method Not Allowed";
        case msg_code::not_acceptable:               return "Not Acceptable";
        case msg_code::precondition_failed:          return "Precondition Failed";
        case msg_code::request_entity_too_large:     return "Request Entity Too Large";
        case msg_code::unsupported_content_format:   return "Unsupported Content-Format";
        case msg_code::internal_server_error:        return "Internal Server Error";
        case msg_code::not_implemented:              return "Not Implemented";
        case msg_code::bad_gateway:                  return "Bad Gateway";
        case msg_code::service_unavailable:          return "Service Unavailable";
        case msg_code::gateway_timeout:              return "Gateway Timeout";
        case msg_code::proxying_not_supported:       return "Proxying Not Supported";
        default: return imp::unknown_str;
    }
}

/**
 * @brief Converts a message layer state enumeration value to a string representation.
 * 
 * @param code Message layer state
 * @return String representation
 */
constexpr auto str_msg_state(msg_state st)
{
    switch (st) {
        case msg_state::closed:         return "CLOSED";
        case msg_state::ack_pending:    return "ACK_PENDING";
        case msg_state::reliable_tx:    return "RELIABLE_TX";
        default: return imp::unknown_str;
    }
}

/**
 * @brief Converts a message layer event enumeration value to a string representation.
 * 
 * @param code Message layer event
 * @return String representation
 */
constexpr auto str_msg_event(msg_event ev)
{
    switch (ev) {
        case msg_event::rx_con:                 return "RX_CON";
        case msg_event::rx_non:                 return "RX_NON";
        case msg_event::rx_ack:                 return "RX_ACK";
        case msg_event::rx_rst:                 return "RX_RST";
        case msg_event::retx_timeout:           return "TIMEOUT(RETX_TIMEOUT)";
        case msg_event::retx_window:            return "TIMEOUT(RETX_WINDOW)";
        case msg_event::cmd_reliable_send:      return "M_CMD(reliable_send)";
        case msg_event::cmd_unreliable_send:    return "M_CMD(unreliable_send)";
        case msg_event::cmd_accept:             return "M_CMD(accept)";
        case msg_event::cmd_cancel:             return "M_CMD(cancel)";
        default: return imp::unknown_str;
    }
}

/**
 * @brief Converts a request/response layer event enumeration value to a string representation.
 * 
 * @param code Request/response layer event
 * @return String representation
 */
constexpr auto str_rr_event(rr_event ev)
{
    switch (ev) {
        case rr_event::rx_con:  return "RR_EVT(rx)[is CON]";
        case rr_event::rx_non:  return "RR_EVT(rx)[is NON]";
        case rr_event::rx_ack:  return "RR_EVT(rx)[is ACK]";
        case rr_event::rx_rst:  return "RR_EVT(rx)[is RST]";
        case rr_event::fail:    return "RR_EVT(fail)";
        default: return imp::unknown_str;
    }
}

/**
 * @brief Converts a request/response layer server state enumeration value to a string representation.
 * 
 * @param code Request/response layer server state
 * @return String representation
 */
constexpr auto str_rr_server_state(rr_server_state st)
{
    switch (st) {
        case rr_server_state::idle: return "IDLE";
        case rr_server_state::serving: return "SERVING";
        case rr_server_state::separate: return "SEPARATE";
        default: return imp::unknown_str;
    }
}

/**
 * @brief Converts a request/response layer server state enumeration value to a string representation.
 * 
 * @param code Request/response layer server state
 * @return String representation
 */
constexpr auto str_rr_client_state(rr_client_state st)
{
    switch (st) {
        case rr_client_state::idle: return "IDLE";
        case rr_client_state::waiting: return "WAITING";
        default: return imp::unknown_str;
    }
}

// struct Log {

//     Log(LogLevel lvl, bool enabled) : enable{enabled}
//     {
//         if (enable) {
//             mtx.lock();
//             switch (lvl) 
//             {
//             case log_lvl_non: break;
//             case log_lvl_dbg: std::cout << K_BLU << "<dbg> "; break;
//             case log_lvl_inf: std::cout << K_NRM << "<inf> "; break;
//             case log_lvl_wrn: std::cout << K_YEL << "<wrn> "; break;
//             case log_lvl_err: std::cout << K_RED << "<err> "; break;
//             }
//         }
//     }

//     ~Log()
//     {
//         if (enable) {
//             std::cout << K_NRM << std::endl;
//             mtx.unlock();
//         }
//     }

//     template<class T>
//     Log& operator<<(const T &msg) 
//     {
//         if (enable)
//             std::cout << msg;
//         return *this;
//     }

// private:
//     const bool enable;
//     inline static Mutex mtx;
// };

// struct log  : Log { log(bool enable = true)  : Log{log_lvl_non, enable} {} };
// struct logd : Log { logd(bool enable = true) : Log{log_lvl_dbg, enable} {} };
// struct logi : Log { logi(bool enable = true) : Log{log_lvl_inf, enable} {} };
// struct logw : Log { logw(bool enable = true) : Log{log_lvl_wrn, enable} {} };
// struct loge : Log { loge(bool enable = true) : Log{log_lvl_err, enable} {} };

// inline std::ostream& operator<<(std::ostream &os, const Packet &pkt)
// {
//     if constexpr (imp::log_packet_raw_hex) {
//         log_hex(pkt.data(), pkt.size());
//         printf("+----------------------------+\n");
//     }
//     printf("| v.%u - %3s [0x%04x]         |\n",
//         pkt.get_ver(),
//         str_type(pkt.get_type()),
//         pkt.get_id());

//     printf("| %u.%02u - %-19s |\n",
//         pkt.get_code_class(), 
//         pkt.get_code_detail(), 
//         str_code(pkt.get_code()));

//     const auto tok = pkt.get_token();
//     if (tok.size()) {
//         printf("| Token - 0x");
//         for (auto b : tok)
//             printf("%02x", b);
//         printf(" %*s\n", int(17 - tok.size() * 2), "|");
//     }

//     const auto payload = pkt.get_payload();
//     if (payload.size()) {
//         printf("+----------------------------+\n");
//         printf("| Payload                    |\n");
//         utl::log_hex(payload.data(), payload.size());
//     }

//     for (const auto [dat, len, num] : pkt.options()) {
//         printf("+----------------------------+\n");
//         printf("| [%u] \"%s\" %*s\n", num, str_opt(num), int(22 - utl::ilen(num) - strlen(str_opt(num))), " |");
//         utl::log_hex(dat, len);
//     }
//     return os;
// }

// inline std::ostream& operator<<(std::ostream &os, const Address &addr)
// {
//     switch (addr.type) 
//     {
//     case addr_ipv4:
//         printf("%u.%u.%u.%u:%u", addr.data.ipv4[0], addr.data.ipv4[1], addr.data.ipv4[2], addr.data.ipv4[3], addr.port);
//         break;
//     case addr_ipv6:
//         // TODO
//         break;
//     case addr_unspec:
//         printf("<unspecified>");
//         break;
//     default:
//         printf("<unknown>");
//         break;
//     }
//     return os;
// }

}

#endif