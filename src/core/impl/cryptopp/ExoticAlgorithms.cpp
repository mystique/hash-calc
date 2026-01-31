#include "ExoticAlgorithms.h"
#include "../../HashAlgorithmFactory.h"

namespace core {
namespace impl {

// Register algorithms
static HashAlgorithmRegistrar<TigerHash> s_tiger("Tiger");
static HashAlgorithmRegistrar<WhirlpoolHash> s_whirlpool("Whirlpool");
static HashAlgorithmRegistrar<SM3Hash> s_sm3("SM3");

} // namespace impl
} // namespace core
