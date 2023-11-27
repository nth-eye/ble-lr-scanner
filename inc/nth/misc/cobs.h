#ifndef NTH_MISC_COBS_H
#define NTH_MISC_COBS_H

#include "nth/util/meta.h"
#include "nth/misc/crc.h"

namespace nth {

/**
 * @brief Callback function signature for writing COBS output.
 *
 * @param data[in] Pointer to bytes to write.
 * @param size[in] Number of bytes to write.
 * @return The number of bytes that were actually written.
 */
using cobs_write_handler = size_t(const byte* data, size_t size);

/**
 * @brief Incremental COBS encoder of byte streams with internal buffering.
 *
 * Designed for scenarios where data arrives in fragments and needs to be encoded 
 * on-the-fly without preallocating large output buffers. It provides methods for 
 * resetting the internal state, "sinking" incoming data segments, and finalizing 
 * the encoding output through a user-defined callback. Internals take 256 bytes.
 *
 * @note Final chunk includes 0x00 delimiter.
 */
struct cobs_pipe_encoder {

    constexpr void reset()
    {
        code = 0;
    }
    constexpr void sink(std::span<const byte> in, cobs_write_handler write)
    {
        for (auto b : in) step(b, write);
    }
    constexpr void stop(cobs_write_handler write)
    {
        buf[code++] = 0;
        write(&code, code + 1);
        reset();
    }
protected:
    constexpr void step(byte b, cobs_write_handler write)
    {
        if (code == 0xfe || !b) {
            code++;
            write(&code, code);
            code = 0;
            if (!b) return;
        }
        buf[code++] = b;
    }
protected:
    byte code = 0;
    byte buf[255] = {};
};

/**
 * @brief Incremental COBS encoder with integrated CRC-32-IEEE 802.3 checksum.
 * 
 * Enhances 'cobs_pipe_encoder' by computing a CRC (polynomial 0x04C11DB7) on 
 * input data, adding integrity check to the frames, which is often a requirement 
 * for serial interfaces. The checksum is COBS-encoded as if it was appended in 
 * little-endian byte order to the input. In theory, must be more efficient than 
 * calculating checksum separatery (1 pass through data instead of 2).
 *
 * @note Final chunk includes 0x00 delimiter.
 */
struct cobs_pipe_encoder_with_crc : cobs_pipe_encoder {

