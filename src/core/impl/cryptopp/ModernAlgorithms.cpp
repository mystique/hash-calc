#include "ModernAlgorithms.h"
#include "../../HashAlgorithmFactory.h"

namespace core {
namespace impl {

// Register algorithms
static HashAlgorithmRegistrar<BLAKE2bHash> s_blake2b("BLAKE2b");
static HashAlgorithmRegistrar<BLAKE2sHash> s_blake2s("BLAKE2s");
static HashAlgorithmRegistrar<LSH256Hash> s_lsh256("LSH-256");
static HashAlgorithmRegistrar<LSH512Hash> s_lsh512("LSH-512");

} // namespace impl
} // namespace core
