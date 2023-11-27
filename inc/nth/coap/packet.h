#ifndef NTH_COAP_PACKET_H
#define NTH_COAP_PACKET_H

#include "nth/coap/option.h"
#include "nth/coap/config.h"
#include "nth/util/bit.h"

namespace nth::coap {

struct packet_view {

    // ANCHOR Member types

    using value_type                = byte;
    using size_type                 = size_t;
    using difference_type           = ptrdiff_t;
    using pointer                   = value_type*;
    using const_pointer             = const value_type*;
    using reference                 = value_type&;
    using const_reference           = const value_type&;
    using universal_reference       = value_type&&;
    using iterator                  = pointer;
    using const_iterator            = const_pointer;

    // ANCHOR Constructors

    constexpr packet_view() = delete;
    constexpr packet_view(ospan buf) : buf{buf} 
    { 
        assert(buf.size() >= min_hdl);
        assert(buf.size() <= max_pkl); 
    }

    // ANCHOR Capacity

    constexpr size_type capacity() const    { return buf.size(); }
    constexpr size_type max_size() const    { return buf.size(); }
    constexpr size_type size() const        { return len; }
    constexpr bool empty() const            { return len == 0; }
    constexpr bool full() const             { return len == capacity(); }

    // ANCHOR Iterators

    constexpr pointer begin()               { return buf.data(); }
    constexpr const_pointer begin() const   { return buf.data(); }
    constexpr const_pointer cbegin() const  { return begin(); }
    constexpr pointer end()                 { return buf.data() + len; }
    constexpr const_pointer end() const     { return buf.data() + len; }
    constexpr const_pointer cend() const    { return end(); }

    // ANCHOR Access

    constexpr reference operator[](size_type i)             { assert(capacity() > i); return buf[i]; }
    constexpr const_reference operator[](size_type i) const { assert(capacity() > i); return buf[i]; }
    constexpr reference front()                             { assert(!empty()); return buf[0]; }
    constexpr const_reference front() const                 { assert(!empty()); return buf[0]; }
    constexpr reference back()                              { assert(!empty()); return buf[len - 1]; }
    constexpr const_reference back() const                  { assert(!empty()); return buf[len - 1]; }
    constexpr pointer data()                                { return buf.data(); }
    constexpr const_pointer data() const                    { return buf.data(); }

    // ANCHOR CoAP specific

    constexpr bool resize(size_t new_size);
    constexpr bool resize(ispan new_content);
    constexpr bool setup(msg_type t, msg_code c, word id, ispan tok);
    constexpr bool strip();
    constexpr void flush();
    constexpr void clear_body();
    constexpr void clear();
    constexpr error parse();
    constexpr error parse(ispan raw);

    constexpr byte get_ver() const                      { return (buf[0] & 0xc0) >> 6; }
    constexpr byte get_type() const                     { return (buf[0] & 0x30) >> 4; }
    constexpr byte get_tkl() const                      { return (buf[0] & 0x0f) >> 0; }
    constexpr byte get_code() const                     { return  buf[1]; }
    constexpr byte get_code_class() const               { return  buf[1] >> 5; }
    constexpr byte get_code_detail() const              { return  buf[1] & 0x1f; }
    constexpr word get_id() const                       { return (buf[2] << 8) | buf[3]; }
    constexpr auto get_token() const                    { return ispan{tok_cbegin(), get_tkl()}; }
    constexpr auto get_payload() const                  { return ispan{opt_cend(), end()}; }
    constexpr auto get_options() const                  { return      option_range{opt_cbegin(), opt_cend()}; }
    constexpr auto get_options(option_num num) const    { return same_option_range{opt_get(num), opt_cend()}; }

    constexpr void set_ver(int v) const         { buf[0] = (buf[0] & ~0xc0) | ((int(v) & 0x3) << 6); }
    constexpr void set_type(msg_type t) const   { buf[0] = (buf[0] & ~0x30) | ((int(t) & 0x3) << 4); }
    constexpr void set_code(msg_code c) const   { buf[1] = +c; }
    constexpr void set_id(word id) const        { buf[2] = id >> 8; buf[3] = id; }
    constexpr bool set_token(ispan tok);
    constexpr bool set_payload(ispan pld);

