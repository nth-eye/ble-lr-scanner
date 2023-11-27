#ifndef NTH_MISC_QR_H
#define NTH_MISC_QR_H

#include "nth/util/bit.h"
#include "nth/math/galois.h"

namespace nth {

/**
 * @brief QR code error correction level.
 * 
 */
enum qr_ecc { 
    qr_ecc_l, 
    qr_ecc_m, 
    qr_ecc_q, 
    qr_ecc_h,
};

/**
 * @brief QR code encoding mode.
 * 
 */
enum qr_mode { 
    qr_mode_numeric,
    qr_mode_alphanumeric,
    qr_mode_byte,
    qr_mode_kanji,
};

namespace imp {

/**
 * @brief Size of ECC block with respect to level and version.
 * 
 */
inline constexpr int qr_ecc_codewords_per_block[4][40] = {
    {  7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }, 
    { 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28 }, 
    { 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }, 
    { 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }, 
};

/**
 * @brief Number of Ecc blocks with respect to level and version.
 * 
 */
inline constexpr int qr_num_ecc_blocks[4][40] = {
    { 1, 1, 1, 1, 1, 2, 2, 2, 2, 4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25 },
    { 1, 1, 1, 2, 2, 4, 4, 4, 5, 5,  5,  8,  9,  9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49 },
    { 1, 1, 2, 2, 4, 4, 6, 6, 8, 8,  8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68 },
    { 1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81 },
};

/**
 * @brief Positions of central modules of alignment patterns according to version.
 * 
 */
inline constexpr int qr_align_pos[40][7] = {
    { 0 },
    { 6, 18 },
    { 6, 22 },
    { 6, 26 },
    { 6, 30 },
    { 6, 34 },
    { 6, 22, 38 },
    { 6, 24, 42 },
    { 6, 26, 46 },
    { 6, 28, 50 },
    { 6, 30, 54 },
    { 6, 32, 58 },
    { 6, 34, 62 },
    { 6, 26, 46, 66 },
    { 6, 26, 48, 70 },
    { 6, 26, 50, 74 },
    { 6, 30, 54, 78 },
    { 6, 30, 56, 82 },
    { 6, 30, 58, 86 },
    { 6, 34, 62, 90 },
    { 6, 28, 50, 72, 94 },
    { 6, 26, 50, 74, 98 },
    { 6, 30, 54, 78, 102 },
    { 6, 28, 54, 80, 106 },
    { 6, 32, 58, 84, 110 },
    { 6, 30, 58, 86, 114 },
    { 6, 34, 62, 90, 118 },
    { 6, 26, 50, 74, 98, 122 },
    { 6, 30, 54, 78, 102, 126 },
    { 6, 26, 52, 78, 104, 130 },
    { 6, 30, 56, 82, 108, 134 },
    { 6, 34, 60, 86, 112, 138 },
    { 6, 30, 58, 86, 114, 142 },
    { 6, 34, 62, 90, 118, 146 },
    { 6, 30, 54, 78, 102, 126, 150 },
    { 6, 24, 50, 76, 102, 128, 154 },
    { 6, 28, 54, 80, 106, 132, 158 },
    { 6, 32, 58, 84, 110, 136, 162 },
    { 6, 26, 54, 82, 110, 138, 166 },
    { 6, 30, 58, 86, 114, 142, 170 },
};

inline constexpr int qr_cci_cnt[4][3] = {
    { 10, 12, 14 },
    { 9,  11, 13 },
    { 8,  16, 16 },
    { 8,  10, 12 },
};

/**
 * @brief Return size of Character Control Indicator in bits for given version and mode.
 * 
 * @param ver Version
 * @param mode Mode
 * @return CCI size
 */
constexpr int qr_cci(int ver, qr_mode mode)
{
    if (ver < 10)
        return qr_cci_cnt[mode][0];
    if (ver < 27)
        return qr_cci_cnt[mode][1];
    return qr_cci_cnt[mode][2];
}

/**
 * @brief Add up to 16 bits to arr. Data starts from MSB as well as each byte of an array.
 * 
 * @param data 
 * @param n 
 * @param arr 
 * @param pos 
 */
constexpr void qr_add_bits(uint16_t data, int n, uint8_t* arr, size_t& pos)
{
    while (n--) {
        arr[pos >> 3] |= ((data >> n) & 1) << (7 - (pos & 7)); 
        ++pos;
    }
}

/**
 * @brief Translate char to alphanumeric encoding value.
 * 
 * @param c Character
 * @return Alphanumeric integral value 
 */
constexpr int qr_alphanumeric(char c)
{
    if (c >= '0' && c <= '9') 
        return c - '0';

    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;

    switch (c) {
        case ' ': return 36;
        case '$': return 37;
        case '%': return 38;
        case '*': return 39;
        case '+': return 40;
        case '-': return 41;
        case '.': return 42;
        case '/': return 43;
        case ':': return 44;
    }
    return -1;
}

/**
 * @brief Check if string can be encoded in alphanumeric mode.
 * 
 * @param str String
 * @return true if yes
 */
constexpr bool qr_is_alphanumeric(std::string_view str) 
{
    for (auto c : str)
        if (qr_alphanumeric(c) == -1) 
            return false;
    return true;
}

/**
 * @brief Check if string can be encoded in numeric mode.
 * 
 * @param str String
 * @return true if yes
 */
constexpr bool qr_is_numeric(std::string_view str) 
{
    for (auto c : str)
        if (c < '0' || c > '9') 
            return false;
    return true;
}

/**
 * @brief Check if string can be encoded in kanji mode.
 * 
 * @param str String
 * @return true if yes
 */
constexpr bool qr_is_kanji(std::string_view str) 
{
    for (size_t i = 0; i < str.size(); i += 2) {
        uint16_t val = uint16_t(str[i]) | (uint16_t(str[i + 1]) << 8);
        if (val < 0x8140 || 
            val > 0xebbf || (val > 0x9ffc && val < 0xe040))
            return false;
    }
    return true;
}

/**
 * @brief Select appropriate encoding mode for string.
 * 
 * @param str String
 * @return Mode
 */
constexpr qr_mode qr_select_mode(std::string_view str)
{
    if (qr_is_numeric(str))
        return qr_mode_numeric;
    if (qr_is_alphanumeric(str))
        return qr_mode_alphanumeric;
    if (qr_is_kanji(str))
        return qr_mode_kanji;
    return qr_mode_byte;
}

}

template<int V>
struct qr_encoder {

