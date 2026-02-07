#ifndef EXOTIC_ALGORITHMS_H
#define EXOTIC_ALGORITHMS_H

#include "CryptoppHashBase.h"
#include <cryptopp/tiger.h>
#include <cryptopp/whrlpool.h>
// Check for SM3 availability or just include it.
// Standard Crypto++ 8.x+ has SM3.
#include <cryptopp/sm3.h>
#include <cryptopp/lsh.h>

namespace core {
namespace impl {

// Tiger
using TigerHash = CryptoppHashBase<CryptoPP::Tiger>;

// Whirlpool
using WhirlpoolHash = CryptoppHashBase<CryptoPP::Whirlpool>;

// SM3
using SM3Hash = CryptoppHashBase<CryptoPP::SM3>;

// LSH Family
using LSH256Hash = CryptoppHashBase<CryptoPP::LSH256>;
using LSH512Hash = CryptoppHashBase<CryptoPP::LSH512>;

} // namespace impl
} // namespace core

#endif // EXOTIC_ALGORITHMS_H
