#ifndef NTH_PRINT_LOG_H
#define NTH_PRINT_LOG_H

#include "nth/util/string.h"
#include "nth/util/bitset.h"
#include <cctype>
#include <cstdio>

namespace nth {

/**
 * @brief Enumeration of various logging levels used to 
 * control verbosity and granularity of log messages.
 */
enum class log_level {
    non, // None level - no logging
    dbg, // Debug level - detailed debugging information
    inf, // Info level - general informational messages
    wrn, // Warning level - potential issues that may affect normal operation
    err, // Error level - issues that may cause the application to fail or behave unexpectedly
    ftl, // Fatal level - severe issues that lead to immediate termination or 100 % malfunction 
};

enum class log_color {
    rst,
    red,
    grn,
    yel,
    blu,
    mag,
    cyn,
    wht,
};

// #define K_RST   "\x1b[0m"
// #define K_RED   "\x1b[31m"
// #define K_GRN   "\x1b[32m"
// #define K_YEL   "\x1b[33m"
// #define K_BLU   "\x1b[34m"
// #define K_MAG   "\x1b[35m"
// #define K_CYN   "\x1b[36m"
// #define K_WHT   "\x1b[37m"

/**
 * @brief Print hex nicely with relevant ASCII representation.
 * 
 * @param dat Data to print
 * @param len Length in bytes
 */
inline void log_hex(const void* dat, size_t len/*, const char* optional_header = nullptr*/)
{
    // printf(": %lu : %s\n", len, optional_header ? optional_header : "");

    if (!dat || !len)
        return;

    auto p = static_cast<const uint8_t*>(dat);

    for (size_t i = 0; i < len; ++i) {

        if (!(i & 15)) {
            putchar('|');
            putchar(' ');
        }
        putchar(bin_to_char(p[i] >> 4));
        putchar(bin_to_char(p[i] & 0xF));
        putchar(' ');
        
        if ((i & 7) == 7)
            putchar(' ');

        if ((i & 15) == 15) {
            putchar('|');
            for (int j = 15; j >= 0; --j) {
                char c = p[i - j];
                putchar(isprint(c) ? c : '.');
            }
            putchar('|');
            putchar('\n');
        }
    }
    int rem = len - ((len >> 4) << 4);
    if (rem) {
        for (int j = (16 - rem) * 3 + ((~rem & 8) >> 3); j >= 0; --j)
            putchar(' ');
        putchar('|');
        for (int j = rem; j; --j) {
            char c = p[len - j];
            putchar(isprint(c) ? c : '.');
        }
        for (int j = 0; j < 16 - rem; ++j)
            putchar('.');
        putchar('|');
        putchar('\n');
    }
}

/**
 * @brief Print bits nicely from offset position with MSB at left and relevant ASCII.
 * 
 * @param dat Memory with bits to log
 * @param len Number of bits
 * @param pos Bit offset in initial byte, be careful to not go out of memory region
 */
inline void log_bits(const void* dat, size_t len, size_t pos = 0)
{
    if (!dat || !len)
        return;

    auto ptr = static_cast<const uint8_t*>(dat);
    auto rem = len & 7;

    uint8_t line[8];

    for (size_t i = 0; i < len; ++i) {

        if (!(i & 63)) {
            putchar('|');
            putchar(' ');
            for (auto &c : line) c = 0;
        }

        if (get_arr_bit(ptr, pos++))
            set_arr_bit(line, i & 63);
            
        if (rem && i == len - 1) {
            uint8_t c = line[(i & 63) >> 3];
            for (int j = rem - 1; j >= 0; --j)
                putchar(get_bit(c, j) + '0');
        }

        if ((i & 7) == 7) {
            uint8_t c = line[(i & 63) >> 3];
            for (int j = 7; j >= 0; --j)
                putchar(get_bit(c, j) + '0');
            putchar(' ');
        }

        if ((i & 63) == 63) {
            putchar(' ');
            putchar('|');
            for (int j = 0; j < 8; ++j) {
                char c = line[j];
                putchar(isprint(c) ? c : '.');
            }
            putchar('|');
            putchar('\n');
        }
    }

    int rem_bits = len - ((len >> 6) << 6);
    int rem_bytes = rem_bits >> 3;
    int fill_bits = 64 - rem_bits;
    int fill_bytes = fill_bits >> 3;

    if (rem_bits) {
        for (int j = fill_bits + bytes_in_bits(fill_bits); j >= 0; --j)
            putchar(' ');
        putchar('|');
        for (int j = 0; j < rem_bytes; ++j) {
            char c = line[j];
            putchar(isprint(c) ? c : '.');
        }
        if (rem)
            putchar('.');
        for (int j = 0; j < fill_bytes; ++j)
            putchar('.');
        putchar('|');
        putchar('\n');
    }
}

template<class type, size_t bits, size_t grow>
void print_bit_vector_info() 
{
    auto array = imp::bit_tree_struct<type, bits, grow>();
    auto depth = imp::bit_tree_depth<type>(bits, grow);
    auto words = sizeof(bitset<type, bits, grow>) / sizeof(type);
    printf("| b_%-10lu ", bits);
    printf("| w_%-9lu ", words);
    printf("| d_%-1lu ", depth);
    printf("[%lu]: ", array.size());
    size_t cnt = 0;
#if (NTH_BIT_VECTOR_REMAINDER_STORED)
    for (auto [head, size, remainder] : array) {
        cnt += size;
        printf("{%lu:%lu:%lu} ", head, size, remainder);
    }
#else
    for (auto [head, size] : array) {
        cnt += size;
        printf("{%lu:%lu} ", head, size);
    }
#endif
    if (depth != array.size() || cnt != words)
        printf("! NOT OKAY !");
    printf("\n");
}

}

#endif