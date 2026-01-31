#ifndef HASH_CALC_DIALOG_H
#define HASH_CALC_DIALOG_H

#include "resource.h"
#include "wxx_dialog.h"
#include "../utils/ConfigManager.h"
#include <atomic>
#include <shobjidl.h>
#include <wrl/client.h>

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
  
  // For handling Enter key in edit boxes
  virtual BOOL PreTranslateMessage(MSG& msg);

private:
  void UpdateTabDisplay(); // Added
  int CountSelectedAlgorithmsForTab(int tabIndex); // Count selected algorithms for a specific tab
  void EnableControls(bool enable);
  void DisableControlsForCalculation();
  void PerformHashCalculation();
  void ShowProgressBar(bool show);
  void UpdateButtonStates(); // Added for control logic
  void SetTaskbarProgress(TBPFLAG state); // Set taskbar progress state
  bool HasAnyAlgorithmSelected(); // Check if any algorithm is selected
  bool HasAllAlgorithmsSelected(); // Check if all algorithms are selected
  bool HasValidInput(); // Check if input is valid
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
  void OnFilePathEnter();
  bool ValidateFilePath(const std::wstring& filePath, std::wstring& errorMsg);
  
  // Configuration management
  void LoadConfiguration();
  void SaveConfiguration();

  CFont m_fontResult;
  
  // Thread management
  HANDLE m_hCalcThread;
  std::atomic<bool> m_bCancelCalculation;
  bool m_bIsCalculating;
  
  // Configuration manager
  ConfigManager m_configManager;
  
  // Taskbar progress
  Microsoft::WRL::ComPtr<ITaskbarList3> m_pTaskbarList;
};

#endif // HASH_CALC_DIALOG_H
