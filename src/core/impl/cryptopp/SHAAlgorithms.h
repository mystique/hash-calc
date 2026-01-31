#ifndef SHA_ALGORITHMS_H
#define SHA_ALGORITHMS_H

#include "CryptoppHashBase.h"
#include <cryptopp/sha.h>
#include <cryptopp/sha3.h>
#include <cryptopp/keccak.h>
#include <cryptopp/shake.h>

namespace core {
namespace impl {

// SHA-1
using SHA1Hash = CryptoppHashBase<CryptoPP::SHA1>;

// SHA-2 Family
using SHA224Hash = CryptoppHashBase<CryptoPP::SHA224>;
using SHA256Hash = CryptoppHashBase<CryptoPP::SHA256>;
using SHA384Hash = CryptoppHashBase<CryptoPP::SHA384>;
using SHA512Hash = CryptoppHashBase<CryptoPP::SHA512>;

// SHA-3 Family
using SHA3_224Hash = CryptoppHashBase<CryptoPP::SHA3_224>;
using SHA3_256Hash = CryptoppHashBase<CryptoPP::SHA3_256>;
using SHA3_384Hash = CryptoppHashBase<CryptoPP::SHA3_384>;
using SHA3_512Hash = CryptoppHashBase<CryptoPP::SHA3_512>;

// SHAKE Family (XOF with fixed output for this app)
template <class T, unsigned int OutSize>
class FixedSHAKE : public T {
public:
    enum { DIGESTSIZE = OutSize };
    static std::string StaticAlgorithmName() { return T::StaticAlgorithmName(); }
    unsigned int DigestSize() const override { return OutSize; }
};

using SHAKE128Hash = CryptoppHashBase<FixedSHAKE<CryptoPP::SHAKE128, 32>>; // 256-bit output
using SHAKE256Hash = CryptoppHashBase<FixedSHAKE<CryptoPP::SHAKE256, 64>>; // 512-bit output

// Keccak Family
using Keccak_224Hash = CryptoppHashBase<CryptoPP::Keccak_224>;
using Keccak_256Hash = CryptoppHashBase<CryptoPP::Keccak_256>;
using Keccak_384Hash = CryptoppHashBase<CryptoPP::Keccak_384>;
using Keccak_512Hash = CryptoppHashBase<CryptoPP::Keccak_512>;

} // namespace impl
} // namespace core

#endif // SHA_ALGORITHMS_H
