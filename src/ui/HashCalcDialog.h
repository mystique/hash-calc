#ifndef HASH_CALC_DIALOG_H
#define HASH_CALC_DIALOG_H

#include "../../res/resource.h"
#include "wxx_dialog.h"
#include "wxx_tab.h"
#include "TabViewSHA.h"
#include "TabViewSHA3.h"
#include "TabViewHAVAL.h"
#include "TabViewChecksum.h"
#include "../utils/ConfigManager.h"
#include <atomic>
#include <memory>
#include <vector>
#include <sstream>
#include <shobjidl.h>
#include <wrl/client.h>

// Forward declarations
namespace core {
  class IHashAlgorithm;
}

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
  void UpdateTabNames(); // Replaces UpdateTabDisplay() - only updates tab text with counts
  bool IsAlgorithmSelected(int algorithmId); // Check if an algorithm is selected in any view
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
  
  // Refactored helper methods
  void SetCheckboxStates(const int* ids, size_t count, bool checked);
  void EnableControlsById(const int* ids, size_t count, bool enable);
  void ComputeHashAlgorithmsForText(std::wstringstream& output, bool& anyComputed, const std::string& inputData);
  void ComputeHashAlgorithmsForFile(std::wstringstream& output, bool& anyComputed, const std::wstring& filePath);
  
  // Static algorithm ID lists
  static const int s_allAlgorithmIds[];
  static const size_t s_allAlgorithmCount;

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

  // Win32++ Tab Control and Views
  Win32xx::CTab m_tabControl;
  CTabViewSHA m_viewSHA;
  CTabViewSHA3 m_viewSHA3;
  CTabViewHAVAL m_viewHAVAL;
  CTabViewChecksum m_viewChecksum;

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
