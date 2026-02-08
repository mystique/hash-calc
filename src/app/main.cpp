/**
 * @file main.cpp
 * @brief Hash Calculator Application Entry Point
 * @author HashCalc Development Team
 * @date 2026
 * @version 1.0
 * @details Main application file that initializes COM, creates the main dialog,
 *          and handles command line arguments for the Hash Calculator application.
 * @note This application provides a GUI for computing various hash values (MD5, SHA-1, SHA-256, SHA-3, BLAKE3, etc.)
 *       for files and text strings. It supports drag-and-drop operations and can process files via command line.
 */

#include "ui/HashCalcDialog.h"
#include "utils/ConfigManager.h"
#include "core/HashAlgorithmFactory.h"
#include "core/IHashAlgorithm.h"
#include "core/AlgorithmIds.h"
#include <objbase.h>
#include <string>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <conio.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>

/**
 * @brief Attach console to the current process for command-line output
 * @param outNeedWait Set to true if we allocated a new console (need to wait for user)
 * @return true if console was successfully attached or allocated
 */
bool AttachConsoleWindow(bool& outNeedWait) {
  outNeedWait = false;

  // Try to attach to parent console first (if launched from cmd/powershell)
  if (::AttachConsole(ATTACH_PARENT_PROCESS)) {
    // Reopen stdout and stderr to the console
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    // Set console output code page to UTF-8
    SetConsoleOutputCP(CP_UTF8);

    // Print a newline to separate from previous command output
    printf("\n");
    return true;
  }

  // If no parent console, allocate a new one
  if (::AllocConsole()) {
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    // Set console output code page to UTF-8
    SetConsoleOutputCP(CP_UTF8);

    // We allocated a new console, so we need to wait for user input before closing
    outNeedWait = true;
    return true;
  }

  return false;
}

/**
 * @brief Print help information to console
 */
void PrintHelp() {
  printf("Hash Calculator - Multi-purpose Hash Computation Tool\n\n");
  printf("Usage:\n");
  printf("  HashCalc.exe                    Launch GUI\n");
  printf("  HashCalc.exe <input>            Launch GUI with input (auto-detect file/text)\n");
  printf("  HashCalc.exe <input> -a <algo>  Launch GUI with specific algorithms\n");
  printf("  HashCalc.exe -t/-f <input> -a   Console mode (requires -a)\n\n");
  printf("Options:\n");
  printf("  -h, --help                Display this help information\n");
  printf("  -l, --list                List all supported hash algorithms\n");
  printf("  -f, --file <path>         Console: Calculate hash of file (requires -a)\n");
  printf("  -t, --text <content>      Console: Calculate hash of text (requires -a)\n");
  printf("  -a, --algorithm <algo>    Specify algorithm (can use multiple times)\n");
  printf("                            Examples: MD5, SHA256, SHA3-256, BLAKE3\n\n");
  printf("Examples:\n");
  printf("  HashCalc.exe test.txt              # GUI mode, auto-detect file\n");
  printf("  HashCalc.exe \"Hello World\"         # GUI mode, treat as text\n");
  printf("  HashCalc.exe test.txt -a MD5       # GUI mode, select MD5\n");
  printf("  HashCalc.exe -f test.txt -a SHA256 -a MD5  # Console mode\n");
  printf("  HashCalc.exe -t \"text\" -a BLAKE3   # Console mode\n\n");
  printf("Notes:\n");
  printf("  - GUI mode: Use config algorithms if -a not specified\n");
  printf("  - Console mode (-t/-f): Must specify at least one -a algorithm\n");
  printf("  - Paths/text with spaces should be quoted\n\n");
}

/**
 * @brief Check if command line indicates console mode
 * @param lpCmdLine Command line string
 * @return true if should run in console mode
 */
bool IsConsoleMode(LPWSTR lpCmdLine) {
  if (lpCmdLine == nullptr || lpCmdLine[0] == L'\0') {
    return false;
  }

  std::wstring cmdLine(lpCmdLine);

  // Check for help and list flags (always console mode)
  if (cmdLine.find(L"--help") != std::wstring::npos ||
      cmdLine.find(L"-h") != std::wstring::npos ||
      cmdLine.find(L"/?") != std::wstring::npos ||
      cmdLine.find(L"--list") != std::wstring::npos ||
      cmdLine.find(L"-l") != std::wstring::npos) {
    return true;
  }

  // Only -t/-f flags indicate console mode (other inputs go to GUI)
  if (cmdLine.find(L"--file") != std::wstring::npos ||
      cmdLine.find(L"-f") != std::wstring::npos ||
      cmdLine.find(L"--text") != std::wstring::npos ||
      cmdLine.find(L"-t") != std::wstring::npos) {
    return true;
  }

  return false;
}

/**
 * @brief Parse -a/--algorithm parameters from command line
 * @param cmdLine Command line string
 * @return Vector of algorithm names
 */
