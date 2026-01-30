#ifndef MD6_H_CRYPTOPP
#define MD6_H_CRYPTOPP

#include "CryptoppHashBase.h"
#include <cryptopp/cryptlib.h>
#include <cryptopp/secblock.h>
#include <cryptopp/misc.h>
#include <array>
#include <sstream>
#include <cstring>
#include <vector>

namespace core {
namespace impl {

/**
 * @brief MD6 implementation compatible with Crypto++ HashTransformation.
 * 
 * MD6 is a cryptographic hash function designed by Ron Rivest for the NIST hash function competition.
 * Pure C++ implementation without dependencies on external C code.
 * 
 * @tparam DIGEST_BITS The desired hash output size in bits (1-512)
 */
template <unsigned int DIGEST_BITS>
class MD6 : public CryptoPP::HashTransformation {
public:
    CRYPTOPP_CONSTANT(DIGESTSIZE = (DIGEST_BITS + 7) / 8);
    CRYPTOPP_CONSTANT(BLOCKSIZE = 64 * 8); // MD6 uses 64 words of 64 bits = 512 bytes per block
    
    // MD6 constants
    static constexpr int W = 64;      // word size in bits
    static constexpr int N = 89;      // compression input block size in words
    static constexpr int C = 16;      // compression output size in words
    static constexpr int B = 64;      // data words per compression block
    static constexpr int Q_SIZE = 15; // Q constant array size
    static constexpr int K_SIZE = 8;  // key size in words
    static constexpr int MAX_STACK_HEIGHT = 29;
    static constexpr int DEFAULT_L = 64;

    MD6() { 
        static_assert(DIGEST_BITS >= 1 && DIGEST_BITS <= 512, 
                     "MD6 digest size must be between 1 and 512 bits");
        Restart(); 
    }

    std::string AlgorithmName() const override {
        std::stringstream ss;
        ss << "MD6-" << DIGEST_BITS;
        return ss.str();
    }
    
    static std::string StaticAlgorithmName() {
        std::stringstream ss;
        ss << "MD6-" << DIGEST_BITS;
        return ss.str();
    }

    unsigned int DigestSize() const override { return DIGESTSIZE; }
    unsigned int BlockSize() const override { return BLOCKSIZE; }

    void Update(const CryptoPP::byte *input, size_t length) override;
    void TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) override;
    void Restart() override;

private:
    // MD6 Q constant (fractional part of sqrt(6))
    static const uint64_t Q[Q_SIZE];
    
    // Tap positions for feedback shift register
    static constexpr int t0 = 17;
    static constexpr int t1 = 18;
    static constexpr int t2 = 21;
    static constexpr int t3 = 31;
    static constexpr int t4 = 67;
    static constexpr int t5 = 89;
    
    // State variables
    int m_d;                  // digest size in bits
    int m_L;                  // mode parameter
    int m_r;                  // number of rounds
    uint64_t m_bits_processed;
    
    std::array<uint64_t, K_SIZE> m_K;  // key (all zeros for default)
    std::array<std::array<uint64_t, B>, MAX_STACK_HEIGHT> m_B;  // data stack
    std::array<unsigned int, MAX_STACK_HEIGHT> m_bits;          // bits in each level
    std::array<uint64_t, MAX_STACK_HEIGHT> m_i_for_level;       // node indices
    int m_top;                // top of stack
    
    std::vector<uint8_t> m_hashval;  // final hash value
    
    // Helper functions
    int defaultRounds(int d) const;
    void compress(uint64_t *C, const uint64_t *N, int r);
    void mainCompressionLoop(uint64_t *A, int r);
    uint64_t makeControlWord(int r, int L, int z, int p, int keylen, int d) const;
    uint64_t makeNodeID(int ell, int i) const;
    void pack(uint64_t *N, const uint64_t *Q, const uint64_t *K,
              int ell, int i, int r, int L, int z, int p, int keylen, int d,
              const uint64_t *B_data);
    void processBlock(int ell, int final);
    void trimHashval();
    
    // Bit manipulation helpers
    static inline uint64_t rotateRight(uint64_t x, int n) {
        return (x >> n) | (x << (64 - n));
    }
    
    static inline uint64_t rotateLeft(uint64_t x, int n) {
        return (x << n) | (x >> (64 - n));
    }
};

// Common MD6 variants as typedefs
using MD6_128 = MD6<128>;
using MD6_160 = MD6<160>;
using MD6_192 = MD6<192>;
using MD6_224 = MD6<224>;
using MD6_256 = MD6<256>;
using MD6_384 = MD6<384>;
using MD6_512 = MD6<512>;

// Wrapper for CryptoppHashBase interaction
template<typename H>
using MD6Wrapper = CryptoppHashBase<H>;

} // namespace impl
} // namespace core

#endif // MD6_H_CRYPTOPP
