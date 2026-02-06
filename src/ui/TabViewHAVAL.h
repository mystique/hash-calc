#ifndef TAB_VIEW_HAVAL_H
#define TAB_VIEW_HAVAL_H

#include "../../res/resource.h"
#include "wxx_dialog.h"
#include <map>

// Tab View for HAVAL & RIPEMD algorithms (Tab 2)
class CTabViewHAVAL : public Win32xx::CDialog {
public:
  CTabViewHAVAL();
  virtual ~CTabViewHAVAL() = default;

  // Set parent dialog for notifications
  void SetParentDialog(HWND hParent) { m_hParentDialog = hParent; }

  // Algorithm management
  int CountSelectedAlgorithms() const;
  void SetAlgorithmStates(const std::map<int, bool>& states);
  std::map<int, bool> GetAlgorithmStates() const;
  void SelectAll();
  void ClearAll();
  void EnableControls(bool enable);

  // HAVAL pass management
  void SetHavalPassStates(bool pass3, bool pass4, bool pass5);
  void GetHavalPassStates(bool& pass3, bool& pass4, bool& pass5) const;

protected:
  virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
  virtual BOOL OnInitDialog() override;
  virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;

private:
  // Algorithm IDs for this tab
  static const int s_algorithmIds[];
  static const size_t s_algorithmCount;

  // HAVAL pass IDs
  static const int s_havalPassIds[];
  static const size_t s_havalPassCount;

  // Parent dialog pointer for notifications
  HWND m_hParentDialog;
};

#endif // TAB_VIEW_HAVAL_H
