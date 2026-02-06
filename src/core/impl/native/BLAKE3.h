#ifndef BLAKE3_H_CRYPTOPP
#define BLAKE3_H_CRYPTOPP

#include "../cryptopp/CryptoppHashBase.h"
#include <cryptopp/cryptlib.h>
#include <cryptopp/secblock.h>
#include <cryptopp/misc.h>
#include <array>
#include <vector>
#include <cstring>
#include <string>

namespace core {
namespace impl {

/**
 * @brief BLAKE3 implementation compatible with Crypto++ HashTransformation.
 * 
 * BLAKE3 is a cryptographic hash function that is much faster than MD5, SHA-1, SHA-2, and SHA-3,
 * yet is at least as secure as the latest standard SHA-3.
 * 
 * This is a pure C++ implementation independent of the reference C code.
 * 
 * Features:
 * - Extremely fast (faster than SHA-1, SHA-2, SHA-3)
 * - Parallelizable (using Merkle tree structure)
 * - Supports arbitrary output length (XOF - Extendable Output Function)
 * - Supports keyed mode and key derivation
 * 
 * @tparam DIGEST_BITS The desired hash output size in bits (default 256)
 */
template <unsigned int DIGEST_BITS = 256>
class BLAKE3 : public CryptoPP::HashTransformation {
public:
    CRYPTOPP_CONSTANT(DIGESTSIZE = (DIGEST_BITS + 7) / 8);
    CRYPTOPP_CONSTANT(BLOCKSIZE = 64);  // BLAKE3 block size
    
    // BLAKE3 constants
    static constexpr unsigned int OUT_LEN = 32;      // Default output length in bytes
    static constexpr unsigned int KEY_LEN = 32;      // Key length in bytes
    static constexpr unsigned int BLOCK_LEN = 64;    // Block length in bytes
    static constexpr unsigned int CHUNK_LEN = 1024;  // Chunk length in bytes
    static constexpr unsigned int MAX_DEPTH = 54;    // Maximum tree depth
    
    // Flags
    enum Flags : uint8_t {
        CHUNK_START         = 1 << 0,
        CHUNK_END           = 1 << 1,
        PARENT              = 1 << 2,
        ROOT                = 1 << 3,
        KEYED_HASH          = 1 << 4,
        DERIVE_KEY_CONTEXT  = 1 << 5,
        DERIVE_KEY_MATERIAL = 1 << 6,
    };

    BLAKE3() { 
        static_assert(DIGEST_BITS >= 8, "BLAKE3 digest size must be at least 8 bits");
        Restart(); 
    }

    std::string AlgorithmName() const override {
        if (DIGEST_BITS == 256) {
            return "BLAKE3";
        }
        return "BLAKE3-" + std::to_string(DIGEST_BITS);
    }
    
    static std::string StaticAlgorithmName() {
        if (DIGEST_BITS == 256) {
            return "BLAKE3";
        }
        return "BLAKE3-" + std::to_string(DIGEST_BITS);
    }

    unsigned int DigestSize() const override { return DIGESTSIZE; }
    unsigned int BlockSize() const override { return BLOCKSIZE; }

    void Update(const CryptoPP::byte *input, size_t length) override;
    void TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) override;
    void Restart() override;

private:
    // BLAKE3 IV (same as BLAKE2 IV)
    static const uint32_t IV[8];
    
    // Message schedule (permutation)
    static const uint8_t MSG_SCHEDULE[7][16];
    
    // Internal state structures
    struct ChunkState {
        uint32_t cv[8];              // Chaining value
        uint64_t chunk_counter;       // Chunk counter
        uint8_t buf[BLOCK_LEN];      // Buffer for partial block
        uint8_t buf_len;             // Current buffer length
        uint8_t blocks_compressed;   // Number of blocks compressed
        uint8_t flags;               // Flags for this chunk
        
        void init(const uint32_t key[8], uint8_t flag);
        void reset(const uint32_t key[8], uint64_t counter);
        size_t len() const;
        size_t fillBuf(const uint8_t* input, size_t input_len);
        void update(const uint8_t* input, size_t input_len);
        uint8_t maybeStartFlag() const;
    };
    
    struct Output {
        uint32_t input_cv[8];
        uint64_t counter;
        uint8_t block[BLOCK_LEN];
        uint8_t block_len;
        uint8_t flags;
        
        void chainingValue(uint8_t cv[32]) const;
        void rootBytes(uint64_t seek, uint8_t* out, size_t out_len) const;
    };
    
    // State variables
    uint32_t m_key[8];                                // Key (IV for default mode)
    ChunkState m_chunk;                               // Current chunk state
    uint8_t m_cv_stack[(MAX_DEPTH + 1) * OUT_LEN];   // Chaining value stack
    uint8_t m_cv_stack_len;                          // Stack length
    
    // Core compression functions
    static void compress(uint32_t cv[8], const uint8_t block[BLOCK_LEN],
                        uint8_t block_len, uint64_t counter, uint8_t flags);
    static void compressXof(const uint32_t cv[8], const uint8_t block[BLOCK_LEN],
                           uint8_t block_len, uint64_t counter, uint8_t flags,
                           uint8_t out[64]);
    
    // Helper functions
    static void g(uint32_t state[16], size_t a, size_t b, size_t c, size_t d,
                 uint32_t mx, uint32_t my);
    static void roundFunction(uint32_t state[16], const uint32_t m[16], size_t round);
    static void permute(uint32_t m[16]);
    
    static Output makeOutput(const uint32_t input_cv[8], const uint8_t block[BLOCK_LEN],
                            uint8_t block_len, uint64_t counter, uint8_t flags);
    static Output chunkOutput(const ChunkState& cs);
    static Output parentOutput(const uint8_t block[BLOCK_LEN], const uint32_t key[8],
                              uint8_t flags);
    
    // Tree building functions
    void hasherPushCv(const uint8_t new_cv[OUT_LEN], uint64_t chunk_counter);
    void hasherMergeCvStack(uint64_t total_len);
    
    // Utility functions
    static uint32_t load32(const void* src);
    static void loadKeyWords(const uint8_t key[KEY_LEN], uint32_t key_words[8]);
    static void store32(void* dst, uint32_t w);
    static void storeCvWords(uint8_t bytes_out[32], const uint32_t cv_words[8]);
    static inline uint32_t rotr32(uint32_t x, int n) {
        return (x >> n) | (x << (32 - n));
    }
    static unsigned int popcnt(uint64_t x);
};

// Common BLAKE3 variants
using BLAKE3_128 = BLAKE3<128>;
using BLAKE3_160 = BLAKE3<160>;
using BLAKE3_192 = BLAKE3<192>;
using BLAKE3_224 = BLAKE3<224>;
using BLAKE3_256 = BLAKE3<256>;
using BLAKE3_384 = BLAKE3<384>;
using BLAKE3_512 = BLAKE3<512>;

// Wrapper for CryptoppHashBase interaction
template<typename H>
using BLAKE3Wrapper = CryptoppHashBase<H>;

} // namespace impl
} // namespace core

#endif // BLAKE3_H_CRYPTOPP
