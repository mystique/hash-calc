/**
 * @file CommandLineParser.cpp
 * @brief Implementation of command-line argument parser
 * @author HashCalc Development Team
 * @date 2026
 * @version 1.0
 */

#include "CommandLineParser.h"
#include "ConfigManager.h"
#include "../core/HashAlgorithmFactory.h"
#include "../core/IHashAlgorithm.h"
#include "../core/AlgorithmIds.h"
#include "../../res/resource.h"
#include <iostream>
#include <chrono>
#include <io.h>
#include <fcntl.h>
#include <conio.h>
#include <objbase.h>

namespace utils {

// Initialize static members
std::map<int, std::string> CommandLineParser::s_displayNames;
std::map<int, std::string> CommandLineParser::s_factoryNames;
bool CommandLineParser::s_initialized = false;

// ============================================================================
// Initialization
// ============================================================================

void CommandLineParser::InitializeAlgorithmMappings() {
  if (s_initialized) return;

  // Display names (for console output)
  s_displayNames = {
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

  // Factory names (must match registered names in HashAlgorithmFactory)
  s_factoryNames = {
    {IDC_SHA_160, "SHA-1"}, {IDC_SHA_224, "SHA-224"}, {IDC_SHA_256, "SHA-256"},
    {IDC_SHA_384, "SHA-384"}, {IDC_SHA_512, "SHA-512"},
    {IDC_MD2, "MD2"}, {IDC_MD4, "MD4"}, {IDC_MD5, "MD5"},
    {IDC_MD6_128, "MD6-128"}, {IDC_MD6_160, "MD6-160"}, {IDC_MD6_192, "MD6-192"},
    {IDC_MD6_224, "MD6-224"}, {IDC_MD6_256, "MD6-256"}, {IDC_MD6_384, "MD6-384"},
    {IDC_MD6_512, "MD6-512"},
    {IDC_SHA3_224, "SHA3-224"}, {IDC_SHA3_256, "SHA3-256"},
    {IDC_SHA3_384, "SHA3-384"}, {IDC_SHA3_512, "SHA3-512"},
    {IDC_KECCAK_224, "Keccak-224"}, {IDC_KECCAK_256, "Keccak-256"},
    {IDC_KECCAK_384, "Keccak-384"}, {IDC_KECCAK_512, "Keccak-512"},
    {IDC_SHAKE_128, "SHAKE128"}, {IDC_SHAKE_256, "SHAKE256"},
    {IDC_BLAKE2B, "BLAKE2b"}, {IDC_BLAKE2S, "BLAKE2s"}, {IDC_BLAKE3, "BLAKE3"},
    {IDC_HAVAL_128, "HAVAL-128"}, {IDC_HAVAL_160, "HAVAL-160"},
    {IDC_HAVAL_192, "HAVAL-192"}, {IDC_HAVAL_224, "HAVAL-224"},
    {IDC_HAVAL_256, "HAVAL-256"},
    {IDC_RIPEMD_128, "RIPEMD-128"}, {IDC_RIPEMD_160, "RIPEMD-160"},
    {IDC_RIPEMD_256, "RIPEMD-256"}, {IDC_RIPEMD_320, "RIPEMD-320"},
    {IDC_CRC8, "CRC-8"}, {IDC_CRC16, "CRC-16"}, {IDC_CRC32, "CRC32"},
    {IDC_CRC32C, "CRC-32C"}, {IDC_CRC64, "CRC-64"},
    {IDC_ADLER32, "Adler32"},
    {IDC_TIGER, "Tiger"}, {IDC_WHIRLPOOL, "Whirlpool"}, {IDC_SM3, "SM3"},
    {IDC_LSH_256, "LSH-256"}, {IDC_LSH_512, "LSH-512"},
    {IDC_GOST94, "GOST-R-34.11-94"},
    {IDC_GOST2012_256, "GOST-R-34.11-2012-256"},
    {IDC_GOST2012_512, "GOST-R-34.11-2012-512"}
  };

  s_initialized = true;
}

// ============================================================================
// Public Interface
// ============================================================================

CommandLineParser::ParsedArgs CommandLineParser::Parse(LPWSTR lpCmdLine) {
  InitializeAlgorithmMappings();

  ParsedArgs args;

  if (lpCmdLine == nullptr || lpCmdLine[0] == L'\0') {
    args.mode = Mode::GUI;
    return args;
  }

  std::wstring cmdLine(lpCmdLine);

  // Check for help and list flags
  if (cmdLine.find(L"--help") != std::wstring::npos ||
      cmdLine.find(L"-h") != std::wstring::npos ||
      cmdLine.find(L"/?") != std::wstring::npos) {
    args.mode = Mode::Console;
    args.isHelp = true;
    return args;
  }

  if (cmdLine.find(L"--list") != std::wstring::npos ||
      cmdLine.find(L"-l") != std::wstring::npos) {
    args.mode = Mode::Console;
    args.isList = true;
    return args;
  }

  // Check for --all flag
  if (cmdLine.find(L"--all") != std::wstring::npos ||
      cmdLine.find(L"-A") != std::wstring::npos) {
    args.allAlgorithms = true;
  }

  // Check for --lowercase flag
  if (cmdLine.find(L"--lowercase") != std::wstring::npos ||
      cmdLine.find(L"-c") != std::wstring::npos) {
    args.lowercase = true;
  }

  // Parse based on mode
  if (IsConsoleMode(lpCmdLine)) {
    args.mode = Mode::Console;
    ParseConsoleModeArgs(cmdLine, args);
  } else {
    args.mode = Mode::GUI;
    ParseGUIModeArgs(cmdLine, args);
  }

  return args;
}

void CommandLineParser::PrintHelp() {
  printf("Hash Calculator - Multi-purpose Hash Computation Tool\n\n");
  printf("Usage:\n");
  printf("  HashCalc.exe                    Launch GUI\n");
  printf("  HashCalc.exe <input>            Launch GUI with input (auto-detect file/text)\n");
  printf("  HashCalc.exe <input> -a <algo>  Launch GUI with specific algorithms\n");
  printf("  HashCalc.exe -t/-f <input> -a   Console mode (requires -a)\n\n");
  printf("Options:\n");
  printf("  -h, --help                Display this help information\n");
  printf("  -l, --list                List all supported hash algorithms\n");
  printf("  -f, --file <path>         Console: Calculate hash of file (requires -a or -A)\n");
  printf("  -t, --text <content>      Console: Calculate hash of text (requires -a or -A)\n");
  printf("  -a, --algorithm <algo>    Specify algorithm (can use multiple times)\n");
  printf("                            Examples: MD5, SHA256, SHA3-256, BLAKE3\n");
  printf("  -A, --all                 Use all available algorithms (GUI and console)\n");
  printf("  -c, --lowercase           Output hash in lowercase (default: uppercase)\n\n");
  printf("Examples:\n");
  printf("  HashCalc.exe test.txt              # GUI mode, auto-detect file\n");
  printf("  HashCalc.exe \"Hello World\"         # GUI mode, treat as text\n");
  printf("  HashCalc.exe test.txt -a MD5       # GUI mode, select MD5\n");
  printf("  HashCalc.exe test.txt -A           # GUI mode, select all algorithms\n");
  printf("  HashCalc.exe -f test.txt -a SHA256 -a MD5  # Console mode\n");
  printf("  HashCalc.exe -f test.txt -A        # Console mode with all algorithms\n");
  printf("  HashCalc.exe -t \"text\" -a BLAKE3   # Console mode\n");
  printf("  HashCalc.exe -f test.txt -a SHA256 -c  # Console mode, lowercase output\n\n");
  printf("Notes:\n");
  printf("  - GUI mode: Use config algorithms if -a/-A not specified\n");
  printf("  - Console mode (-t/-f): Must specify at least one -a algorithm or use -A\n");
  printf("  - Paths/text with spaces should be quoted\n");
  printf("  - For HAVAL algorithms, -A includes all pass variants (3, 4, 5)\n");
  printf("  - Console mode outputs uppercase hex by default, use -c for lowercase\n\n");
}

void CommandLineParser::PrintAlgorithmList() {
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
}

int CommandLineParser::RunConsoleMode(LPWSTR lpCmdLine) {
  bool needWait = false;
  if (!AttachConsoleWindow(needWait)) {
    MessageBoxW(nullptr, L"Failed to create console window", L"Error", MB_OK | MB_ICONERROR);
    return 1;
  }

  ParsedArgs args = Parse(lpCmdLine);

  // Handle --help
  if (args.isHelp) {
    PrintHelp();
    if (needWait) {
      printf("Press any key to exit...");
      _getch();
    } else {
      fflush(stdout);
    }
    return 0;
  }

  // Handle --list
  if (args.isList) {
    PrintAlgorithmList();
    if (needWait) {
      printf("Press any key to exit...");
      _getch();
    } else {
      fflush(stdout);
    }
    return 0;
  }

  // Console mode requires algorithms (either -a or --all)
  if (args.algorithms.empty() && !args.allAlgorithms) {
    printf("Error: Console mode requires at least one algorithm (-a option) or --all flag.\n");
    printf("Example: HashCalc.exe -f file.txt -a SHA256 -a MD5\n");
    printf("     Or: HashCalc.exe -f file.txt -A\n\n");
    printf("Use --help for more information.\n\n");

    if (needWait) {
      printf("Press any key to exit...");
      _getch();
    } else {
      fflush(stdout);
    }
    return 1;
  }

  // Generate algorithm list (either from -a flags or all algorithms)
  std::vector<AlgorithmInfo> algorithmInfos;
  if (args.allAlgorithms) {
    algorithmInfos = GetAllAlgorithms();
  } else {
    algorithmInfos = ConvertAlgorithmsToInfo(args.algorithms);
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

  // Initialize COM
  CoInitialize(NULL);

  // Compute hash
  int result = 0;
  if (args.isFile) {
    if (!args.input.empty()) {
      result = ComputeHashForFileWithAlgorithms(args.input, algorithmInfos, args.lowercase);
    } else {
      printf("Error: No file path specified after --file/-f option\n");
      result = 1;
    }
  } else {
    if (!args.input.empty()) {
      result = ComputeHashForTextWithAlgorithms(args.input, algorithmInfos, args.lowercase);
    } else {
      printf("Error: No text specified after --text/-t option\n");
      result = 1;
    }
  }

  // Cleanup COM
  CoUninitialize();

  // Wait for user input if needed
  if (needWait) {
    printf("\nPress any key to exit...");
    _getch();
  } else {
    fflush(stdout);
  }

  return result;
}

std::string CommandLineParser::GetAlgorithmDisplayName(int algorithmId, int havalPass) {
  InitializeAlgorithmMappings();

  auto it = s_displayNames.find(algorithmId);
  if (it != s_displayNames.end()) {
    if (havalPass > 0 && (algorithmId >= IDC_HAVAL_128 && algorithmId <= IDC_HAVAL_256)) {
      return "HAVAL-" + std::to_string(havalPass) + "-" + it->second.substr(6);
    }
    return it->second;
  }
  return "Unknown";
}

std::string CommandLineParser::GetFactoryAlgorithmName(int algorithmId, int havalPass) {
  InitializeAlgorithmMappings();

  auto it = s_factoryNames.find(algorithmId);
  if (it != s_factoryNames.end()) {
    if (havalPass > 0 && (algorithmId >= IDC_HAVAL_128 && algorithmId <= IDC_HAVAL_256)) {
      std::string bits = it->second.substr(6);
      return "HAVAL-" + bits + "/Pass" + std::to_string(havalPass);
    }
    return it->second;
  }
  return "";
}

int CommandLineParser::ExtractHavalPass(const std::wstring& algoName) {
  if (algoName.find(L"HAVAL-") != 0 || algoName.length() < 11) {
    return 0;
  }

  wchar_t passChar = algoName[6];
  if ((passChar == L'3' || passChar == L'4' || passChar == L'5') && algoName[7] == L'-') {
    return passChar - L'0';
  }

  return 0;
}

std::vector<AlgorithmInfo> CommandLineParser::GetAllAlgorithms() {
  std::vector<AlgorithmInfo> algorithms;

  // Iterate through all algorithm IDs
  for (size_t i = 0; i < core::ALL_ALGORITHM_COUNT; i++) {
    int id = core::ALL_ALGORITHM_IDS[i];

    // For HAVAL algorithms, add all three pass variants (3, 4, 5)
    if (id >= IDC_HAVAL_128 && id <= IDC_HAVAL_256) {
      algorithms.push_back(AlgorithmInfo(id, 3));
      algorithms.push_back(AlgorithmInfo(id, 4));
      algorithms.push_back(AlgorithmInfo(id, 5));
    } else {
      algorithms.push_back(AlgorithmInfo(id, 0));
    }
  }

  return algorithms;
}

std::vector<std::wstring> CommandLineParser::GetAllAlgorithmNames() {
  InitializeAlgorithmMappings();
  std::vector<std::wstring> names;

  // Iterate through all algorithm IDs
  for (size_t i = 0; i < core::ALL_ALGORITHM_COUNT; i++) {
    int id = core::ALL_ALGORITHM_IDS[i];

    // For HAVAL algorithms, add all three pass variants (3, 4, 5)
    if (id >= IDC_HAVAL_128 && id <= IDC_HAVAL_256) {
      auto it = s_displayNames.find(id);
      if (it != s_displayNames.end()) {
        std::string displayName = it->second;
        // Generate HAVAL-3-xxx, HAVAL-4-xxx, HAVAL-5-xxx formats
        for (int pass = 3; pass <= 5; pass++) {
          std::wstring wideName = std::wstring(displayName.begin(), displayName.end());
          // Convert "HAVAL-128" to "HAVAL-3-128", "HAVAL-4-128", "HAVAL-5-128"
          size_t dashPos = wideName.find(L'-');
          if (dashPos != std::wstring::npos) {
            wideName.insert(dashPos + 1, std::to_wstring(pass) + L"-");
          }
          names.push_back(wideName);
        }
      }
    } else {
      // For non-HAVAL algorithms, use display name directly
      auto it = s_displayNames.find(id);
      if (it != s_displayNames.end()) {
        std::string displayName = it->second;
        std::wstring wideName = std::wstring(displayName.begin(), displayName.end());
        names.push_back(wideName);
      }
    }
  }

  return names;
}

// ============================================================================
// Private Helper Methods
// ============================================================================

bool CommandLineParser::IsConsoleMode(LPWSTR lpCmdLine) {
  if (lpCmdLine == nullptr || lpCmdLine[0] == L'\0') {
    return false;
  }

  std::wstring cmdLine(lpCmdLine);

  // Check for help and list flags
  if (cmdLine.find(L"--help") != std::wstring::npos ||
      cmdLine.find(L"-h") != std::wstring::npos ||
      cmdLine.find(L"/?") != std::wstring::npos ||
      cmdLine.find(L"--list") != std::wstring::npos ||
      cmdLine.find(L"-l") != std::wstring::npos) {
    return true;
  }

  // Check for console mode flags (-f/-t)
  if (cmdLine.find(L"--file") != std::wstring::npos ||
      cmdLine.find(L"-f") != std::wstring::npos ||
      cmdLine.find(L"--text") != std::wstring::npos ||
      cmdLine.find(L"-t") != std::wstring::npos) {
    return true;
  }

  return false;
}

bool CommandLineParser::AttachConsoleWindow(bool& outNeedWait) {
  outNeedWait = false;

  // Try to attach to parent console first
  if (::AttachConsole(ATTACH_PARENT_PROCESS)) {
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    SetConsoleOutputCP(CP_UTF8);
    printf("\n");
    return true;
  }

  // Allocate a new console
  if (::AllocConsole()) {
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    SetConsoleOutputCP(CP_UTF8);
    outNeedWait = true;
    return true;
  }

  return false;
}

std::wstring CommandLineParser::ExtractValue(const std::wstring& cmdLine, size_t startPos) {
  if (startPos >= cmdLine.length()) {
    return L"";
  }

  // Check if quoted
  if (cmdLine[startPos] == L'"') {
    size_t endPos = cmdLine.find(L'"', startPos + 1);
    if (endPos != std::wstring::npos) {
      return cmdLine.substr(startPos + 1, endPos - startPos - 1);
    }
    return L"";
  }

  // Not quoted - find end (space, tab, or dash)
  size_t endPos = cmdLine.find_first_of(L" \t-", startPos);

  // If we hit a dash, backtrack to skip trailing whitespace
  if (endPos != std::wstring::npos && cmdLine[endPos] == L'-') {
    while (endPos > startPos && (cmdLine[endPos - 1] == L' ' || cmdLine[endPos - 1] == L'\t')) {
      endPos--;
    }
  }

  if (endPos != std::wstring::npos && endPos > startPos) {
    return cmdLine.substr(startPos, endPos - startPos);
  } else if (endPos == std::wstring::npos) {
    return cmdLine.substr(startPos);
  }

  return L"";
}

void CommandLineParser::ParseConsoleModeArgs(const std::wstring& cmdLine, ParsedArgs& args) {
  // Parse algorithm parameters
  args.algorithms = ParseAlgorithmParameters(cmdLine);

  // Find file or text flag
  size_t filePos = cmdLine.find(L"--file");
  if (filePos == std::wstring::npos) {
    filePos = cmdLine.find(L"-f");
  }

  size_t textPos = cmdLine.find(L"--text");
  if (textPos == std::wstring::npos) {
    textPos = cmdLine.find(L"-t");
  }

  // Parse file input
  if (filePos != std::wstring::npos) {
    args.isFile = true;
    size_t flagLen = (cmdLine[filePos + 1] == L'-') ? 6 : 2;  // --file vs -f
    size_t valueStart = cmdLine.find_first_not_of(L" \t", filePos + flagLen);
    if (valueStart != std::wstring::npos) {
      args.input = ExtractValue(cmdLine, valueStart);
    }
  }
  // Parse text input
  else if (textPos != std::wstring::npos) {
    args.isFile = false;
    size_t flagLen = (cmdLine[textPos + 1] == L'-') ? 6 : 2;  // --text vs -t
    size_t valueStart = cmdLine.find_first_not_of(L" \t", textPos + flagLen);
    if (valueStart != std::wstring::npos) {
      args.input = ExtractValue(cmdLine, valueStart);
    }
  }
}

void CommandLineParser::ParseGUIModeArgs(const std::wstring& cmdLine, ParsedArgs& args) {
  // Parse algorithm parameters
  args.algorithms = ParseAlgorithmParameters(cmdLine);

  // Extract input by removing algorithm parameters
  std::wstring inputOnly = cmdLine;
  RemoveAlgorithmParameters(inputOnly);

  // Trim whitespace
  size_t start = inputOnly.find_first_not_of(L" \t");
  if (start == std::wstring::npos) {
    return;
  }

  size_t end = inputOnly.find_last_not_of(L" \t");
  inputOnly = inputOnly.substr(start, end - start + 1);

  // Remove quotes if present
  if (inputOnly.length() >= 2 &&
      ((inputOnly.front() == L'"' && inputOnly.back() == L'"') ||
       (inputOnly.front() == L'\'' && inputOnly.back() == L'\''))) {
    inputOnly = inputOnly.substr(1, inputOnly.length() - 2);
  }

  args.input = inputOnly;
}

void CommandLineParser::RemoveAlgorithmParameters(std::wstring& cmdLine) {
  // Remove -a parameters
  size_t pos = 0;
  while ((pos = cmdLine.find(L"-a", pos)) != std::wstring::npos) {
    // Check if it's a standalone flag
    if (pos > 0 && cmdLine[pos - 1] != L' ' && cmdLine[pos - 1] != L'\t') {
      pos++;
      continue;
    }

    // Find the end of this parameter (flag + value)
    size_t endPos = pos + 2;  // Skip "-a"

    // Skip whitespace after flag
    while (endPos < cmdLine.length() && (cmdLine[endPos] == L' ' || cmdLine[endPos] == L'\t')) {
      endPos++;
    }

    // Skip the value (quoted or unquoted)
    if (endPos < cmdLine.length()) {
      if (cmdLine[endPos] == L'"') {
        endPos = cmdLine.find(L'"', endPos + 1);
        if (endPos != std::wstring::npos) endPos++;
      } else {
        while (endPos < cmdLine.length() && cmdLine[endPos] != L' ' && cmdLine[endPos] != L'\t') {
          endPos++;
        }
      }
    }

    if (endPos != std::wstring::npos) {
      cmdLine.erase(pos, endPos - pos);
    } else {
      break;
    }
  }

  // Remove --algorithm parameters
  pos = 0;
  while ((pos = cmdLine.find(L"--algorithm", pos)) != std::wstring::npos) {
    if (pos > 0 && cmdLine[pos - 1] != L' ' && cmdLine[pos - 1] != L'\t') {
      pos++;
      continue;
    }

    size_t endPos = pos + 11;  // Skip "--algorithm"

    while (endPos < cmdLine.length() && (cmdLine[endPos] == L' ' || cmdLine[endPos] == L'\t')) {
      endPos++;
    }

    if (endPos < cmdLine.length()) {
      if (cmdLine[endPos] == L'"') {
        endPos = cmdLine.find(L'"', endPos + 1);
        if (endPos != std::wstring::npos) endPos++;
      } else {
        while (endPos < cmdLine.length() && cmdLine[endPos] != L' ' && cmdLine[endPos] != L'\t') {
          endPos++;
        }
      }
    }

    if (endPos != std::wstring::npos) {
      cmdLine.erase(pos, endPos - pos);
    } else {
      break;
    }
  }
}

std::vector<std::wstring> CommandLineParser::ParseAlgorithmParameters(const std::wstring& cmdLine) {
  std::vector<std::wstring> algorithms;
  size_t pos = 0;

  while (pos < cmdLine.length()) {
    // Find next -a or --algorithm
    size_t aPos = cmdLine.find(L"-a", pos);
    size_t algoPos = cmdLine.find(L"--algorithm", pos);

    size_t flagPos = std::wstring::npos;
    size_t flagLen = 0;

    // Determine which flag comes first
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
      break;
    }

    // Ensure it's a standalone flag
    if (flagPos > 0 && cmdLine[flagPos - 1] != L' ' && cmdLine[flagPos - 1] != L'\t') {
      pos = flagPos + 1;
      continue;
    }

    // Extract algorithm name
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

std::vector<AlgorithmInfo> CommandLineParser::ConvertAlgorithmsToInfo(
  const std::vector<std::wstring>& algorithms
) {
  std::vector<AlgorithmInfo> algorithmInfos;
  ConfigManager configManager;
  configManager.Initialize();

  for (const auto& algoName : algorithms) {
    int id = configManager.GetAlgorithmIdFromName(algoName);
    if (id == 0) {
      wprintf(L"Warning: Unknown algorithm '%s', skipping.\n", algoName.c_str());
      continue;
    }

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

  return algorithmInfos;
}

bool CommandLineParser::ComputeSingleHash(
  const AlgorithmInfo& algorithmInfo,
  const std::wstring& inputData,
  bool isFile,
  bool lowercase
) {
  std::string factoryName = GetFactoryAlgorithmName(algorithmInfo.id, algorithmInfo.havalPass);
  std::string displayName = GetAlgorithmDisplayName(algorithmInfo.id, algorithmInfo.havalPass);

  if (factoryName.empty()) {
    printf("%-15s: Error - Unknown algorithm ID\n", displayName.c_str());
    return false;
  }

  try {
    auto hashAlgo = core::HashAlgorithmFactory::create(factoryName);
    std::vector<uint8_t> digest;

    if (isFile) {
      digest = hashAlgo->computeFile(inputData);
    } else {
      // Convert wide string to UTF-8
      int utf8Size = WideCharToMultiByte(CP_UTF8, 0, inputData.c_str(), -1, nullptr, 0, nullptr, nullptr);
      if (utf8Size <= 0) {
        printf("%-15s: Error - Failed to convert text to UTF-8\n", displayName.c_str());
        return false;
      }

      std::vector<char> utf8Buffer(utf8Size);
      WideCharToMultiByte(CP_UTF8, 0, inputData.c_str(), -1, utf8Buffer.data(), utf8Size, nullptr, nullptr);
      std::string inputStr(utf8Buffer.data());

      digest = hashAlgo->computeString(inputStr);
    }

    // Use uppercase by default, lowercase if flag is set
    std::string hash = core::IHashAlgorithm::toHexString(digest, !lowercase);
    printf("%-15s: %s\n", displayName.c_str(), hash.c_str());
    return true;
  } catch (const std::exception& ex) {
    printf("%-15s: Error - %s\n", displayName.c_str(), ex.what());
    return false;
  }
}

int CommandLineParser::ComputeHashForTextWithAlgorithms(
  const std::wstring& text,
  const std::vector<AlgorithmInfo>& algorithmInfos,
  bool lowercase
) {
  // Convert text to UTF-8 to calculate size
  int utf8Size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
  size_t textBytes = (utf8Size > 0) ? (utf8Size - 1) : 0; // Exclude null terminator

  // Print start message with text information
  wprintf(L"Calculating hash of %zu bytes string `%s`...\n\n", textBytes, text.c_str());

  // Start timing
  auto start_time = std::chrono::high_resolution_clock::now();

  bool anyComputed = false;
  for (const AlgorithmInfo& info : algorithmInfos) {
    if (ComputeSingleHash(info, text, false, lowercase)) {
      anyComputed = true;
    }
  }

  // End timing
  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_time - start_time;

  if (!anyComputed) {
    printf("\nFailed to compute any hashes.\n");
    return 1;
  }

  // Print elapsed time
  printf("\nCalculation took %.3f seconds\n", elapsed.count());

  return 0;
}

int CommandLineParser::ComputeHashForFileWithAlgorithms(
  const std::wstring& filePath,
  const std::vector<AlgorithmInfo>& algorithmInfos,
  bool lowercase
) {
  // Validate file exists
  DWORD fileAttr = GetFileAttributesW(filePath.c_str());
  if (fileAttr == INVALID_FILE_ATTRIBUTES) {
    wprintf(L"Error: File not found: %s\n", filePath.c_str());
    return 1;
  }

  if (fileAttr & FILE_ATTRIBUTE_DIRECTORY) {
    wprintf(L"Error: Path is a directory, not a file: %s\n", filePath.c_str());
    return 1;
  }

  // Get file size
  HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  LARGE_INTEGER fileSize = {0};
  if (hFile != INVALID_HANDLE_VALUE) {
    GetFileSizeEx(hFile, &fileSize);
    CloseHandle(hFile);
  }

  // Extract just the filename from full path
  std::wstring fileName = filePath;
  size_t lastSlash = filePath.find_last_of(L"\\/");
  if (lastSlash != std::wstring::npos) {
    fileName = filePath.substr(lastSlash + 1);
  }

  // Print start message with file information
  wprintf(L"Calculating hash of file: %s\n", fileName.c_str());
  wprintf(L"File size: %lld bytes\n\n", fileSize.QuadPart);

  // Start timing
  auto start_time = std::chrono::high_resolution_clock::now();

  bool anyComputed = false;
  for (const AlgorithmInfo& info : algorithmInfos) {
    if (ComputeSingleHash(info, filePath, true, lowercase)) {
      anyComputed = true;
    }
  }

  // End timing
  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_time - start_time;

  if (!anyComputed) {
    printf("\nFailed to compute any hashes.\n");
    return 1;
  }

  // Print elapsed time
  printf("\nCalculation took %.3f seconds\n", elapsed.count());

  return 0;
}

} // namespace utils
