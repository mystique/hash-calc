#include "CRC.h"
#include "../../HashAlgorithmFactory.h"
#include <cryptopp/misc.h>
#include <cstring>

namespace core {
namespace impl {

// ============================================================================
// CRC-8 Implementation
// ============================================================================

void CRC8::InitTable() {
    if (m_tableInitialized) return;

    const uint8_t polynomial = 0x07; // CRC-8/CCITT polynomial

    for (uint32_t i = 0; i < 256; ++i) {
        uint8_t crc = static_cast<uint8_t>(i);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc = crc << 1;
            }
        }
        m_table[i] = crc;
    }

    m_tableInitialized = true;
}

void CRC8::Restart() {
    InitTable();
    m_crc = 0x00; // Initial value for CRC-8/CCITT
}

void CRC8::Update(const CryptoPP::byte *input, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        uint8_t index = m_crc ^ input[i];
        m_crc = m_table[index];
    }
}

void CRC8::TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) {
    ThrowIfInvalidTruncatedSize(digestSize);

    // No final XOR for this variant
    digest[0] = static_cast<CryptoPP::byte>(m_crc);

    Restart();
}

// ============================================================================
// CRC-16 Implementation
// ============================================================================

void CRC16::InitTable() {
    if (m_tableInitialized) return;
    
    const uint16_t polynomial = 0x1021; // CRC-16/CCITT-FALSE polynomial
    
    for (uint32_t i = 0; i < 256; ++i) {
        uint16_t crc = static_cast<uint16_t>(i << 8);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc = crc << 1;
            }
        }
        m_table[i] = crc;
    }
    
    m_tableInitialized = true;
}

void CRC16::Restart() {
    InitTable();
    m_crc = 0xFFFF; // Initial value for CRC-16/CCITT-FALSE
}

void CRC16::Update(const CryptoPP::byte *input, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        uint8_t index = (m_crc >> 8) ^ input[i];
        m_crc = (m_crc << 8) ^ m_table[index];
    }
}

void CRC16::TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) {
    ThrowIfInvalidTruncatedSize(digestSize);
    
    // No final XOR for this variant
    // Store as big-endian
    digest[0] = static_cast<CryptoPP::byte>(m_crc >> 8);
    digest[1] = static_cast<CryptoPP::byte>(m_crc & 0xFF);
    
    Restart();
}

// ============================================================================
// CRC-32C Implementation
// ============================================================================

void CRC32C::InitTable() {
    if (m_tableInitialized) return;
    
    const uint32_t polynomial = 0x1EDC6F41; // CRC-32C (Castagnoli) polynomial
    
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc = crc >> 1;
            }
        }
        m_table[i] = crc;
    }
    
    m_tableInitialized = true;
}

void CRC32C::Restart() {
    InitTable();
    m_crc = 0xFFFFFFFF; // Initial value
}

void CRC32C::Update(const CryptoPP::byte *input, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        uint8_t index = (m_crc ^ input[i]) & 0xFF;
        m_crc = (m_crc >> 8) ^ m_table[index];
    }
}

void CRC32C::TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) {
    ThrowIfInvalidTruncatedSize(digestSize);
    
    // Final XOR with 0xFFFFFFFF
    uint32_t final_crc = m_crc ^ 0xFFFFFFFF;
    
    // Store as little-endian (common for CRC-32)
    digest[0] = static_cast<CryptoPP::byte>(final_crc & 0xFF);
    digest[1] = static_cast<CryptoPP::byte>((final_crc >> 8) & 0xFF);
    digest[2] = static_cast<CryptoPP::byte>((final_crc >> 16) & 0xFF);
    digest[3] = static_cast<CryptoPP::byte>((final_crc >> 24) & 0xFF);
    
    Restart();
}

// ============================================================================
// CRC-64 Implementation
// ============================================================================

void CRC64::InitTable() {
    if (m_tableInitialized) return;
    
    const uint64_t polynomial = 0x42F0E1EBA9EA3693ULL; // CRC-64/ECMA-182 polynomial
    
    for (uint32_t i = 0; i < 256; ++i) {
        uint64_t crc = static_cast<uint64_t>(i) << 56;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000000000000000ULL) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc = crc << 1;
            }
        }
        m_table[i] = crc;
    }
    
    m_tableInitialized = true;
}

void CRC64::Restart() {
    InitTable();
    m_crc = 0x0000000000000000ULL; // Initial value for CRC-64/ECMA-182
}

void CRC64::Update(const CryptoPP::byte *input, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        uint8_t index = (m_crc >> 56) ^ input[i];
        m_crc = (m_crc << 8) ^ m_table[index];
    }
}

void CRC64::TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) {
    ThrowIfInvalidTruncatedSize(digestSize);
    
    // No final XOR for this variant
    // Store as big-endian
    digest[0] = static_cast<CryptoPP::byte>((m_crc >> 56) & 0xFF);
    digest[1] = static_cast<CryptoPP::byte>((m_crc >> 48) & 0xFF);
    digest[2] = static_cast<CryptoPP::byte>((m_crc >> 40) & 0xFF);
    digest[3] = static_cast<CryptoPP::byte>((m_crc >> 32) & 0xFF);
    digest[4] = static_cast<CryptoPP::byte>((m_crc >> 24) & 0xFF);
    digest[5] = static_cast<CryptoPP::byte>((m_crc >> 16) & 0xFF);
    digest[6] = static_cast<CryptoPP::byte>((m_crc >> 8) & 0xFF);
    digest[7] = static_cast<CryptoPP::byte>(m_crc & 0xFF);
    
    Restart();
}

// ============================================================================
// Algorithm Registration
// ============================================================================

static HashAlgorithmRegistrar<CRCWrapper<CRC8>> reg_crc8("CRC-8");
static HashAlgorithmRegistrar<CRCWrapper<CRC16>> reg_crc16("CRC-16");
static HashAlgorithmRegistrar<CRCWrapper<CRC32C>> reg_crc32c("CRC-32C");
static HashAlgorithmRegistrar<CRCWrapper<CRC64>> reg_crc64("CRC-64");

} // namespace impl
} // namespace core
