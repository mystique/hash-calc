#ifndef MODERN_ALGORITHMS_H
#define MODERN_ALGORITHMS_H

#include "CryptoppHashBase.h"
#include <cryptopp/blake2.h>

namespace core {
namespace impl {

// BLAKE2 Family
using BLAKE2bHash = CryptoppHashBase<CryptoPP::BLAKE2b>;
using BLAKE2sHash = CryptoppHashBase<CryptoPP::BLAKE2s>;

} // namespace impl
} // namespace core

#endif // MODERN_ALGORITHMS_H
