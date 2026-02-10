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
#include "utils/CommandLineParser.h"
#include <objbase.h>
#include <string>

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

      // Parse command line arguments
      auto args = utils::CommandLineParser::Parse(m_lpCmdLine);

      // Set command line input if provided
      if (!args.input.empty()) {
        dialog.SetCommandLineInput(args.input);
      }

      // Set algorithms if specified
      if (args.allAlgorithms) {
        // Use all available algorithms
        auto allAlgorithms = utils::CommandLineParser::GetAllAlgorithmNames();
        dialog.SetCommandLineAlgorithms(allAlgorithms);
      } else if (!args.algorithms.empty()) {
        // Use specified algorithms
        dialog.SetCommandLineAlgorithms(args.algorithms);
      }

      // Auto-start calculation if input and algorithms were provided
      if (!args.input.empty() && (args.allAlgorithms || !args.algorithms.empty())) {
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
  // Parse command line to determine execution mode
  auto args = utils::CommandLineParser::Parse(lpCmdLine);

  // Check if we should run in console mode
  if (args.mode == utils::CommandLineParser::Mode::Console) {
    return utils::CommandLineParser::RunConsoleMode(lpCmdLine);
  }

  // GUI mode - create application instance
  CHashCalcApp app;

  // Store command line arguments for later use
  app.SetCommandLine(lpCmdLine);

  // Run application message loop
  return app.Run();
}
