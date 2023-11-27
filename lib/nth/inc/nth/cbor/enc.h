#ifndef NTH_CBOR_ENC_H
#define NTH_CBOR_ENC_H

#include "nth/util/buffer.h"
#include "nth/util/literal.h"
#include "nth/util/string.h"
#include "nth/cbor/dec.h"

namespace nth::cbor {
namespace enc {

enum class arr : size_t     {};
enum class map : size_t     {};
enum class tag : uint64_t   {};
struct idat {};
struct itxt {};
struct iarr {};
struct imap {};
struct stop {};

}

template <size_t N = dynamic_extent>
struct encoder : buffer_handler<N> {

    using base = buffer_handler<N>;
    using base::base;

    // ANCHOR Helpers

    constexpr operator seq() const      { return {base::begin(), base::end()}; }
    constexpr seq_iter begin() const    { return {base::begin(), base::end()}; }
    constexpr seq_iter end() const      { return {}; }

    // ANCHOR Variadic interface

    template<class ...Args>
    constexpr err encode_(Args&&... args)
    {
        err e = err::ok;
        ((+(e = encode(std::forward<Args>(args)))) || ...);
        return e;
    }

    // ANCHOR Implicit interface

    constexpr err encode(integer auto val)      { return std::is_signed_v<decltype(val)> ? encode_sint(val): encode_uint(val); }
    constexpr err encode(boolean auto val)      { return encode_bool(val); }
    constexpr err encode(prim_t val)            { return encode_prim(val); }
#if (NTH_UTIL_HALF_NATIVE)
    constexpr err encode(std::float16_t val)    { return encode_half(val); }
    constexpr err encode(std::float32_t val)    { return encode_float(val); }
    constexpr err encode(std::float64_t val)    { return encode_double(val); }
#endif
    constexpr err encode(float val)             { return encode_float(val); }
    constexpr err encode(double val)            { return encode_double(val); }
    constexpr err encode(span val)              { return encode_data(val); }
    constexpr err encode(list val)              { return encode_data(val); }
    constexpr err encode(std::string_view val)  { return encode_text(val); }
    constexpr err encode(enc::arr val)          { return encode_arr(+val); }
    constexpr err encode(enc::map val)          { return encode_map(+val); }
    constexpr err encode(enc::tag val)          { return encode_tag(+val); }
    constexpr err encode(enc::idat)             { return encode_indef_dat(); }
    constexpr err encode(enc::itxt)             { return encode_indef_txt(); }
    constexpr err encode(enc::iarr)             { return encode_indef_arr(); }
    constexpr err encode(enc::imap)             { return encode_indef_map(); }
    constexpr err encode(enc::stop)             { return encode_break(); }

