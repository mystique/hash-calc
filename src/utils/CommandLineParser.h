/**
 * @file CommandLineParser.h
 * @brief Command-line argument parser for HashCalc application
 * @author HashCalc Development Team
 * @date 2026
 * @version 1.0
 * @details Handles parsing and processing of command-line arguments,
 *          including mode detection, algorithm parsing, and console operations.
 */

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>

// Forward declarations
class ConfigManager;

namespace utils {

/**
 * @brief Structure to hold algorithm information with optional pass number
 */
struct AlgorithmInfo {
  int id;         // Algorithm ID from resource.h
  int havalPass;  // HAVAL pass number (3, 4, or 5), or 0 for non-HAVAL algorithms

  AlgorithmInfo(int _id = 0, int _pass = 0) : id(_id), havalPass(_pass) {}
};

/**
 * @brief Command-line parser for HashCalc application
 * @details Provides static methods for parsing command-line arguments,
 *          detecting execution mode (GUI vs Console), and running console operations.
 */
class CommandLineParser {
public:
  /**
   * @brief Execution mode enumeration
   */
  enum class Mode {
    GUI,      // GUI mode - show main window
    Console   // Console mode - run command-line operations and exit
  };

  /**
   * @brief Parsed command-line arguments structure
   */
  struct ParsedArgs {
    Mode mode;                              // Execution mode (GUI or Console)
    std::wstring input;                     // Input (file path or text)
    std::vector<std::wstring> algorithms;   // Algorithm names specified via -a flag
    bool isHelp;                            // --help flag
    bool isList;                            // --list flag
    bool isFile;                            // true = file input (-f), false = text input (-t)
    bool allAlgorithms;                     // --all flag (use all available algorithms)
    bool lowercase;                         // --lowercase flag (use lowercase output in console mode)

    ParsedArgs()
      : mode(Mode::GUI), isHelp(false), isList(false), isFile(false), allAlgorithms(false), lowercase(false) {}
  };

  // ============================================================================
  // Public Interface
  // ============================================================================

  /**
   * @brief Parse command-line arguments
   * @param lpCmdLine Raw command line from WinMain
   * @return Parsed arguments structure
   */
  static ParsedArgs Parse(LPWSTR lpCmdLine);

  /**
   * @brief Run application in console mode
   * @param lpCmdLine Command line arguments
   * @return Exit code (0 = success, 1 = error)
   */
  static int RunConsoleMode(LPWSTR lpCmdLine);

  /**
   * @brief Print help information to console
   */
  static void PrintHelp();

  /**
   * @brief Print list of supported algorithms to console
   */
  static void PrintAlgorithmList();

  // ============================================================================
  // Helper Functions (public for testing/GUI integration)
  // ============================================================================

  /**
   * @brief Get algorithm display name from ID
   * @param algorithmId Algorithm ID from resource.h
   * @param havalPass HAVAL pass number (3/4/5) if applicable
   * @return Human-readable algorithm name (e.g., "SHA-256", "HAVAL-3-256")
   */
  static std::string GetAlgorithmDisplayName(int algorithmId, int havalPass = 0);

  /**
   * @brief Get factory algorithm name from ID
   * @param algorithmId Algorithm ID from resource.h
   * @param havalPass HAVAL pass number (3/4/5) if applicable
   * @return Algorithm name for HashAlgorithmFactory (must match registered names)
   */
  static std::string GetFactoryAlgorithmName(int algorithmId, int havalPass = 0);

  /**
   * @brief Extract HAVAL pass number from algorithm name
   * @param algoName Algorithm name (e.g., "HAVAL-3-256")
   * @return Pass number (3, 4, or 5), or 0 if not a HAVAL name
   */
  static int ExtractHavalPass(const std::wstring& algoName);

  /**
   * @brief Get all available algorithms
   * @return Vector of AlgorithmInfo structures for all available algorithms
   * @details For HAVAL algorithms, includes all three pass variants (3, 4, 5)
   */
  static std::vector<AlgorithmInfo> GetAllAlgorithms();

