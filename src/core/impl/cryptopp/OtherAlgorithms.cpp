#include "OtherAlgorithms.h"
#include "../../HashAlgorithmFactory.h"
#include <algorithm>

namespace core {
namespace impl {

// CRC32Hash implementation with proper byte order
CRC32Hash::CRC32Hash() : m_hash(std::make_unique<CryptoPP::CRC32>()) {}

void CRC32Hash::update(const uint8_t* data, size_t length) {
    m_hash->Update(data, length);
}

std::vector<uint8_t> CRC32Hash::finalize() {
    std::vector<uint8_t> digest(4);
    m_hash->Final(digest.data());
    // Crypto++ CRC32 outputs in little-endian, but standard CRC-32 display is big-endian
    // Reverse the bytes to match standard CRC-32 output
    std::reverse(digest.begin(), digest.end());
    return digest;
}

void CRC32Hash::reset() {
    m_hash->Restart();
}

// Register algorithms
static HashAlgorithmRegistrar<MD5Hash> s_md5("MD5");
static HashAlgorithmRegistrar<MD2Hash> s_md2("MD2");
static HashAlgorithmRegistrar<MD4Hash> s_md4("MD4");
static HashAlgorithmRegistrar<RIPEMD128Hash> s_ripemd128("RIPEMD-128");
static HashAlgorithmRegistrar<RIPEMD160Hash> s_ripemd160("RIPEMD-160");
static HashAlgorithmRegistrar<RIPEMD256Hash> s_ripemd256("RIPEMD-256");
static HashAlgorithmRegistrar<RIPEMD320Hash> s_ripemd320("RIPEMD-320");
static HashAlgorithmRegistrar<CRC32Hash> s_crc32("CRC32");
static HashAlgorithmRegistrar<Adler32Hash> s_adler32("Adler32");

} // namespace impl
} // namespace core
