#include "ui/HashCalcDialog.h"
#include <objbase.h>

class CHashCalcApp : public Win32xx::CWinApp {
public:
  CHashCalcApp() = default;
  virtual ~CHashCalcApp() override = default;

  virtual BOOL InitInstance() override {
    // Initialize COM for taskbar progress and other COM features
    CoInitialize(NULL);

    // Create and show the main dialog in a scope
    // This ensures the dialog is destroyed BEFORE CoUninitialize
    {
      CHashCalcDialog dialog;
      dialog.DoModal();
    }

    // Uninitialize COM after dialog is destroyed
    CoUninitialize();

    // Return FALSE to exit the application after dialog closes
    return FALSE;
  }
};

// Windows application entry point
int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
  CHashCalcApp app;
  return app.Run();
}