  /**
   * @brief Get all algorithm names for GUI mode
   * @return Vector of wide string algorithm names (e.g., "SHA-256", "MD5", "HAVAL-3-256", "HAVAL-4-256", "HAVAL-5-256")
   * @details For HAVAL algorithms, returns all three pass variants (3, 4, 5)
   */
  static std::vector<std::wstring> GetAllAlgorithmNames();

private:
  // ============================================================================
  // Private Helper Methods
  // ============================================================================

  /**
   * @brief Check if command line indicates console mode
   * @param lpCmdLine Command line string
   * @return true if should run in console mode
   */
  static bool IsConsoleMode(LPWSTR lpCmdLine);

  /**
   * @brief Attach console to the current process for command-line output
   * @param outNeedWait Set to true if we allocated a new console (need to wait for user)
   * @return true if console was successfully attached or allocated
   */
  static bool AttachConsoleWindow(bool& outNeedWait);

  /**
   * @brief Parse -a/--algorithm parameters from command line
   * @param cmdLine Command line string
   * @return Vector of algorithm names
   */
  static std::vector<std::wstring> ParseAlgorithmParameters(const std::wstring& cmdLine);

  /**
   * @brief Extract quoted or unquoted string value after a position
   * @param cmdLine Command line string
   * @param startPos Position to start extraction
   * @return Extracted string value
   */
  static std::wstring ExtractValue(const std::wstring& cmdLine, size_t startPos);

  /**
   * @brief Parse console mode arguments (-f/-t and input)
   * @param cmdLine Command line string
   * @param args Output parsed arguments
   */
  static void ParseConsoleModeArgs(const std::wstring& cmdLine, ParsedArgs& args);

  /**
   * @brief Parse GUI mode arguments (input and -a flags)
   * @param cmdLine Command line string
   * @param args Output parsed arguments
   */
  static void ParseGUIModeArgs(const std::wstring& cmdLine, ParsedArgs& args);

  /**
   * @brief Remove algorithm parameters from command line string
   * @param cmdLine Command line string to modify
   */
  static void RemoveAlgorithmParameters(std::wstring& cmdLine);

  /**
   * @brief Convert algorithm names to AlgorithmInfo structures
   * @param algorithms Algorithm names
   * @return Vector of AlgorithmInfo structures
   */
  static std::vector<AlgorithmInfo> ConvertAlgorithmsToInfo(
    const std::vector<std::wstring>& algorithms
  );

  /**
   * @brief Compute hash for text input using specified algorithms
   * @param text Text to hash
   * @param algorithmInfos Vector of algorithm information to use
   * @param lowercase True to output lowercase hex, false for uppercase
   * @return 0 on success, 1 on error
   */
  static int ComputeHashForTextWithAlgorithms(
    const std::wstring& text,
    const std::vector<AlgorithmInfo>& algorithmInfos,
    bool lowercase = false
  );

  /**
   * @brief Compute hash for file input using specified algorithms
   * @param filePath Path to file to hash
   * @param algorithmInfos Vector of algorithm information to use
   * @param lowercase True to output lowercase hex, false for uppercase
   * @return 0 on success, 1 on error
   */
  static int ComputeHashForFileWithAlgorithms(
    const std::wstring& filePath,
    const std::vector<AlgorithmInfo>& algorithmInfos,
    bool lowercase = false
  );

  /**
   * @brief Compute single hash with error handling
   * @param algorithmInfo Algorithm information
   * @param inputData Input data to hash
   * @param isFile True if hashing a file path, false if hashing string data
   * @param lowercase True to output lowercase hex, false for uppercase
   * @return true if hash was computed successfully
   */
  static bool ComputeSingleHash(
    const AlgorithmInfo& algorithmInfo,
    const std::wstring& inputData,
    bool isFile,
    bool lowercase = false
  );

  /**
   * @brief Initialize algorithm name mappings
   */
  static void InitializeAlgorithmMappings();

  // ============================================================================
  // Static Data Members
  // ============================================================================

  static std::map<int, std::string> s_displayNames;   // Algorithm ID -> Display name
  static std::map<int, std::string> s_factoryNames;   // Algorithm ID -> Factory name
  static bool s_initialized;                          // Initialization flag
};

} // namespace utils

#endif // COMMANDLINEPARSER_H
