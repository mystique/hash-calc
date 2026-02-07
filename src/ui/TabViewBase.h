#ifndef TAB_VIEW_BASE_H
#define TAB_VIEW_BASE_H

#include "../../res/resource.h"
#include "wxx_dialog.h"
#include <map>

// Base class for all tab views
// Provides common functionality for algorithm management
class CTabViewBase : public Win32xx::CDialog {
public:
  CTabViewBase(UINT nResID);
  virtual ~CTabViewBase() = default;

  // Set parent dialog for notifications
  void SetParentDialog(HWND hParent) { m_hParentDialog = hParent; }

  // Algorithm management - common interface
  virtual int CountSelectedAlgorithms() const;
  void SetAlgorithmStates(const std::map<int, bool>& states);
  std::map<int, bool> GetAlgorithmStates() const;
  void SelectAll();
  void ClearAll();
  void EnableControls(bool enable);

protected:
  virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
  virtual BOOL OnInitDialog() override;
  virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;

  // Derived classes must provide their algorithm IDs
  virtual const int* GetAlgorithmIds() const = 0;
  virtual size_t GetAlgorithmCount() const = 0;

  // Optional: Additional control IDs (e.g., HAVAL passes)
  virtual const int* GetAdditionalControlIds() const { return nullptr; }
  virtual size_t GetAdditionalControlCount() const { return 0; }

  // Optional: Custom behavior hooks
  virtual void OnSelectAllExtra() {}
  virtual void OnClearAllExtra() {}
  virtual void OnEnableControlsExtra(bool enable) { (void)enable; }

  // Parent dialog pointer for notifications
  HWND m_hParentDialog;
};

#endif // TAB_VIEW_BASE_H