std::vector<std::wstring> ParseAlgorithmParameters(const std::wstring& cmdLine) {
  std::vector<std::wstring> algorithms;
  size_t pos = 0;

  while (pos < cmdLine.length()) {
    // Find next -a or --algorithm
    size_t aPos = cmdLine.find(L"-a", pos);
    size_t algoPos = cmdLine.find(L"--algorithm", pos);
    
    size_t flagPos = std::wstring::npos;
    size_t flagLen = 0;
    
    if (aPos != std::wstring::npos && algoPos != std::wstring::npos) {
      if (aPos < algoPos) {
        flagPos = aPos;
        flagLen = 2;
      } else {
        flagPos = algoPos;
        flagLen = 11;
      }
    } else if (aPos != std::wstring::npos) {
      flagPos = aPos;
      flagLen = 2;
    } else if (algoPos != std::wstring::npos) {
      flagPos = algoPos;
      flagLen = 11;
    } else {
      break; // No more algorithm flags
    }

    // Ensure it's a standalone flag (preceded by space or at start)
    if (flagPos > 0 && cmdLine[flagPos - 1] != L' ' && cmdLine[flagPos - 1] != L'\t') {
      pos = flagPos + 1;
      continue;
    }

    // Extract algorithm name after the flag
    size_t nameStart = cmdLine.find_first_not_of(L" \t", flagPos + flagLen);
    if (nameStart != std::wstring::npos) {
      std::wstring algoName;
      
      // Check if quoted
      if (cmdLine[nameStart] == L'"') {
        size_t nameEnd = cmdLine.find(L'"', nameStart + 1);
        if (nameEnd != std::wstring::npos) {
          algoName = cmdLine.substr(nameStart + 1, nameEnd - nameStart - 1);
          pos = nameEnd + 1;
        }
      } else {
        // Find end of name (space or end of string)
        size_t nameEnd = cmdLine.find_first_of(L" \t", nameStart);
        if (nameEnd != std::wstring::npos) {
          algoName = cmdLine.substr(nameStart, nameEnd - nameStart);
          pos = nameEnd;
        } else {
          algoName = cmdLine.substr(nameStart);
          pos = cmdLine.length();
        }
      }
      
      if (!algoName.empty()) {
        algorithms.push_back(algoName);
      }
    } else {
      pos = flagPos + flagLen;
    }
  }

  return algorithms;
}

/**
 * @brief Extract HAVAL pass number from algorithm name
 * @param algoName Algorithm name (e.g., "HAVAL-3-256")
 * @return Pass number (3, 4, or 5), or 0 if not a valid HAVAL name
 */
int ExtractHavalPass(const std::wstring& algoName) {
  // Check if it's a HAVAL algorithm with pass specification
  if (algoName.find(L"HAVAL-") != 0) {
    return 0;  // Not a HAVAL algorithm
  }
  
  // Format: HAVAL-X-YYY where X is pass number (3/4/5), YYY is bits (128/160/192/224/256)
  if (algoName.length() < 11) {  // HAVAL-3-128 is minimum length
    return 0;  // Not long enough for pass-bits format
  }
  
  wchar_t passChar = algoName[6];  // Position of pass number in "HAVAL-X-YYY"
  if (passChar == L'3' || passChar == L'4' || passChar == L'5') {
    if (algoName[7] == L'-') {  // Must have dash after pass number
      return passChar - L'0';
    }
  }
  
  return 0;  // Default to 0 if no specific pass found
}

/**
 * @brief Get algorithm name for display
 * @param algorithmId Algorithm ID from resource.h
 * @param havalPass HAVAL pass number (3/4/5) if applicable
 * @return Display name for the algorithm
 */
std::string GetAlgorithmDisplayName(int algorithmId, int havalPass = 0) {
  // Map algorithm IDs to display names (matching the UI logic)
  static std::map<int, std::string> displayNames = {
    {IDC_SHA_160, "SHA-160"}, {IDC_SHA_224, "SHA-224"}, {IDC_SHA_256, "SHA-256"},
    {IDC_SHA_384, "SHA-384"}, {IDC_SHA_512, "SHA-512"},
    {IDC_MD2, "MD2"}, {IDC_MD4, "MD4"}, {IDC_MD5, "MD5"},
    {IDC_MD6_128, "MD6-128"}, {IDC_MD6_160, "MD6-160"}, {IDC_MD6_192, "MD6-192"},
    {IDC_MD6_224, "MD6-224"}, {IDC_MD6_256, "MD6-256"}, {IDC_MD6_384, "MD6-384"},
    {IDC_MD6_512, "MD6-512"},
    {IDC_SHA3_224, "SHA3-224"}, {IDC_SHA3_256, "SHA3-256"},
    {IDC_SHA3_384, "SHA3-384"}, {IDC_SHA3_512, "SHA3-512"},
    {IDC_KECCAK_224, "KECCAK-224"}, {IDC_KECCAK_256, "KECCAK-256"},
    {IDC_KECCAK_384, "KECCAK-384"}, {IDC_KECCAK_512, "KECCAK-512"},
    {IDC_SHAKE_128, "SHAKE-128"}, {IDC_SHAKE_256, "SHAKE-256"},
    {IDC_BLAKE2B, "BLAKE2B"}, {IDC_BLAKE2S, "BLAKE2S"}, {IDC_BLAKE3, "BLAKE3"},
    {IDC_HAVAL_128, "HAVAL-128"}, {IDC_HAVAL_160, "HAVAL-160"},
    {IDC_HAVAL_192, "HAVAL-192"}, {IDC_HAVAL_224, "HAVAL-224"},
    {IDC_HAVAL_256, "HAVAL-256"},
    {IDC_RIPEMD_128, "RIPEMD-128"}, {IDC_RIPEMD_160, "RIPEMD-160"},
    {IDC_RIPEMD_256, "RIPEMD-256"}, {IDC_RIPEMD_320, "RIPEMD-320"},
    {IDC_CRC8, "CRC8"}, {IDC_CRC16, "CRC16"}, {IDC_CRC32, "CRC32"},
    {IDC_CRC32C, "CRC32C"}, {IDC_CRC64, "CRC64"}, {IDC_ADLER32, "ADLER32"},
    {IDC_TIGER, "TIGER"}, {IDC_WHIRLPOOL, "WHIRLPOOL"}, {IDC_SM3, "SM3"},
    {IDC_LSH_256, "LSH-256"}, {IDC_LSH_512, "LSH-512"},
    {IDC_GOST94, "GOST-94"}, {IDC_GOST2012_256, "GOST-256"},
    {IDC_GOST2012_512, "GOST-512"}
  };

  auto it = displayNames.find(algorithmId);
  if (it != displayNames.end()) {
    // Add pass number for HAVAL algorithms
    if (havalPass > 0 && (algorithmId >= IDC_HAVAL_128 && algorithmId <= IDC_HAVAL_256)) {
      return "HAVAL-" + std::to_string(havalPass) + "-" + it->second.substr(6);  // Skip "HAVAL-" prefix
    }
    return it->second;
  }
  return "Unknown";
}

