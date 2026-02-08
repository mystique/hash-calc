#ifndef GOST_H_NATIVE
#define GOST_H_NATIVE

#include "../cryptopp/CryptoppHashBase.h"
#include <cryptopp/cryptlib.h>
#include <cryptopp/secblock.h>
#include <array>
#include <vector>
#include <cstring>
#include <string>

namespace core {
namespace impl {

/**
 * @brief GOST R 34.11-94 hash algorithm implementation.
 *
 * GOST R 34.11-94 is a Russian cryptographic hash function standard.
 * It produces a 256-bit (32-byte) hash value.
 *
 * This implementation is based on the GOST R 34.11-94 specification
 * and uses the GOST 28147-89 block cipher as its compression function.
 *
 * Features:
 * - 256-bit output
 * - Uses GOST 28147-89 cipher
 * - Processes 256-bit blocks
 */
class GOST94 : public CryptoPP::HashTransformation {
public:
    CRYPTOPP_CONSTANT(DIGESTSIZE = 32);  // 256 bits
    CRYPTOPP_CONSTANT(BLOCKSIZE = 32);   // 256 bits

    GOST94() { Restart(); }

    std::string AlgorithmName() const override {
        return "GOST-R-34.11-94";
    }

    static std::string StaticAlgorithmName() {
        return "GOST-R-34.11-94";
    }

    unsigned int DigestSize() const override { return DIGESTSIZE; }
    unsigned int BlockSize() const override { return BLOCKSIZE; }

    void Update(const CryptoPP::byte *input, size_t length) override;
    void TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) override;
    void Restart() override;

    // GOST 28147-89 S-boxes (standard paramset) - public for table initialization
    static const uint8_t SBOX[8][16];

private:

    // Internal state
    uint32_t m_H[8];         // Hash state (256 bits as 8x32-bit words)
    uint32_t m_S[8];         // Checksum (256 bits as 8x32-bit words)
    uint8_t m_buffer[32];    // Input buffer
    size_t m_bufferLen;      // Current buffer length
    uint64_t m_totalLen;     // Total length processed
};

/**
 * @brief GOST R 34.11-2012 hash algorithm implementation.
 *
 * GOST R 34.11-2012 (Streebog) is the updated Russian cryptographic hash standard.
 * It supports both 256-bit and 512-bit output lengths.
 *
 * This is a modern hash function with improved security properties compared to GOST 94.
 *
 * Features:
 * - Supports 256-bit and 512-bit output
 * - Based on AES-like compression function
 * - Processes 512-bit blocks
 * - Uses Merkle-Damgård construction
 *
 * @tparam DIGEST_BITS The desired hash output size in bits (256 or 512)
 */
template <unsigned int DIGEST_BITS = 256>
class GOST2012 : public CryptoPP::HashTransformation {
public:
    CRYPTOPP_CONSTANT(DIGESTSIZE = DIGEST_BITS / 8);
    CRYPTOPP_CONSTANT(BLOCKSIZE = 64);  // 512 bits

    // Union type for 512-bit values (compatible with reference implementation)
    union uint512_u {
        uint64_t QWORD[8];
        uint8_t B[64];
    };

    GOST2012() {
        static_assert(DIGEST_BITS == 256 || DIGEST_BITS == 512,
                      "GOST 2012 only supports 256 or 512 bit digests");
        Restart();
    }

    std::string AlgorithmName() const override {
        if (DIGEST_BITS == 256) {
            return "GOST-R-34.11-2012-256";
        }
        return "GOST-R-34.11-2012-512";
    }

    static std::string StaticAlgorithmName() {
        if (DIGEST_BITS == 256) {
            return "GOST-R-34.11-2012-256";
        }
        return "GOST-R-34.11-2012-512";
    }

    unsigned int DigestSize() const override { return DIGESTSIZE; }
    unsigned int BlockSize() const override { return BLOCKSIZE; }

    void Update(const CryptoPP::byte *input, size_t length) override;
    void TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) override;
    void Restart() override;

private:
    // Internal state (512 bits each)
    uint64_t m_h[8];         // Hash state
    uint64_t m_N[8];         // Block counter
    uint64_t m_Sigma[8];     // Checksum
    uint8_t m_buffer[64];    // Input buffer
    size_t m_bufferLen;      // Current buffer length

    // Core compression function stages
    void stage2(const uint8_t data[64]);
    void stage3();

    // Helper functions
    static void g(uint64_t h[8], const uint64_t N[8], const uint64_t m[8]);
    static void XLPS(const uint64_t x[8], const uint64_t y[8], uint64_t data[8]);
    static void X(const uint64_t a[8], const uint64_t b[8], uint64_t result[8]);

    // Utility functions
    static void add512(uint64_t x[8], const uint64_t y[8]);
};

// Common GOST 2012 variants
using GOST2012_256 = GOST2012<256>;
using GOST2012_512 = GOST2012<512>;

// Wrapper for CryptoppHashBase interaction
using GOST94Wrapper = CryptoppHashBase<GOST94>;
template<typename H>
using GOST2012Wrapper = CryptoppHashBase<H>;

} // namespace impl
} // namespace core

#endif // GOST_H_NATIVE
