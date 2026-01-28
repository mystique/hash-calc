#include "OtherAlgorithms.h"
#include "../HashAlgorithmFactory.h"

namespace core {
namespace impl {

// Register algorithms
static HashAlgorithmRegistrar<MD5Hash> s_md5("MD5");
static HashAlgorithmRegistrar<MD2Hash> s_md2("MD2");
static HashAlgorithmRegistrar<MD4Hash> s_md4("MD4");
static HashAlgorithmRegistrar<RIPEMD128Hash> s_ripemd128("RIPEMD-128");
static HashAlgorithmRegistrar<RIPEMD160Hash> s_ripemd160("RIPEMD-160");
static HashAlgorithmRegistrar<RIPEMD256Hash> s_ripemd256("RIPEMD-256");
static HashAlgorithmRegistrar<RIPEMD320Hash> s_ripemd320("RIPEMD-320");
static HashAlgorithmRegistrar<CRC32Hash> s_crc32("CRC32");
static HashAlgorithmRegistrar<Adler32Hash> s_adler32("Adler32");

} // namespace impl
} // namespace core