/**
 * @brief Get factory algorithm name from algorithm ID and HAVAL pass
 * @param algorithmId Algorithm ID from resource.h
 * @param havalPass HAVAL pass number (3/4/5) if applicable
 * @return Algorithm name used by HashAlgorithmFactory (must match registered names exactly)
 */
std::string GetFactoryAlgorithmName(int algorithmId, int havalPass = 0) {
  // Map algorithm IDs to factory names (MUST match the names used in HashAlgorithmRegistrar)
  static std::map<int, std::string> factoryNames = {
    {IDC_SHA_160, "SHA-1"}, {IDC_SHA_224, "SHA-224"}, {IDC_SHA_256, "SHA-256"},
    {IDC_SHA_384, "SHA-384"}, {IDC_SHA_512, "SHA-512"},
    {IDC_MD2, "MD2"}, {IDC_MD4, "MD4"}, {IDC_MD5, "MD5"},
    {IDC_MD6_128, "MD6-128"}, {IDC_MD6_160, "MD6-160"}, {IDC_MD6_192, "MD6-192"},
    {IDC_MD6_224, "MD6-224"}, {IDC_MD6_256, "MD6-256"}, {IDC_MD6_384, "MD6-384"},
    {IDC_MD6_512, "MD6-512"},
    {IDC_SHA3_224, "SHA3-224"}, {IDC_SHA3_256, "SHA3-256"},
    {IDC_SHA3_384, "SHA3-384"}, {IDC_SHA3_512, "SHA3-512"},
    // KECCAK: Registered as "Keccak-224", "Keccak-256", etc. (capitalized, not uppercase)
    {IDC_KECCAK_224, "Keccak-224"}, {IDC_KECCAK_256, "Keccak-256"},
    {IDC_KECCAK_384, "Keccak-384"}, {IDC_KECCAK_512, "Keccak-512"},
    // SHAKE: Registered as "SHAKE128", "SHAKE256" (no hyphen)
    {IDC_SHAKE_128, "SHAKE128"}, {IDC_SHAKE_256, "SHAKE256"},
    // BLAKE: Registered as "BLAKE2b", "BLAKE2s" (lowercase b/s), "BLAKE3"
    {IDC_BLAKE2B, "BLAKE2b"}, {IDC_BLAKE2S, "BLAKE2s"}, {IDC_BLAKE3, "BLAKE3"},
    {IDC_HAVAL_128, "HAVAL-128"}, {IDC_HAVAL_160, "HAVAL-160"},
    {IDC_HAVAL_192, "HAVAL-192"}, {IDC_HAVAL_224, "HAVAL-224"},
    {IDC_HAVAL_256, "HAVAL-256"},
    {IDC_RIPEMD_128, "RIPEMD-128"}, {IDC_RIPEMD_160, "RIPEMD-160"},
    {IDC_RIPEMD_256, "RIPEMD-256"}, {IDC_RIPEMD_320, "RIPEMD-320"},
    // CRC: Registered as "CRC-8", "CRC-16", "CRC32" (note: CRC32 has no hyphen)
    {IDC_CRC8, "CRC-8"}, {IDC_CRC16, "CRC-16"}, {IDC_CRC32, "CRC32"},
    {IDC_CRC32C, "CRC-32C"}, {IDC_CRC64, "CRC-64"},
    // ADLER32: Registered as "Adler32" (capitalized, not uppercase)
    {IDC_ADLER32, "Adler32"},
    // Others: Registered as "Tiger", "Whirlpool" (capitalized)
    {IDC_TIGER, "Tiger"}, {IDC_WHIRLPOOL, "Whirlpool"}, {IDC_SM3, "SM3"},
    {IDC_LSH_256, "LSH-256"}, {IDC_LSH_512, "LSH-512"},
    // GOST: Registered as "GOST-R-34.11-94", "GOST-R-34.11-2012-256", "GOST-R-34.11-2012-512"
    {IDC_GOST94, "GOST-R-34.11-94"}, 
    {IDC_GOST2012_256, "GOST-R-34.11-2012-256"},
    {IDC_GOST2012_512, "GOST-R-34.11-2012-512"}
  };

  auto it = factoryNames.find(algorithmId);
  if (it != factoryNames.end()) {
    // For HAVAL algorithms, build factory name with pass specification
    if (havalPass > 0 && (algorithmId >= IDC_HAVAL_128 && algorithmId <= IDC_HAVAL_256)) {
      // Factory expects format like "HAVAL-256/Pass3"
      std::string bits = it->second.substr(6);  // Get "128", "160", etc. from "HAVAL-128"
      return "HAVAL-" + bits + "/Pass" + std::to_string(havalPass);
    }
    return it->second;
  }
  return "";
}

