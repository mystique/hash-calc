#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include "../../res/resource.h"
#include "wxx_dialog.h"
#include "HoverButton.h"
#include <shellapi.h>
#include <string>

class CAboutDialog : public Win32xx::CDialog {
public:
  CAboutDialog();
  virtual ~CAboutDialog() override = default;

protected:
  virtual BOOL OnInitDialog() override;
  virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;
  virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
  std::wstring GetVersionFromResource();
  std::wstring GetBuildDate();
  std::wstring GetCopyrightFromResource();
};

#endif // ABOUT_DIALOG_H
