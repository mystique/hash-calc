#include "ui/HashCalcDialog.h"
#include <objbase.h>
#include <string>

// Helper function to parse command line arguments
std::wstring ParseCommandLine(LPWSTR lpCmdLine) {
  if (lpCmdLine == nullptr || lpCmdLine[0] == L'\0') {
    return L"";
  }

  std::wstring cmdLine(lpCmdLine);

  // Trim leading and trailing whitespace
  size_t start = cmdLine.find_first_not_of(L" \t");
  if (start == std::wstring::npos) {
    return L"";
  }

  size_t end = cmdLine.find_last_not_of(L" \t");
  cmdLine = cmdLine.substr(start, end - start + 1);

  // Remove surrounding quotes if present
  if (cmdLine.length() >= 2) {
    if ((cmdLine.front() == L'"' && cmdLine.back() == L'"') ||
        (cmdLine.front() == L'\'' && cmdLine.back() == L'\'')) {
      cmdLine = cmdLine.substr(1, cmdLine.length() - 2);
    }
  }

  return cmdLine;
}

class CHashCalcApp : public Win32xx::CWinApp {
public:
  CHashCalcApp() : m_lpCmdLine(nullptr) {}
  virtual ~CHashCalcApp() override = default;

  void SetCommandLine(LPWSTR lpCmdLine) {
    m_lpCmdLine = lpCmdLine;
  }

  virtual BOOL InitInstance() override {
    // Initialize COM for taskbar progress and other COM features
    CoInitialize(NULL);

    // Create and show the main dialog in a scope
    // This ensures the dialog is destroyed BEFORE CoUninitialize
    {
      CHashCalcDialog dialog;

      // Parse and set command line input if provided
      std::wstring cmdInput = ParseCommandLine(m_lpCmdLine);
      if (!cmdInput.empty()) {
        dialog.SetCommandLineInput(cmdInput);
      }

      dialog.DoModal();
    }

    // Uninitialize COM after dialog is destroyed
    CoUninitialize();

    // Return FALSE to exit the application after dialog closes
    return FALSE;
  }

private:
  LPWSTR m_lpCmdLine;
};

// Windows application entry point
int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR lpCmdLine, _In_ int) {
  CHashCalcApp app;
  app.SetCommandLine(lpCmdLine);
  return app.Run();
}