/**
 * @brief Structure to hold algorithm information
 */
struct AlgorithmInfo {
  int id;         // Algorithm ID
  int havalPass;  // HAVAL pass number (0 for non-HAVAL)
  
  AlgorithmInfo(int _id, int _pass = 0) : id(_id), havalPass(_pass) {}
};

/**
 * @brief Compute hash for text input using specified algorithms
 * @param text Text to hash
 * @param algorithmInfos Vector of algorithm information to use
 * @return 0 on success, 1 on error
 */
int ComputeHashForTextWithAlgorithms(const std::wstring& text, const std::vector<AlgorithmInfo>& algorithmInfos) {
  // Convert wide string to UTF-8
  int utf8Size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (utf8Size <= 0) {
    printf("Error: Failed to convert text to UTF-8\n");
    return 1;
  }

  std::vector<char> utf8Buffer(utf8Size);
  WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, utf8Buffer.data(), utf8Size, nullptr, nullptr);
  std::string inputData(utf8Buffer.data());

  printf("Computing hashes for text...\n\n");

  bool anyComputed = false;
  for (const AlgorithmInfo& info : algorithmInfos) {
    std::string factoryName = GetFactoryAlgorithmName(info.id, info.havalPass);
    std::string displayName = GetAlgorithmDisplayName(info.id, info.havalPass);

    if (factoryName.empty()) {
      printf("%-15s: Error - Unknown algorithm ID\n", displayName.c_str());
      continue;
    }

    // Create hash algorithm
    try {
      auto hashAlgo = core::HashAlgorithmFactory::create(factoryName);
      std::vector<uint8_t> digest = hashAlgo->computeString(inputData);
      std::string hash = core::IHashAlgorithm::toHexString(digest, false);
      printf("%-15s: %s\n", displayName.c_str(), hash.c_str());
      anyComputed = true;
    } catch (const std::exception& ex) {
      printf("%-15s: Error - %s\n", displayName.c_str(), ex.what());
    }
  }

  if (!anyComputed) {
    printf("Failed to compute any hashes.\n");
    return 1;
  }

  return 0;
}

/**
 * @brief Compute hash for file input using specified algorithms
 * @param filePath Path to file to hash
 * @param algorithmInfos Vector of algorithm information to use
 * @return 0 on success, 1 on error
 */
int ComputeHashForFileWithAlgorithms(const std::wstring& filePath, const std::vector<AlgorithmInfo>& algorithmInfos) {
  // Check if file exists
  DWORD fileAttr = GetFileAttributesW(filePath.c_str());
  if (fileAttr == INVALID_FILE_ATTRIBUTES) {
    wprintf(L"Error: File not found: %s\n", filePath.c_str());
    return 1;
  }

  if (fileAttr & FILE_ATTRIBUTE_DIRECTORY) {
    wprintf(L"Error: Path is a directory, not a file: %s\n", filePath.c_str());
    return 1;
  }

  wprintf(L"Computing hashes for file: %s\n\n", filePath.c_str());

  bool anyComputed = false;
  for (const AlgorithmInfo& info : algorithmInfos) {
    std::string factoryName = GetFactoryAlgorithmName(info.id, info.havalPass);
    std::string displayName = GetAlgorithmDisplayName(info.id, info.havalPass);

    if (factoryName.empty()) {
      printf("%-15s: Error - Unknown algorithm ID\n", displayName.c_str());
      continue;
    }

    // Create hash algorithm
    try {
      auto hashAlgo = core::HashAlgorithmFactory::create(factoryName);
      std::vector<uint8_t> digest = hashAlgo->computeFile(filePath);
      std::string hash = core::IHashAlgorithm::toHexString(digest, false);
      printf("%-15s: %s\n", displayName.c_str(), hash.c_str());
      anyComputed = true;
    } catch (const std::exception& ex) {
      printf("%-15s: Error - %s\n", displayName.c_str(), ex.what());
    }
  }

  if (!anyComputed) {
    printf("Failed to compute any hashes.\n");
    return 1;
  }

  return 0;
}

