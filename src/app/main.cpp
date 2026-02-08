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
#include <objbase.h>
#include <string>

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

      // Parse and set command line input if provided
      std::wstring cmdInput = ParseCommandLine(m_lpCmdLine);
      if (!cmdInput.empty()) {
        dialog.SetCommandLineInput(cmdInput);
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
  // Create application instance
  CHashCalcApp app;
  
  // Store command line arguments for later use
  app.SetCommandLine(lpCmdLine);
  
  // Run application message loop
  return app.Run();
}
