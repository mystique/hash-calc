#include "ui/HashCalcDialog.h"

// Application class
class CHashCalcApp : public Win32xx::CWinApp {
public:
  CHashCalcApp() = default;
  virtual ~CHashCalcApp() override = default;

  virtual BOOL InitInstance() override {
    // Create and show the main dialog
    m_dialog.DoModal();
    // Return FALSE to exit the application after dialog closes
    return FALSE;
  }

private:
  CHashCalcDialog m_dialog;
};

// Entry point
int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
  CHashCalcApp app;
  return app.Run();
}
