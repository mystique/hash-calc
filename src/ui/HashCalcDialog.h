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
// Custom message for progress update
#define WM_HASH_PROGRESS (WM_USER + 2)

// Structure for progress update data
struct ProgressData {
  std::wstring algorithmName;
  uint64_t bytesProcessed;
  uint64_t totalBytes;
  int algorithmIndex;  // Current algorithm being processed
  int totalAlgorithms; // Total number of algorithms to process
};

class CHashCalcDialog : public Win32xx::CDialog {
public:
  CHashCalcDialog();
  virtual ~CHashCalcDialog() override;

  // Set command line argument (file path or text)
  void SetCommandLineInput(const std::wstring& input);

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
  void UpdateProgress(const ProgressData& data); // Update progress bar and text
  void SetTaskbarProgress(TBPFLAG state); // Set taskbar progress state
  void SetTaskbarProgressValue(uint64_t completed, uint64_t total); // Set taskbar progress value
  void UpdateButtonStates(); // Added for control logic
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

  // System tray functions
  void CreateTrayIcon();
  void RemoveTrayIcon();
  void ShowTrayMenu();
  void RestoreFromTray();
  void MinimizeToTray();

  // Font (should be destroyed last among UI elements)
  CFont m_fontResult;

  // Win32++ Tab Control and Views
  // Views are heap-allocated because CTab takes ownership via unique_ptr
  Win32xx::CTab m_tabControl;
  CTabViewSHA* m_pViewSHA;
  CTabViewSHA3* m_pViewSHA3;
  CTabViewHAVAL* m_pViewHAVAL;
  CTabViewChecksum* m_pViewChecksum;

  // Thread management
  HANDLE m_hCalcThread;
  std::atomic<bool> m_bCancelCalculation;
  bool m_bIsCalculating;

  // Progress tracking
  std::wstring m_currentAlgorithm;
  int m_currentAlgorithmIndex;
  int m_totalAlgorithms;
  int m_lastReportedPercentage; // Last reported progress percentage for throttling

  // Configuration manager
  ConfigManager m_configManager;

  // Taskbar progress (COM object)
  Microsoft::WRL::ComPtr<ITaskbarList3> m_pTaskbarList;

  // System tray
  NOTIFYICONDATA m_nid;
  bool m_bTrayIconCreated;

  // Icon handle (shared resource, don't destroy)
  // Must be destroyed AFTER m_nid since m_nid.hIcon points to it
  HICON m_hAppIcon;

  // Command line input (file path or text)
  std::wstring m_cmdLineInput;
};

#endif // HASH_CALC_DIALOG_H