/**
 * @brief Compute hash for text input using configured algorithms
 * @param text Text to hash
 * @param configManager Configuration manager with algorithm selections
 * @return 0 on success, 1 on error
 */
int ComputeHashForText(const std::wstring& text, ConfigManager& configManager) {
  // Convert wide string to UTF-8
  int utf8Size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (utf8Size <= 0) {
    printf("Error: Failed to convert text to UTF-8\n");
    return 1;
  }

  std::vector<char> utf8Buffer(utf8Size);
  WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, utf8Buffer.data(), utf8Size, nullptr, nullptr);
  std::string inputData(utf8Buffer.data());

  // Get enabled algorithms from configuration
  auto algorithms = configManager.GetAllAlgorithms();
  bool anyComputed = false;

  printf("Computing hashes for text...\n\n");

  // Iterate through all algorithms in order (matching UI logic)
  for (size_t i = 0; i < core::ALL_ALGORITHM_COUNT; i++) {
    int algorithmId = core::ALL_ALGORITHM_IDS[i];
    
    // Check if algorithm is enabled
    auto it = algorithms.find(algorithmId);
    if (it == algorithms.end() || !it->second) {
      continue; // Algorithm not enabled, skip
    }

    std::string factoryName = GetFactoryAlgorithmName(algorithmId);
    std::string displayName = GetAlgorithmDisplayName(algorithmId);

    if (factoryName.empty()) {
      continue; // Unknown algorithm
    }

    // Create hash algorithm
    auto hashAlgo = core::HashAlgorithmFactory::create(factoryName);
    if (!hashAlgo) {
      printf("%-12s: Error - Algorithm not supported\n", displayName.c_str());
      continue;
    }

    // Compute hash
    try {
      std::vector<uint8_t> digest = hashAlgo->computeString(inputData);
      std::string hash = core::IHashAlgorithm::toHexString(digest, false);
      printf("%-12s: %s\n", displayName.c_str(), hash.c_str());
      anyComputed = true;
    } catch (const std::exception& ex) {
      printf("%-12s: Error - %s\n", displayName.c_str(), ex.what());
    }
  }

  if (!anyComputed) {
    printf("No algorithms were enabled in the configuration.\n");
    printf("Please run the GUI and select algorithms, or edit the configuration file.\n");
    return 1;
  }

  return 0;
}

/**
 * @brief Compute hash for file input using configured algorithms
 * @param filePath Path to file to hash
 * @param configManager Configuration manager with algorithm selections
 * @return 0 on success, 1 on error
 */
int ComputeHashForFile(const std::wstring& filePath, ConfigManager& configManager) {
  // Check if file exists
  DWORD fileAttr = GetFileAttributesW(filePath.c_str());
  if (fileAttr == INVALID_FILE_ATTRIBUTES) {
    wprintf(L"Error: File not found: %s\n", filePath.c_str());
    return 1;
  }

  if (fileAttr & FILE_ATTRIBUTE_DIRECTORY) {
    wprintf(L"Error: Path is a directory, not a file: %s\n", filePath.c_str());
    return 1;
  }

  // Get enabled algorithms from configuration
  auto algorithms = configManager.GetAllAlgorithms();
  bool anyComputed = false;

  wprintf(L"Computing hashes for file: %s\n\n", filePath.c_str());

  // Iterate through all algorithms in order (matching UI logic)
  for (size_t i = 0; i < core::ALL_ALGORITHM_COUNT; i++) {
    int algorithmId = core::ALL_ALGORITHM_IDS[i];
    
    // Check if algorithm is enabled
    auto it = algorithms.find(algorithmId);
    if (it == algorithms.end() || !it->second) {
      continue; // Algorithm not enabled, skip
    }

    std::string factoryName = GetFactoryAlgorithmName(algorithmId);
    std::string displayName = GetAlgorithmDisplayName(algorithmId);

    if (factoryName.empty()) {
      continue; // Unknown algorithm
    }

    // Create hash algorithm
    auto hashAlgo = core::HashAlgorithmFactory::create(factoryName);
    if (!hashAlgo) {
      printf("%-12s: Error - Algorithm not supported\n", displayName.c_str());
      continue;
    }

    // Compute hash from file
    try {
      std::vector<uint8_t> digest = hashAlgo->computeFile(filePath);
      std::string hash = core::IHashAlgorithm::toHexString(digest, false);
      printf("%-12s: %s\n", displayName.c_str(), hash.c_str());
      anyComputed = true;
    } catch (const std::exception& ex) {
      printf("%-12s: Error - %s\n", displayName.c_str(), ex.what());
    }
  }

  if (!anyComputed) {
    printf("No algorithms were enabled in the configuration.\n");
    printf("Please run the GUI and select algorithms, or edit the configuration file.\n");
    return 1;
  }

  return 0;
}

/**
 * @brief Run application in console mode
 * @param lpCmdLine Command line arguments
 * @return Exit code
 */
