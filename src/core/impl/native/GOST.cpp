#include "GOST.h"
#include "../../HashAlgorithmFactory.h"
#include "GOST2012Tables.h"  // Must be included before namespace to ensure visibility
#include <algorithm>
#include <cstring>

// Define alignment macro for compatibility
#ifdef _MSC_VER
# define ALIGN(x) __declspec(align(x))
#else
# define ALIGN(x) __attribute__ ((__aligned__(x)))
#endif

namespace core {
namespace impl {

// ============================================================================
// GOST R 34.11-94 Implementation
// Based on reference implementation by Markku-Juhani Saarinen
// Copyright (c) 1998 SSH Communications Security, Finland
// ============================================================================

// GOST 28147-89 S-boxes (standard paramset from reference implementation)
const uint8_t GOST94::SBOX[8][16] = {
    {  4, 10,  9,  2, 13,  8,  0, 14,  6, 11,  1, 12,  7, 15,  5,  3 },
    { 14, 11,  4, 12,  6, 13, 15, 10,  2,  3,  8,  1,  0,  7,  5,  9 },
    {  5,  8,  1, 13, 10,  3,  4,  2, 14, 15, 12,  7,  6,  0,  9, 11 },
    {  7, 13, 10,  1,  0,  8,  9, 15, 14,  4,  6, 12, 11,  2,  5,  3 },
    {  6, 12,  7,  1,  5, 15, 13,  8,  4, 10,  9, 14,  0,  3, 11,  2 },
    {  4, 11, 10,  0,  7,  2,  1, 13,  3,  6,  8,  5,  9, 12, 15, 14 },
    { 13, 11,  4,  1,  3, 15,  5,  9,  0, 10, 14,  7,  6,  8,  2, 12 },
    {  1, 15, 13,  0,  5,  7, 10,  4,  9,  2,  3, 14,  6, 11,  8, 12 }
};

// Precomputed lookup tables for GOST encryption (initialized once)
static uint32_t gost_sbox_1[256];
static uint32_t gost_sbox_2[256];
static uint32_t gost_sbox_3[256];
static uint32_t gost_sbox_4[256];
static bool gost_sbox_initialized = false;

// Initialize the precomputed S-box lookup tables
// Combines S-box substitution with rotations for efficiency
static void gost_init_table() {
    if (gost_sbox_initialized) return;

    int i = 0;
    for (int a = 0; a < 16; a++) {
        uint32_t ax = GOST94::SBOX[1][a] << 15;
        uint32_t bx = GOST94::SBOX[3][a] << 23;
        uint32_t cx = GOST94::SBOX[5][a];
        cx = (cx >> 1) | (cx << 31);  // Rotate right by 1 = rotate left by 31
        uint32_t dx = GOST94::SBOX[7][a] << 7;

        for (int b = 0; b < 16; b++) {
            gost_sbox_1[i] = ax | (GOST94::SBOX[0][b] << 11);
            gost_sbox_2[i] = bx | (GOST94::SBOX[2][b] << 19);
            gost_sbox_3[i] = cx | (GOST94::SBOX[4][b] << 27);
            gost_sbox_4[i] = dx | (GOST94::SBOX[6][b] << 3);
            i++;
        }
    }

    gost_sbox_initialized = true;
}

// GOST 28147-89 encryption round macro implementation
// Performs a full encryption round using precomputed lookup tables
static inline void gost_encrypt_round(uint32_t& r, uint32_t& l, uint32_t k1, uint32_t k2) {
    uint32_t t;

    // First half-round
    t = k1 + r;
    l ^= gost_sbox_1[t & 0xff] ^
         gost_sbox_2[(t >> 8) & 0xff] ^
         gost_sbox_3[(t >> 16) & 0xff] ^
         gost_sbox_4[t >> 24];

    // Second half-round
    t = k2 + l;
    r ^= gost_sbox_1[t & 0xff] ^
         gost_sbox_2[(t >> 8) & 0xff] ^
         gost_sbox_3[(t >> 16) & 0xff] ^
         gost_sbox_4[t >> 24];
}

// GOST 28147-89 block encryption
static void gost_encrypt(const uint32_t key[8], uint32_t& r, uint32_t& l) {
    // 32 rounds total: 3 forward passes (keys 0-7) + 1 reverse pass (keys 7-0)
    gost_encrypt_round(r, l, key[0], key[1]);
    gost_encrypt_round(r, l, key[2], key[3]);
    gost_encrypt_round(r, l, key[4], key[5]);
    gost_encrypt_round(r, l, key[6], key[7]);

    gost_encrypt_round(r, l, key[0], key[1]);
    gost_encrypt_round(r, l, key[2], key[3]);
    gost_encrypt_round(r, l, key[4], key[5]);
    gost_encrypt_round(r, l, key[6], key[7]);

    gost_encrypt_round(r, l, key[0], key[1]);
    gost_encrypt_round(r, l, key[2], key[3]);
    gost_encrypt_round(r, l, key[4], key[5]);
    gost_encrypt_round(r, l, key[6], key[7]);

    gost_encrypt_round(r, l, key[7], key[6]);
    gost_encrypt_round(r, l, key[5], key[4]);
    gost_encrypt_round(r, l, key[3], key[2]);
    gost_encrypt_round(r, l, key[1], key[0]);

    // Final swap
    uint32_t t = r;
    r = l;
    l = t;
}

// "chi" compression function - implements the core GOST R 34.11-94 compression
// This matches the reference implementation exactly
static void gosthash_compress(uint32_t h[8], const uint32_t m[8]) {
    uint32_t l, r, t, key[8], u[8], v[8], w[8], s[8];

    std::memcpy(u, h, sizeof(u));
    std::memcpy(v, m, sizeof(v));

    for (int i = 0; i < 8; i += 2) {
        // w = u XOR v
        w[0] = u[0] ^ v[0];
        w[1] = u[1] ^ v[1];
        w[2] = u[2] ^ v[2];
        w[3] = u[3] ^ v[3];
        w[4] = u[4] ^ v[4];
        w[5] = u[5] ^ v[5];
        w[6] = u[6] ^ v[6];
        w[7] = u[7] ^ v[7];

        // P-Transformation: byte-level permutation to create encryption key
        key[0] = (w[0] & 0x000000ff) | ((w[2] & 0x000000ff) << 8) |
                 ((w[4] & 0x000000ff) << 16) | ((w[6] & 0x000000ff) << 24);
        key[1] = ((w[0] & 0x0000ff00) >> 8) | (w[2] & 0x0000ff00) |
                 ((w[4] & 0x0000ff00) << 8) | ((w[6] & 0x0000ff00) << 16);
        key[2] = ((w[0] & 0x00ff0000) >> 16) | ((w[2] & 0x00ff0000) >> 8) |
                 (w[4] & 0x00ff0000) | ((w[6] & 0x00ff0000) << 8);
        key[3] = ((w[0] & 0xff000000) >> 24) | ((w[2] & 0xff000000) >> 16) |
                 ((w[4] & 0xff000000) >> 8) | (w[6] & 0xff000000);
        key[4] = (w[1] & 0x000000ff) | ((w[3] & 0x000000ff) << 8) |
                 ((w[5] & 0x000000ff) << 16) | ((w[7] & 0x000000ff) << 24);
        key[5] = ((w[1] & 0x0000ff00) >> 8) | (w[3] & 0x0000ff00) |
                 ((w[5] & 0x0000ff00) << 8) | ((w[7] & 0x0000ff00) << 16);
        key[6] = ((w[1] & 0x00ff0000) >> 16) | ((w[3] & 0x00ff0000) >> 8) |
                 (w[5] & 0x00ff0000) | ((w[7] & 0x00ff0000) << 8);
        key[7] = ((w[1] & 0xff000000) >> 24) | ((w[3] & 0xff000000) >> 16) |
                 ((w[5] & 0xff000000) >> 8) | (w[7] & 0xff000000);

        // Encryption transformation
        r = h[i];
        l = h[i + 1];
        gost_encrypt(key, r, l);

        s[i] = r;
        s[i + 1] = l;

        if (i == 6)
            break;

        // U = A(U): linear feedback shift
        l = u[0] ^ u[2];
        r = u[1] ^ u[3];
        u[0] = u[2];
        u[1] = u[3];
        u[2] = u[4];
        u[3] = u[5];
        u[4] = u[6];
        u[5] = u[7];
        u[6] = l;
        u[7] = r;

        // Add constant C_3 at iteration 2
        if (i == 2) {
            u[0] ^= 0xff00ff00;
            u[1] ^= 0xff00ff00;
            u[2] ^= 0x00ff00ff;
            u[3] ^= 0x00ff00ff;
            u[4] ^= 0x00ffff00;
            u[5] ^= 0xff0000ff;
            u[6] ^= 0x000000ff;
            u[7] ^= 0xff00ffff;
        }

        // V = A(A(V)): apply linear transformation twice
        l = v[0];
        r = v[2];
        v[0] = v[4];
        v[2] = v[6];
        v[4] = l ^ r;
        v[6] = v[0] ^ r;
        l = v[1];
        r = v[3];
        v[1] = v[5];
        v[3] = v[7];
        v[5] = l ^ r;
        v[7] = v[1] ^ r;
    }

    // 12 rounds of LFSR (computed from a product matrix) and XOR with message
    u[0] = m[0] ^ s[6];
    u[1] = m[1] ^ s[7];
    u[2] = m[2] ^ (s[0] << 16) ^ (s[0] >> 16) ^ (s[0] & 0xffff) ^
           (s[1] & 0xffff) ^ (s[1] >> 16) ^ (s[2] << 16) ^ s[6] ^ (s[6] << 16) ^
           (s[7] & 0xffff0000) ^ (s[7] >> 16);
    u[3] = m[3] ^ (s[0] & 0xffff) ^ (s[0] << 16) ^ (s[1] & 0xffff) ^
           (s[1] << 16) ^ (s[1] >> 16) ^ (s[2] << 16) ^ (s[2] >> 16) ^
           (s[3] << 16) ^ s[6] ^ (s[6] << 16) ^ (s[6] >> 16) ^ (s[7] & 0xffff) ^
           (s[7] << 16) ^ (s[7] >> 16);
    u[4] = m[4] ^
           (s[0] & 0xffff0000) ^ (s[0] << 16) ^ (s[0] >> 16) ^
           (s[1] & 0xffff0000) ^ (s[1] >> 16) ^ (s[2] << 16) ^ (s[2] >> 16) ^
           (s[3] << 16) ^ (s[3] >> 16) ^ (s[4] << 16) ^ (s[6] << 16) ^
           (s[6] >> 16) ^ (s[7] & 0xffff) ^ (s[7] << 16) ^ (s[7] >> 16);
    u[5] = m[5] ^ (s[0] << 16) ^ (s[0] >> 16) ^ (s[0] & 0xffff0000) ^
           (s[1] & 0xffff) ^ s[2] ^ (s[2] >> 16) ^ (s[3] << 16) ^ (s[3] >> 16) ^
           (s[4] << 16) ^ (s[4] >> 16) ^ (s[5] << 16) ^ (s[6] << 16) ^
           (s[6] >> 16) ^ (s[7] & 0xffff0000) ^ (s[7] << 16) ^ (s[7] >> 16);
    u[6] = m[6] ^ s[0] ^ (s[1] >> 16) ^ (s[2] << 16) ^ s[3] ^ (s[3] >> 16) ^
           (s[4] << 16) ^ (s[4] >> 16) ^ (s[5] << 16) ^ (s[5] >> 16) ^ s[6] ^
           (s[6] << 16) ^ (s[6] >> 16) ^ (s[7] << 16);
    u[7] = m[7] ^ (s[0] & 0xffff0000) ^ (s[0] << 16) ^ (s[1] & 0xffff) ^
           (s[1] << 16) ^ (s[2] >> 16) ^ (s[3] << 16) ^ s[4] ^ (s[4] >> 16) ^
           (s[5] << 16) ^ (s[5] >> 16) ^ (s[6] >> 16) ^ (s[7] & 0xffff) ^
           (s[7] << 16) ^ (s[7] >> 16);

    // 16 * 1 round of LFSR and XOR with hash
    v[0] = h[0] ^ (u[1] << 16) ^ (u[0] >> 16);
    v[1] = h[1] ^ (u[2] << 16) ^ (u[1] >> 16);
    v[2] = h[2] ^ (u[3] << 16) ^ (u[2] >> 16);
    v[3] = h[3] ^ (u[4] << 16) ^ (u[3] >> 16);
    v[4] = h[4] ^ (u[5] << 16) ^ (u[4] >> 16);
    v[5] = h[5] ^ (u[6] << 16) ^ (u[5] >> 16);
    v[6] = h[6] ^ (u[7] << 16) ^ (u[6] >> 16);
    v[7] = h[7] ^ (u[0] & 0xffff0000) ^ (u[0] << 16) ^ (u[7] >> 16) ^
           (u[1] & 0xffff0000) ^ (u[1] << 16) ^ (u[6] << 16) ^ (u[7] & 0xffff0000);

    // 61 rounds of LFSR, mixing hash (computed from a product matrix)
    h[0] = (v[0] & 0xffff0000) ^ (v[0] << 16) ^ (v[0] >> 16) ^ (v[1] >> 16) ^
           (v[1] & 0xffff0000) ^ (v[2] << 16) ^ (v[3] >> 16) ^ (v[4] << 16) ^
           (v[5] >> 16) ^ v[5] ^ (v[6] >> 16) ^ (v[7] << 16) ^ (v[7] >> 16) ^
           (v[7] & 0xffff);
    h[1] = (v[0] << 16) ^ (v[0] >> 16) ^ (v[0] & 0xffff0000) ^ (v[1] & 0xffff) ^
           v[2] ^ (v[2] >> 16) ^ (v[3] << 16) ^ (v[4] >> 16) ^ (v[5] << 16) ^
           (v[6] << 16) ^ v[6] ^ (v[7] & 0xffff0000) ^ (v[7] >> 16);
    h[2] = (v[0] & 0xffff) ^ (v[0] << 16) ^ (v[1] << 16) ^ (v[1] >> 16) ^
           (v[1] & 0xffff0000) ^ (v[2] << 16) ^ (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^
           (v[5] >> 16) ^ v[6] ^ (v[6] >> 16) ^ (v[7] & 0xffff) ^ (v[7] << 16) ^
           (v[7] >> 16);
    h[3] = (v[0] << 16) ^ (v[0] >> 16) ^ (v[0] & 0xffff0000) ^
           (v[1] & 0xffff0000) ^ (v[1] >> 16) ^ (v[2] << 16) ^ (v[2] >> 16) ^ v[2] ^
           (v[3] << 16) ^ (v[4] >> 16) ^ v[4] ^ (v[5] << 16) ^ (v[6] << 16) ^
           (v[7] & 0xffff) ^ (v[7] >> 16);
    h[4] = (v[0] >> 16) ^ (v[1] << 16) ^ v[1] ^ (v[2] >> 16) ^ v[2] ^
           (v[3] << 16) ^ (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^ (v[5] >> 16) ^
           v[5] ^ (v[6] << 16) ^ (v[6] >> 16) ^ (v[7] << 16);
    h[5] = (v[0] << 16) ^ (v[0] & 0xffff0000) ^ (v[1] << 16) ^ (v[1] >> 16) ^
           (v[1] & 0xffff0000) ^ (v[2] << 16) ^ v[2] ^ (v[3] >> 16) ^ v[3] ^
           (v[4] << 16) ^ (v[4] >> 16) ^ v[4] ^ (v[5] << 16) ^ (v[6] << 16) ^
           (v[6] >> 16) ^ v[6] ^ (v[7] << 16) ^ (v[7] >> 16) ^ (v[7] & 0xffff0000);
    h[6] = v[0] ^ v[2] ^ (v[2] >> 16) ^ v[3] ^ (v[3] << 16) ^ v[4] ^
           (v[4] >> 16) ^ (v[5] << 16) ^ (v[5] >> 16) ^ v[5] ^ (v[6] << 16) ^
           (v[6] >> 16) ^ v[6] ^ (v[7] << 16) ^ v[7];
    h[7] = v[0] ^ (v[0] >> 16) ^ (v[1] << 16) ^ (v[1] >> 16) ^ (v[2] << 16) ^
           (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^ v[4] ^ (v[5] >> 16) ^ v[5] ^
           (v[6] << 16) ^ (v[6] >> 16) ^ (v[7] << 16) ^ v[7];
}

// Mix in a 32-byte chunk and update checksum
static void gosthash_bytes(uint32_t hash[8], uint32_t sum[8], const uint8_t *buf) {
    uint32_t m[8];
    uint32_t a, c;

    // Convert bytes to little-endian 32-bit words and compute sum
    int j = 0;
    c = 0;
    for (int i = 0; i < 8; i++) {
        a = (static_cast<uint32_t>(buf[j])) |
            (static_cast<uint32_t>(buf[j + 1]) << 8) |
            (static_cast<uint32_t>(buf[j + 2]) << 16) |
            (static_cast<uint32_t>(buf[j + 3]) << 24);
        j += 4;
        m[i] = a;
        c = a + c + sum[i];
        sum[i] = c;
        c = (c < a) ? 1 : 0;
    }

    // Apply compression function
    gosthash_compress(hash, m);
}

void GOST94::Restart() {
    // Initialize lookup tables on first use
    gost_init_table();

    // Clear all state
    std::memset(m_H, 0, sizeof(m_H));
    std::memset(m_S, 0, sizeof(m_S));
    std::memset(m_buffer, 0, sizeof(m_buffer));
    m_bufferLen = 0;
    m_totalLen = 0;
}

void GOST94::Update(const CryptoPP::byte *input, size_t length) {
    const uint8_t* buf = input;
    size_t len = length;

    // Fill partial buffer if it has data
    size_t i = m_bufferLen;
    size_t j = 0;
    while (i < 32 && j < len) {
        m_buffer[i++] = buf[j++];
    }

    if (i < 32) {
        m_bufferLen = i;
        m_totalLen += len;
        return;
    }

    // Process the now-complete buffer
    gosthash_bytes(m_H, m_S, m_buffer);
    m_bufferLen = 0;

    // Process complete 32-byte blocks
    while ((j + 32) <= len) {
        gosthash_bytes(m_H, m_S, &buf[j]);
        j += 32;
    }

    // Buffer remaining bytes
    i = 0;
    while (j < len) {
        m_buffer[i++] = buf[j++];
    }
    m_bufferLen = i;
    m_totalLen += len;
}

void GOST94::TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) {
    uint32_t len[8];

    // Prepare length in bits (256-bit little-endian)
    std::memset(len, 0, sizeof(len));
    uint64_t bitLen = m_totalLen * 8;
    len[0] = static_cast<uint32_t>(bitLen & 0xFFFFFFFF);
    len[1] = static_cast<uint32_t>(bitLen >> 32);

    // Process final partial block if present
    if (m_bufferLen > 0) {
        std::memset(&m_buffer[m_bufferLen], 0, 32 - m_bufferLen);
        gosthash_bytes(m_H, m_S, m_buffer);
    }

    // Mix in length and checksum
    gosthash_compress(m_H, len);
    gosthash_compress(m_H, m_S);

    // Convert output to bytes (little-endian)
    size_t outputSize = std::min(digestSize, static_cast<size_t>(32));
    int outIdx = 0;
    for (int i = 0; i < 8 && outIdx < static_cast<int>(outputSize); i++) {
        uint32_t a = m_H[i];
        if (outIdx < static_cast<int>(outputSize)) digest[outIdx++] = static_cast<uint8_t>(a);
        if (outIdx < static_cast<int>(outputSize)) digest[outIdx++] = static_cast<uint8_t>(a >> 8);
        if (outIdx < static_cast<int>(outputSize)) digest[outIdx++] = static_cast<uint8_t>(a >> 16);
        if (outIdx < static_cast<int>(outputSize)) digest[outIdx++] = static_cast<uint8_t>(a >> 24);
    }

    Restart();
}

// ============================================================================
// GOST R 34.11-2012 Implementation (Streebog)
// Based on reference implementation by Alexey Degtyarev
// ============================================================================

// Define uint512_u union for constants (must be defined before including constants)
// This matches the reference implementation structure
ALIGN(16) union uint512_u {
    uint64_t QWORD[8];
    uint8_t B[64];
};

// Constants from GOST R 34.11-2012 specification
// buffer0: all zeros
ALIGN(16) static const union uint512_u buffer0 = {{
    0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
}};

// buffer512: represents 512 bits (0x200 = 512 in little-endian)
ALIGN(16) static const union uint512_u buffer512 = {{
    0x0000000000000200ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL
}};

// C[12]: Round constants for GOST R 34.11-2012
ALIGN(16) static const union uint512_u C[12] = {
    {{
         0xdd806559f2a64507ULL, 0x05767436cc744d23ULL, 0xa2422a08a460d315ULL, 0x4b7ce09192676901ULL,
         0x714eb88d7585c4fcULL, 0x2f6a76432e45d016ULL, 0xebcb2f81c0657c1fULL, 0xb1085bda1ecadae9ULL
    }},
    {{
         0xe679047021b19bb7ULL, 0x55dda21bd7cbcd56ULL, 0x5cb561c2db0aa7caULL, 0x9ab5176b12d69958ULL,
         0x61d55e0f16b50131ULL, 0xf3feea720a232b98ULL, 0x4fe39d460f70b5d7ULL, 0x6fa3b58aa99d2f1aULL
    }},
    {{
         0x991e96f50aba0ab2ULL, 0xc2b6f443867adb31ULL, 0xc1c93a376062db09ULL, 0xd3e20fe490359eb1ULL,
         0xf2ea7514b1297b7bULL, 0x06f15e5f529c1f8bULL, 0x0a39fc286a3d8435ULL, 0xf574dcac2bce2fc7ULL
    }},
    {{
         0x220cbebc84e3d12eULL, 0x3453eaa193e837f1ULL, 0xd8b71333935203beULL, 0xa9d72c82ed03d675ULL,
         0x9d721cad685e353fULL, 0x488e857e335c3c7dULL, 0xf948e1a05d71e4ddULL, 0xef1fdfb3e81566d2ULL
    }},
    {{
         0x601758fd7c6cfe57ULL, 0x7a56a27ea9ea63f5ULL, 0xdfff00b723271a16ULL, 0xbfcd1747253af5a3ULL,
         0x359e35d7800fffbdULL, 0x7f151c1f1686104aULL, 0x9a3f410c6ca92363ULL, 0x4bea6bacad474799ULL
    }},
    {{
         0xfa68407a46647d6eULL, 0xbf71c57236904f35ULL, 0x0af21f66c2bec6b6ULL, 0xcffaa6b71c9ab7b4ULL,
         0x187f9ab49af08ec6ULL, 0x2d66c4f95142a46cULL, 0x6fa4c33b7a3039c0ULL, 0xae4faeae1d3ad3d9ULL
    }},
    {{
         0x8886564d3a14d493ULL, 0x3517454ca23c4af3ULL, 0x06476983284a0504ULL, 0x0992abc52d822c37ULL,
         0xd3473e33197a93c9ULL, 0x399ec6c7e6bf87c9ULL, 0x51ac86febf240954ULL, 0xf4c70e16eeaac5ecULL
    }},
    {{
         0xa47f0dd4bf02e71eULL, 0x36acc2355951a8d9ULL, 0x69d18d2bd1a5c42fULL, 0xf4892bcb929b0690ULL,
         0x89b4443b4ddbc49aULL, 0x4eb7f8719c36de1eULL, 0x03e7aa020c6e4141ULL, 0x9b1f5b424d93c9a7ULL
    }},
    {{
         0x7261445183235adbULL, 0x0e38dc92cb1f2a60ULL, 0x7b2b8a9aa6079c54ULL, 0x800a440bdbb2ceb1ULL,
         0x3cd955b7e00d0984ULL, 0x3a7d3a1b25894224ULL, 0x944c9ad8ec165fdeULL, 0x378f5a541631229bULL
    }},
    {{
         0x74b4c7fb98459cedULL, 0x3698fad1153bb6c3ULL, 0x7a1e6c303b7652f4ULL, 0x9fe76702af69334bULL,
         0x1fffe18a1b336103ULL, 0x8941e71cff8a78dbULL, 0x382ae548b2e4f3f3ULL, 0xabbedea680056f52ULL
    }},
    {{
         0x6bcaa4cd81f32d1bULL, 0xdea2594ac06fd85dULL, 0xefbacd1d7d476e98ULL, 0x8a1d71efea48b9caULL,
         0x2001802114846679ULL, 0xd8fa6bbbebab0761ULL, 0x3002c6cd635afe94ULL, 0x7bcd9ed0efc889fbULL
    }},
    {{
         0x48bc924af11bd720ULL, 0xfaf417d5d9b21b99ULL, 0xe71da4aa88e12852ULL, 0x5d80ef9d1891cc86ULL,
         0xf82012d430219f9bULL, 0xcda43c32bcdf1d77ULL, 0xd21380b00449b17aULL, 0x378ee767f11631baULL
    }}
};

// GOST 2012 S-box (π substitution) - from GOST R 34.11-2012 standard
template<unsigned int DIGEST_BITS>
const uint8_t GOST2012<DIGEST_BITS>::SBOX[256] = {
    0xFC, 0xEE, 0xDD, 0x11, 0xCF, 0x6E, 0x31, 0x16, 0xFB, 0xC4, 0xFA, 0xDA, 0x23, 0xC5, 0x04, 0x4D,
    0xE9, 0x77, 0xF0, 0xDB, 0x93, 0x2E, 0x99, 0xBA, 0x17, 0x36, 0xF1, 0xBB, 0x14, 0xCD, 0x5F, 0xC1,
    0xF9, 0x18, 0x65, 0x5A, 0xE2, 0x5C, 0xEF, 0x21, 0x81, 0x1C, 0x3C, 0x42, 0x8B, 0x01, 0x8E, 0x4F,
    0x05, 0x84, 0x02, 0xAE, 0xE3, 0x6A, 0x8F, 0xA0, 0x06, 0x0B, 0xED, 0x98, 0x7F, 0xD4, 0xD3, 0x1F,
    0xEB, 0x34, 0x2C, 0x51, 0xEA, 0xC8, 0x48, 0xAB, 0xF2, 0x2A, 0x68, 0xA2, 0xFD, 0x3A, 0xCE, 0xCC,
    0xB5, 0x70, 0x0E, 0x56, 0x08, 0x0C, 0x76, 0x12, 0xBF, 0x72, 0x13, 0x47, 0x9C, 0xB7, 0x5D, 0x87,
    0x15, 0xA1, 0x96, 0x29, 0x10, 0x7B, 0x9A, 0xC7, 0xF3, 0x91, 0x78, 0x6F, 0x9D, 0x9E, 0xB2, 0xB1,
    0x32, 0x75, 0x19, 0x3D, 0xFF, 0x35, 0x8A, 0x7E, 0x6D, 0x54, 0xC6, 0x80, 0xC3, 0xBD, 0x0D, 0x57,
    0xDF, 0xF5, 0x24, 0xA9, 0x3E, 0xA8, 0x43, 0xC9, 0xD7, 0x79, 0xD6, 0xF6, 0x7C, 0x22, 0xB9, 0x03,
    0xE0, 0x0F, 0xEC, 0xDE, 0x7A, 0x94, 0xB0, 0xBC, 0xDC, 0xE8, 0x28, 0x50, 0x4E, 0x33, 0x0A, 0x4A,
    0xA7, 0x97, 0x60, 0x73, 0x1E, 0x00, 0x62, 0x44, 0x1A, 0xB8, 0x38, 0x82, 0x64, 0x9F, 0x26, 0x41,
    0xAD, 0x45, 0x46, 0x92, 0x27, 0x5E, 0x55, 0x2F, 0x8C, 0xA3, 0xA5, 0x7D, 0x69, 0xD5, 0x95, 0x3B,
    0x07, 0x58, 0xB3, 0x40, 0x86, 0xAC, 0x1D, 0xF7, 0x30, 0x37, 0x6B, 0xE4, 0x88, 0xD9, 0xE7, 0x89,
    0xE1, 0x1B, 0x83, 0x49, 0x4C, 0x3F, 0xF8, 0xFE, 0x8D, 0x53, 0xAA, 0x90, 0xCA, 0xD8, 0x85, 0x61,
    0x20, 0x71, 0x67, 0xA4, 0x2D, 0x2B, 0x09, 0x5B, 0xCB, 0x9B, 0x25, 0xD0, 0xBE, 0xE5, 0x6C, 0x52,
    0x59, 0xA6, 0x74, 0xD2, 0xE6, 0xF4, 0xB4, 0xC0, 0xD1, 0x66, 0xAF, 0xC2, 0x39, 0x4B, 0x63, 0xB6
};

// ============================================================================
// GOST 2012 Utility Functions
// ============================================================================

// Load 64-bit value from byte array (little-endian)
template<unsigned int DIGEST_BITS>
uint64_t GOST2012<DIGEST_BITS>::load64(const uint8_t* src) {
    return (static_cast<uint64_t>(src[0]) << 0)  |
           (static_cast<uint64_t>(src[1]) << 8)  |
           (static_cast<uint64_t>(src[2]) << 16) |
           (static_cast<uint64_t>(src[3]) << 24) |
           (static_cast<uint64_t>(src[4]) << 32) |
           (static_cast<uint64_t>(src[5]) << 40) |
           (static_cast<uint64_t>(src[6]) << 48) |
           (static_cast<uint64_t>(src[7]) << 56);
}

// Store 64-bit value to byte array (little-endian)
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::store64(uint8_t* dst, uint64_t value) {
    dst[0] = static_cast<uint8_t>(value >> 0);
    dst[1] = static_cast<uint8_t>(value >> 8);
    dst[2] = static_cast<uint8_t>(value >> 16);
    dst[3] = static_cast<uint8_t>(value >> 24);
    dst[4] = static_cast<uint8_t>(value >> 32);
    dst[5] = static_cast<uint8_t>(value >> 40);
    dst[6] = static_cast<uint8_t>(value >> 48);
    dst[7] = static_cast<uint8_t>(value >> 56);
}

// 512-bit addition (x = x + y) - from reference implementation
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::add512(uint64_t x[8], const uint64_t y[8]) {
    uint64_t carry = 0;
    for (int i = 0; i < 8; i++) {
        const uint64_t left = x[i];
        uint64_t sum = left + y[i] + carry;
        // Carry detection: if sum changed, check if it's less than left
        if (sum != left)
            carry = (sum < left) ? 1 : 0;
        x[i] = sum;
    }
}

// XOR operation: result = a XOR b
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::X(const uint64_t a[8], const uint64_t b[8], uint64_t result[8]) {
    result[0] = a[0] ^ b[0];
    result[1] = a[1] ^ b[1];
    result[2] = a[2] ^ b[2];
    result[3] = a[3] ^ b[3];
    result[4] = a[4] ^ b[4];
    result[5] = a[5] ^ b[5];
    result[6] = a[6] ^ b[6];
    result[7] = a[7] ^ b[7];
}

// ============================================================================
// GOST 2012 Core Transformations (from reference implementation)
// ============================================================================

// XLPS - Combined X (XOR), L (Linear), P (Permutation), S (Substitution) transformation
// Uses precomputed tables Ax for efficiency - matches reference implementation
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::XLPS(const uint64_t x[8], const uint64_t y[8], uint64_t data[8]) {
    // Step 1: XOR x and y
    uint64_t r0 = x[0] ^ y[0];
    uint64_t r1 = x[1] ^ y[1];
    uint64_t r2 = x[2] ^ y[2];
    uint64_t r3 = x[3] ^ y[3];
    uint64_t r4 = x[4] ^ y[4];
    uint64_t r5 = x[5] ^ y[5];
    uint64_t r6 = x[6] ^ y[6];
    uint64_t r7 = x[7] ^ y[7];

    // Step 2: Apply S-box, permutation, and linear transformation using precomputed tables
    // The Ax tables combine S-box substitution, bit permutation, and linear transformation
    // Process 8 bytes (one from each uint64_t) - matches reference implementation
    for (int i = 0; i < 8; i++) {
        data[i]  = Ax[0][r0 & 0xFF];
        data[i] ^= Ax[1][r1 & 0xFF];
        data[i] ^= Ax[2][r2 & 0xFF];
        data[i] ^= Ax[3][r3 & 0xFF];
        data[i] ^= Ax[4][r4 & 0xFF];
        data[i] ^= Ax[5][r5 & 0xFF];
        data[i] ^= Ax[6][r6 & 0xFF];
        data[i] ^= Ax[7][r7 & 0xFF];

        // Shift to next byte
        r0 >>= 8;
        r1 >>= 8;
        r2 >>= 8;
        r3 >>= 8;
        r4 >>= 8;
        r5 >>= 8;
        r6 >>= 8;
        r7 >>= 8;
    }
}

// E - Encryption function (implements the key schedule and 12 rounds)
// This is NOT a separate function in reference - it's inlined in g()
// Keeping for compatibility but matching reference behavior
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::E(uint64_t K[8], const uint64_t m[8], uint64_t state[8]) {
    // Reference implementation does this differently - see g() below
    // This function is kept for interface compatibility but not used
    X(K, m, state);
    for (int i = 0; i < 12; i++) {
        XLPS(K, C[i].QWORD, K);
        XLPS(state, K, state);
    }
}

// g - Compression function (the heart of GOST 2012)
// EXACTLY matches reference implementation gost3411-2012-core.c lines 122-140
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::g(uint64_t h[8], const uint64_t N[8], const uint64_t m[8]) {
    uint64_t Ki[8], data[8];

    // XLPS(h, N, &data)
    XLPS(h, N, data);

    // Starting E() - Ki = data
    std::memcpy(Ki, data, sizeof(Ki));

    // XLPS(&Ki, (const union uint512_u *)&m[0], &data)
    // In reference, m is unsigned char*, cast to uint512_u*
    // Here m is already uint64_t[8], so we can use it directly
    XLPS(Ki, m, data);

    // 11 rounds (i = 0 to 10)
    for (unsigned int i = 0; i < 11; i++) {
        // ROUND(i, &Ki, &data) expands to:
        // XLPS(&Ki, &C[i], &Ki)
        XLPS(Ki, C[i].QWORD, Ki);
        // XLPS(&Ki, &data, &data)
        XLPS(Ki, data, data);
    }

    // Final round (i = 11) - different from loop
    // XLPS(&Ki, &C[11], &Ki)
    XLPS(Ki, C[11].QWORD, Ki);
    // X(&Ki, &data, &data)
    X(Ki, data, data);
    // E() done

    // X(&data, h, &data) - data = data XOR h
    X(data, h, data);
    // X(&data, m, h) - h = data XOR m
    X(data, m, h);
}

// ============================================================================
// GOST 2012 Main Hash Functions (from reference implementation)
// ============================================================================

// Initialize hash context - matches reference GOST34112012Init
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::Restart() {
    // Clear all state
    std::memset(m_h, 0, sizeof(m_h));
    std::memset(m_N, 0, sizeof(m_N));
    std::memset(m_Sigma, 0, sizeof(m_Sigma));
    std::memset(m_buffer, 0, sizeof(m_buffer));
    m_bufferLen = 0;

    // Set initial value based on digest size
    // For 256-bit: IV = 0x0101...01
    // For 512-bit: IV = 0x0000...00
    if (DIGEST_BITS == 256) {
        for (int i = 0; i < 8; i++) {
            m_h[i] = 0x0101010101010101ULL;
        }
    }
    // else: already zeroed by memset for 512-bit
}

// Process a complete 512-bit block - matches reference stage2
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::stage2(const uint64_t data[8]) {
    // Apply compression function: h = g(h, N, data)
    g(m_h, m_N, data);

    // Update block counter: N = N + 512
    add512(m_N, buffer512.QWORD);

    // Update checksum: Sigma = Sigma + data
    add512(m_Sigma, data);
}

// Finalization stage (padding and final compressions) - matches reference stage3
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::stage3() {
    // Create length block BEFORE padding (length in bits of the final partial block)
    uint64_t lengthBlock[8] = {0};
    lengthBlock[0] = m_bufferLen * 8;  // Convert bytes to bits

    // Pad the buffer with 0x01 followed by zeros
    m_buffer[m_bufferLen] = 0x01;
    std::memset(m_buffer + m_bufferLen + 1, 0, 64 - m_bufferLen - 1);

    // Convert buffer to uint64_t array
    uint64_t data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = load64(m_buffer + i * 8);
    }

    // Process the padded block
    g(m_h, m_N, data);

    // Update N with the length of the final block (BEFORE padding)
    add512(m_N, lengthBlock);

    // Update checksum with padded data
    add512(m_Sigma, data);

    // Final compressions with zero block
    // g(h, 0, N)
    g(m_h, buffer0.QWORD, m_N);

    // g(h, 0, Sigma)
    g(m_h, buffer0.QWORD, m_Sigma);
}

// Update hash with new data - matches reference GOST34112012Update
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::Update(const CryptoPP::byte *input, size_t length) {
    const uint8_t* data = input;

    // Handle buffered data first
    if (m_bufferLen > 0) {
        size_t toCopy = std::min(length, static_cast<size_t>(64 - m_bufferLen));
        std::memcpy(m_buffer + m_bufferLen, data, toCopy);
        m_bufferLen += toCopy;
        data += toCopy;
        length -= toCopy;

        // If buffer is full, process it
        if (m_bufferLen == 64) {
            uint64_t block[8];
            for (int i = 0; i < 8; i++) {
                block[i] = load64(m_buffer + i * 8);
            }
            stage2(block);
            m_bufferLen = 0;
        }
    }

    // Process complete 64-byte blocks
    while (length >= 64) {
        uint64_t block[8];
        for (int i = 0; i < 8; i++) {
            block[i] = load64(data + i * 8);
        }
        stage2(block);
        data += 64;
        length -= 64;
    }

    // Buffer remaining data
    if (length > 0) {
        std::memcpy(m_buffer, data, length);
        m_bufferLen = length;
    }
}

// Finalize and output the hash - matches reference GOST34112012Final
template<unsigned int DIGEST_BITS>
void GOST2012<DIGEST_BITS>::TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) {
    // Perform finalization
    stage3();

    // Output the hash - matches reference implementation
    size_t outputSize = std::min(digestSize, static_cast<size_t>(DIGESTSIZE));

    if (DIGEST_BITS == 256) {
        // For 256-bit: use upper 32 bytes (m_h[4] through m_h[7])
        // Reference: memcpy(digest, &(CTX->hash.QWORD[4]), 32);
        std::memcpy(digest, reinterpret_cast<const uint8_t*>(&m_h[4]), outputSize);
    } else {
        // For 512-bit: use all 64 bytes (m_h[0] through m_h[7])
        // Reference: memcpy(digest, &(CTX->hash.QWORD[0]), 64);
        std::memcpy(digest, reinterpret_cast<const uint8_t*>(&m_h[0]), outputSize);
    }

    // Reset for next use
    Restart();
}

// ============================================================================
// Template Instantiations
// ============================================================================

template class GOST2012<256>;
template class GOST2012<512>;

// ============================================================================
// Algorithm Registration
// ============================================================================

// Register GOST 94
static HashAlgorithmRegistrar<GOST94Wrapper> reg_gost94("GOST-R-34.11-94");

// Register GOST 2012 variants
static HashAlgorithmRegistrar<GOST2012Wrapper<GOST2012_256>> reg_gost2012_256("GOST-R-34.11-2012-256");
static HashAlgorithmRegistrar<GOST2012Wrapper<GOST2012_512>> reg_gost2012_512("GOST-R-34.11-2012-512");

// Aliases
static HashAlgorithmRegistrar<GOST2012Wrapper<GOST2012_256>> reg_streebog256("Streebog-256");
static HashAlgorithmRegistrar<GOST2012Wrapper<GOST2012_512>> reg_streebog512("Streebog-512");

} // namespace impl
} // namespace core
