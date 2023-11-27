// #ifndef NTH_CRTYPTO_MAC_POLY1305_H
// #define NTH_CRTYPTO_MAC_POLY1305_H

// #include "nth/crypto/util.h"

// namespace nth {
// namespace imp {

// constexpr void poly1305_inner(uint32_t* acc, const uint32_t* r, const void* data, size_t len)
// {
//     const byte* buf = static_cast<const byte*>(data);

//     uint32_t r0 = r[0];
//     uint32_t r1 = r[1];
//     uint32_t r2 = r[2];
//     uint32_t r3 = r[3];
//     uint32_t r4 = r[4];

//     uint32_t u1 = r1 * 5;
//     uint32_t u2 = r2 * 5;
//     uint32_t u3 = r3 * 5;
//     uint32_t u4 = r4 * 5;

//     uint32_t a0 = acc[0];
//     uint32_t a1 = acc[1];
//     uint32_t a2 = acc[2];
//     uint32_t a3 = acc[3];
//     uint32_t a4 = acc[4];

//     while (len > 0) {

//         uint64_t w0, w1, w2, w3, w4;
//         uint64_t c;
//         byte tmp[16];

//         if (len < 16) {
//             zero(tmp, sizeof(tmp));
//             copy(tmp, buf, len);
//             buf = tmp;
//             len = 16;
//         }

//         a0 +=  getle<uint32_t>(buf) & 0x03FFFFFF;
//         a1 += (getle<uint32_t>(buf +  3) >> 2) & 0x03FFFFFF;
//         a2 += (getle<uint32_t>(buf +  6) >> 4) & 0x03FFFFFF;
//         a3 += (getle<uint32_t>(buf +  9) >> 6) & 0x03FFFFFF;
//         a4 += (getle<uint32_t>(buf + 12) >> 8) | 0x01000000;

//         auto mul = [] (auto x, auto y) { return uint64_t(x) * uint64_t(y); };

//         w0 = mul(a0, r0) + mul(a1, u4) + mul(a2, u3) + mul(a3, u2) + mul(a4, u1);
//         w1 = mul(a0, r1) + mul(a1, r0) + mul(a2, u4) + mul(a3, u3) + mul(a4, u2);
//         w2 = mul(a0, r2) + mul(a1, r1) + mul(a2, r0) + mul(a3, u4) + mul(a4, u3);
//         w3 = mul(a0, r3) + mul(a1, r2) + mul(a2, r1) + mul(a3, r0) + mul(a4, u4);
//         w4 = mul(a0, r4) + mul(a1, r3) + mul(a2, r2) + mul(a3, r1) + mul(a4, r0);

//         auto step = [] (uint64_t& c, uint32_t& a, uint64_t& w) {
//             c = w >> 26;
//             a = uint32_t(w) & 0x3ffffff;
//         };
//         step(c, a0, w0); w1 += c;
//         step(c, a1, w1); w2 += c;
//         step(c, a2, w2); w3 += c;
//         step(c, a3, w3); w4 += c;
//         step(c, a4, w4);

//         a0 += uint32_t(c) * 5;
//         a1 += a0 >> 26;
//         a0 &= 0x3ffffff;

//         buf += 16;
//         len -= 16;
//     }

//     acc[0] = a0;
//     acc[1] = a1;
//     acc[2] = a2;
//     acc[3] = a3;
//     acc[4] = a4;
// }

// constexpr void poly1305_run(
//     const void *key, 
//     const void *iv,
// 	void *data, size_t len, 
//     const void *aad, size_t aad_len,
// 	void *tag, 
//     br_chacha20_run ichacha, 
//     int encrypt)
// {
// 	byte pkey[32] = {};
//     byte foot[16];
// 	uint32_t r[5];
//     uint32_t acc[5];
//     uint32_t cc;
//     uint32_t ctl;
//     uint32_t hi;
// 	uint64_t w;
// 	int i;

// 	/*
// 	 * Compute the MAC key. The 'r' value is the first 16 bytes of
// 	 * pkey[].
// 	 */
// 	ichacha(key, iv, 0, pkey, sizeof pkey);

