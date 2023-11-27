#ifndef NTH_COAP_SESSION_H
#define NTH_COAP_SESSION_H

#include "nth/coap/packet.h"
#include "nth/coap/hal.h"
#include "nth/coap/log.h"

#define ENABLE_PACKET   true
#define STORE_RREVT     true

namespace nth::coap {

/**
 * @brief Message layer FSM for parsing packets on reception and
 * handling response. In case of reliable (CON) message, it will 
 * automatically retransmit when advanced. Derived classes 
 * (server and client) are responsible for constructing packets 
 * and triggering M_CMD events.
 * 
 * @tparam T CRTP derived class
 */
template<class T>
struct session {
    constexpr void init(packet_view storage);
    constexpr void feed(ispan input);
    constexpr bool closed() const;
    constexpr bool finished() const;
protected:
    constexpr void advance();
    constexpr void process_rrevt(rr_event ev);
    constexpr void process(msg_event ev);
private:
#if (STORE_RREVT)
    constexpr void process_closed(msg_event ev);
    constexpr void process_ackpending(msg_event ev);
    constexpr void process_reliabletx(msg_event ev);
#else
    constexpr rr_event process_closed(msg_event ev);
    constexpr rr_event process_ackpending(msg_event ev);
    constexpr rr_event process_reliabletx(msg_event ev);
#endif
protected:
#if (ENABLE_PACKET)
    packet_view pkt;
#endif
private:
    msg_state   state = msg_state::closed;
    timestamp   retx_ts;
    int         retx_count;
#if (STORE_RREVT)
    rr_event    rrevt;
#endif
    bool        rx_pending;
#if (STORE_RREVT)
    using dispatch_fn = void (session::*)(msg_event);
#else
    using dispatch_fn = rr_event (session::*)(msg_event);
#endif
    static constexpr dispatch_fn dispatch_table[] = {
        &session::process_closed, 
        &session::process_ackpending, 
        &session::process_reliabletx,
    };
};

template<class T>
constexpr void session<T>::init(packet_view pkt_storage)
{
    pkt = pkt_storage;
    pkt.clear();
}

template<class T>
constexpr void session<T>::feed(ispan input)
{
    rx_pending = pkt.resize(input);
}

template<class T>
constexpr bool session<T>::closed() const
{
    return state == msg_state::closed; 
}

template<class T>
constexpr bool session<T>::finished() const
{
    return state == msg_state::closed && static_cast<const T*>(this)->idle(); 
}

template<class T>
constexpr void session<T>::advance()
{
    if (rx_pending) {
        rx_pending = false;
#if (ENABLE_PACKET)
        error ret = pkt.parse();
        
        switch (ret) 
        {
        case error::ok:
            switch (msg_type(pkt.get_type())) {
                case msg_type::con: process(msg_event::rx_con); return;
                case msg_type::non: process(msg_event::rx_non); return;
                case msg_type::ack: process(msg_event::rx_ack); return;
                case msg_type::rst: process(msg_event::rx_rst); return;
            }
            return;
        case error::invalid_version:
        case error::invalid_token_length:
        case error::invalid_option:
        case error::too_short_header:
        case error::too_short_token:
        case error::too_short_option:
        case error::too_short_payload:
            app_handler(app_event_type::error, {.err = {error::packet_parsing_error}});
        break;
        default:
            app_handler(app_event_type::error, {.err = {error::unreachable}});
        }
        return;
#endif
    }

    if (state == msg_state::reliable_tx) {
        if (retx_ts <= app_get_time()) {
            retx_ts += imp::ack_timeout << retx_count;
            if (retx_count++ < imp::max_retransmit_cnt) {
                process(msg_event::retx_timeout);
            } else {
                process(msg_event::retx_window);
                retx_count = 0;
            }
        }
    }
}

template<class T>
constexpr void session<T>::process_rrevt(rr_event ev)
{
    if (ev != rr_event::idle)
        static_cast<T*>(this)->handle_rrevt(ev);
}

template<class T>
constexpr void session<T>::process(msg_event ev)
{
#if (STORE_RREVT)
    (this->*dispatch_table[+state])(ev);
    process_rrevt(rrevt);
#else
    process_rrevt((this->*dispatch_table[+state])(ev));
#endif
}

template<class T>
constexpr void session<T>::process_closed(msg_event ev)
{
    switch (ev) 
    {
    case msg_event::rx_con:
        state = msg_state::ack_pending;
#if (STORE_RREVT)
        rrevt = rr_event::rx_con;
#else
        return rr_event::rx_con;
#endif
    break;
    case msg_event::rx_non:
#if (STORE_RREVT)
        rrevt = rr_event::rx_non;
#else
        return rr_event::rx_non;
#endif
    break;
    case msg_event::rx_ack:
    break;
    case msg_event::rx_rst:
        app_handler(app_event_type::remove_observer); // FIXME   
    break;
    case msg_event::cmd_reliable_send:
        state = msg_state::reliable_tx;
        retx_ts = app_get_time() + imp::ack_timeout;
#if (ENABLE_PACKET)
        pkt.set_type(msg_type::con);
        // pkt.set_id(get_next_id()); // FIXME
#endif
        app_handler(app_event_type::transmission); // FIXME
    break;
    case msg_event::cmd_unreliable_send:
#if (ENABLE_PACKET)
        pkt.set_type(msg_type::non);
        // pkt.set_id(get_next_id()); // FIXME
#endif
        app_handler(app_event_type::transmission); // FIXME
    break;
    default:
        app_handler(app_event_type::error, {.err = {error::unexpected_event}});
    }
#if !(STORE_RREVT)
    return rr_event::idle;
#endif
}

template<class T>
constexpr void session<T>::process_ackpending(msg_event ev)
{
    switch (ev) 
    {
    case msg_event::cmd_accept: 
        state = msg_state::closed;
#if (ENABLE_PACKET)
        pkt.set_type(msg_type::ack);
#endif
        app_handler(app_event_type::transmission); // FIXME
    break;
    default:
        app_handler(app_event_type::error, {.err = {error::unexpected_event}});
    }
#if !(STORE_RREVT)
    return rr_event::idle;
#endif
}

template<class T>
constexpr void session<T>::process_reliabletx(msg_event ev)
{
    switch (ev) 
    {
    case msg_event::rx_con:
        state = msg_state::ack_pending;
#if (STORE_RREVT)
        rrevt = rr_event::rx_con;
#else
        return rr_event::rx_con;
#endif
    break;
    case msg_event::rx_non:
        state = msg_state::closed;
#if (STORE_RREVT)
        rrevt = rr_event::rx_non;
#else
        return rr_event::rx_non;
#endif
    break;
    case msg_event::rx_ack:
        state = msg_state::closed;
#if (STORE_RREVT)
        rrevt = rr_event::rx_ack;
#else
        return rr_event::rx_ack;
#endif
    break;      
    case msg_event::rx_rst:
    case msg_event::retx_window:
        state = msg_state::closed;
#if (STORE_RREVT)
        rrevt = rr_event::fail;
#else
        return rr_event::fail;
#endif
    break;
    case msg_event::retx_timeout:
        app_handler(app_event_type::transmission); // FIXME
    break;
    case msg_event::cmd_cancel:
        state = msg_state::closed;
    break;
    default:
        app_handler(app_event_type::error, {.err = {error::unexpected_event}});
    }
#if !(STORE_RREVT)
    return rr_event::idle;
#endif
}

}

#endif