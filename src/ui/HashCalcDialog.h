#ifndef HASH_CALC_DIALOG_H
#define HASH_CALC_DIALOG_H

#include "resource.h"
#include "wxx_dialog.h"
#include <atomic>

// Custom message for hash calculation completion
#define WM_HASH_COMPLETE (WM_USER + 1)

class CHashCalcDialog : public Win32xx::CDialog {
public:
  CHashCalcDialog();
  virtual ~CHashCalcDialog() override = default;

protected:
  // Virtual function overrides
  virtual BOOL OnInitDialog() override;
  virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;
  virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
  virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override; // Added
  virtual void OnCancel() override;

private:
  void UpdateTabDisplay(); // Added
  void EnableControls(bool enable);
  void DisableControlsForCalculation();
  void PerformHashCalculation();
  void ShowProgressBar(bool show);
  static DWORD WINAPI CalculateHashThread(LPVOID lpParam);

private:
  void OnExit();
  void OnSelectAll();
  void OnClearAll();
  void OnCalculate();
  void OnAbout();
  void OnBrowse();
  void OnStayOnTop();
  void OnDropFiles(HDROP hDrop);

  CFont m_fontResult;
  
  // Thread management
  HANDLE m_hCalcThread;
  std::atomic<bool> m_bCancelCalculation;
  bool m_bIsCalculating;
};

#endif // HASH_CALC_DIALOG_H