// 	/*
// 	 * If encrypting, ChaCha20 must run first, followed by Poly1305.
// 	 * When decrypting, the operations are reversed.
// 	 */
// 	if (encrypt) {
// 		ichacha(key, iv, 1, data, len);
// 	}

// 	/*
// 	 * Run Poly1305. We must process the AAD, then ciphertext, then
// 	 * the footer (with the lengths). Note that the AAD and ciphertext
// 	 * are meant to be padded with zeros up to the next multiple of 16,
// 	 * and the length of the footer is 16 bytes as well.
// 	 */

// 	r[0] =  getle<uint32_t>(pkey)            & 0x03ffffff;
// 	r[1] = (getle<uint32_t>(pkey +  3) >> 2) & 0x03ffff03;
// 	r[2] = (getle<uint32_t>(pkey +  6) >> 4) & 0x03ffc0ff;
// 	r[3] = (getle<uint32_t>(pkey +  9) >> 6) & 0x03f03fff;
// 	r[4] = (getle<uint32_t>(pkey + 12) >> 8) & 0x000fffff;

// 	zero(acc, sizeof(acc));

//     putle<uint64_t>(uint64_t(aad_len),  foot);
//     putle<uint64_t>(uint64_t(len),      foot + 8);
//     poly1305_inner(acc, r, aad, aad_len);
//     poly1305_inner(acc, r, data, len);
//     poly1305_inner(acc, r, foot, sizeof foot);

// 	/*
// 	 * Finalise modular reduction. This is done with carry propagation
// 	 * and applying the '2^130 = -5 mod p' rule. Note that the output
// 	 * of poly1035_inner() is already mostly reduced, since only
// 	 * acc[1] may be (very slightly) above 2^26. A single loop back
// 	 * to acc[1] will be enough to make the value fit in 130 bits.
// 	 */
// 	cc = 0;
// 	for (i = 1; i <= 6; i ++) {
// 		int j;

// 		j = (i >= 5) ? i - 5 : i;
// 		acc[j] += cc;
// 		cc = acc[j] >> 26;
// 		acc[j] &= 0x03FFFFFF;
// 	}

// 	/*
// 	 * We may still have a value in the 2^130-5..2^130-1 range, in
// 	 * which case we must reduce it again. The code below selects,
// 	 * in constant-time, between 'acc' and 'acc-p',
// 	 */
// 	ctl = GT(acc[0], 0x03FFFFFA);
// 	for (i = 1; i < 5; i ++) {
// 		ctl &= EQ(acc[i], 0x03FFFFFF);
// 	}
// 	cc = 5;
// 	for (i = 0; i < 5; i ++) {
// 		uint32_t t;

// 		t = (acc[i] + cc);
// 		cc = t >> 26;
// 		t &= 0x03FFFFFF;
// 		acc[i] = MUX(ctl, t, acc[i]);
// 	}

// 	/*
// 	 * Convert back the accumulator to 32-bit words, and add the
// 	 * 's' value (second half of pkey[]). That addition is done
// 	 * modulo 2^128.
// 	 */
// 	w = (uint64_t)acc[0] + ((uint64_t)acc[1] << 26) + br_dec32le(pkey + 16);
// 	br_enc32le((unsigned char *)tag, (uint32_t)w);
// 	w = (w >> 32) + ((uint64_t)acc[2] << 20) + br_dec32le(pkey + 20);
// 	br_enc32le((unsigned char *)tag + 4, (uint32_t)w);
// 	w = (w >> 32) + ((uint64_t)acc[3] << 14) + br_dec32le(pkey + 24);
// 	br_enc32le((unsigned char *)tag + 8, (uint32_t)w);
// 	hi = (uint32_t)(w >> 32) + (acc[4] << 8) + br_dec32le(pkey + 28);
// 	br_enc32le((unsigned char *)tag + 12, hi);

// 	/*
// 	 * If decrypting, then ChaCha20 runs _after_ Poly1305.
// 	 */
// 	if (!encrypt) {
// 		ichacha(key, iv, 1, data, len);
// 	}
// }

// }

// // TODO

// }

// #endif