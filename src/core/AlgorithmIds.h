#ifndef ALGORITHM_IDS_H
#define ALGORITHM_IDS_H

#include "../res/resource.h"

namespace core {

// Centralized list of all algorithm IDs
// This list should be updated whenever a new algorithm is added
// Used by: HashCalcDialog, ConfigManager, and other components
inline constexpr int ALL_ALGORITHM_IDS[] = {
  // Tab 1: SHA & MD
  IDC_SHA_160, IDC_SHA_224, IDC_SHA_256, IDC_SHA_384, IDC_SHA_512,
  IDC_HAVAL_128, IDC_HAVAL_160, IDC_HAVAL_192, IDC_HAVAL_224, IDC_HAVAL_256,
  IDC_RIPEMD_128, IDC_RIPEMD_160, IDC_RIPEMD_256, IDC_RIPEMD_320,
  IDC_MD2, IDC_MD4, IDC_MD5, IDC_CRC32, IDC_ADLER32, IDC_CRC16, IDC_CRC32C, IDC_CRC64, IDC_CRC8,
  IDC_MD6_128, IDC_MD6_160, IDC_MD6_192, IDC_MD6_224, IDC_MD6_256, IDC_MD6_384, IDC_MD6_512,
  // Tab 2: SHA-3 & Modern
  IDC_SHA3_224, IDC_SHA3_256, IDC_SHA3_384, IDC_SHA3_512,
  IDC_KECCAK_224, IDC_KECCAK_256, IDC_KECCAK_384, IDC_KECCAK_512,
  IDC_SHAKE_128, IDC_SHAKE_256,
  IDC_BLAKE2B, IDC_BLAKE2S, IDC_BLAKE3,
  // Tab 3: HAVAL & RIPEMD (already included above)
  // Tab 4: Checksum & Others
  IDC_TIGER, IDC_WHIRLPOOL, IDC_LSH_256, IDC_SM3, IDC_LSH_512,
  IDC_GOST94, IDC_GOST2012_256, IDC_GOST2012_512
};

inline constexpr size_t ALL_ALGORITHM_COUNT = sizeof(ALL_ALGORITHM_IDS) / sizeof(int);

} // namespace core

#endif // ALGORITHM_IDS_H
