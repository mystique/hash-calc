#ifndef HAVAL_H
#define HAVAL_H

#include "../cryptopp/CryptoppHashBase.h"
#include <cryptopp/cryptlib.h>
#include <cryptopp/secblock.h>
#include <cryptopp/misc.h>
#include <array>
#include <sstream>
#include <cstring>

namespace core {
namespace impl {

/**
 * @brief HAVAL implementation compatible with Crypto++ HashTransformation.
 * 
 * HAVAL is a variable-length, variable-pass hash algorithm.
 * Output sizes: 128, 160, 192, 224, 256 bits.
 * Passes: 3, 4, 5.
 */
template <unsigned int DIGEST_BITS, unsigned int PASSES>
class Haval : public CryptoPP::HashTransformation {
public:
    CRYPTOPP_CONSTANT(DIGESTSIZE = DIGEST_BITS / 8);
    CRYPTOPP_CONSTANT(BLOCKSIZE = 128); // HAVAL uses 1024-bit (128-byte) blocks

    Haval() { Restart(); }

    std::string AlgorithmName() const override {
        std::stringstream ss;
        ss << "HAVAL-" << DIGEST_BITS << "/" << PASSES;
        return ss.str();
    }
    
    static std::string StaticAlgorithmName() {
        std::stringstream ss;
        ss << "HAVAL-" << DIGEST_BITS << "/" << PASSES;
        return ss.str();
    }

    unsigned int DigestSize() const override { return DIGESTSIZE; }
    unsigned int BlockSize() const override { return BLOCKSIZE; }

    void Update(const CryptoPP::byte *input, size_t length) override {
        // Handle buffering and processing blocks
        size_t spaceLeft = BLOCKSIZE - m_count;
        if (length >= spaceLeft) {
            std::memcpy(m_buffer + m_count, input, spaceLeft);
            Transform(m_buffer);
            input += spaceLeft;
            length -= spaceLeft;
            m_count = 0;
            m_processed += BLOCKSIZE;

            while (length >= BLOCKSIZE) {
                Transform(input);
                input += BLOCKSIZE;
                length -= BLOCKSIZE;
                m_processed += BLOCKSIZE;
            }
        }
        
        if (length > 0) {
            std::memcpy(m_buffer + m_count, input, length);
            m_count += length;
        }
    }

    void TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) override {
        ThrowIfInvalidTruncatedSize(digestSize);

        // HAVAL Padding
        // 1. Append 1 bit (using 0x01 as per SharpHash/Spec for LSB)
        // 2. Pad with 0s until length % 128 == 118 (leave 10 bytes: 2 for version info, 8 for length)
        // 3. Append Version/Pass/Size (2 bytes)
        // 4. Append Length in bits (8 bytes)
        
        // Capture original length in bits
        uint64_t totalBits = (m_processed + m_count) * 8;

        // Append padding byte
        m_buffer[m_count++] = 0x01; 
        
        // Check if we have enough space for the trailer (10 bytes: 2 for version + 8 for length)
        if (m_count > (BLOCKSIZE - 10)) {
            // Not enough space in current block, pad to end and transform
            std::memset(m_buffer + m_count, 0, BLOCKSIZE - m_count);
            Transform(m_buffer);
            m_count = 0;
        }
        
        // Pad with zeros to position BLOCKSIZE - 10 (118)
        std::memset(m_buffer + m_count, 0, (BLOCKSIZE - 10) - m_count);
        m_count = BLOCKSIZE - 10;
        
        // Append Version info (2 bytes)
        // Byte 1: (PASSES << 3) | VERSION(1)
        // Byte 2: (DIGEST_BITS >> 2) (Matches DIGEST_BYTES << 1)
        m_buffer[m_count++] = (CryptoPP::byte)((PASSES << 3) | 0x01);
        m_buffer[m_count++] = (CryptoPP::byte)((DIGEST_BITS >> 2) & 0xFF);
        
        // Append Length in bits (64-bit little endian)
        CryptoPP::PutWord(false, CryptoPP::LITTLE_ENDIAN_ORDER, m_buffer + m_count, (CryptoPP::word64)totalBits); 
        m_count += 8;
        
        // Transform the final block
        Transform(m_buffer);

        // Apply final tailoring
        Tailor();

        // Copy output
        // HAVAL output is usually little-endian words
        for (unsigned int i = 0; i < DIGESTSIZE; i++) {
             digest[i] = ((CryptoPP::byte*)m_state)[i]; 
        }
        
        Restart();
    }

    void Restart() override {
        m_count = 0;
        m_processed = 0;
        // Init state based on DIGEST_BITS and PASSES
        // HAVAL initializes state with constants
        // HAVAL standard initialization vector:
        m_state[0] = 0x243F6A88;
        m_state[1] = 0x85A308D3;
        m_state[2] = 0x13198A2E;
        m_state[3] = 0x03707344;
        m_state[4] = 0xA4093822;
        m_state[5] = 0x299F31D0;
        m_state[6] = 0x082EFA98;
        m_state[7] = 0xEC4E6C89;
    }

private:
    void Transform(const CryptoPP::byte *block);
    void Tailor();
    CryptoPP::word32 m_state[8];
    CryptoPP::byte m_buffer[BLOCKSIZE];
    size_t m_count;
    uint64_t m_processed;
};

// Typedefs for registration
using Haval128_3 = Haval<128, 3>;
using Haval160_3 = Haval<160, 3>;
using Haval192_3 = Haval<192, 3>;
using Haval224_3 = Haval<224, 3>;
using Haval256_3 = Haval<256, 3>;

using Haval128_4 = Haval<128, 4>;
using Haval160_4 = Haval<160, 4>;
using Haval192_4 = Haval<192, 4>;
using Haval224_4 = Haval<224, 4>;
using Haval256_4 = Haval<256, 4>;

using Haval128_5 = Haval<128, 5>;
using Haval160_5 = Haval<160, 5>;
using Haval192_5 = Haval<192, 5>;
using Haval224_5 = Haval<224, 5>;
using Haval256_5 = Haval<256, 5>;

// Wrapper for CryptoppHashBase interaction
template<typename H>
using HavalWrapper = CryptoppHashBase<H>;

} // namespace impl
} // namespace core

#endif // HAVAL_H
