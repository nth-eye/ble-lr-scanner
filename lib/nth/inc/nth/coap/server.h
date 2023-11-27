#ifndef NTH_COAP_SERVER_H
#define NTH_COAP_SERVER_H

#include "nth/coap/session.h"
// #include "nth/coap/resource.h"

namespace nth::coap {

/**
 * @brief Actually not a "server", but server FSM session, handler
 * for a single endpoint connection. Responsible only for parsing
 * responses, provided to it, and for transmission (that's why socket
 * argument needed). Handles retransmission in case of reliable (CON)
 * message.
 * 
 */
struct server : public session<server> {
    constexpr bool idle() const;
    constexpr void advance();
    constexpr void separate(ispan payload);
    constexpr void piggybacked(ispan payload);
    constexpr void emptyack();
    // void respond(msg_code code, ispan payload);
    // void respond(msg_code code);
    // void respond();
private:
    constexpr void handle_rrevt(rr_event ev);
    constexpr void process(rr_server_event ev);
    constexpr void process_idle(rr_server_event ev);
    constexpr void process_serving(rr_server_event ev);
    constexpr void process_separate(rr_server_event ev);
private:
    rr_server_state state = rr_server_state::idle;
    word bckp_id;
    byte bckp_tok[8];
    byte bckp_tkl;
    bool confirmable;

    using dispatch_fn = void (server::*)(rr_server_event);

    static constexpr dispatch_fn dispatch_table[] = {
        &server::process_idle, 
        &server::process_serving, 
        &server::process_separate,
    };
    friend session<server>;
};

// ANCHOR Definitions

constexpr void server::advance()
{
    session::advance();
}

constexpr void server::separate(ispan payload)
{
#if (ENABLE_PACKET)
    pkt.setup(msg_type::ack, msg_code::content, 0, {bckp_tok, bckp_tkl});
    pkt.set_payload(payload);
#endif
    process(rr_server_event::separate); 
}

constexpr void server::piggybacked(ispan payload)
{
#if (ENABLE_PACKET)
    pkt.clear_body();
    pkt.set_code(msg_code::content);
    pkt.set_payload(payload);
#endif
    process(rr_server_event::piggybacked);   
}

constexpr void server::emptyack()
{
#if (ENABLE_PACKET)
    bckp_tkl = pkt.get_tkl();
    std::copy_n(pkt.get_token().data(), bckp_tkl, bckp_tok);
    pkt.setup(msg_type::ack, msg_code::empty, pkt.get_id(), {});
#endif
    process(rr_server_event::emptyack);
}

constexpr void server::handle_rrevt(rr_event ev)
{
    process(rr_server_event(ev));
}

constexpr void server::process(rr_server_event ev)
{
    (this->*dispatch_table[+state])(ev);
}

constexpr void server::process_idle(rr_server_event ev)
{
    switch (ev) 
    {
    case rr_server_event::rx_con: 
        state = rr_server_state::serving; 
        confirmable = true; 
    break;
    case rr_server_event::rx_non: 
        state = rr_server_state::separate; 
        confirmable = false; 
    break;
    default: 
        app_handler(app_event_type::error, {.err = {error::unexpected_event}});
    }
}

constexpr void server::process_serving(rr_server_event ev)
{
    switch (ev) 
    {
    case rr_server_event::emptyack: 
        state = rr_server_state::separate;
        session::process(msg_event::cmd_accept);
    break;
    case rr_server_event::piggybacked: 
        state = rr_server_state::idle;
        session::process(msg_event::cmd_accept);
    break;
    default: 
        app_handler(app_event_type::error, {.err = {error::unexpected_event}});
    }
}

constexpr void server::process_separate(rr_server_event ev)
{
    switch (ev) 
    {
    case rr_server_event::separate: 
        state = rr_server_state::idle;
        session::process(confirmable ? 
            msg_event::cmd_reliable_send : 
            msg_event::cmd_unreliable_send); 
    break;
    default: 
        app_handler(app_event_type::error, {.err = {error::unexpected_event}});
    }
}

constexpr bool server::idle() const
{
    return state == rr_server_state::idle;
}

// SECTION OLD

// void Server::respond(Code code, span payload)
// {
//     pack.clear_body();
//     pack.set_code(code);
//     pack.set_payload(payload);
//     respond();
// }

// void Server::respond(Code code)
// {
//     pack.clear_body();
//     pack.set_code(code);
//     respond();
// }

// void Server::respond()
// {
//     switch (state) 
//     {
//     case server_st_serving:
//         Session::process(ev_cmd_accept);
//     break;
//     case server_st_separate:
//         Session::process(rsp_confirmable ? ev_cmd_reliable_send : ev_cmd_unreliable_send); 
//     break;
//     default:
//         loge() << "illegal server state " << state;
//     }
//     state = server_st_idle;
// }

// Resource* find(ResourceList resources, const Packet &pack)
// {
//     Resource *res = nullptr;

//     for (const auto opt : pack.options(option_num_uri_path)) {

//         res = nullptr;

//         const std::string_view path_part = {reinterpret_cast<const char*>(opt.dat), opt.len};

//         for (auto &tmp : resources) {
//             if (tmp.path == path_part) {
//                 res = &tmp;
//                 resources = tmp.sub;
//                 break;
//             }
//         }
//         if (!res)
//             break;
//     }
//     return res;
// }

// !SECTION OLD

}

#endif