    /**
     * @brief Get side size in terms of modules.
     * 
     * @return Side size
     */
    constexpr auto side_size() const 
    { 
        return side; 
    }

    /**
     * @brief Get color of a module from left-to-right and top-to-bottom. Black is true.
     * 
     * @param x Horizontal coordinate
     * @param y Vertical coordinate
     * @return true if black
     */
    constexpr bool module(int x, int y) const
    {
        return get_arr_bit(code, y * side + x);
    }

    /**
     * @brief Create QR code with given error correction level. If mask == -1, 
     * then the best mask is selected automatically. NOTE: Automatic mask is 
     * the most expensive operation, takes about 95 % of all computation time.
     * 
     * @param str String to encode
     * @param ecc Error correction level
     * @param mask Mask, -1 if automatic, [0, 7] if manual
     * @return true on success
     */
    constexpr bool encode(std::string_view str, qr_ecc ecc, int mask = -1)
    {
        uint8_t data[N_DAT_BYTES]           = {};
        uint8_t data_with_ecc[N_DAT_BYTES]  = {};
        uint8_t patterns[N_BYTES]           = {};

        if (!encode_data(str, ecc, data))
            return false;

        encode_ecc(data, ecc, data_with_ecc);

        reserve_patterns(patterns);
        memcpy(code, patterns, N_BYTES);

        add_data(data_with_ecc, patterns);
        add_patterns();
        add_version();

        mask = mask != -1 ? mask & 7 : select_mask(ecc, patterns);

        add_format(ecc, mask);
        apply_mask(mask, patterns);

        return true;
    }
private:
    constexpr bool encode_data(std::string_view data, qr_ecc ecc, uint8_t* out);
    constexpr void encode_ecc(const uint8_t* data, qr_ecc ecc, uint8_t* out);

    constexpr void add_data(const uint8_t* data, const uint8_t* patterns);
    constexpr void add_patterns();
    constexpr void add_version();
    constexpr void add_format(qr_ecc ecc, int mask);
    constexpr void reserve_patterns(uint8_t* out);

    template<bool Black>
    constexpr void draw_rect(int y, int x, int height, int width, uint8_t* out);
    template<bool Black>
    constexpr void draw_bound(int y, int x, int height, int width, uint8_t* out);
    
