#ifndef CRC_H
#define CRC_H

#include "../cryptopp/CryptoppHashBase.h"
#include <cryptopp/cryptlib.h>
#include <cryptopp/secblock.h>
#include <array>
#include <sstream>

namespace core {
namespace impl {

/**
 * @brief CRC-16 implementation (CRC-16/CCITT-FALSE variant)
 * Polynomial: 0x1021
 * Initial value: 0xFFFF
 * No final XOR, no reflection
 */
class CRC16 : public CryptoPP::HashTransformation {
public:
    CRYPTOPP_CONSTANT(DIGESTSIZE = 2);  // 16 bits = 2 bytes
    CRYPTOPP_CONSTANT(BLOCKSIZE = 1);

    CRC16() { Restart(); }

    std::string AlgorithmName() const override {
        return "CRC-16";
    }
    
    static std::string StaticAlgorithmName() {
        return "CRC-16";
    }

    unsigned int DigestSize() const override { return DIGESTSIZE; }
    unsigned int BlockSize() const override { return BLOCKSIZE; }

    void Update(const CryptoPP::byte *input, size_t length) override;
    void TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) override;
    void Restart() override;

private:
    void InitTable();
    uint16_t m_crc;
    std::array<uint16_t, 256> m_table;
    bool m_tableInitialized;
};

/**
 * @brief CRC-32C implementation (Castagnoli)
 * Polynomial: 0x1EDC6F41
 * Initial value: 0xFFFFFFFF
 * Final XOR: 0xFFFFFFFF
 * Used in iSCSI, SCTP, etc.
 */
class CRC32C : public CryptoPP::HashTransformation {
public:
    CRYPTOPP_CONSTANT(DIGESTSIZE = 4);  // 32 bits = 4 bytes
    CRYPTOPP_CONSTANT(BLOCKSIZE = 1);

    CRC32C() { Restart(); }

    std::string AlgorithmName() const override {
        return "CRC-32C";
    }
    
    static std::string StaticAlgorithmName() {
        return "CRC-32C";
    }

    unsigned int DigestSize() const override { return DIGESTSIZE; }
    unsigned int BlockSize() const override { return BLOCKSIZE; }

    void Update(const CryptoPP::byte *input, size_t length) override;
    void TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) override;
    void Restart() override;

private:
    void InitTable();
    uint32_t m_crc;
    std::array<uint32_t, 256> m_table;
    bool m_tableInitialized;
};

/**
 * @brief CRC-64 implementation (ECMA-182 variant)
 * Polynomial: 0x42F0E1EBA9EA3693
 * Initial value: 0x0000000000000000
 * No final XOR, no reflection
 */
class CRC64 : public CryptoPP::HashTransformation {
public:
    CRYPTOPP_CONSTANT(DIGESTSIZE = 8);  // 64 bits = 8 bytes
    CRYPTOPP_CONSTANT(BLOCKSIZE = 1);

    CRC64() { Restart(); }

    std::string AlgorithmName() const override {
        return "CRC-64";
    }
    
    static std::string StaticAlgorithmName() {
        return "CRC-64";
    }

    unsigned int DigestSize() const override { return DIGESTSIZE; }
    unsigned int BlockSize() const override { return BLOCKSIZE; }

    void Update(const CryptoPP::byte *input, size_t length) override;
    void TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) override;
    void Restart() override;

private:
    void InitTable();
    uint64_t m_crc;
    std::array<uint64_t, 256> m_table;
    bool m_tableInitialized;
};

// Wrapper types for registration
template<typename CRCImpl>
using CRCWrapper = CryptoppHashBase<CRCImpl>;

} // namespace impl
} // namespace core

#endif // CRC_H