    constexpr option opt_next(option opt) const;
    constexpr option opt_get(option_num num) const;
    constexpr bool opt_is_set(option_num num) const;
    constexpr bool opt_delete(option_num num);
    constexpr bool opt_insert(option opt);
private:
    constexpr pointer tok_begin()                   { return begin() + min_hdl; }
    constexpr pointer opt_begin()                   { return begin() + min_hdl + get_tkl(); }
    constexpr pointer opt_end()                     { return end() - pld_offset; }
    constexpr const_pointer tok_cbegin() const      { return begin() + min_hdl; }
    constexpr const_pointer opt_cbegin() const      { return begin() + min_hdl + get_tkl(); }
    constexpr const_pointer opt_cend() const        { return end() - pld_offset; }
    constexpr error parse_option(const_pointer& p, word& delta);
protected:
    static constexpr size_t min_hdl = 4;        // Minimum header length
    static constexpr size_t max_tkl = 8;        // Maximum token length
    static constexpr size_t max_pkl = 65535;    // Maximum packet length
    static constexpr byte pld_mark = 0xff;      // Payload marker
private:
    ospan buf = {};
    word len = 0;
    word pld_offset = 0;
#if (NTH_COAP_OPTION_CACHE)
    std::array<word, option_bit_> opt_offset = {};
    std::array<word, option_bit_> opt_deltas = {};
    std::array<word, bytes_in_bits(option_bit_)> opt_mask = {};
#else
    bool opt_unknown_present = false;
#endif
};

/**
 * @brief Self-contained packet with fixed-size array. Absolute 
 * minimum, which can fit CoAP fixed header and token of maximum 
 * length, is 12 bytes. Although sometimes it's sufficient to have 
 * only header without token, so minimum valid length is 4. Maximum
 * packet length is defined by CoAP standard as 65535.
 * 
 * @tparam N Packet size in bytes
 */
template<size_t N = imp::default_packet_size>
struct packet : packet_view {
    static_assert(N >= packet_view::min_hdl && 
                  N <= packet_view::max_pkl);
    constexpr packet() : packet_view{buf}   { }
    constexpr packet(ispan data) : packet{} { parse(data); }
private:
    std::array<byte, N> buf = {};
};

// ANCHOR Definitions

/**
 * @brief Resize buffer without changing content. If new size is 
 * greater than capacity, ignore the request and return false.
 * 
 * @param new_size New size in bytes
 * @return true on success
 */
constexpr bool packet_view::resize(size_t new_size)
{
    if (new_size <= capacity()) {
        len = word(new_size);
        return true;
    }
    return false;
}

/**
 * @brief Resize buffer and copy new content into it without
 * parsing. If new size is greater than capacity, ignore the 
 * request and return false.
 * 
 * @param new_content New raw packet content
 * @return true on success
 */
constexpr bool packet_view::resize(ispan new_content)
{
    if (resize(new_content.size())) {
        std::ranges::copy(new_content, begin());
        return true;
    }
    return false;
}

/**
 * @brief Setup packet with given header info. Version is hardcoded 
 * in library configs and takes 2 bits. If token length is greater 
 * than 8 or packet_view isn't initialized properly, setup will fail. 
 * 
 * @param t Message type, 2 bits
 * @param c Message code, 8 bits
 * @param id Message ID, 16 bits
 * @param tok Token, length takes 4 bits, data is 8 bytes max
 * @return true on success
 */
constexpr bool packet_view::setup(msg_type t, msg_code c, word id, ispan tok)
{
    if (tok.size() > max_tkl)
        return false;
    if (resize(min_hdl + tok.size()) == false)
        return false;
    buf[0] = byte((imp::version << 6) | ((int(t) & 0x3) << 4) | tok.size());
    buf[1] = byte(c);
    buf[2] = byte(id >> 8);
    buf[3] = byte(id);
    std::ranges::copy(tok, begin() + min_hdl);
    return true;
}

/**
 * @brief Leave only header, but preserving cached data.
 * 
 * @return true on success
 */
constexpr bool packet_view::strip()
{
    return resize(min_hdl + get_tkl());
}

/**
 * @brief Delete cached data for payload, also for options 
 * if `NTH_COAP_OPTION_CACHE == true`.
 * 
 */
constexpr void packet_view::flush()
{
#if (NTH_COAP_OPTION_CACHE)
    opt_offset = {};
    opt_deltas = {};
    opt_mask = {};
#else
    opt_unknown_present = false;
#endif
    pld_offset = 0;
}

/**
 * @brief Delete options and payload. Removes all cached 
 * data and leaves only header.
 * 
 */
constexpr void packet_view::clear_body()
{
    flush();
    strip();
}

/**
 * @brief Zero out everything, including provided buffer.
 * 
 */
constexpr void packet_view::clear()
{
    flush();
    std::ranges::fill(*this, 0);
    len = 0;
}

/**
 * @brief Check packet validity, parse and cache payload and option's 
 * (if `NTH_COAP_OPTION_CACHE == true`) offsets. If provided buffer 
 * storage is smaller than minimal possible packet (4 bytes), then 
 * `error::too_short_header` will be returned.
 * 
 * @return Error status
 */
constexpr error packet_view::parse()
{
    using enum error;

    flush();
    
    if (size() < min_hdl)
        return too_short_header;

    if (get_ver() != imp::version)
        return invalid_version;
    
    if (get_tkl() > max_tkl)
        return invalid_token_length;

    if (get_tkl() > size() - min_hdl)
        return too_short_token;

    auto p = opt_cbegin();
    word delta = 0;

    while (p < end() && *p != pld_mark) {
        if (error e = parse_option(p, delta); e != ok)
            return e;
    }
    if (p == end() || ++p < end()) {
        pld_offset = word(end() - p);
        return ok;
    }
    return too_short_payload;
}

/**
 * @brief Store and parse bytes from provided buffer as CoAP message.
 * If raw buffer is greater than packet's own internally referenced 
 * memory buffer, then error is returned and no parsing begins.
 * 
 * @param raw External buffer to copy and parse
 * @return Error status
 */
constexpr error packet_view::parse(ispan raw)
{
    return resize(raw) ? parse() : error::too_small_buffer;
}

/**
 * @brief Parse next option from given start position and delta. If 
 * `NTH_COAP_OPTION_CACHE == true`, necessary info will be cached 
 * inside packet for fast access later with `opt_` functions. For any 
 * option, which is unknown to the implementation, only offset of the 
 * last one is saved as `option_num::unknown`. This special enum number 
 * can be used to check if any unknown option is present, but not to 
 * query them individually (except last one) with `opt_get()`. Otherwise, 
 * if `NTH_COAP_OPTION_CACHE == false`, only single flag will be stored, 
 * which indicates presence of unknown option. Quering `option_num::unknown` 
 * with `opt_get()` will just return invalid option in this case.
 * 
 * @param p Pointer to last parsed byte, for first call must be start of options
 * @param delta Previous option delta, for first call must be 0
 * @return Status 
 */
constexpr error packet_view::parse_option(const_pointer& p, word& delta)
{
    auto opt = opt_decode(p, end(), delta);

    if (opt_valid(opt) == false)
        return error::invalid_option;
    
    if (opt_edge(opt) > end())
        return error::too_short_option;

#if (NTH_COAP_OPTION_CACHE)
    if (opt.num != delta) {
        auto idx = +opt_num_to_bit(opt.num);
        set_arr_bit(opt_mask.data(), idx);
        opt_offset[idx] = p - opt_begin();
        opt_deltas[idx] = delta;
        delta = opt.num;
    }
#else
    if (opt_num_to_bit(opt.num) == option_bit::unknown)
        opt_unknown_present = true;
    delta = opt.num;
#endif
    p = opt_edge(opt);

    return error::ok;
}

/**
 * @brief Update token separately from other header components
 * except TKL. Preserves options and payload.
 * 
 * @param tok Token, 8 bytes maximum
 * @return true on succes
 */
constexpr bool packet_view::set_token(ispan tok)
{
    if (tok.size() > max_tkl)
        return false;

    auto diff = difference_type(tok.size() - get_tkl());
    auto endp = cend();

    if (resize(size() + diff) == false)
        return false;

    buf[0] &= 0xf0;
    buf[0] |= tok.size();

    if (diff > 0)
        std::copy_backward(opt_cbegin(), endp, opt_begin() + diff);
    else if (diff < 0) 
        std::copy(opt_cbegin(), endp, opt_begin() - diff);

    std::ranges::copy(tok, begin() + min_hdl);

    return true;
}

/**
 * @brief Replace old payload or delete if new is empty. Can be 
 * called without prior `setup()`, so header fields can be set later 
 * individually. If current size is less than 4 (uninitialized), 
 * this function reserves those header bytes.
 * 
 * @param pld Payload
 */
constexpr bool packet_view::set_payload(ispan pld)
{
    auto old_len = std::max(size(), size_type(4)) - (pld_offset + bool(pld_offset));

    if (resize(old_len + pld.size() + !pld.empty()) == false)
        return false;

    if (!pld.empty()) {
        auto pld_dst = begin() + old_len;
        *pld_dst++ = pld_mark;
        std::ranges::copy(pld, pld_dst);
    }
    pld_offset = word(pld.size());

    return true;
}

/**
 * @brief Decode next adjacent option. If input option is invalid or 
 * last within packet, default-constructed (invalid) will be returned.
 * 
 * @param opt Previous option
 * @return Decoded next option
 */
constexpr option packet_view::opt_next(option opt) const
{
    if (auto p = opt_edge(opt);
        p >= opt_cbegin() && 
        p <= opt_cend()) 
    {
        opt = opt_decode(p, opt_cend(), opt.num);
        return opt_edge(opt) <= opt_cend() ? opt : option{};
    }
    return {};
}

/**
 * @brief Get first option with given number. If `COAP_OPTION_CACHE 
 * == true`, packet must be parsed prior to this call, otherwise 
 * cache will be empty. For more info about `option_num::unknown` 
 * refer to the function `parse_option()`.
 * 
 * @param num Option number
 * @return Option
 */
constexpr option packet_view::opt_get(option_num num) const
{
#if (NTH_COAP_OPTION_CACHE)
    auto idx = +opt_num_to_bit(+num);
    if (!opt_is_set(num))
        return {};
    return opt_decode(opt_cbegin() + opt_offset[idx], opt_cend(), opt_deltas[idx]);
#else
    for (const auto tmp : get_options()) {
        if (tmp.num == +num)
            return tmp;
    }
    return {};
#endif
}

/**
 * @brief Check if option is present in packet. If `NTH_COAP_OPTION_CACHE 
 * == true`, packet must be parsed prior to this call, otherwise cache 
 * will be empty. For more info about `option_num::unknown` refer to 
 * the function `parse_option()`.
 * 
 * @param num Option number
 * @return true if present
 */
constexpr bool packet_view::opt_is_set(option_num num) const
{
#if (NTH_COAP_OPTION_CACHE)
    return get_arr_bit(opt_mask.data(), +opt_num_to_bit(+num));
#else
    if (num == option_num::unknown)
        return opt_unknown_present;
    for (const auto tmp : get_options()) {
        if (tmp.num == +num)
            return true;
    }
    return false;
#endif
}

/**
 * @brief Remove option with given number (including all repeated)
 * and adjust option cache (if used) and payload offset. Uses memmove.
 * 
 * @param num Option number
 * @return true on success
 */
constexpr bool packet_view::opt_delete(option_num num)
{
    option prev_opt;
    option next_opt;

    pointer src_ptr = nullptr;

    for (auto tmp : get_options()) {
        if (tmp.num == +num) {
            src_ptr = const_cast<pointer>(opt_edge(tmp));
        } else if (tmp.num > +num) {
            next_opt = tmp;
            break;
        } else if (tmp.num < +num) {
            prev_opt = tmp;
        }
    }
    if (src_ptr == nullptr)
        return false;

    auto dst_ptr = opt_valid(prev_opt) ? const_cast<pointer>(opt_edge(prev_opt)) : opt_begin();

    if (opt_valid(next_opt)) {
        word next_old_delta = next_opt.num - +num;
        word next_new_delta = next_opt.num - prev_opt.num;
        src_ptr -= opt_head_size(next_new_delta, next_opt.len) -
                   opt_head_size(next_old_delta, next_opt.len);
        opt_encode_head(src_ptr, next_new_delta, next_opt.len);
#if (NTH_COAP_OPTION_CACHE)
        auto idx = +opt_num_to_bit(next_opt.num);
        opt_offset[idx] = word(dst_ptr - opt_begin());
        opt_deltas[idx] = prev_opt.num;
#endif
    }
    std::copy(src_ptr, end(), dst_ptr);
    resize(size() - (src_ptr - dst_ptr));
#if (NTH_COAP_OPTION_CACHE)
    clr_arr_bit(opt_mask.data(), int(opt_num_to_bit(word(num))));
#endif
    return true;
}

/**
 * @brief Add option and adjust option cache (if used) and payload 
 * offset. If option with the same number already exists, new one 
 * will be appended even if it's not repeatable, so user should 
 * check this himself. 
 * 
 * @param opt Option to add
 * @return true on success
 */
constexpr bool packet_view::opt_insert(option opt)
{
    // 0. Find place to insert the new option.
    // 1. Calculate how much memory needed for the new option.
    // 2. Calculate how much memory needed for the subseqent option (delta can change).
    // 3. Calculate how much memory needed in total and check if packet has enough capacity.
    // 4. Encode the subseqent option with new delta in-place.
    // 5. Move subsequent options AND PAYLOAD to free memory for the new option.
    // 6. Encode the new option.

    option prev_opt;
    option next_opt;

    for (const auto tmp : get_options()) {
        if (tmp.num > opt.num) {
            next_opt = tmp;
            break;
        }
        prev_opt = tmp;
    }
    size_t opt_size = opt_total_size(opt.num - prev_opt.num, opt.len);
    size_t next_delta_diff_size = 0;
    size_t next_new_delta;

    if (opt_valid(next_opt)) {
        auto next_old_delta = next_opt.num - prev_opt.num;
        next_new_delta      = next_opt.num - opt.num;
        next_delta_diff_size = 
            opt_head_size(word(next_old_delta), next_opt.len) - 
            opt_head_size(word(next_new_delta), next_opt.len);
    }
    size_t new_size = size() + opt_size - next_delta_diff_size;

    if (new_size > capacity())
        return false;

    pointer opt_ptr = opt_valid(prev_opt) ? const_cast<pointer>(opt_edge(prev_opt)) : opt_begin();
    pointer dst_ptr = opt_ptr + opt_size;
    pointer src_ptr = opt_ptr + next_delta_diff_size;
    
    std::copy(src_ptr, end(), dst_ptr); // memmove(dst_ptr, src_ptr, end() - src_ptr);

    resize(new_size);

    if (opt_valid(next_opt)) {
#if (NTH_COAP_OPTION_CACHE)
        auto idx = +opt_num_to_bit(next_opt.num);
        opt_offset[idx] = word(dst_ptr - opt_begin());
        opt_deltas[idx] = opt.num;
#endif
        opt_encode_head(dst_ptr, word(next_new_delta), next_opt.len);
    }
#if (NTH_COAP_OPTION_CACHE)
    if (opt.num != prev_opt.num) {
        auto idx = +opt_num_to_bit(opt.num);
        opt_offset[idx] = word(opt_ptr - opt_begin());
        opt_deltas[idx] = prev_opt.num;
        set_arr_bit(opt_mask.data(), idx);
    }
#endif
    opt_encode(opt_ptr, opt.num - prev_opt.num, opt.len, opt.dat);

    return true;
}

}

#endif
