#ifndef CRYPTOPP_HASH_BASE_H
#define CRYPTOPP_HASH_BASE_H

#include "../IHashAlgorithm.h"
#include <cryptopp/cryptlib.h>
#include <memory>

namespace core {
namespace impl {

/**
 * @brief Base class for Crypto++ based hash implementations.
 * 
 * Template wrapper that works with any CryptoPP::HashTransformation.
 */
template<typename HashImpl>
class CryptoppHashBase : public IHashAlgorithm {
public:
    CryptoppHashBase() : m_hash(std::make_unique<HashImpl>()) {}

    std::string getName() const override {
        return HashImpl::StaticAlgorithmName();
    }

    size_t getDigestSize() const override {
        return HashImpl::DIGESTSIZE;
    }

    void update(const uint8_t* data, size_t length) override {
        m_hash->Update(data, length);
    }

    std::vector<uint8_t> finalize() override {
        std::vector<uint8_t> digest(getDigestSize());
        m_hash->Final(digest.data());
        return digest;
    }

    void reset() override {
        m_hash->Restart();
    }

private:
    std::unique_ptr<HashImpl> m_hash;
};

} // namespace impl
} // namespace core

#endif // CRYPTOPP_HASH_BASE_H
