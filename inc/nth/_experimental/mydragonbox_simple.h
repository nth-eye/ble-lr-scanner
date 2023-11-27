// Copyright 2020 Alexander Bolz
//
// Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
#ifndef MYDRAGONBOX_SIMPLE_H
#define MYDRAGONBOX_SIMPLE_H

#include <cstdint>
#include <limits>
#include <bit>

namespace nth::dragonbox {

// char* output_end = Dtoa(buffer, value);
//
// Converts the given double-precision number into decimal form and stores the result in the given
// buffer.
//
// The buffer must be large enough, i.e. >= DtoaMinBufferLength.
// The output format is similar to printf("%g").
// The output is _not_ null-terminted.
//
// The output is optimal, i.e. the output string
//  1. rounds back to the input number when read in (using round-to-nearest-even)
//  2. is as short as possible,
//  3. is as close to the input number as possible.
//
// Note:
// This function may temporarily write up to DtoaMinBufferLength characters into the buffer.

struct Double {
    static_assert(std::numeric_limits<double>::is_iec559
               && std::numeric_limits<double>::digits == 53
               && std::numeric_limits<double>::max_exponent == 1024,
        "IEEE-754 double-precision implementation required");

    using value_type = double;
    using bits_type = uint64_t;

//  static constexpr int32_t   MaxDigits10     = std::numeric_limits<value_type>::max_digits10;
    static constexpr int32_t   SignificandSize = std::numeric_limits<value_type>::digits; // = p   (includes the hidden bit)
    static constexpr int32_t   ExponentBias    = std::numeric_limits<value_type>::max_exponent - 1 + (SignificandSize - 1);
//  static constexpr int32_t   MaxExponent     = std::numeric_limits<value_type>::max_exponent - 1 - (SignificandSize - 1);
//  static constexpr int32_t   MinExponent     = std::numeric_limits<value_type>::min_exponent - 1 - (SignificandSize - 1);
    static constexpr bits_type MaxIeeeExponent = bits_type{2 * std::numeric_limits<value_type>::max_exponent - 1};
    static constexpr bits_type HiddenBit       = bits_type{1} << (SignificandSize - 1);   // = 2^(p-1)
    static constexpr bits_type SignificandMask = HiddenBit - 1;                           // = 2^(p-1) - 1
    static constexpr bits_type ExponentMask    = MaxIeeeExponent << (SignificandSize - 1);
    static constexpr bits_type SignMask        = ~(~bits_type{0} >> 1);

    bits_type bits;

    explicit Double(bits_type bits_) : bits(bits_) {}
    explicit Double(value_type value) : bits(std::bit_cast<bits_type>(value)) {}

    bits_type PhysicalSignificand() const {
        return bits & SignificandMask;
    }

    bits_type PhysicalExponent() const {
        return (bits & ExponentMask) >> (SignificandSize - 1);
    }

    bool IsFinite() const {
        return (bits & ExponentMask) != ExponentMask;
    }

    bool IsInf() const {
        return (bits & ExponentMask) == ExponentMask && (bits & SignificandMask) == 0;
    }

    bool IsNaN() const {
        return (bits & ExponentMask) == ExponentMask && (bits & SignificandMask) != 0;
    }

    bool IsZero() const {
        return (bits & ~SignMask) == 0;
    }

    bool SignBit() const {
        return (bits & SignMask) != 0;
    }
};

constexpr int DtoaMinBufferLength = 64;

char* Dtoa(char* buffer, double value);

} // namespace dragonbox

#endif