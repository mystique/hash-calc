#include "IHashAlgorithm.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace core {

std::vector<uint8_t> IHashAlgorithm::computeString(const std::string& str) {
    reset();
    update(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    return finalize();
}

std::vector<uint8_t> IHashAlgorithm::computeFile(const std::wstring& filePath) {
    reset();
    
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for hashing");
    }

    constexpr size_t BUFFER_SIZE = 64 * 1024; // 64KB buffer
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    while (file) {
        file.read(reinterpret_cast<char*>(buffer.data()), BUFFER_SIZE);
        std::streamsize bytesRead = file.gcount();
        if (bytesRead > 0) {
            update(buffer.data(), static_cast<size_t>(bytesRead));
        }
    }

    return finalize();
}

std::string IHashAlgorithm::toHexString(const std::vector<uint8_t>& digest, bool uppercase) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    if (uppercase) {
        oss << std::uppercase;
    }
    for (uint8_t byte : digest) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    return oss.str();
}

std::wstring IHashAlgorithm::toHexWString(const std::vector<uint8_t>& digest, bool uppercase) {
    std::wostringstream oss;
    oss << std::hex << std::setfill(L'0');
    if (uppercase) {
        oss << std::uppercase;
    }
    for (uint8_t byte : digest) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    return oss.str();
}

} // namespace core
