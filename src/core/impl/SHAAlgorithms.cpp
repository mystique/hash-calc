#include "SHAAlgorithms.h"
#include "../HashAlgorithmFactory.h"

namespace core {
namespace impl {

// Auto-register SHA algorithms with factory
static HashAlgorithmRegistrar<SHA1Hash> s_sha1("SHA-1");
static HashAlgorithmRegistrar<SHA224Hash> s_sha224("SHA-224");
static HashAlgorithmRegistrar<SHA256Hash> s_sha256("SHA-256");
static HashAlgorithmRegistrar<SHA384Hash> s_sha384("SHA-384");
static HashAlgorithmRegistrar<SHA512Hash> s_sha512("SHA-512");
static HashAlgorithmRegistrar<SHA3_224Hash> s_sha3_224("SHA3-224");
static HashAlgorithmRegistrar<SHA3_256Hash> s_sha3_256("SHA3-256");
static HashAlgorithmRegistrar<SHA3_384Hash> s_sha3_384("SHA3-384");
static HashAlgorithmRegistrar<SHA3_512Hash> s_sha3_512("SHA3-512");
static HashAlgorithmRegistrar<Keccak_224Hash> s_keccak_224("Keccak-224");
static HashAlgorithmRegistrar<Keccak_256Hash> s_keccak_256("Keccak-256");
static HashAlgorithmRegistrar<Keccak_384Hash> s_keccak_384("Keccak-384");
static HashAlgorithmRegistrar<Keccak_512Hash> s_keccak_512("Keccak-512");
static HashAlgorithmRegistrar<SHAKE128Hash> s_shake128("SHAKE128");
static HashAlgorithmRegistrar<SHAKE256Hash> s_shake256("SHAKE256");

} // namespace impl
} // namespace core