    constexpr void reset()
    {
        code = 0;
        crc = 0xffffffff;
    }
    constexpr void sink(std::span<const byte> in, cobs_write_handler write)
    {
        for (auto b : in) {
            step(b, write);
            crc = imp::crc_table<uint32_t(0x04c11db7)>[(crc ^ b) & 0xff] ^ (crc >> 8);
        }
    }
    constexpr void stop(cobs_write_handler write)
    {
        crc ^= 0xffffffff;
        step(byte(crc >>  0), write);
        step(byte(crc >>  8), write);
        step(byte(crc >> 16), write);
        step(byte(crc >> 24), write);
        buf[code++] = 0;
        write(&code, code + 1);
        reset();
    }
private:
    uint32_t crc = 0xffffffff;
};

/**
 * @brief Encode data using Consistent Overhead Byte Stuffing (COBS) with a custom output function.
 *
 * Encodes the input data using COBS and writes the encoded data using the provided 
 * `write` function. This version of the `cobs_encode` function is useful when the 
 * input data is available all at once and needed to be sent to a non-contiguous 
 * storage or directly to a communication interface, e.g. `stdio`. @note Does NOT
 * write 0x00 delimiter.
 *
 * @param[in] in Input span of bytes to encode.
 * @param[in] write Function to call for writing the encoded bytes.
 * @return Total number of bytes written, including the COBS control bytes.
 */
constexpr size_t cobs_encode(std::span<const byte> in, cobs_write_handler write)
{
    auto pdat = in.data();
    auto code = byte(1);
    auto written = size_t(0);
    auto write_chunk = [&] (const byte* p) {
        written += write(&code, 1);
        written += write(pdat, p - pdat);
    };
    for (const auto& b : in) {
        if (code == 0xff || !b) {
            write_chunk(&b);
            pdat = &b + !b;
            code = 1;
        }
        if (b) ++code;
    }
    write_chunk(in.data() + in.size());
    return written;
}

/**
 * @brief Encode data using Consistent Overhead Byte Stuffing (COBS) directly into an output buffer.
 *
 * Encodes the input data using COBS and writes the encoded data directly to an 
 * output buffer. This version of the `cobs_encode` function is useful when the 
 * input data is available all at once and needed to be stored in a contiguous 
 * memory area. @note Does NOT write 0x00 delimiter.
 *
 * @param[in] in Input span of bytes to encode.
 * @param[out] out Output span for the encoded data.
 * @return Size of the encoded output. Zero if output span is too small.
 */
constexpr size_t cobs_encode(std::span<const byte> in, std::span<byte> out)
{
    if (out.size() < in.size() + 1) {
        return 0;
    }
    auto code = byte(1);
    auto plen = out.begin();
    auto pdat = out.begin() + 1;
    for (auto b : in) {
        if (code == 0xff || !b) {
            plen[0] = code; 
            plen = pdat++; 
            code = 1;
        }
        if (pdat + bool(b) > out.end()) {
            return 0;
        }
        if (b) {
            *pdat++ = b;
            ++code;
        }
    }
    *plen = code;
    return pdat - out.begin();
}

/**
 * @brief Encode data with write callback using COBS with an appended CRC-32-IEEE 802.3.
 *
 * Enhances 'cobs_encode' by computing a CRC (polynomial 0x04C11DB7) on input data on-the-fly, 
 * adding integrity check to the frames, which is often a requirement for transmission over 
 * serial interfaces. The checksum is COBS-encoded as if it was appended in little-endian 
 * byte order to the input. In theory, must be more efficient than calculating checksum
 * separatery (1 pass through data instead of 2). @note Does NOT write 0x00 delimiter.
 *
 * @param[in] in Input span of bytes to encode.
 * @param[in] write Function to call for writing the encoded bytes.
 * @return Total number of bytes written, including the COBS control bytes.
 */
constexpr size_t cobs_encode_with_crc(std::span<const byte> in, cobs_write_handler write)
{
    auto crc = uint32_t(0xffffffff);
    auto code = byte(1);
    auto ptmp = in.data();
    auto written = size_t(0);
    auto step = [&] (const byte& b) {
        if (code == 0xff || !b) {
            written += write(&code, 1);
            written += write(ptmp, &b - ptmp);
            ptmp = &b + !b;
            code = 1;
        }
        if (b) ++code;
    };
    for (const auto& b : in) {
        step(b);
        crc = imp::crc_table<uint32_t(0x04c11db7)>[(crc ^ b) & 0xff] ^ (crc >> 8);
    }
    crc ^= 0xffffffff;
    auto pdat = ptmp;
    auto cb = std::array {
        byte(crc >>  0),
        byte(crc >>  8),
        byte(crc >> 16),
        byte(crc >> 24),
    };
    ptmp = cb.data();
    for (const auto& b : cb) step(b);
    written += write(&code, 1);
    written += write(pdat, in.data() + in.size() - pdat);
    written += write(ptmp, cb.data() + cb.size() - ptmp);
    return written;
}

/**
 * @brief Encode data into output buffer using COBS with an appended CRC-32-IEEE 802.3.
 *
 * Enhances 'cobs_encode' by computing a CRC (polynomial 0x04C11DB7) on input data on-the-fly, 
 * adding integrity check to the frames, which is often a requirement for transmission over 
 * serial interfaces. The checksum is COBS-encoded as if it was appended in little-endian 
 * byte order to the input. In theory, must be more efficient than calculating checksum
 * separatery (1 pass through data instead of 2). @note Does NOT write 0x00 delimiter.
 *
 * @param[in] in Input span of bytes to encode.
 * @param[out] out Output span for the encoded data.
 * @return Size of the encoded output, including CRC. Zero if output span is too small.
 */
constexpr size_t cobs_encode_with_crc(std::span<const byte> in, std::span<byte> out)
{
    if (out.size() < in.size() + 5) {
        return 0;
    }
    auto crc = uint32_t(0xffffffff);
    auto code = byte(1);
    auto plen = out.begin();
    auto pdat = out.begin() + 1;
    auto step = [&] (byte b) {
        if (code == 0xff || !b) {
            plen[0] = code;
            plen = pdat++;
            code = 1;
        }
        if (pdat + bool(b) > out.end()) {
            return false;
        }
        if (b) {
            *pdat++ = b;
            ++code;
        }
        return true;
    };
    for (auto b : in) {
        crc = imp::crc_table<uint32_t(0x04c11db7)>[(crc ^ b) & 0xff] ^ (crc >> 8);
        if (step(b) == false) return 0;
    }
    crc ^= 0xffffffff;
    for (auto b : { byte(crc >>  0), byte(crc >>  8), 
                    byte(crc >> 16), byte(crc >> 24) }) 
    {
        if (step(b) == false) return 0;
    }
    *plen = code;
    return pdat - out.begin();
}

// ANCHOR WIP

using cobs_decoder_handler = void(const byte* data, size_t size, size_t left);

struct cobs_pipe_decoder {

    constexpr void reset()
    {
        size = 0;
        code = 0;
    }
    constexpr void sink(std::span<const byte> in, cobs_decoder_handler write)
    {
        for (auto b : in) step(b, write);
    }
private:
    constexpr void step(byte b, cobs_decoder_handler write)
    {
        if (b == 0x00) {
            write(buf, size, code - size);
            reset();
            return;
        }
        if (code == size) {
            write(buf, size, 0); // successful chunk feeback
            size = 0;
            if (code && b != 0xff)
                buf[size++] = 0;
            code = b;
        } else {
            buf[size++] = b;
        }
    }
private:
    uint8_t size = 0;
    uint8_t code = 0;
    uint8_t buf[254] = {};
};

}

#endif

        // switch (parser_state)
        // {
        // case cobs_state::idle:
        //     if (c) {
        //         rx_buffer_size = 0;
        //         chunk_code = c;
        //         chunk_size = c;
        //         parser_state = cobs_state::data;
        //     }
        // break;
        // case cobs_state::data:
        //     --chunk_size;
        //     if (c) {
        //         if (chunk_size == 0) {
        //             if (chunk_code != 0xff) {
        //                 rx_buffer[rx_buffer_size++] = 0;
        //             }
        //             chunk_code = c;
        //             chunk_size = c;
        //         } else {
        //             rx_buffer[rx_buffer_size++] = c;
        //         }
        //     } else {
        //         parser_state = cobs_state::idle;
        //         return chunk_size ? 
        //             cobs_event::frame_malformed:
        //             cobs_event::frame_end;
        //     }
        // break;
        // case cobs_state::over:
        //     if (!c) {
        //         parser_state = cobs_state::idle;
        //         return cobs_event::frame_start;
        //     }
        // break;
        // }
        // if (rx_buffer_size >= sizeof(rx_buffer)) {
        //     rx_buffer_size = 0;
        //     parser_state = cobs_state::over;
        //     return cobs_event::buffer_overflow;
        // }
        // return cobs_event::idle;