int RunConsoleMode(LPWSTR lpCmdLine) {
  bool needWait = false;
  if (!AttachConsoleWindow(needWait)) {
    MessageBoxW(nullptr, L"Failed to create console window", L"Error", MB_OK | MB_ICONERROR);
    return 1;
  }

  std::wstring cmdLine(lpCmdLine);

  // Handle --help
  if (cmdLine.find(L"--help") != std::wstring::npos ||
      cmdLine.find(L"-h") != std::wstring::npos ||
      cmdLine.find(L"/?") != std::wstring::npos) {
    PrintHelp();

    // If we allocated a new console, wait for user input
    if (needWait) {
      printf("Press any key to exit...");
      _getch();
    } else {
      fflush(stdout);
    }
    return 0;
  }

  // Handle --list
  if (cmdLine.find(L"--list") != std::wstring::npos ||
      cmdLine.find(L"-l") != std::wstring::npos) {
    printf("Supported Hash Algorithms:\n\n");
    
    printf("MD Family:\n");
    printf("  MD2, MD4, MD5\n");
    printf("  MD6-128, MD6-160, MD6-192, MD6-224, MD6-256, MD6-384, MD6-512\n\n");
    
    printf("SHA Family:\n");
    printf("  SHA-1, SHA-224, SHA-256, SHA-384, SHA-512\n\n");
    
    printf("SHA-3 Family:\n");
    printf("  SHA3-224, SHA3-256, SHA3-384, SHA3-512\n\n");
    
    printf("BLAKE Family:\n");
    printf("  BLAKE2b, BLAKE2s, BLAKE3\n\n");
    
    printf("Keccak Family:\n");
    printf("  Keccak-224, Keccak-256, Keccak-384, Keccak-512\n");
    printf("  SHAKE128, SHAKE256\n\n");
    
    printf("HAVAL Family (specify rounds: 3/4/5):\n");
    printf("  HAVAL-3-128, HAVAL-3-160, HAVAL-3-192, HAVAL-3-224, HAVAL-3-256\n");
    printf("  HAVAL-4-128, HAVAL-4-160, HAVAL-4-192, HAVAL-4-224, HAVAL-4-256\n");
    printf("  HAVAL-5-128, HAVAL-5-160, HAVAL-5-192, HAVAL-5-224, HAVAL-5-256\n\n");
    
    printf("RIPEMD Family:\n");
    printf("  RIPEMD-128, RIPEMD-160, RIPEMD-256, RIPEMD-320\n\n");
    
    printf("Other Hashes:\n");
    printf("  Tiger, Whirlpool, SM3\n");
    printf("  LSH-256, LSH-512\n");
    printf("  GOST-R-34.11-94, GOST-R-34.11-2012-256, GOST-R-34.11-2012-512\n\n");
    
    printf("Checksums:\n");
    printf("  CRC-8, CRC-16, CRC32, CRC-32C, CRC-64, Adler32\n\n");

    // If we allocated a new console, wait for user input
    if (needWait) {
      printf("Press any key to exit...");
      _getch();
    } else {
      fflush(stdout);
    }
    return 0;
  }

  // Handle --file and --text options (Console mode - requires -a)
  // Parse algorithm parameters
  auto algoParams = ParseAlgorithmParameters(cmdLine);
  
  if (algoParams.empty()) {
    printf("Error: Console mode requires at least one algorithm (-a option).\n");
    printf("Example: HashCalc.exe -f file.txt -a SHA256 -a MD5\n\n");
    printf("Use --help for more information.\n\n");
    
    if (needWait) {
      printf("Press any key to exit...");
      _getch();
    } else {
      fflush(stdout);
    }
    return 1;
  }

  // Convert algorithm names to IDs and extract HAVAL pass information
  std::vector<AlgorithmInfo> algorithmInfos;
  ConfigManager configManager;
  configManager.Initialize();
  
  for (const auto& algoName : algoParams) {
    int id = configManager.GetAlgorithmIdFromName(algoName);
    if (id == 0) {
      wprintf(L"Warning: Unknown algorithm '%s', skipping.\n", algoName.c_str());
    } else {
      // Extract HAVAL pass if applicable
      int havalPass = ExtractHavalPass(algoName);
      
      // For HAVAL algorithms, require pass specification
      if (id >= IDC_HAVAL_128 && id <= IDC_HAVAL_256) {
        if (havalPass == 0) {
          wprintf(L"Warning: HAVAL algorithm requires pass specification (e.g., HAVAL-3-256), skipping '%s'.\n", algoName.c_str());
          continue;
        }
      }
      
      algorithmInfos.push_back(AlgorithmInfo(id, havalPass));
    }
  }

  if (algorithmInfos.empty()) {
    printf("Error: No valid algorithms specified.\n");
    printf("Use --list to see available algorithms.\n\n");
    
    if (needWait) {
      printf("Press any key to exit...");
      _getch();
    } else {
      fflush(stdout);
    }
    return 1;
  }

  // Initialize COM for hash algorithm operations
  CoInitialize(NULL);

  int result = 0;
  bool commandHandled = false;

  // Handle --file option
  size_t filePos = cmdLine.find(L"--file");
  if (filePos == std::wstring::npos) {
    filePos = cmdLine.find(L"-f");
  }

  if (filePos != std::wstring::npos) {
    // Find the file path after the flag
    size_t pathStart = cmdLine.find_first_not_of(L" \t", filePos + (cmdLine[filePos + 1] == L'-' ? 6 : 2));
    if (pathStart != std::wstring::npos) {
      std::wstring filePath;
      
      // Check if path is quoted
      if (cmdLine[pathStart] == L'"') {
        size_t pathEnd = cmdLine.find(L'"', pathStart + 1);
        if (pathEnd != std::wstring::npos) {
          filePath = cmdLine.substr(pathStart + 1, pathEnd - pathStart - 1);
        }
      } else {
        // Find end of path (space, -a flag, or end of string)
        size_t pathEnd = cmdLine.find_first_of(L" \t-", pathStart);
        if (pathEnd != std::wstring::npos && cmdLine[pathEnd] == L'-') {
          // Found a flag, backtrack to before it
          while (pathEnd > pathStart && (cmdLine[pathEnd - 1] == L' ' || cmdLine[pathEnd - 1] == L'\t')) {
            pathEnd--;
          }
        }
        if (pathEnd != std::wstring::npos && pathEnd > pathStart) {
          filePath = cmdLine.substr(pathStart, pathEnd - pathStart);
        } else if (pathEnd == std::wstring::npos) {
          filePath = cmdLine.substr(pathStart);
        }
      }

      if (!filePath.empty()) {
        result = ComputeHashForFileWithAlgorithms(filePath, algorithmInfos);
        commandHandled = true;
      } else {
        printf("Error: No file path specified after --file/-f option\n");
        result = 1;
        commandHandled = true;
      }
    } else {
      printf("Error: No file path specified after --file/-f option\n");
      result = 1;
      commandHandled = true;
    }
  }

  // Handle --text option (if file wasn't processed)
  if (!commandHandled) {
    size_t textPos = cmdLine.find(L"--text");
    if (textPos == std::wstring::npos) {
      textPos = cmdLine.find(L"-t");
    }

    if (textPos != std::wstring::npos) {
      // Find the text after the flag
      size_t textStart = cmdLine.find_first_not_of(L" \t", textPos + (cmdLine[textPos + 1] == L'-' ? 6 : 2));
      if (textStart != std::wstring::npos) {
        std::wstring text;
        
        // Check if text is quoted
        if (cmdLine[textStart] == L'"') {
          size_t textEnd = cmdLine.find(L'"', textStart + 1);
          if (textEnd != std::wstring::npos) {
            text = cmdLine.substr(textStart + 1, textEnd - textStart - 1);
          }
        } else {
          // Find end of text (before -a flag or end of string)
          size_t textEnd = cmdLine.find_first_of(L" \t-", textStart);
          if (textEnd != std::wstring::npos && cmdLine[textEnd] == L'-') {
            // Found a flag, backtrack to before it
            while (textEnd > textStart && (cmdLine[textEnd - 1] == L' ' || cmdLine[textEnd - 1] == L'\t')) {
              textEnd--;
            }
          }
          if (textEnd != std::wstring::npos && textEnd > textStart) {
            text = cmdLine.substr(textStart, textEnd - textStart);
          } else if (textEnd == std::wstring::npos) {
            text = cmdLine.substr(textStart);
          }
        }

        if (!text.empty()) {
          result = ComputeHashForTextWithAlgorithms(text, algorithmInfos);
          commandHandled = true;
        } else {
          printf("Error: No text specified after --text/-t option\n");
          result = 1;
          commandHandled = true;
        }
      } else {
        printf("Error: No text specified after --text/-t option\n");
        result = 1;
        commandHandled = true;
      }
    }
  }

  // If no recognized command was handled, show error
  if (!commandHandled) {
    printf("Error: Must specify either --file/-f or --text/-t in console mode.\n");
    printf("Use --help to see available options.\n\n");
    result = 1;
  }

  // Cleanup COM
  CoUninitialize();

  // If we allocated a new console, wait for user input
  if (needWait) {
    printf("\nPress any key to exit...");
    _getch();
  } else {
    fflush(stdout);
  }

  return result;
}

