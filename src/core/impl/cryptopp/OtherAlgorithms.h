#ifndef OTHER_ALGORITHMS_H
#define OTHER_ALGORITHMS_H

#include "CryptoppHashBase.h"

// In newer Crypto++ versions, MD5 might be in Weak namespace
// We'll define a helper to handle pulling it in or just use the header
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>
#include <cryptopp/md2.h>
#include <cryptopp/md4.h>
#include <cryptopp/ripemd.h>
#include <cryptopp/crc.h>
#include <cryptopp/adler32.h>

namespace core {
namespace impl {

// MD5
using MD5Hash = CryptoppHashBase<CryptoPP::Weak::MD5>;

// MD2
using MD2Hash = CryptoppHashBase<CryptoPP::Weak::MD2>;

// MD4
using MD4Hash = CryptoppHashBase<CryptoPP::Weak::MD4>;

// RIPEMD Family
using RIPEMD128Hash = CryptoppHashBase<CryptoPP::RIPEMD128>;
using RIPEMD160Hash = CryptoppHashBase<CryptoPP::RIPEMD160>;
using RIPEMD256Hash = CryptoppHashBase<CryptoPP::RIPEMD256>;
using RIPEMD320Hash = CryptoppHashBase<CryptoPP::RIPEMD320>;

// CRC32 with proper byte order handling
class CRC32Hash : public IHashAlgorithm {
public:
    CRC32Hash();

    std::string getName() const override {
        return "CRC32";
    }

    size_t getDigestSize() const override {
        return 4;
    }

    void update(const uint8_t* data, size_t length) override;
    std::vector<uint8_t> finalize() override;
    void reset() override;

private:
    std::unique_ptr<CryptoPP::CRC32> m_hash;
};

// Adler32
using Adler32Hash = CryptoppHashBase<CryptoPP::Adler32>;

} // namespace impl
} // namespace core

#endif // OTHER_ALGORITHMS_H
