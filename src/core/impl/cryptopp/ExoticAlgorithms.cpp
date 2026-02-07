#include "ExoticAlgorithms.h"
#include "../../HashAlgorithmFactory.h"

namespace core {
namespace impl {

// Register algorithms
static HashAlgorithmRegistrar<TigerHash> s_tiger("Tiger");
static HashAlgorithmRegistrar<WhirlpoolHash> s_whirlpool("Whirlpool");
static HashAlgorithmRegistrar<SM3Hash> s_sm3("SM3");
static HashAlgorithmRegistrar<LSH256Hash> s_lsh256("LSH-256");
static HashAlgorithmRegistrar<LSH512Hash> s_lsh512("LSH-512");

} // namespace impl
} // namespace core