    // ANCHOR: Explicit interface

#if (NTH_UTIL_HALF_NATIVE)
    constexpr err encode_half(std::float16_t val)
    {
        if (val != val)
            return encode_nan();
        return encode_base(mt_simple | byte(prim_float_16), std::bit_cast<uint16_t>(val), 2);
    }
    constexpr err encode_float(std::float32_t val)
    {
        if (val != val)
            return encode_nan();
        if (val == std::float16_t(val))
            return encode_half(std::float16_t(val));
        return encode_base(mt_simple | byte(prim_float_32), std::bit_cast<uint32_t>(val), 4);
    }
    constexpr err encode_double(std::float64_t val)
    {
        if (val != val)
            return encode_nan();
        if (val == std::float32_t(val))
            return encode_float(std::float32_t(val));
        return encode_base(mt_simple | byte(prim_float_64), std::bit_cast<uint64_t>(val), 8);
    }
#else
    constexpr err encode_float(float val)
    {
        if (val != val)
            return encode_nan();

        auto u32 = std::bit_cast<uint32_t>(val);
        auto u16 = float_to_half(u32);

        if (val != std::bit_cast<float>(half_to_float(u16)))
            return encode_base(mt_simple | byte(prim_float_32), u32, 4);
        
        if ((u16 & 0x7fff) <= 0x7c00)
            return encode_base(mt_simple | byte(prim_float_16), u16, 2);

        return encode_nan();
    }
    constexpr err encode_double(double val)
    {
        if (val != val)
            return encode_nan();
        if (val == float(val))
            return encode_float(val);
        return encode_base(mt_simple | byte(prim_float_64), std::bit_cast<uint64_t>(val), 8);
    }
#endif
    constexpr err encode_prim(prim_t val)
    {
        return val < 24 || val > 31 ? encode_head(mt_simple, val) : err::invalid_simple;
    }
    constexpr err encode_sint(int64_t val)
    {
        uint64_t ui = val >> 63;
        return encode_head(mt_t(ui & 0x20), ui ^ val);
    }
    constexpr err encode_uint(uint64_t val)         { return encode_head(mt_uint, val); }
    constexpr err encode_text(std::string_view val) { return encode_string(mt_text, val.data(), val.size()); }
    constexpr err encode_text(span val)             { return encode_string(mt_text, val.data(), val.size()); }
    constexpr err encode_text(list val)             { return encode_string(mt_text, val.begin(), val.size()); }
    constexpr err encode_data(span val)             { return encode_string(mt_data, val.data(), val.size()); }
    constexpr err encode_data(list val)             { return encode_string(mt_data, val.begin(), val.size()); }
    constexpr err encode_bool(bool val)             { return encode_byte(mt_simple | (prim_false + val)); }
    constexpr err encode_arr(size_t size)           { return encode_head(mt_arr, size); }
    constexpr err encode_map(size_t size)           { return encode_head(mt_map, size); }
    constexpr err encode_tag(uint64_t val)          { return encode_head(mt_tag, val); }
    constexpr err encode_indef_dat()                { return encode_byte(mt_data | byte(ai_indef)); }
    constexpr err encode_indef_txt()                { return encode_byte(mt_text | byte(ai_indef)); }
    constexpr err encode_indef_arr()                { return encode_byte(mt_arr | byte(ai_indef)); }
    constexpr err encode_indef_map()                { return encode_byte(mt_map | byte(ai_indef)); }
    constexpr err encode_break()                    { return encode_byte(0xff); }
private:
    constexpr err encode_nan()
    { 
        return encode_base(mt_simple | byte(prim_float_16), 0x7e00, 2); 
    }
    constexpr err encode_byte(byte b)
    {
        if (base::size() < base::capacity()) {
            base::push_back(b);
            return err::ok;
        }
        return err::no_memory;
    }
    constexpr err encode_base(byte start, uint64_t val, size_t ai_len, size_t add_len = 0)
    {
        if (base::size() + ai_len + add_len + 1 > base::capacity())
            return err::no_memory;
        base::push_back(start);
        for (int i = 8 * ai_len - 8; i >= 0; i -= 8)
            base::push_back(val >> i);
        return err::ok;
    }
    constexpr err encode_head(mt_t mt, uint64_t val, size_t add_len = 0)
    {
        byte ai;
        if (val <= ai_0)
            ai = val;
        else if (val <= 0xff)
            ai = ai_1;
        else if (val <= 0xffff)
            ai = ai_2;
        else if (val <= 0xffffffff)
            ai = ai_4;
        else
            ai = ai_8;
        size_t ai_len = (ai <= ai_0) ? 0 : bit(ai - ai_1);
        return encode_base(mt | ai, val, ai_len, add_len);
    }
    template<class BytePtr>
    constexpr err encode_string(mt_t mt, BytePtr data, size_t len)
    {
        err e = encode_head(mt, len, len);
        if (e == err::ok && len)
            base::push_back({data, len});
        return e;
    }
};

using view = buffer_interface<encoder>;
#if (NTH_UTIL_BUFFER_CONST_INTERFACE)
using const_view = const_buffer_interface<encoder>;
#endif

namespace literals {

constexpr auto operator"" _arr(unsigned long long x) { return enc::arr(x); }
constexpr auto operator"" _map(unsigned long long x) { return enc::map(x); }
constexpr auto operator"" _tag(unsigned long long x) { return enc::tag(x); }
constexpr auto operator"" _pri(unsigned long long x) { return prim_t(x); }

}

inline constexpr enc::idat idat;
inline constexpr enc::itxt itxt;
inline constexpr enc::iarr iarr;
inline constexpr enc::imap imap;
inline constexpr enc::stop stop;

}

#endif