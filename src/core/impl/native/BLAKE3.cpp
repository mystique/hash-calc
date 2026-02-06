#include "BLAKE3.h"
#include "../../HashAlgorithmFactory.h"
#include <algorithm>
#include <cstring>

namespace core {
namespace impl {

// BLAKE3 IV (same as BLAKE2)
template<unsigned int DIGEST_BITS>
const uint32_t BLAKE3<DIGEST_BITS>::IV[8] = {
    0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
    0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

// Message schedule permutation for 7 rounds
template<unsigned int DIGEST_BITS>
const uint8_t BLAKE3<DIGEST_BITS>::MSG_SCHEDULE[7][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {2, 6, 3, 10, 7, 0, 4, 13, 1, 11, 12, 5, 9, 14, 15, 8},
    {3, 4, 10, 12, 13, 2, 7, 14, 6, 5, 9, 0, 11, 15, 8, 1},
    {10, 7, 12, 9, 14, 3, 13, 15, 4, 0, 11, 2, 5, 8, 1, 6},
    {12, 13, 9, 11, 15, 10, 14, 8, 7, 2, 5, 3, 0, 1, 6, 4},
    {9, 14, 11, 5, 8, 12, 15, 1, 13, 3, 0, 10, 2, 6, 4, 7},
    {11, 15, 5, 0, 1, 9, 8, 6, 14, 10, 2, 12, 3, 4, 7, 13},
};

// ============================================================================
// Utility Functions
// ============================================================================

template<unsigned int DIGEST_BITS>
uint32_t BLAKE3<DIGEST_BITS>::load32(const void* src) {
    const uint8_t* p = static_cast<const uint8_t*>(src);
    return (static_cast<uint32_t>(p[0]) << 0)  |
           (static_cast<uint32_t>(p[1]) << 8)  |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::store32(void* dst, uint32_t w) {
    uint8_t* p = static_cast<uint8_t*>(dst);
    p[0] = static_cast<uint8_t>(w >> 0);
    p[1] = static_cast<uint8_t>(w >> 8);
    p[2] = static_cast<uint8_t>(w >> 16);
    p[3] = static_cast<uint8_t>(w >> 24);
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::loadKeyWords(const uint8_t key[KEY_LEN], uint32_t key_words[8]) {
    for (int i = 0; i < 8; i++) {
        key_words[i] = load32(&key[i * 4]);
    }
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::storeCvWords(uint8_t bytes_out[32], const uint32_t cv_words[8]) {
    for (int i = 0; i < 8; i++) {
        store32(&bytes_out[i * 4], cv_words[i]);
    }
}

template<unsigned int DIGEST_BITS>
unsigned int BLAKE3<DIGEST_BITS>::popcnt(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned int>(__builtin_popcountll(x));
#else
    unsigned int count = 0;
    while (x != 0) {
        count += 1;
        x &= x - 1;
    }
    return count;
#endif
}

// ============================================================================
// Core Compression Functions
// ============================================================================

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::g(uint32_t state[16], size_t a, size_t b, size_t c, size_t d,
                            uint32_t mx, uint32_t my) {
    state[a] = state[a] + state[b] + mx;
    state[d] = rotr32(state[d] ^ state[a], 16);
    state[c] = state[c] + state[d];
    state[b] = rotr32(state[b] ^ state[c], 12);
    state[a] = state[a] + state[b] + my;
    state[d] = rotr32(state[d] ^ state[a], 8);
    state[c] = state[c] + state[d];
    state[b] = rotr32(state[b] ^ state[c], 7);
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::roundFunction(uint32_t state[16], const uint32_t m[16], size_t round) {
    // Select message schedule for this round
    const uint8_t* schedule = MSG_SCHEDULE[round % 7];
    
    // Column step
    g(state, 0, 4, 8,  12, m[schedule[0]], m[schedule[1]]);
    g(state, 1, 5, 9,  13, m[schedule[2]], m[schedule[3]]);
    g(state, 2, 6, 10, 14, m[schedule[4]], m[schedule[5]]);
    g(state, 3, 7, 11, 15, m[schedule[6]], m[schedule[7]]);
    
    // Diagonal step
    g(state, 0, 5, 10, 15, m[schedule[8]],  m[schedule[9]]);
    g(state, 1, 6, 11, 12, m[schedule[10]], m[schedule[11]]);
    g(state, 2, 7, 8,  13, m[schedule[12]], m[schedule[13]]);
    g(state, 3, 4, 9,  14, m[schedule[14]], m[schedule[15]]);
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::compress(uint32_t cv[8], const uint8_t block[BLOCK_LEN],
                                   uint8_t block_len, uint64_t counter, uint8_t flags) {
    // Load message words
    uint32_t m[16];
    for (size_t i = 0; i < 16; i++) {
        m[i] = load32(&block[i * 4]);
    }
    
    // Initialize state
    uint32_t state[16] = {
        cv[0], cv[1], cv[2], cv[3],
        cv[4], cv[5], cv[6], cv[7],
        IV[0], IV[1], IV[2], IV[3],
        static_cast<uint32_t>(counter),
        static_cast<uint32_t>(counter >> 32),
        static_cast<uint32_t>(block_len),
        static_cast<uint32_t>(flags)
    };
    
    // 7 rounds
    for (size_t round = 0; round < 7; round++) {
        roundFunction(state, m, round);
    }
    
    // Finalize: XOR state with original CV
    for (size_t i = 0; i < 8; i++) {
        cv[i] = state[i] ^ state[i + 8];
    }
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::compressXof(const uint32_t cv[8], const uint8_t block[BLOCK_LEN],
                                      uint8_t block_len, uint64_t counter, uint8_t flags,
                                      uint8_t out[64]) {
    // Load message words
    uint32_t m[16];
    for (size_t i = 0; i < 16; i++) {
        m[i] = load32(&block[i * 4]);
    }
    
    // Initialize state
    uint32_t state[16] = {
        cv[0], cv[1], cv[2], cv[3],
        cv[4], cv[5], cv[6], cv[7],
        IV[0], IV[1], IV[2], IV[3],
        static_cast<uint32_t>(counter),
        static_cast<uint32_t>(counter >> 32),
        static_cast<uint32_t>(block_len),
        static_cast<uint32_t>(flags)
    };
    
    // 7 rounds
    for (size_t round = 0; round < 7; round++) {
        roundFunction(state, m, round);
    }
    
    // Output full 64 bytes (16 words)
    for (size_t i = 0; i < 8; i++) {
        store32(&out[i * 4], state[i] ^ cv[i]);
    }
    for (size_t i = 8; i < 16; i++) {
        store32(&out[i * 4], state[i] ^ cv[i - 8]);
    }
}

// ============================================================================
// ChunkState Implementation
// ============================================================================

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::ChunkState::init(const uint32_t key[8], uint8_t flag) {
    std::memcpy(cv, key, 32);
    chunk_counter = 0;
    std::memset(buf, 0, BLOCK_LEN);
    buf_len = 0;
    blocks_compressed = 0;
    flags = flag;
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::ChunkState::reset(const uint32_t key[8], uint64_t counter) {
    std::memcpy(cv, key, 32);
    chunk_counter = counter;
    blocks_compressed = 0;
    std::memset(buf, 0, BLOCK_LEN);
    buf_len = 0;
}

template<unsigned int DIGEST_BITS>
size_t BLAKE3<DIGEST_BITS>::ChunkState::len() const {
    return (BLOCK_LEN * static_cast<size_t>(blocks_compressed)) + static_cast<size_t>(buf_len);
}

template<unsigned int DIGEST_BITS>
size_t BLAKE3<DIGEST_BITS>::ChunkState::fillBuf(const uint8_t* input, size_t input_len) {
    size_t take = BLOCK_LEN - static_cast<size_t>(buf_len);
    if (take > input_len) {
        take = input_len;
    }
    uint8_t* dest = buf + static_cast<size_t>(buf_len);
    std::memcpy(dest, input, take);
    buf_len += static_cast<uint8_t>(take);
    return take;
}

template<unsigned int DIGEST_BITS>
uint8_t BLAKE3<DIGEST_BITS>::ChunkState::maybeStartFlag() const {
    return (blocks_compressed == 0) ? CHUNK_START : 0;
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::ChunkState::update(const uint8_t* input, size_t input_len) {
    if (buf_len > 0) {
        size_t take = fillBuf(input, input_len);
        input += take;
        input_len -= take;
        if (input_len > 0) {
            compress(cv, buf, BLOCK_LEN, chunk_counter, flags | maybeStartFlag());
            blocks_compressed += 1;
            buf_len = 0;
            std::memset(buf, 0, BLOCK_LEN);
        }
    }
    
    while (input_len > BLOCK_LEN) {
        compress(cv, input, BLOCK_LEN, chunk_counter, flags | maybeStartFlag());
        blocks_compressed += 1;
        input += BLOCK_LEN;
        input_len -= BLOCK_LEN;
    }
    
    fillBuf(input, input_len);
}

// ============================================================================
// Output Implementation
// ============================================================================

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::Output::chainingValue(uint8_t cv[32]) const {
    uint32_t cv_words[8];
    std::memcpy(cv_words, input_cv, 32);
    compress(cv_words, block, block_len, counter, flags);
    storeCvWords(cv, cv_words);
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::Output::rootBytes(uint64_t seek, uint8_t* out, size_t out_len) const {
    if (out_len == 0) {
        return;
    }
    
    uint64_t output_block_counter = seek / 64;
    size_t offset_within_block = static_cast<size_t>(seek % 64);
    uint8_t wide_buf[64];
    
    // Handle offset within first block
    if (offset_within_block) {
        compressXof(input_cv, block, block_len, output_block_counter, flags | ROOT, wide_buf);
        size_t available_bytes = 64 - offset_within_block;
        size_t bytes = (out_len > available_bytes) ? available_bytes : out_len;
        std::memcpy(out, wide_buf + offset_within_block, bytes);
        out += bytes;
        out_len -= bytes;
        output_block_counter += 1;
    }
    
    // Generate remaining full blocks
    while (out_len > 0) {
        compressXof(input_cv, block, block_len, output_block_counter, flags | ROOT, wide_buf);
        size_t bytes = (out_len > 64) ? 64 : out_len;
        std::memcpy(out, wide_buf, bytes);
        out += bytes;
        out_len -= bytes;
        output_block_counter += 1;
    }
}

// ============================================================================
// Output Helper Functions
// ============================================================================

template<unsigned int DIGEST_BITS>
typename BLAKE3<DIGEST_BITS>::Output BLAKE3<DIGEST_BITS>::makeOutput(
    const uint32_t input_cv[8], const uint8_t block[BLOCK_LEN],
    uint8_t block_len, uint64_t counter, uint8_t flags) {
    Output ret;
    std::memcpy(ret.input_cv, input_cv, 32);
    std::memcpy(ret.block, block, BLOCK_LEN);
    ret.block_len = block_len;
    ret.counter = counter;
    ret.flags = flags;
    return ret;
}

template<unsigned int DIGEST_BITS>
typename BLAKE3<DIGEST_BITS>::Output BLAKE3<DIGEST_BITS>::chunkOutput(const ChunkState& cs) {
    uint8_t block_flags = cs.flags | cs.maybeStartFlag() | CHUNK_END;
    return makeOutput(cs.cv, cs.buf, cs.buf_len, cs.chunk_counter, block_flags);
}

template<unsigned int DIGEST_BITS>
typename BLAKE3<DIGEST_BITS>::Output BLAKE3<DIGEST_BITS>::parentOutput(
    const uint8_t block[BLOCK_LEN], const uint32_t key[8], uint8_t flags) {
    return makeOutput(key, block, BLOCK_LEN, 0, flags | PARENT);
}

// ============================================================================
// Tree Building Functions
// ============================================================================

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::hasherMergeCvStack(uint64_t total_len) {
    size_t post_merge_stack_len = static_cast<size_t>(popcnt(total_len));
    while (m_cv_stack_len > post_merge_stack_len) {
        uint8_t* parent_node = &m_cv_stack[(m_cv_stack_len - 2) * OUT_LEN];
        Output output = parentOutput(parent_node, m_key, m_chunk.flags);
        output.chainingValue(parent_node);
        m_cv_stack_len -= 1;
    }
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::hasherPushCv(const uint8_t new_cv[OUT_LEN], uint64_t chunk_counter) {
    hasherMergeCvStack(chunk_counter);
    std::memcpy(&m_cv_stack[m_cv_stack_len * OUT_LEN], new_cv, OUT_LEN);
    m_cv_stack_len += 1;
}

// ============================================================================
// Public Interface Implementation
// ============================================================================

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::Restart() {
    std::memcpy(m_key, IV, 32);
    m_chunk.init(m_key, 0);
    m_cv_stack_len = 0;
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::Update(const CryptoPP::byte* input, size_t length) {
    if (length == 0) {
        return;
    }
    
    const uint8_t* input_bytes = static_cast<const uint8_t*>(input);
    
    // If we have some partial chunk bytes, finish that chunk first
    if (m_chunk.len() > 0) {
        size_t take = CHUNK_LEN - m_chunk.len();
        if (take > length) {
            take = length;
        }
        m_chunk.update(input_bytes, take);
        input_bytes += take;
        length -= take;
        
        // If we've filled the current chunk and there's more coming
        if (length > 0) {
            Output output = chunkOutput(m_chunk);
            uint8_t chunk_cv[32];
            output.chainingValue(chunk_cv);
            hasherPushCv(chunk_cv, m_chunk.chunk_counter);
            m_chunk.reset(m_key, m_chunk.chunk_counter + 1);
        } else {
            return;
        }
    }
    
    // Process complete chunks
    while (length > CHUNK_LEN) {
        // For simplicity, process one chunk at a time
        // A more optimized version would use SIMD and process multiple chunks
        m_chunk.update(input_bytes, CHUNK_LEN);
        Output output = chunkOutput(m_chunk);
        uint8_t chunk_cv[32];
        output.chainingValue(chunk_cv);
        hasherPushCv(chunk_cv, m_chunk.chunk_counter);
        m_chunk.reset(m_key, m_chunk.chunk_counter + 1);
        input_bytes += CHUNK_LEN;
        length -= CHUNK_LEN;
    }
    
    // Add remaining bytes to chunk state
    if (length > 0) {
        m_chunk.update(input_bytes, length);
        hasherMergeCvStack(m_chunk.chunk_counter);
    }
}

template<unsigned int DIGEST_BITS>
void BLAKE3<DIGEST_BITS>::TruncatedFinal(CryptoPP::byte* digest, size_t digestSize) {
    if (digestSize == 0) {
        return;
    }
    
    // If the subtree stack is empty, then the current chunk is the root
    if (m_cv_stack_len == 0) {
        Output output = chunkOutput(m_chunk);
        output.rootBytes(0, digest, digestSize);
        return;
    }
    
    // Finalize the tree
    Output output;
    size_t cvs_remaining;
    
    if (m_chunk.len() > 0) {
        cvs_remaining = m_cv_stack_len;
        output = chunkOutput(m_chunk);
    } else {
        // There are always at least 2 CVs in the stack in this case
        cvs_remaining = m_cv_stack_len - 2;
        output = parentOutput(&m_cv_stack[cvs_remaining * 32], m_key, m_chunk.flags);
    }
    
    while (cvs_remaining > 0) {
        cvs_remaining -= 1;
        uint8_t parent_block[BLOCK_LEN];
        std::memcpy(parent_block, &m_cv_stack[cvs_remaining * 32], 32);
        output.chainingValue(&parent_block[32]);
        output = parentOutput(parent_block, m_key, m_chunk.flags);
    }
    
    output.rootBytes(0, digest, digestSize);
}

// ============================================================================
// Template Instantiations
// ============================================================================

// Explicitly instantiate the most common variants
template class BLAKE3<256>;
template class BLAKE3<512>;
template class BLAKE3<128>;
template class BLAKE3<160>;
template class BLAKE3<192>;
template class BLAKE3<224>;
template class BLAKE3<384>;

// Registration macro for BLAKE3 variants
#define REGISTER_BLAKE3(bits) \
    static HashAlgorithmRegistrar<BLAKE3Wrapper<BLAKE3_##bits>> \
        reg_blake3_##bits("BLAKE3-" #bits)

// Register common BLAKE3 variants
REGISTER_BLAKE3(256);
REGISTER_BLAKE3(512);
REGISTER_BLAKE3(128);
REGISTER_BLAKE3(160);
REGISTER_BLAKE3(192);
REGISTER_BLAKE3(224);
REGISTER_BLAKE3(384);

// Special registration for default BLAKE3 (256-bit)
static HashAlgorithmRegistrar<BLAKE3Wrapper<BLAKE3_256>> reg_blake3("BLAKE3");

} // namespace impl
} // namespace core