/**
 * @brief Parse command line arguments
 * @param lpCmdLine Command line string passed to the application
 * @return Parsed command line string with whitespace trimmed and quotes removed
 */
std::wstring ParseCommandLine(LPWSTR lpCmdLine) {
  // Check for empty command line
  if (lpCmdLine == nullptr || lpCmdLine[0] == L'\0') {
    return L"";
  }

  std::wstring cmdLine(lpCmdLine);

  // Trim leading whitespace
  size_t start = cmdLine.find_first_not_of(L" \t");
  if (start == std::wstring::npos) {
    return L"";
  }

  // Trim trailing whitespace
  size_t end = cmdLine.find_last_not_of(L" \t");
  cmdLine = cmdLine.substr(start, end - start + 1);

  // Remove surrounding quotes (single or double) if present
  if (cmdLine.length() >= 2) {
    if ((cmdLine.front() == L'"' && cmdLine.back() == L'"') ||
        (cmdLine.front() == L'\'' && cmdLine.back() == L'\'')) {
      cmdLine = cmdLine.substr(1, cmdLine.length() - 2);
    }
  }

  return cmdLine;
}

/**
 * @brief Hash Calculator Application Class
 * @details Main application class derived from Win32xx::CWinApp.
 *          Manages application initialization and COM lifecycle.
 */
