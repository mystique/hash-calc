#ifndef TAB_VIEW_SHA3_H
#define TAB_VIEW_SHA3_H

#include "../../res/resource.h"
#include "wxx_dialog.h"
#include <map>

// Tab View for SHA-3 & Modern algorithms (Tab 1)
class CTabViewSHA3 : public Win32xx::CDialog {
public:
  CTabViewSHA3();
  virtual ~CTabViewSHA3() = default;

  // Set parent dialog for notifications
  void SetParentDialog(HWND hParent) { m_hParentDialog = hParent; }

  // Algorithm management
  int CountSelectedAlgorithms() const;
  void SetAlgorithmStates(const std::map<int, bool>& states);
  std::map<int, bool> GetAlgorithmStates() const;
  void SelectAll();
  void ClearAll();
  void EnableControls(bool enable);

protected:
  virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
  virtual BOOL OnInitDialog() override;
  virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;

private:
  // Algorithm IDs for this tab
  static const int s_algorithmIds[];
  static const size_t s_algorithmCount;

  // Parent dialog pointer for notifications
  HWND m_hParentDialog;
};

#endif // TAB_VIEW_SHA3_H
