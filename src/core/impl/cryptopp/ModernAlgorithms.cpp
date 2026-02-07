#include "ModernAlgorithms.h"
#include "../../HashAlgorithmFactory.h"

namespace core {
namespace impl {

// Register algorithms
static HashAlgorithmRegistrar<BLAKE2bHash> s_blake2b("BLAKE2b");
static HashAlgorithmRegistrar<BLAKE2sHash> s_blake2s("BLAKE2s");

} // namespace impl
} // namespace core
