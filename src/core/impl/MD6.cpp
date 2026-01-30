#include "MD6.h"
#include "../HashAlgorithmFactory.h"
#include <algorithm>
#include <cstring>

namespace core {
namespace impl {

// MD6 Q constant: initial 960 bits of fractional part of sqrt(6)
template<unsigned int DIGEST_BITS>
const uint64_t MD6<DIGEST_BITS>::Q[Q_SIZE] = {
    0x7311c2812425cfa0ULL, 0x6432286434aac8e7ULL, 0xb60450e9ef68b7c1ULL,
    0xe8fb23908d9f06f1ULL, 0xdd2e76cba691e5bfULL, 0x0cd0d63b2c30bc41ULL,
    0x1f8ccf6823058f8aULL, 0x54e5ed5b88e3775dULL, 0x4ad12aae0a6d6031ULL,
    0x3e7f16bb88222e0dULL, 0x8af8671d3fb50c2cULL, 0x995ad1178bd25c31ULL,
    0xc878c1dd04c4b633ULL, 0x3b72066c7a1552acULL, 0x0d6f3522631effcbULL
};

template<unsigned int DIGEST_BITS>
int MD6<DIGEST_BITS>::defaultRounds(int d) const {
    int r = 40 + (d / 4);
    return r;
}

template<unsigned int DIGEST_BITS>
void MD6<DIGEST_BITS>::Restart() {
    m_d = DIGEST_BITS;
    m_L = DEFAULT_L;
    m_r = defaultRounds(m_d);
    m_bits_processed = 0;
    m_top = 1;
    
    // Clear key (use all zeros)
    m_K.fill(0);
    
    // Clear state
    for (auto& b : m_B) {
        b.fill(0);
    }
    m_bits.fill(0);
    m_i_for_level.fill(0);
    
    // For sequential mode at level L+1, initialize with IV=0
    // (This is handled dynamically in processBlock when needed)
    
    m_hashval.resize(C * (W / 8));  // Allocate space for full C words
    std::fill(m_hashval.begin(), m_hashval.end(), 0);
}

template<unsigned int DIGEST_BITS>
uint64_t MD6<DIGEST_BITS>::makeControlWord(int r, int L, int z, int p, int keylen, int d) const {
    uint64_t V = 0;
    V |= (uint64_t(d) & 0xFFF);           // 12 bits for d (bits 0-11)
    V |= (uint64_t(keylen) & 0xFF) << 12; // 8 bits for keylen (bits 12-19)
    V |= (uint64_t(p) & 0xFFFF) << 20;    // 16 bits for p (bits 20-35)
    V |= (uint64_t(z) & 0xF) << 36;       // 4 bits for z (bits 36-39)
    V |= (uint64_t(L) & 0xFF) << 40;      // 8 bits for L (bits 40-47)
    V |= (uint64_t(r) & 0xFFF) << 48;     // 12 bits for r (bits 48-59)
    // bits 60-63 reserved (0)
    return V;
}

template<unsigned int DIGEST_BITS>
uint64_t MD6<DIGEST_BITS>::makeNodeID(int ell, int i) const {
    return (uint64_t(ell) << 56) | (uint64_t(i) & 0x00FFFFFFFFFFFFFFULL);
}

template<unsigned int DIGEST_BITS>
void MD6<DIGEST_BITS>::pack(uint64_t *N, const uint64_t *Q, const uint64_t *K,
                            int ell, int i, int r, int L, int z, int p, int keylen, int d,
                            const uint64_t *B_data) {
    int ni = 0;
    // Q part
    for (int j = 0; j < Q_SIZE; j++) {
        N[ni++] = Q[j];
    }
    // Key part
    for (int j = 0; j < K_SIZE; j++) {
        N[ni++] = K[j];
    }
    // Unique node ID
    N[ni++] = makeNodeID(ell, i);
    // Control word
    N[ni++] = makeControlWord(r, L, z, p, keylen, d);
    // Data (B_data is the input data block, 64 is the block size)
    for (int j = 0; j < 64; j++) {
        N[ni++] = B_data[j];
    }
}

template<unsigned int DIGEST_BITS>
void MD6<DIGEST_BITS>::mainCompressionLoop(uint64_t *A, int r) {
    // S constants for round constants
    const uint64_t S0 = 0x0123456789abcdefULL;
    const uint64_t Smask = 0x7311c2812425cfa0ULL;
    
    // Shift amounts for each step in a round
    const int rs[16] = {10, 5, 13, 10, 11, 12, 2, 7, 14, 15, 7, 13, 11, 7, 6, 12};
    const int ls[16] = {11, 24, 9, 16, 15, 9, 27, 15, 6, 2, 29, 8, 15, 5, 31, 9};
    
    uint64_t x, S = S0;
    int i = MD6::N;  // starting position in A
    
    for (int j = 0; j < r * C; j++) {
        int step = j % 16;  // step within current round
        
        // Compute next word
        x = S;                                    // feedback constant
        x ^= A[i + step - t5];                    // end-around feedback
        x ^= A[i + step - t0];                    // linear feedback
        x ^= (A[i + step - t1] & A[i + step - t2]); // first quadratic term
        x ^= (A[i + step - t3] & A[i + step - t4]); // second quadratic term
        x ^= (x >> rs[step]);                     // right shift
        A[i + step] = x ^ (x << ls[step]);        // left shift and store
        
        // After each complete round (16 steps), update S and advance i
        if (step == 15) {
            S = (S << 1) ^ (S >> (W - 1)) ^ (S & Smask);
            i += 16;
        }
    }
}

template<unsigned int DIGEST_BITS>
void MD6<DIGEST_BITS>::compress(uint64_t *C_out, const uint64_t *N_input, int r) {
    // Allocate working array: size is r*c + n
    std::vector<uint64_t> A(r * C + MD6::N);
    
    // Initialize A with N (first n=89 words)
    std::memcpy(A.data(), N_input, MD6::N * sizeof(uint64_t));
    
    // Run main compression loop
    mainCompressionLoop(A.data(), r);
    
    // Extract output: C words from position (r-1)*c + n
    std::memcpy(C_out, A.data() + (r - 1) * C + MD6::N, C * sizeof(uint64_t));
}

template<unsigned int DIGEST_BITS>
void MD6<DIGEST_BITS>::processBlock(int ell, int final) {
    uint64_t N[89];
    uint64_t C_out[16];
    
    int z = (final && ell == m_top) ? 1 : 0;
    int p = B * W - m_bits[ell];  // padding bits
    
    // Determine next level to check if this is a SEQ node
    int next_level = std::min(ell + 1, m_L + 1);
    bool is_seq_node = (next_level == m_L + 1);
    
    // Reverse bytes for level 1 data (little-endian)
    if (ell == 1) {
        if (!is_seq_node) {  // PAR (tree) node - reverse all data
            for (int i = 0; i < B; i++) {
                m_B[ell][i] = ((m_B[ell][i] & 0x00000000000000FFULL) << 56) |
                              ((m_B[ell][i] & 0x000000000000FF00ULL) << 40) |
                              ((m_B[ell][i] & 0x0000000000FF0000ULL) << 24) |
                              ((m_B[ell][i] & 0x00000000FF000000ULL) << 8)  |
                              ((m_B[ell][i] & 0x000000FF00000000ULL) >> 8)  |
                              ((m_B[ell][i] & 0x0000FF0000000000ULL) >> 24) |
                              ((m_B[ell][i] & 0x00FF000000000000ULL) >> 40) |
                              ((m_B[ell][i] & 0xFF00000000000000ULL) >> 56);
            }
        } else {  // SEQ node - don't reverse chaining vars (first C words)
            for (int i = C; i < B; i++) {
                m_B[ell][i] = ((m_B[ell][i] & 0x00000000000000FFULL) << 56) |
                              ((m_B[ell][i] & 0x000000000000FF00ULL) << 40) |
                              ((m_B[ell][i] & 0x0000000000FF0000ULL) << 24) |
                              ((m_B[ell][i] & 0x00000000FF000000ULL) << 8)  |
                              ((m_B[ell][i] & 0x000000FF00000000ULL) >> 8)  |
                              ((m_B[ell][i] & 0x0000FF0000000000ULL) >> 24) |
                              ((m_B[ell][i] & 0x00FF000000000000ULL) >> 40) |
                              ((m_B[ell][i] & 0xFF00000000000000ULL) >> 56);
            }
        }
    }
    
    // Pack the block
    pack(N, Q, m_K.data(), ell, static_cast<int>(m_i_for_level[ell]),
         m_r, m_L, z, p, 0, m_d, m_B[ell].data());
    
    // Compress
    compress(C_out, N, m_r);
    
    // Save final chaining value if this is the last compression
    if (z == 1) {
        std::memcpy(m_hashval.data(), C_out, C * sizeof(uint64_t));
        return;
    }
    
    // Determine next level (already calculated above for is_seq_node check)
    // int next_level = std::min(ell + 1, m_L + 1);
    
    // For sequential mode (level L+1), first C words are chaining values (IV=0 initially)
    // These were already initialized to 0 in Restart(), no special action needed
    
    // Boundary check for next level
    if (next_level >= MAX_STACK_HEIGHT) {
        return;  // Safety check - should not happen with proper L value
    }
    
    // Copy C words to next level
    std::memcpy(reinterpret_cast<uint8_t*>(m_B[next_level].data()) + m_bits[next_level] / 8,
                C_out, C * sizeof(uint64_t));
    m_bits[next_level] += C * W;
    
    if (next_level > m_top) {
        m_top = next_level;
    }
    
    // Clear current level
    m_bits[ell] = 0;
    m_i_for_level[ell]++;
    m_B[ell].fill(0);
    
    // Recursively process next level if needed
    if (final || m_bits[next_level] >= B * W) {
        processBlock(next_level, final);
    }
}

template<unsigned int DIGEST_BITS>
void MD6<DIGEST_BITS>::Update(const CryptoPP::byte *input, size_t length) {
    const uint8_t *data = input;
    uint64_t databitlen = length * 8;  // convert to bits
    
    m_bits_processed += databitlen;
    
    int ell = 1;  // Always add data at level 1
    
    // Boundary check for level index
    if (ell >= MAX_STACK_HEIGHT) {
        return;  // Safety check
    }
    
    uint64_t j = 0;  // bits processed so far
    
    while (j < databitlen) {
        // Calculate portion size (in bits)
        unsigned int portion_size = std::min(static_cast<unsigned int>(databitlen - j),
                                            static_cast<unsigned int>(B * W - m_bits[ell]));
        
        // Check if we can use simple memcpy (byte-aligned)
        if ((portion_size % 8 == 0) && (m_bits[ell] % 8 == 0) && (j % 8 == 0)) {
            // Simple case: byte-aligned copy
            std::memcpy(reinterpret_cast<uint8_t*>(m_B[ell].data()) + m_bits[ell] / 8,
                       &data[j / 8],
                       portion_size / 8);
        } else {
            // Complex case: bit-level copy (not usually needed for hash inputs)
            // For simplicity, convert to byte copy with padding
            unsigned int bytes = (portion_size + 7) / 8;
            std::memcpy(reinterpret_cast<uint8_t*>(m_B[ell].data()) + m_bits[ell] / 8,
                       &data[j / 8],
                       bytes);
        }
        
        j += portion_size;
        m_bits[ell] += portion_size;
        
        // Process block if full and more data coming
        if (m_bits[ell] == B * W && j < databitlen) {
            processBlock(ell, 0);
        }
    }
}

template<unsigned int DIGEST_BITS>
void MD6<DIGEST_BITS>::trimHashval() {
    // Trim hashval to desired length d bits
    // High-order bit of a byte is considered its *first* bit
    int full_or_partial_bytes = (m_d + 7) / 8;
    int bits = m_d % 8;  // bits in partial byte
    
    // Boundary check
    if (full_or_partial_bytes > static_cast<int>(m_hashval.size())) {
        return;  // Safety check
    }
    
    // Shift left if we have a partial byte
    if (bits > 0) {
        for (int i = 0; i < full_or_partial_bytes; i++) {
            m_hashval[i] = (m_hashval[i] << (8 - bits));
            if ((i + 1) < full_or_partial_bytes && (i + 1) < static_cast<int>(m_hashval.size())) {
                m_hashval[i] |= (m_hashval[i + 1] >> bits);
            }
        }
    }
}

template<unsigned int DIGEST_BITS>
void MD6<DIGEST_BITS>::TruncatedFinal(CryptoPP::byte *digest, size_t digestSize) {
    ThrowIfInvalidTruncatedSize(digestSize);
    
    // Find starting level for final processing
    int ell;
    if (m_top == 1) {
        ell = 1;
    } else {
        for (ell = 1; ell <= m_top && ell < MAX_STACK_HEIGHT; ell++) {
            if (m_bits[ell] > 0) break;
        }
    }
    
    // Boundary check
    if (ell >= MAX_STACK_HEIGHT) {
        ell = 1;  // Fallback to level 1
    }
    
    // Process starting at level ell, up to root
    processBlock(ell, 1);
    
    // Hash value is now in m_hashval (set by processBlock with final=1)
    // It's stored as C words in little-endian, need to reverse for extraction
    uint64_t temp[C];
    
    // Safety check: ensure m_hashval has enough space
    size_t copy_size = std::min(C * sizeof(uint64_t), m_hashval.size());
    std::memcpy(temp, m_hashval.data(), copy_size);
    
    // Zero out any unfilled portion of temp
    if (copy_size < C * sizeof(uint64_t)) {
        std::memset(reinterpret_cast<uint8_t*>(temp) + copy_size, 0, 
                    C * sizeof(uint64_t) - copy_size);
    }
    
    // Reverse byte order within each word (little-endian to big-endian)
    for (int i = 0; i < C; i++) {
        temp[i] = ((temp[i] & 0x00000000000000FFULL) << 56) |
                  ((temp[i] & 0x000000000000FF00ULL) << 40) |
                  ((temp[i] & 0x0000000000FF0000ULL) << 24) |
                  ((temp[i] & 0x00000000FF000000ULL) << 8)  |
                  ((temp[i] & 0x000000FF00000000ULL) >> 8)  |
                  ((temp[i] & 0x0000FF0000000000ULL) >> 24) |
                  ((temp[i] & 0x00FF000000000000ULL) >> 40) |
                  ((temp[i] & 0xFF00000000000000ULL) >> 56);
    }
    
    // Extract last d bits from the C words
    int full_or_partial_bytes = (m_d + 7) / 8;
    int total_bytes = C * 8;  // Total bytes in C words
    
    // Ensure we don't exceed buffer sizes
    if (full_or_partial_bytes > total_bytes || full_or_partial_bytes > static_cast<int>(m_hashval.size())) {
        full_or_partial_bytes = std::min(total_bytes, static_cast<int>(m_hashval.size()));
    }
    
    // Copy from the END (last d bits)
    uint8_t *temp_bytes = reinterpret_cast<uint8_t*>(temp);
    for (int i = 0; i < full_or_partial_bytes; i++) {
        m_hashval[i] = temp_bytes[total_bytes - full_or_partial_bytes + i];
    }
    
    // Trim to exact bit length if needed
    trimHashval();
    
    std::memcpy(digest, m_hashval.data(), std::min(digestSize, m_hashval.size()));
    
    // Restart for next use
    Restart();
}

// Template instantiations for common MD6 variants
template class MD6<128>;
template class MD6<160>;
template class MD6<192>;
template class MD6<224>;
template class MD6<256>;
template class MD6<384>;
template class MD6<512>;

// Registration macro for MD6 variants
#define REGISTER_MD6(bits) \
    static HashAlgorithmRegistrar<MD6Wrapper<MD6_##bits>> \
        reg_md6_##bits("MD6-" #bits)

// Register common MD6 variants
REGISTER_MD6(128);
REGISTER_MD6(160);
REGISTER_MD6(192);
REGISTER_MD6(224);
REGISTER_MD6(256);
REGISTER_MD6(384);
REGISTER_MD6(512);

} // namespace impl
} // namespace core