class CHashCalcApp : public Win32xx::CWinApp {
public:
  CHashCalcApp() : m_lpCmdLine(nullptr) {}
  virtual ~CHashCalcApp() override = default;

  void SetCommandLine(LPWSTR lpCmdLine) {
    m_lpCmdLine = lpCmdLine;
  }

  /**
   * @brief Initialize application instance
   * @return FALSE to exit after dialog closes
   * @details Initializes COM, creates and shows the main dialog,
   *          then uninitializes COM after dialog destruction.
   */
  virtual BOOL InitInstance() override {
    // Initialize COM library for this thread (required for file dialogs and shell operations)
    CoInitialize(NULL);

    {
      // Create main dialog instance
      CHashCalcDialog dialog;

      // Parse command line for input and algorithms
      std::wstring cmdLine(m_lpCmdLine ? m_lpCmdLine : L"");
      
      // Extract algorithm parameters (if any)
      auto algoParams = ParseAlgorithmParameters(cmdLine);
      
      // Remove algorithm parameters from command line to get input
      std::wstring inputOnly = cmdLine;
      size_t pos = 0;
      while ((pos = inputOnly.find(L"-a", pos)) != std::wstring::npos) {
        // Check if it's a standalone -a flag
        if (pos == 0 || inputOnly[pos - 1] == L' ' || inputOnly[pos - 1] == L'\t') {
          // Find the end of this -a parameter
          size_t endPos = pos + 2;
          // Skip whitespace
          while (endPos < inputOnly.length() && (inputOnly[endPos] == L' ' || inputOnly[endPos] == L'\t')) {
            endPos++;
          }
          // Skip the algorithm name
          if (endPos < inputOnly.length()) {
            if (inputOnly[endPos] == L'"') {
              endPos = inputOnly.find(L'"', endPos + 1);
              if (endPos != std::wstring::npos) endPos++;
            } else {
              while (endPos < inputOnly.length() && inputOnly[endPos] != L' ' && inputOnly[endPos] != L'\t') {
                endPos++;
              }
            }
          }
          if (endPos != std::wstring::npos) {
            inputOnly.erase(pos, endPos - pos);
          }
        } else {
          pos++;
        }
      }
      // Also remove --algorithm
      pos = 0;
      while ((pos = inputOnly.find(L"--algorithm", pos)) != std::wstring::npos) {
        if (pos == 0 || inputOnly[pos - 1] == L' ' || inputOnly[pos - 1] == L'\t') {
          size_t endPos = pos + 11;
          while (endPos < inputOnly.length() && (inputOnly[endPos] == L' ' || inputOnly[endPos] == L'\t')) {
            endPos++;
          }
          if (endPos < inputOnly.length()) {
            if (inputOnly[endPos] == L'"') {
              endPos = inputOnly.find(L'"', endPos + 1);
              if (endPos != std::wstring::npos) endPos++;
            } else {
              while (endPos < inputOnly.length() && inputOnly[endPos] != L' ' && inputOnly[endPos] != L'\t') {
                endPos++;
              }
            }
          }
          if (endPos != std::wstring::npos) {
            inputOnly.erase(pos, endPos - pos);
          }
        } else {
          pos++;
        }
      }
      
      // Trim the input
      size_t start = inputOnly.find_first_not_of(L" \t");
      if (start != std::wstring::npos) {
        size_t end = inputOnly.find_last_not_of(L" \t");
        inputOnly = inputOnly.substr(start, end - start + 1);
        
        // Remove quotes if present
        if (inputOnly.length() >= 2 && 
            ((inputOnly.front() == L'"' && inputOnly.back() == L'"') ||
             (inputOnly.front() == L'\'' && inputOnly.back() == L'\''))) {
          inputOnly = inputOnly.substr(1, inputOnly.length() - 2);
        }
      } else {
        inputOnly.clear();
      }

      // Set command line input if provided
      if (!inputOnly.empty()) {
        dialog.SetCommandLineInput(inputOnly);
      }
      
      // Set algorithms if specified
      if (!algoParams.empty()) {
        dialog.SetCommandLineAlgorithms(algoParams);
      }
      
      // Auto-start calculation if input was provided
      if (!inputOnly.empty()) {
        dialog.SetAutoStartCalculation(true);
      }

      // Show modal dialog and wait for user interaction
      dialog.DoModal();
    }

    // Uninitialize COM library
    CoUninitialize();

    // Return FALSE to exit application after dialog closes
    return FALSE;
  }

private:
  LPWSTR m_lpCmdLine;
};

/**
 * @brief Windows application entry point
 * @param hInstance Application instance handle (unused)
 * @param hPrevInstance Previous instance handle (unused)
 * @param lpCmdLine Command line string
 * @param nCmdShow Show state (unused)
 * @return Application return code
 */
int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR lpCmdLine, _In_ int) {
  // Check if we should run in console mode
  if (IsConsoleMode(lpCmdLine)) {
    return RunConsoleMode(lpCmdLine);
  }

  // GUI mode - create application instance
  CHashCalcApp app;

  // Store command line arguments for later use
  app.SetCommandLine(lpCmdLine);

  // Run application message loop
  return app.Run();
}
