#ifndef NTH_CBOR_DEC_H
#define NTH_CBOR_DEC_H

#include "nth/util/half.h"
#include <tuple>
#if (NTH_UTIL_HALF_NATIVE)
#include <stdfloat>
#endif

#define NTH_CBOR_ITEM_NEW_API           true
#define NTH_CBOR_ITEM_FAILURE_DEFAULT   false
#define NTH_CBOR_ITEM_FAILURE_ASSERT    !NTH_CBOR_ITEM_FAILURE_DEFAULT

namespace nth::cbor {

using span = std::span<const byte>;
using list = std::initializer_list<byte>;
using pntr = const byte*;

struct seq_iter;
struct map_iter;
struct item;

/**
 * @brief CBOR major type (3 bits).
 * 
 */
enum mt_t {
    mt_uint     = 0 << 5,
    mt_nint     = 1 << 5,
    mt_data     = 2 << 5,
    mt_text     = 3 << 5,
    mt_arr      = 4 << 5,
    mt_map      = 5 << 5,
    mt_tag      = 6 << 5,
    mt_simple   = 7 << 5,
};

/**
 * @brief CBOR additional info (5 bits).
 * 
 */
enum ai_t {
    ai_0        = 23,
    ai_1        = 24,
    ai_2        = 25,
    ai_4        = 26,
    ai_8        = 27,
    ai_indef    = 31,
};

/**
 * @brief Enum for primitive (simple) values. Includes float 
 * markers which are used during en/decoding.
 * 
 */
enum prim_t : byte {
    prim_false      = 20,
    prim_true       = 21,
    prim_null       = 22,
    prim_undefined  = 23,
    prim_float_16   = 25,
    prim_float_32   = 26,
    prim_float_64   = 27,
};

/**
 * @brief Type for CBOR objects (not major type).
 * 
 */
enum type_t {
    type_uint,
    type_sint,
    type_data,
    type_text,
    type_array,
    type_map,
    type_tag,
    type_prim,
    type_floating,
    type_indef_data,
    type_indef_text,
#if (NTH_CBOR_ITEM_NEW_API)
    type_bool,
#endif
    type_invalid,
};

/**
 * @brief General codec errors.
 * 
 */
enum class err {
    ok,
    no_memory,
    out_of_bounds,
    reserved_ai,
    invalid_break,
    invalid_simple,
    invalid_indef_mt,
    invalid_indef_string,
};

/**
 * @brief CBOR sequence, read-only wrapper for traversal on-the-fly.
 * 
 */
struct seq : span {
    using span::span;
    constexpr seq_iter begin() const;
    constexpr seq_iter end() const;
};

namespace dec {

#if !(NTH_CBOR_ITEM_NEW_API)
using data = span;
#endif

/**
 * @brief String view with <cbor::byte> as underlying character type. 
 * May be expanded with additional "text"-type specific functionality, 
 * e.g comparison with std::string_view. This workwaround is necessary 
 * to make decode() constexpr, because reinterpret_cast is not allowed 
 * and it's not possible to create regular std::string_view from <byte*> 
 * during parsing. 
 * 
 */
struct text : std::basic_string_view<byte> {
    using base = std::basic_string_view<byte>;
    using base::base;
    friend constexpr bool operator==(const text& lhs, const std::string_view& rhs)
    {
        if (lhs.size() != rhs.size())
            return false;
        if (std::is_constant_evaluated()) {
            for (size_t i = 0; i < lhs.size(); ++i) {
                if (lhs[i] != rhs[i])
                    return false;
            }
            return true;
        }
        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    friend constexpr bool operator==(const std::string_view& lhs, const text& rhs)
    {
        return operator==(rhs, lhs);
    }
};

/**
 * @brief Sequence wrapper for CBOR indefinite byte and text strings.
 * 
 */
struct istr : seq {
    using seq::seq;
};

/**
 * @brief Sequence wrapper for CBOR array.
 * 
 */
struct arr : seq {
    constexpr arr(pntr head, pntr tail, size_t len) : seq{head, tail}, len{len} {}
    constexpr auto size() const     { return len; }
    constexpr bool indef() const    { return len == size_t(-1); } 
    constexpr item operator[](size_t i) const;
private:
    size_t len;
};

/**
 * @brief Sequence wrapper for CBOR map.
 * 
 */
struct map : arr {
    using arr::arr;
    constexpr map_iter begin() const;
    constexpr map_iter end() const;
    constexpr item operator[](uint64_t key) const;
    constexpr item operator[](int64_t key) const;
    constexpr item operator[](std::string_view key) const;
};

/**
 * @brief CBOR tag with number (stored) and content (decoded on-the-fly).
 * 
 */
struct tag : seq {
    constexpr tag(pntr head, pntr tail, uint64_t number) : seq{head, tail}, number{number} {}
    constexpr uint64_t num() const { return number; }
    constexpr item content() const;
private:
    uint64_t number;
};

/**
 * @brief Check additional info field.
 * 
 * @param ai Additional indo byte
 * @param p Begin pointer, must be valid pointer
 * @param end End pointer, must be valid pointer
 * @return Tuple with decoded length, error status and pointer past last character interpreted
 */
constexpr std::tuple<err, uint64_t, pntr> ai_check(byte ai, pntr p, const pntr end)
{
#if (1)
    if (ai < ai_1 || ai > 30)
        return {err::ok, ai, p};

    if (ai < 28 && p + bit(ai - ai_1) > end)
        return {err::out_of_bounds, 0, p};

    switch (ai) 
    {
    case ai_1: return {err::ok, *p, p + 1};
    case ai_2: return {err::ok, getbe<uint16_t>(p), p + 2};
    case ai_4: return {err::ok, getbe<uint32_t>(p), p + 4};
    case ai_8: return {err::ok, getbe<uint64_t>(p), p + 8};
    case 28:
    case 29:
    case 30: return {err::reserved_ai, 0, p};
    }
    return {err::ok, ai, p};
#else
    switch (ai) 
    {
    case ai_1:
    case ai_2:
    case ai_4:
    case ai_8: {
        size_t len = bit(ai - ai_1);
        if (p + len > end)
            return {err::out_of_bounds, 0, p};
        uint64_t val = 0;
        for (int i = 8 * len - 8; i >= 0; i -= 8)
            val |= uint64_t(*p++) << i;
        return {err::ok, val, p};
    }
    break;
    case 28:
    case 29:
    case 30: return {err::reserved_ai, 0, p};
    }
    return {err::ok, ai, p};
#endif
}

}

constexpr std::tuple<item, err, pntr> decode(pntr p, const pntr end);

/**
 * @brief Generic decoded CBOR object which can hold any type.
 * 
 */
struct item {
#if (NTH_CBOR_ITEM_NEW_API)
    constexpr item(type_t t = type_invalid) : type_{t} {}
#if (NTH_CBOR_ITEM_FAILURE_DEFAULT)
#define NTH_CBOR_GET(t, v, d)   \
    return t == type_ ? v : d;
#else
#define NTH_CBOR_GET(t, v, d)   \
    assert(t == type_);         \
    return v;
#endif
    constexpr bool is(type_t t) const   { return type_ == t; }
    constexpr bool valid() const        { return type_ != type_invalid; }
    constexpr auto type() const         { return type_; }
    constexpr auto uint() const         { NTH_CBOR_GET(type_uint, uint_, uint64_t(0)); }
    constexpr auto sint() const         { NTH_CBOR_GET(type_sint, sint_, int64_t(0)); }
    constexpr auto prim() const         { NTH_CBOR_GET(type_prim, prim_, prim_null); }
    constexpr auto boolean() const      { NTH_CBOR_GET(type_bool, bool_, false); }
    constexpr auto floating() const     { NTH_CBOR_GET(type_floating, fp64_, NAN); }
    constexpr auto bytes() const        { NTH_CBOR_GET(type_data, data_, span{}); }
    constexpr auto string() const       { NTH_CBOR_GET(type_text, text_, dec::text{}); }
    constexpr auto tag() const          { NTH_CBOR_GET(type_tag, tag_, dec::tag{}); }
    constexpr auto arr() const          { NTH_CBOR_GET(type_array, arr_, dec::arr{}); }
    constexpr auto map() const          { NTH_CBOR_GET(type_map, map_, dec::map{}); }
    constexpr auto indef_string() const { NTH_CBOR_GET(type_indef_text, istr_, dec::istr_{}); }
    constexpr auto indef_bytes() const  { NTH_CBOR_GET(type_indef_data, istr_, dec::istr_{}); }
#undef NTH_CBOR_GET
private:
    type_t type_;
    union {
        uint64_t uint_;
        int64_t sint_;
        prim_t prim_;
        bool bool_;
        double fp64_;
        span data_;
        dec::text text_;
        dec::istr istr_;
        dec::arr arr_;
        dec::map map_;
        dec::tag tag_;
    };
    friend constexpr std::tuple<item, err, pntr> decode(pntr p, const pntr end);
#else
    constexpr item(type_t t = type_invalid) : type{t} {}
    constexpr bool valid() const { return type != type_invalid; }
    constexpr auto uint() const
    type_t type;
    union {
        uint64_t uint;
        int64_t sint;
        prim_t prim;
        dec::data data;
        dec::text text;
        dec::istr istr;
        double fp;
        dec::arr arr;
        dec::map map;
        dec::tag tag;
    };
#endif
};

/**
 * @brief Decode next adjacent CBOR item. Almost all validity checks always performed 
 * throughout decoding process: out-of-bounds, reserved AI, invalid indef MT, nested 
 * indefinite strings, break without start. What isn't checked is number of elements 
 * within nested containers (if at least one of them is indefinite). For example, 0x9f82ff 
 * will be first parsed as valid indefinite array, and then, only if user starts to traverse 
 * over its elements, decoding of malformed nested array will report err_out_of_bounds.
 * 
 * @param p Begin pointer, must be valid pointer
 * @param end End pointer, must be valid pointer
 * @return Tuple with decoded object, error status and pointer past last character interpreted
 */
constexpr std::tuple<item, err, pntr> decode(pntr p, const pntr end)
{
    if (p >= end)
        return {{}, err::out_of_bounds, end};

    pntr head           = nullptr;
    byte mt             = *p   & 0xe0;
    byte ai             = *p++ & 0x1f;
    item obj            = type_t(mt >> 5);
    uint64_t val        = ai;
    uint64_t size       = 0;
    size_t nest         = 0;
    size_t skip         = 0;
    err e;

    if (ai == ai_indef) {
        switch (mt)
        {
        case mt_data: obj.type_ = type_indef_data; break;
        case mt_text: obj.type_ = type_indef_text; break;
        case mt_arr:
        case mt_map: size = size_t(-1); break;
        case mt_simple: return {{}, err::invalid_break, p};
        default: return {{}, err::invalid_indef_mt, p};
        }
        head = p;
        nest = 1;
    } else {
        std::tie(e, val, p) = dec::ai_check(ai, p, end);

        if (e != err::ok)
            return {{}, e, p};

        switch (mt) 
        {
        case mt_uint: obj.uint_ = val; break;
        case mt_nint: obj.sint_ = ~val; break;
        case mt_data:
            if (p + val > end)
                return {{}, err::out_of_bounds, p};
            obj.data_ = {p, size_t(val)};
            p += val;
        break;
        case mt_text:
            if (p + val > end)
                return {{}, err::out_of_bounds, p};
            obj.text_ = {p, size_t(val)};
            p += val;
        break;
        case mt_map:
            head = p;
            size = val;
            skip = val << 1;
        break;
        case mt_arr:
            head = p;
            size = val;
            skip = val;
        break;
        case mt_tag:
            head = p;
            size = val;
            skip = 1;
        break;
        case mt_simple:
            switch (ai) 
            {
            case prim_float_16:
#if (NTH_UTIL_HALF_NATIVE)
                obj.fp64_   = std::bit_cast<std::float16_t>(uint16_t(val));
#else
                obj.fp64_   = std::bit_cast<float>(half_to_float(val));
#endif
                obj.type_   = type_floating;
            break;
            case prim_float_32:
                obj.fp64_   = std::bit_cast<float>(uint32_t(val));
                obj.type_   = type_floating;
            break;
            case prim_float_64:
                obj.fp64_   = std::bit_cast<double>(val);
                obj.type_   = type_floating;
            break;
            case prim_false:
            case prim_true:
                obj.bool_   = val == prim_true;
                obj.type_   = type_bool;
            break;
            default:
                obj.prim_   = prim_t(val);
            break;
            }
        break;
        }
    }

    if (obj.type_ == type_indef_data || 
        obj.type_ == type_indef_text) 
    {
        while (true) {
            
            if (p >= end)
                return {{}, err::out_of_bounds, end};

            if (*p == 0xff) {
                ++p;
                break;
            }
            val = *p & 0x1f;

            if (obj.type_ != ((*p++ & 0xe0) >> 5) + 7 || val == ai_indef)
                return {{}, err::invalid_indef_string, p};

            std::tie(e, val, p) = dec::ai_check(val, p, end);

            if (e != err::ok)
                return {{}, e, p};

            p += val;
        }
    } else {
        while (skip || nest) {

            if (p >= end)
                return {{}, err::out_of_bounds, end};

            mt  = *p   & 0xe0;
            val = *p++ & 0x1f;

            if (val == ai_indef) {
                switch (mt) {
                case mt_data:
                case mt_text:
                case mt_arr:
                case mt_map: ++nest; 
                break;
                case mt_simple:
                    if (!nest)
                        return {{}, err::invalid_break, p};
                    --nest;
                break;
                default: return {{}, err::invalid_indef_mt, p};
                }
            } else {
                std::tie(e, val, p) = dec::ai_check(val, p, end);

                if (e != err::ok) 
                    return {{}, e, p};
                    
                switch (mt) {
                case mt_data:
                case mt_text:   
                    if (p + val > end)
                        return {{}, err::out_of_bounds, p};
                    p += val; 
                break;
                case mt_arr: if (!nest) skip += val;        break;
                case mt_map: if (!nest) skip += val << 1;   break;
                case mt_tag: if (!nest) skip += 1;          break;
                }
            }
            if (skip && !nest)
                skip--;
        }
    }
    switch (obj.type_) 
    {
    case type_array:        obj.arr_  = {head, p, size}; break;
    case type_map:          obj.map_  = {head, p, size}; break;
    case type_tag:          obj.tag_  = {head, p, size}; break;
    case type_indef_data:   obj.istr_ = {head, p}; break;
    case type_indef_text:   obj.istr_ = {head, p}; break;
    default:;
    }
    return {obj, err::ok, p};
}

/**
 * @brief Sequence iterator which holds range (begin and end pointers). Used 
 * to traverse CBOR sequence (RFC-8742), which is just series of adjacent 
 * objects. Used to traverse arr_t, istr_t or any series of bytes as item 
 * one by one. Only exception is map_t.
 * 
 */
struct seq_iter {
    constexpr seq_iter() = default;
    constexpr seq_iter(pntr head, pntr tail) : head{head}, tail{tail}
    {
        step(key);
    }
    constexpr bool operator!=(const seq_iter&) const 
    { 
        return key.valid();
    }
    constexpr auto& operator*() const 
    { 
        return key; 
    }
    constexpr auto& operator++()
    {
        step(key);
        return *this;
    }
    constexpr auto operator++(int) 
    { 
        auto tmp = *this; 
        ++(*this); 
        return tmp; 
    }
protected:
    constexpr void step(item& o) 
    {
        std::tie(o, std::ignore, head) = decode(head, tail); 
    }
protected:
    pntr head = nullptr;
    pntr tail = nullptr;
    item key;
};

/**
 * @brief Map iterator, same as seq_iter, but parses two objects in a 
 * row and returns them as pair.
 * 
 */
struct map_iter : seq_iter {
    constexpr map_iter() = default;
    constexpr map_iter(pntr head, pntr tail) : seq_iter{head, tail}
    {
        if (key.valid()) 
            step(val);
    }
    constexpr bool operator!=(const map_iter&) const 
    { 
        return key.valid() && val.valid();
    }
    constexpr auto operator*() const 
    { 
        return std::pair<const item&, const item&>{key, val}; 
    }
    constexpr auto& operator++()
    {
        step(key);
        if (key.valid()) 
            step(val);
        return *this;
    }
    constexpr auto operator++(int) 
    { 
        auto tmp = *this; 
        ++(*this); 
        return tmp; 
    }
private:
    item val;
};

namespace dec {

constexpr item arr::operator[](size_t idx) const
{
    size_t i = 0;
    for (auto val : *this) {
        if (i++ == idx)
            return val;
    }
    return {};
}

constexpr item map::operator[](uint64_t key) const
{
    for (auto [k, v] : *this) {
        if (k.is(type_uint) && k.uint() == key)
            return v;
    }
    return {};
}

constexpr item map::operator[](int64_t key) const
{
    for (auto [k, v] : *this) {
        if (k.is(type_sint) && k.sint() == key)
            return v;
    }
    return {};
}

constexpr item map::operator[](std::string_view key) const
{
    for (auto [k, v] : *this) {
        if (k.is(type_text) && k.string() == key)
            return v;
    }
    return {};
}

}

constexpr seq_iter seq::begin() const       { return {data(), data() + size()}; }
constexpr seq_iter seq::end() const         { return {}; }
constexpr map_iter dec::map::begin() const  { return {data(), data() + seq::size()}; }
constexpr map_iter dec::map::end() const    { return {}; }
constexpr item dec::tag::content() const    { return std::get<item>(decode(data(), data() + size())); }

/**
 * @brief Get human readable name for type enum.
 * 
 * @param t Type enumeration
 * @return String with type name 
 */
constexpr auto str_type(type_t t)
{
    switch (t) {
        case type_uint: return "unsigned";
        case type_sint: return "negative";
        case type_data: return "data";
        case type_text: return "text";
        case type_array: return "array";
        case type_map: return "map";
        case type_tag: return "tag";
        case type_prim: return "simple";
        case type_floating: return "float";
        case type_indef_data: return "indefinite data";
        case type_indef_text: return "indefinite text";
        case type_invalid: return "<invalid>";
        default: return "<unknown>";
    }
}

/**
 * @brief Get human readable name for error enum.
 * 
 * @param e Error enumeration
 * @return String with error name
 */
constexpr auto str_err(err e)
{
    switch (e) {
        case err::ok: return "ok";
        case err::no_memory: return "no_memory";
        case err::out_of_bounds: return "out_of_bounds";
        case err::reserved_ai: return "reserved_ai";
        case err::invalid_break: return "invalid_break";
        case err::invalid_simple: return "invalid_simple";
        case err::invalid_indef_mt: return "invalid_indef_mt";
        case err::invalid_indef_string: return "invalid_indef_string";
        default: return "<unknown>";
    }
}

}

#endif