    template<bool Horizontal>
    constexpr int rule_1_3_score();
    constexpr int penalty_score();
    constexpr int select_mask(qr_ecc ecc, const uint8_t* patterns);
    constexpr void apply_mask(int mask, const uint8_t* patterns);
private:
    static_assert(V >= 1 && V <= 40, "invalid version");
    static constexpr int side           = 17 + V * 4;
    static constexpr int N_BITS         = side * side;
    static constexpr int N_ALIGN        = V == 1 ? 0 : V / 7 + 2;
    static constexpr int N_ALIGN_BITS   = V > 1 ? (N_ALIGN * N_ALIGN - 3) * 25 : 0;
    static constexpr int N_TIMING_BITS  = (side - 16) * 2 - (10 * (V > 1 ? N_ALIGN - 2 : 0));
    static constexpr int N_VER_BITS     = V > 6 ? 36 : 0;
    static constexpr int N_DAT_BITS     = N_BITS - (192 + N_ALIGN_BITS + N_TIMING_BITS + 31 + N_VER_BITS);
    static constexpr int N_BYTES        = bytes_in_bits(N_BITS);        // Actual number of bytes_in_bits required to store whole qr_encoder code
    static constexpr int N_DAT_BYTES    = bytes_in_bits(N_DAT_BITS);    // Actual number of bytes_in_bits required to store [data + ecc]
    static constexpr int N_DAT_CAPACITY = N_DAT_BITS >> 3;              // Capacity of [data + ecc] without remainder bits
private:
    uint8_t code[N_BYTES] = {};
};

template<int V>
constexpr bool qr_encoder<V>::encode_data(std::string_view data, qr_ecc ecc, uint8_t* out)
{
    auto mode = imp::qr_select_mode(data);

    size_t n_bits = (N_DAT_CAPACITY - imp::qr_ecc_codewords_per_block[ecc][V - 1] * imp::qr_num_ecc_blocks[ecc][V - 1]) << 3;
    size_t pos = 0;

    imp::qr_add_bits(1 << mode, 4, out, pos);
    imp::qr_add_bits(data.size(), imp::qr_cci(V, mode), out, pos);

    if (mode == qr_mode_numeric) {

        const size_t triplets       = data.size() / 3;
        const size_t triplets_size  = triplets * 3;
        const size_t rem            = data.size() % 3;
        const size_t rem_bits       = rem == 2 ? 7 : rem == 1 ? 4 : 0;
        const size_t total_size     = 10 * triplets + rem_bits;

        if (pos + total_size > n_bits)
            return false;

        char buf[4] = {};

        for (size_t i = 0; i < triplets_size; i += 3) {
            buf[0] = data[i];
            buf[1] = data[i + 1];
            buf[2] = data[i + 2];
            uint16_t num = strtol(buf, NULL, 10);
            imp::qr_add_bits(num, 10, out, pos);
        }

        if (rem) {
            buf[0] = data[triplets_size];
            buf[1] = data[triplets_size + 1];
            buf[rem] = 0;
            uint16_t num = strtol(buf, NULL, 10);
            imp::qr_add_bits(num, rem_bits, out, pos);
        }
    } else if (mode == qr_mode_alphanumeric) {

        if (pos + 11 * (int(data.size() & ~1ul) / 2) > n_bits)
            return false;

        for (int i = 0; i < int(data.size() & ~1ul); i += 2) {
            uint16_t num = imp::qr_alphanumeric(data[i]) * 45 + imp::qr_alphanumeric(data[i + 1]);
            imp::qr_add_bits(num, 11, out, pos);
        }
        if (data.size() & 1) {
            if (pos + 6 > n_bits)
                return false;
            imp::qr_add_bits(imp::qr_alphanumeric(data[data.size() - 1]), 6, out, pos);
        }

    } else if (mode == qr_mode_byte) {

        if (pos + data.size() * 8 > n_bits)
            return false;

        for (size_t i = 0; i < data.size(); ++i)
            imp::qr_add_bits(data[i], 8, out, pos);

    } else {

        if (pos + 13 * (data.size() / 2) > n_bits)
            return false;

        for (size_t i = 0; i < data.size(); i += 2) {
            uint16_t val = ((uint8_t) data[i]) | (((uint8_t) data[i + 1]) << 8);
            uint16_t res = 0;
            val -= val < 0x9FFC ? 0x8140 : 0xC140;
            res += val & 0xff;
            res += (val >> 8) * 0xc0;
            imp::qr_add_bits(res, 13, out, pos);
        }
    }

    size_t padding = n_bits - pos;
    size_t i = 0;

    imp::qr_add_bits(0, padding > 4 ? 4 : padding, out, pos);
    
    if (pos & 7)
        imp::qr_add_bits(0, (8 - pos) & 7, out, pos);

    while (pos < n_bits)
        imp::qr_add_bits(++i & 1 ? 0xec : 0x11, 8, out, pos);

    return true;
}

template<int V>
constexpr void qr_encoder<V>::encode_ecc(const uint8_t* data, qr_ecc ecc, uint8_t* out)
{
    int n_blocks        = imp::qr_num_ecc_blocks[ecc][V - 1];
    int ecc_len         = imp::qr_ecc_codewords_per_block[ecc][V - 1];

    int n_data_bytes    = N_DAT_CAPACITY - ecc_len * n_blocks;

    int n_short_blocks  = n_blocks - N_DAT_CAPACITY % n_blocks;
    int short_len       = N_DAT_CAPACITY / n_blocks - ecc_len;

    uint8_t gen_poly[30];
    uint8_t ecc_buf[30];

    gf_gen_poly(ecc_len, gen_poly);

    auto data_ptr = data;

    for (int i = 0; i < n_blocks; ++i) {

        int data_len = short_len;

        if (i >= n_short_blocks)
            ++data_len;

        gf_poly_div(data_ptr, data_len, gen_poly, ecc_len, ecc_buf);

        for (int j = 0, k = i; j < data_len; ++j, k += n_blocks) {
            if (j == short_len)
                k -= n_short_blocks;
            out[k] = data_ptr[j];
        }
        for (int j = 0, k = n_data_bytes + i; j < ecc_len; ++j, k += n_blocks)
            out[k] = ecc_buf[j];

        data_ptr += data_len;
    }
}

template<int V>
constexpr void qr_encoder<V>::add_data(const uint8_t* data, const uint8_t* patterns)
{
    /**
     * @brief Return n-th bit of arr starting from MSB.
     * 
     */
    auto get_bit_r = [] (const uint8_t *arr, int n) -> byte
    {
        return (arr[n >> 3] >> (7 - (n & 7))) & 1;
    };
    int data_pos = 0;

    for (int x = side - 1; x >= 1; x -= 2) {

        if (x == 6)
            x = 5;

        for (int i = 0; i < side; ++i) {

            int y = !((x + 1) & 2) ? side - 1 - i : i;
            int coord = y * side + x;

            if (!get_arr_bit(patterns, coord)) {
                if (get_bit_r(data, data_pos))
                    set_arr_bit(code, coord);
                ++data_pos;
            }

            if (!get_arr_bit(patterns, coord - 1)) {
                if (get_bit_r(data, data_pos))
                    set_arr_bit(code, coord - 1);
                ++data_pos;
            }
        }
    }
}

template<int V>
constexpr void qr_encoder<V>::add_patterns()
{
    // White bounds inside finders
    draw_bound<false>(1, 1, 5, 5, code);
    draw_bound<false>(1, side - 6, 5, 5, code);
    draw_bound<false>(side - 6, 1, 5, 5, code);

    // Finish alignment patterns
    for (int i = 0; i < N_ALIGN; ++i) {
        for (int j = 0; j < N_ALIGN; ++j) {
            if ((!i && !j) || 
                (!i && j == N_ALIGN - 1) || 
                (!j && i == N_ALIGN - 1) )
                continue;
            draw_bound<false>(imp::qr_align_pos[V - 1][i] - 1, imp::qr_align_pos[V - 1][j] - 1, 3, 3, code);
        }
    }

    // Draw white separators
    draw_rect<false>(7, 0, 1, 8, code);
    draw_rect<false>(0, 7, 8, 1, code);
    draw_rect<false>(side - 8, 0, 1, 8, code);
    draw_rect<false>(side - 8, 7, 8, 1, code);
    draw_rect<false>(7, side - 8, 1, 8, code);
    draw_rect<false>(0, side - 8, 8, 1, code);

    // Perforate timing patterns
    for (int i = 7; i < side - 7; i += 2) {
        clr_arr_bit(code, 6 * side + i);
        clr_arr_bit(code, i * side + 6);
    }
}

template<int V>
constexpr void qr_encoder<V>::add_version()
{
    if (V < 7)
        return;

    uint32_t rem = V;

    for (uint8_t i = 0; i < 12; ++i)
        rem = (rem << 1) ^ ((rem >> 11) * 0x1F25);

    uint32_t data = V << 12 | rem;
    
    for (int x = 0; x < 6; ++x) {
        for (int j = 0; j < 3; ++j) {

            int y = side - 11 + j;

            bool black = (data >> (x * 3 + j)) & 1;

            if (!black) {
                clr_arr_bit(code, y * side + x);
                clr_arr_bit(code, y + side * x);
            }
        }
    }
}

template<int V>
constexpr void qr_encoder<V>::add_format(qr_ecc ecc, int mask)
{
    int data = (ecc ^ 1) << 3 | mask;
    int rem = data;

    for (int i = 0; i < 10; i++)
        rem = (rem << 1) ^ ((rem >> 9) * 0b10100110111);

    int res = (data << 10 | rem) ^ 0b101010000010010;

    for (int i = 0; i < 6; ++i) {
        if ((res >> i) & 1) {
            set_arr_bit(code, side * 8 + side - 1 - i);
            set_arr_bit(code, side * i + 8);
        } else {
            clr_arr_bit(code, side * 8 + side - 1 - i);
            clr_arr_bit(code, side * i + 8);
        }
    }

    for (int i = 6; i < 8; ++i) {
        if ((res >> i) & 1) {
            set_arr_bit(code, side * 8 + side - 1 - i);
            set_arr_bit(code, side * (i + 1) + 8);
        } else {
            clr_arr_bit(code, side * 8 + side - 1 - i);
            clr_arr_bit(code, side * (i + 1) + 8);
        }
    }

    if ((res >> 8) & 1) {
        set_arr_bit(code, side * 8 + 7);
        set_arr_bit(code, side * (side - 7) + 8);
    } else {
        clr_arr_bit(code, side * 8 + 7);
        clr_arr_bit(code, side * (side - 7) + 8); 
    }

    for (int i = 9, j = 5; i < 15; ++i, --j) {
        if ((res >> i) & 1) {
            set_arr_bit(code, side * 8 + j);
            set_arr_bit(code, side * (side - 1 - j) + 8);
        } else {
            clr_arr_bit(code, side * 8 + j);
            clr_arr_bit(code, side * (side - 1 - j) + 8);
        }
    }
}

template<int V>
template<bool B>
constexpr void qr_encoder<V>::draw_rect(int y, int x, int height, int width, uint8_t* out)
{
    if (B) {
        for (int dy = y * side; dy < (y + height) * side; dy += side)
            for (int dx = x; dx < x + width; ++dx) 
                set_arr_bit(out, dy + dx);
    } else {
        for (int dy = y * side; dy < (y + height) * side; dy += side)
            for (int dx = x; dx < x + width; ++dx)
                clr_arr_bit(out, dy + dx);
    }
}

template<int V>
template<bool B>
constexpr void qr_encoder<V>::draw_bound(int y, int x, int height, int width, uint8_t* out)
{
    if constexpr (B) {
        for (int i = y * side + x;              i < y * side + x+width;                 ++i)
            set_arr_bit(out, i);
        for (int i = (y+height-1) * side + x;   i < (y+height-1) * side + x+width;      ++i)
            set_arr_bit(out, i);
        for (int i = (y+1) * side + x;          i < (y+height-1) * side + x;            i += side)
            set_arr_bit(out, i);
        for (int i = (y+1) * side + x+width-1;  i < (y+height-1) * side + x+width-1;    i += side)
            set_arr_bit(out, i);
    } else {
        for (int i = y * side + x;              i < y * side + x+width;                 ++i)
            clr_arr_bit(out, i);
        for (int i = (y+height-1) * side + x;   i < (y+height-1) * side + x+width;      ++i)
            clr_arr_bit(out, i);
        for (int i = (y+1) * side + x;          i < (y+height-1) * side + x;            i += side)
            clr_arr_bit(out, i);
        for (int i = (y+1) * side + x+width-1;  i < (y+height-1) * side + x+width-1;    i += side)
            clr_arr_bit(out, i);
    }
}

template<int V>
constexpr void qr_encoder<V>::reserve_patterns(uint8_t* out)
{
    draw_rect<true>(0, 6, side, 1, out);
    draw_rect<true>(6, 0, 1, side, out);
    
    draw_rect<true>(0, 0, 9, 9, out);
    draw_rect<true>(side - 8, 0, 8, 9, out);
    draw_rect<true>(0, side - 8, 9, 8, out);

    for (int i = 0; i < N_ALIGN; ++i) {
        for (int j = 0; j < N_ALIGN; ++j) {
            if ((!i && !j) || 
                (!i && j == N_ALIGN - 1) || 
                (!j && i == N_ALIGN - 1) )
                continue;
            draw_rect<true>(imp::qr_align_pos[V - 1][i] - 2, imp::qr_align_pos[V - 1][j] - 2, 5, 5, out);
        }
    }

    if (V >= 7) {
        draw_rect<true>(side - 11, 0, 3, 6, out);
        draw_rect<true>(0, side - 11, 6, 3, out);
    }
}

template<int V>
template<bool H>
constexpr int qr_encoder<V>::rule_1_3_score()
{
    constexpr int y_max  = H ? N_BITS : side;
    constexpr int x_max  = H ? side : N_BITS;
    constexpr int y_step = H ? side : 1;
    constexpr int x_step = H ? 1 : side;
    
    int res = 0;

    for (int y = 0; y < y_max; y += y_step) {
        bool color = get_arr_bit(code, y);
        int finder = color;
        int cnt    = 1;
        for (int x = 1; x < x_max; x += x_step) {
            if (get_arr_bit(code, y + x) == color) {
                ++cnt;
                if (cnt == 5)
                    res += 3;
                if (cnt > 5)
                    ++res;
            } else {
                color = !color;
                cnt = 1;
            }
            // Finder-like
            finder = ((finder << 1) & 0x7ff) | color;
            if (x >= x_step * 10) {
                if (finder == 0x05d || finder == 0x5d0)
                    res += 40;
            }
        }
    }
    return res;
}

template<int V>
constexpr int qr_encoder<V>::penalty_score()
{
    int res = 0;

    res += rule_1_3_score<true>();
    res += rule_1_3_score<false>();

    for (int y = 0; y < N_BITS - side; y += side) {
        for (int x = 0; x < side - 1; ++x) {

            bool c = get_arr_bit(code, y + x);

            if (c == get_arr_bit(code, y + x + 1)  &&
                c == get_arr_bit(code, y + x + side) &&
                c == get_arr_bit(code, y + x + side + 1))
                res += 3;
        }
    }

    int black = 0;
    for (int y = 0; y < N_BITS; y += side) {
        for (int x = 0; x < side; ++x)
            black += get_arr_bit(code, y + x);
    }
    res += abs((black * 100) / N_BITS - 50) / 5 * 10;

    return res;
}

template<int V>
constexpr int qr_encoder<V>::select_mask(qr_ecc ecc, const uint8_t* patterns)
{
    unsigned min_score = -1;
    unsigned score = 0;
    uint8_t mask = 0;

    for (int i = 0; i < 8; ++i) {
        add_format(ecc, i);
        apply_mask(i, patterns);
        score = penalty_score();
        if (score < min_score) {
            mask = i;
            min_score = score;
        }
        apply_mask(i, patterns);
    }
    return mask;
}

template<int V>
constexpr void qr_encoder<V>::apply_mask(int mask, const uint8_t* patterns)
{
    for (int y = 0, dy = 0; y < side; ++y, dy += side) {
        for (int x = 0; x < side; ++x) {

            int coord = dy + x;

            if (get_arr_bit(patterns, coord))
                continue;

            bool keep = true;

            switch (mask) {
                case 0: keep =  (x + y) & 1;                    break;
                case 1: keep =   y & 1;                         break;
                case 2: keep =   x % 3;                         break;
                case 3: keep =  (x + y) % 3;                    break;
                case 4: keep =  (y / 2 + x / 3) & 1;            break;
                case 5: keep =   x * y  % 2 + x * y % 3;        break;
                case 6: keep =  (x * y  % 2 + x * y % 3) & 1;   break;
                case 7: keep = ((x + y) % 2 + x * y % 3) & 1;   break;
            }

            if (!keep) {
                if (get_arr_bit(code, coord))
                    clr_arr_bit(code, coord);
                else
                    set_arr_bit(code, coord);
            }
        }
    }
}

}

#endif