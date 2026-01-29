#include "IHashAlgorithm.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <windows.h>

namespace core {

std::vector<uint8_t> IHashAlgorithm::computeString(const std::string& str) {
    reset();
    update(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    return finalize();
}

std::vector<uint8_t> IHashAlgorithm::computeFile(const std::wstring& filePath) {
    reset();
    
    // Use CreateFile with FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE
    // to allow reading files that are currently in use by other processes
    // FILE_FLAG_SEQUENTIAL_SCAN optimizes for sequential file reading
    HANDLE hFile = CreateFile(
        filePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::ostringstream oss;
        oss << "Cannot open file for hashing. Error code: " << error;
        throw std::runtime_error(oss.str());
    }

    // RAII wrapper to ensure file handle is always closed
    struct FileHandleCloser {
        HANDLE handle;
        ~FileHandleCloser() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
    };
    FileHandleCloser fileGuard{hFile};

    constexpr size_t BUFFER_SIZE = 64 * 1024; // 64KB buffer
    std::vector<uint8_t> buffer(BUFFER_SIZE);
    DWORD bytesRead = 0;

    while (true) {
        // Check for cancellation before reading next chunk
        if (m_cancelCallback && m_cancelCallback()) {
            throw std::runtime_error("Operation cancelled by user");
        }

        BOOL success = ReadFile(hFile, buffer.data(), BUFFER_SIZE, &bytesRead, NULL);
        
        // Check for read errors
        if (!success) {
            DWORD error = GetLastError();
            std::ostringstream oss;
            oss << "Error reading file. Error code: " << error;
            throw std::runtime_error(oss.str());
        }
        
        // End of file reached
        if (bytesRead == 0) {
            break;
        }
        
        update(buffer.data(), static_cast<size_t>(bytesRead));
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
