#ifndef IHASH_ALGORITHM_H
#define IHASH_ALGORITHM_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace core {

/**
 * @brief Abstract interface for hash algorithms.
 * 
 * This interface provides a unified API for all hash algorithms,
 * regardless of the underlying implementation (Crypto++, OpenSSL, etc.).
 * 
 * Usage:
 *   auto sha256 = HashAlgorithmFactory::create("SHA-256");
 *   sha256->update(data, len);
 *   auto digest = sha256->finalize();
 */
class IHashAlgorithm {
public:
    /// @brief Callback function type for cancellation check
    /// @return true if operation should be cancelled
    using CancelCallback = std::function<bool()>;

    /// @brief Callback function type for progress reporting
    /// @param bytesProcessed Number of bytes processed so far
    /// @param totalBytes Total number of bytes to process (0 if unknown)
    using ProgressCallback = std::function<void(uint64_t bytesProcessed, uint64_t totalBytes)>;

    virtual ~IHashAlgorithm() = default;

    /// @return Algorithm name (e.g., "SHA-256", "MD5")
    virtual std::string getName() const = 0;

    /// @return Digest size in bytes
    virtual size_t getDigestSize() const = 0;

    /// @brief Update hash with data
    virtual void update(const uint8_t* data, size_t length) = 0;

    /// @brief Finalize and return digest (resets state)
    virtual std::vector<uint8_t> finalize() = 0;

    /// @brief Reset hash state for reuse
    virtual void reset() = 0;

    // ===== Convenience methods =====

    /// @brief Compute hash of a string
    std::vector<uint8_t> computeString(const std::string& str);

    /// @brief Compute hash of a file
    /// @throws std::runtime_error if file cannot be opened
    std::vector<uint8_t> computeFile(const std::wstring& filePath);

    /// @brief Set cancellation callback
    /// @param callback Function to check if operation should be cancelled
    void setCancelCallback(CancelCallback callback) { m_cancelCallback = callback; }

    /// @brief Set progress callback
    /// @param callback Function to report progress updates
    void setProgressCallback(ProgressCallback callback) { m_progressCallback = callback; }

    /// @brief Convert digest to hex string
    static std::string toHexString(const std::vector<uint8_t>& digest, bool uppercase = false);

    /// @brief Convert digest to hex wstring
    static std::wstring toHexWString(const std::vector<uint8_t>& digest, bool uppercase = false);

    // ===== Helper overloads =====
    
    void update(const std::string& str) {
        update(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }

    void update(const std::vector<uint8_t>& data) {
        update(data.data(), data.size());
    }

protected:
    /// @brief Callback for cancellation check
    CancelCallback m_cancelCallback;

    /// @brief Callback for progress reporting
    ProgressCallback m_progressCallback;
};

} // namespace core

#endif // IHASH_ALGORITHM_H
