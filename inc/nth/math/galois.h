#ifndef NTH_MATH_GALOIS_H
#define NTH_MATH_GALOIS_H

#include <cstdint>
#include <cstring>
#include <algorithm>

namespace nth {

/**
 * @brief Galois 2^8 field multiplication using Russian 
 * Peasant Multiplication algorithm.
 * 
 * @param x Multiplicand
 * @param y Multiplier
 * @return Product
 */
constexpr uint8_t gf_mul(uint8_t x, uint8_t y) 
{
    uint8_t r = 0; 

    while (y) {
        if (y & 1)
            r ^= x; 
        x = (x << 1) ^ ((x >> 7) * 0x11d);
        y >>= 1;
    }
    return r;
}

/**
 * @brief Reed-Solomon Ecc generator polynomial for the given degree.
 * 
 * @param degree 
 * @param poly 
 */
constexpr void gf_gen_poly(int degree, uint8_t *poly)
{
    std::fill_n(poly, degree, 0);
    uint8_t root = poly[degree - 1] = 1;
    for (int i = 0; i < degree; ++i) {
        for (int j = 0; j < degree - 1; ++j)
            poly[j] = gf_mul(poly[j], root) ^ poly[j + 1];
        poly[degree - 1] = gf_mul(poly[degree - 1], root);
        root = (root << 1) ^ ((root >> 7) * 0x11d);
    }
}

/**
 * @brief Polynomial division in Galois Field.
 * 
 * @param dividend 
 * @param len 
 * @param divisor 
 * @param degree 
 * @param result 
 */
constexpr void gf_poly_div(const uint8_t* dividend, size_t len, const uint8_t* divisor, int degree, uint8_t* result) 
{
    std::fill_n(result, degree, 0);
    for (size_t i = 0; i < len; ++i) {
        uint8_t factor = dividend[i] ^ result[0];
        memmove(&result[0], &result[1], degree - 1);
        result[degree - 1] = 0;
        for (int j = 0; j < degree; ++j)
            result[j] ^= gf_mul(divisor[j], factor);
    }
}

}

#endif