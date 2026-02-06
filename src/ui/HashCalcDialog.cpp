#include "HashCalcDialog.h"
#include "HoverButton.h"
#include "utils/EditUtils.h"
#include <Commdlg.h>
#include <Shellapi.h>
#include "../core/HashAlgorithmFactory.h"
#include "../core/IHashAlgorithm.h"
#include <sstream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <process.h>

// Ensure COM is properly linked
#pragma comment(lib, "ole32.lib")

// Static algorithm ID list definition
const int CHashCalcDialog::s_allAlgorithmIds[] = {
  IDC_SHA_160, IDC_SHA_224, IDC_SHA_256, IDC_SHA_384, IDC_SHA_512,
  IDC_HAVAL_128, IDC_HAVAL_160, IDC_HAVAL_192, IDC_HAVAL_224, IDC_HAVAL_256,
  IDC_RIPEMD_128, IDC_RIPEMD_160, IDC_RIPEMD_256, IDC_RIPEMD_320,
  IDC_MD2, IDC_MD4, IDC_MD5, IDC_CRC32, IDC_ADLER32, IDC_CRC16, IDC_CRC32C, IDC_CRC64, IDC_CRC8,
  IDC_MD6_128, IDC_MD6_160, IDC_MD6_192, IDC_MD6_224, IDC_MD6_256, IDC_MD6_384, IDC_MD6_512,
  IDC_SHA3_224, IDC_SHA3_256, IDC_SHA3_384, IDC_SHA3_512,
  IDC_KECCAK_224, IDC_KECCAK_256, IDC_KECCAK_384, IDC_KECCAK_512,
  IDC_SHAKE_128, IDC_SHAKE_256,
  IDC_TIGER, IDC_SM3, IDC_WHIRLPOOL,
  IDC_BLAKE2B, IDC_BLAKE2S,
  IDC_LSH_256, IDC_LSH_512
};

const size_t CHashCalcDialog::s_allAlgorithmCount = sizeof(s_allAlgorithmIds) / sizeof(int);

CHashCalcDialog::CHashCalcDialog()
  : CDialog(IDD_MAIN_DIALOG),
    m_hCalcThread(NULL),
    m_bCancelCalculation(false),
    m_bIsCalculating(false),
    m_bTrayIconCreated(false) {
  // Initialize NOTIFYICONDATA structure
  ZeroMemory(&m_nid, sizeof(m_nid));
}

BOOL CHashCalcDialog::PreTranslateMessage(MSG& msg) {
  // Handle Enter key in edit boxes
  if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
    HWND hFocus = ::GetFocus();
    HWND hFileEdit = GetDlgItem(IDC_EDIT_FILE);
    HWND hTextEdit = GetDlgItem(IDC_EDIT_TEXT);
    
    if (hFocus == hFileEdit && IsDlgButtonChecked(IDC_RADIO_FILE)) {
      // User pressed Enter in file path edit box
      OnFilePathEnter();
      return TRUE; // Message handled
    } else if (hFocus == hTextEdit && IsDlgButtonChecked(IDC_RADIO_TEXT)) {
      // User pressed Enter in text input box
      OnCalculate();
      return TRUE; // Message handled
    }
  }
  
  return CDialog::PreTranslateMessage(msg);
}

BOOL CHashCalcDialog::OnInitDialog() {
  // Call base class implementation
  CDialog::OnInitDialog();

  // Set the icon for this dialog
  HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
  SetIcon(hIcon, TRUE);   // Set big icon
  SetIcon(hIcon, FALSE);  // Set small icon

  // Set monospace font for result box
  m_fontResult.CreatePointFont(90, L"Consolas");
  GetDlgItem(IDC_EDIT_RESULT).SetFont(m_fontResult);

  // Set default input mode to Text
  CheckRadioButton(IDC_RADIO_TEXT, IDC_RADIO_FILE, IDC_RADIO_TEXT);

  // Enable text input by default, disable file path input
  GetDlgItem(IDC_EDIT_TEXT).EnableWindow(TRUE);
  GetDlgItem(IDC_EDIT_FILE).EnableWindow(FALSE);
  GetDlgItem(IDC_BUTTON_BROWSE).EnableWindow(FALSE);

  // Apply visual styles (hover effect) to buttons
  AttachHoverButton(GetDlgItem(IDC_SELECT_ALL));
  AttachHoverButton(GetDlgItem(IDC_CLEAR_ALL));
  AttachHoverButton(GetDlgItem(IDC_BUTTON_BROWSE));
  AttachHoverButton(GetDlgItem(IDC_BUTTON_CALCULATE));
  AttachHoverButton(GetDlgItem(IDC_BUTTON_ABOUT));
  AttachHoverButton(GetDlgItem(IDC_BUTTON_EXIT));

  // Center text vertically in input boxes
  AttachVerticalCenteredEdit(GetDlgItem(IDC_EDIT_TEXT));
  AttachVerticalCenteredEdit(GetDlgItem(IDC_EDIT_FILE));
  
  // Apply custom 3D border to result text box with proper scrollbar support
  AttachAutoScrollingEdit(GetDlgItem(IDC_EDIT_RESULT));

  // Default HAVAL pass: None checked initially (User request)
  // CheckRadioButton(IDC_HAVAL_PASS3, IDC_HAVAL_PASS5, IDC_HAVAL_PASS3);

  // Initialize Tab Control with Win32++ CTab
  m_tabControl.AttachDlgItem(IDC_TAB_MAIN, *this);

  // Create tab views (they will create their own windows from dialog templates)
  m_viewSHA.Create(*this);
  m_viewSHA3.Create(*this);
  m_viewHAVAL.Create(*this);
  m_viewChecksum.Create(*this);

  // Set parent dialog for notifications
  m_viewSHA.SetParentDialog(*this);
  m_viewSHA3.SetParentDialog(*this);
  m_viewHAVAL.SetParentDialog(*this);
  m_viewChecksum.SetParentDialog(*this);

  // Add views to tab control
  m_tabControl.AddTabPage(&m_viewSHA, L"SHA && MD");
  m_tabControl.AddTabPage(&m_viewSHA3, L"SHA-3 && Modern");
  m_tabControl.AddTabPage(&m_viewHAVAL, L"HAVAL && RIPEMD");
  m_tabControl.AddTabPage(&m_viewChecksum, L"Checksum && Others");

  // Select first tab
  m_tabControl.SelectPage(0);

  // Update tab names with initial counts
  UpdateTabNames();

  // Initialize progress bar (marquee style, initially hidden)
  HWND hProgress = GetDlgItem(IDC_PROGRESS_CALC);
  ::SendMessage(hProgress, PBM_SETMARQUEE, 0, 0);
  ::SetWindowLong(hProgress, GWL_STYLE, ::GetWindowLong(hProgress, GWL_STYLE) | PBS_MARQUEE);

  // Enable drag and drop for files
  DragAcceptFiles(TRUE);

  // Load saved configuration
  LoadConfiguration();

  // Initialize button states based on initial conditions
  UpdateButtonStates();

  // Initialize taskbar progress (COM interface)
  // CoInitialize should already be called by the application
  HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARGS(&m_pTaskbarList));
  if (SUCCEEDED(hr) && m_pTaskbarList) {
    m_pTaskbarList->HrInit();
  }

  // Initialize system tray icon (but don't show it yet)
  CreateTrayIcon();

  return TRUE;
}

BOOL CHashCalcDialog::OnCommand(WPARAM wparam, LPARAM lparam) {
  UNREFERENCED_PARAMETER(lparam);

  UINT id = LOWORD(wparam);
  UINT code = HIWORD(wparam);

  switch (id) {
  case IDC_BUTTON_EXIT:
    OnExit();
    return TRUE;

  case IDC_SELECT_ALL:
    OnSelectAll();
    return TRUE;

  case IDC_CLEAR_ALL:
    OnClearAll();
    return TRUE;

  case IDC_BUTTON_CALCULATE:
    OnCalculate();
    return TRUE;

  case IDC_BUTTON_ABOUT:
    OnAbout();
    return TRUE;

  case IDC_BUTTON_BROWSE:
    OnBrowse();
    return TRUE;

  case IDC_RADIO_TEXT:
    if (code == BN_CLICKED) {
      GetDlgItem(IDC_EDIT_TEXT).EnableWindow(TRUE);
      GetDlgItem(IDC_EDIT_FILE).EnableWindow(FALSE);
      GetDlgItem(IDC_BUTTON_BROWSE).EnableWindow(FALSE);
      UpdateButtonStates();
      SaveConfiguration();
    }
    return TRUE;

  case IDC_RADIO_FILE:
    if (code == BN_CLICKED) {
      GetDlgItem(IDC_EDIT_TEXT).EnableWindow(FALSE);
      GetDlgItem(IDC_EDIT_FILE).EnableWindow(TRUE);
      GetDlgItem(IDC_BUTTON_BROWSE).EnableWindow(TRUE);
      // Set focus to file path edit box
      ::SetFocus(GetDlgItem(IDC_EDIT_FILE));
      UpdateButtonStates();
      SaveConfiguration();
    }
    return TRUE;

  case IDC_EDIT_FILE:
    if (code == EN_CHANGE) {
      // File path changed - update button states
      UpdateButtonStates();
    }
    return TRUE;

  case IDC_EDIT_TEXT:
    if (code == EN_CHANGE) {
      // Text input changed - update button states
      UpdateButtonStates();
    }
    return TRUE;

  case IDC_CHECK_STAY_ON_TOP:
    OnStayOnTop();
    SaveConfiguration();
    return TRUE;

  // System tray menu commands
  case IDM_TRAY_RESTORE:
    RestoreFromTray();
    return TRUE;

  case IDM_TRAY_STOP:
    if (m_bIsCalculating) {
      OnCalculate(); // This will stop the calculation
    }
    return TRUE;

  case IDM_TRAY_EXIT:
    OnExit();
    return TRUE;
  }

  // Handle all algorithm checkbox changes
  if (code == BN_CLICKED) {
    int algorithmIDs[] = {
      IDC_SHA_160, IDC_SHA_224, IDC_SHA_256, IDC_SHA_384, IDC_SHA_512,
      IDC_HAVAL_128, IDC_HAVAL_160, IDC_HAVAL_192, IDC_HAVAL_224, IDC_HAVAL_256,
      IDC_RIPEMD_128, IDC_RIPEMD_160, IDC_RIPEMD_256, IDC_RIPEMD_320,
      IDC_MD2, IDC_MD4, IDC_MD5, IDC_CRC32, IDC_ADLER32, IDC_CRC16, IDC_CRC32C, IDC_CRC64, IDC_CRC8,
      IDC_MD6_128, IDC_MD6_160, IDC_MD6_192, IDC_MD6_224, IDC_MD6_256, IDC_MD6_384, IDC_MD6_512,
      IDC_SHA3_224, IDC_SHA3_256, IDC_SHA3_384, IDC_SHA3_512,
      IDC_KECCAK_224, IDC_KECCAK_256, IDC_KECCAK_384, IDC_KECCAK_512,
      IDC_SHAKE_128, IDC_SHAKE_256,
      IDC_TIGER, IDC_SM3, IDC_WHIRLPOOL,
      IDC_BLAKE2B, IDC_BLAKE2S,
      IDC_LSH_256, IDC_LSH_512
    };
    
    for (int algoId : algorithmIDs) {
      if (id == algoId) {
        UpdateButtonStates();
        UpdateTabNames(); // Update tab names with algorithm count
        SaveConfiguration();
        return TRUE;
      }
    }
    
    // Handle HAVAL pass selection changes
    int havalPassIDs[] = {IDC_HAVAL_PASS3, IDC_HAVAL_PASS4, IDC_HAVAL_PASS5};
    for (int passId : havalPassIDs) {
      if (id == passId) {
        UpdateButtonStates(); // Update button states when HAVAL pass selection changes
        UpdateTabNames(); // Update tab names when HAVAL pass selection changes
        SaveConfiguration();
        return TRUE;
      }
    }
  }

  return FALSE;
}

LRESULT CHashCalcDialog::OnNotify(WPARAM wparam, LPARAM lparam) {
  LPNMHDR pnmh = (LPNMHDR)lparam;

  // CTab handles tab switching automatically
  // We only need to update tab names when selection changes
  if (pnmh->hwndFrom == m_tabControl.GetHwnd() && pnmh->code == TCN_SELCHANGE) {
    UpdateTabNames();  // Just update the text, not show/hide
    return 0;
  }

  return CDialog::OnNotify(wparam, lparam);
}

INT_PTR CHashCalcDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  // Handle system tray icon messages
  if (uMsg == WM_TRAYICON) {
    if (lParam == WM_LBUTTONDOWN || lParam == WM_LBUTTONDBLCLK) {
      // Left click or double click - restore window
      RestoreFromTray();
      return TRUE;
    } else if (lParam == WM_RBUTTONDOWN) {
      // Right click - show context menu
      ShowTrayMenu();
      return TRUE;
    }
  }

  // Handle window minimize - minimize to tray instead
  if (uMsg == WM_SYSCOMMAND) {
    if (wParam == SC_MINIMIZE) {
      MinimizeToTray();
      return TRUE;
    }
  }

  // Handle Enter key in file path edit box
  if (uMsg == WM_COMMAND) {
    UINT id = LOWORD(wParam);
    UINT code = HIWORD(wParam);

    if (id == IDC_EDIT_FILE && code == EN_SETFOCUS) {
      // Store original procedure for subclassing if needed
    }
  }

  if (uMsg == WM_DROPFILES) {
    OnDropFiles((HDROP)wParam);
    return TRUE;
  }
  
  // Handle hash calculation completion
  if (uMsg == WM_HASH_COMPLETE) {
    std::wstring* pResult = reinterpret_cast<std::wstring*>(wParam);
    if (pResult) {
      SetDlgItemText(IDC_EDIT_RESULT, pResult->c_str());
      delete pResult;
    }
    
    // Re-enable controls
    EnableControls(true);
    SetDlgItemText(IDC_BUTTON_CALCULATE, L"Calculate");
    ::InvalidateRect(GetDlgItem(IDC_BUTTON_CALCULATE), NULL, TRUE);
    m_bIsCalculating = false;
    
    // Hide progress bar
    ShowProgressBar(false);
    
    // Clear taskbar progress
    SetTaskbarProgress(TBPF_NOPROGRESS);
    
    // Update button states after calculation completes
    UpdateButtonStates();
    
    if (m_hCalcThread) {
      CloseHandle(m_hCalcThread);
      m_hCalcThread = NULL;
    }
    
    return TRUE;
  }
  
  return CDialog::DialogProc(uMsg, wParam, lParam);
}

void CHashCalcDialog::UpdateTabNames() {
  const wchar_t* baseTabNames[] = {
    L"SHA && MD",
    L"SHA-3 && Modern",
    L"HAVAL && RIPEMD",
    L"Checksum && Others"
  };

  static wchar_t tabNameBuffers[4][100];

  // Get counts from each view
  for (int i = 0; i < 4; i++) {
    int count = 0;
    if (i == 0) count = m_viewSHA.CountSelectedAlgorithms();
    else if (i == 1) count = m_viewSHA3.CountSelectedAlgorithms();
    else if (i == 2) count = m_viewHAVAL.CountSelectedAlgorithms();
    else if (i == 3) count = m_viewChecksum.CountSelectedAlgorithms();

    if (count > 0) {
      swprintf_s(tabNameBuffers[i], 100, L"%s (%d)", baseTabNames[i], count);
    } else {
      wcscpy_s(tabNameBuffers[i], 100, baseTabNames[i]);
    }

    m_tabControl.SetTabText(i, tabNameBuffers[i]);
  }
}

void CHashCalcDialog::OnCancel() {
  // Handle Escape key press - close the dialog
  OnExit();
}

void CHashCalcDialog::OnExit() {
  // Remove tray icon before closing
  RemoveTrayIcon();

  // Save configuration before exiting
  SaveConfiguration();

  // Close the dialog
  EndDialog(IDOK);
}

void CHashCalcDialog::OnSelectAll() {
  // Delegate to all views
  m_viewSHA.SelectAll();
  m_viewSHA3.SelectAll();
  m_viewHAVAL.SelectAll();
  m_viewChecksum.SelectAll();

  // Update button states after selection
  UpdateButtonStates();
  // Update tab names with algorithm count
  UpdateTabNames();
  // Save configuration
  SaveConfiguration();
}

void CHashCalcDialog::OnClearAll() {
  // Delegate to all views
  m_viewSHA.ClearAll();
  m_viewSHA3.ClearAll();
  m_viewHAVAL.ClearAll();
  m_viewChecksum.ClearAll();

  // Update button states after clearing
  UpdateButtonStates();
  // Update tab names with algorithm count
  UpdateTabNames();
  // Save configuration
  SaveConfiguration();
}

void CHashCalcDialog::EnableControls(bool enable) {
  // Determine current input mode
  bool isTextMode = IsDlgButtonChecked(IDC_RADIO_TEXT) == BST_CHECKED;

  // Enable/disable input mode radio buttons
  GetDlgItem(IDC_RADIO_TEXT).EnableWindow(enable);
  GetDlgItem(IDC_RADIO_FILE).EnableWindow(enable);

  // Enable/disable input controls based on mode
  if (enable) {
    GetDlgItem(IDC_EDIT_TEXT).EnableWindow(isTextMode);
    GetDlgItem(IDC_EDIT_FILE).EnableWindow(!isTextMode);
    GetDlgItem(IDC_BUTTON_BROWSE).EnableWindow(!isTextMode);
  } else {
    GetDlgItem(IDC_EDIT_TEXT).EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT_FILE).EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_BROWSE).EnableWindow(FALSE);
  }

  // Enable/disable tab control
  m_tabControl.EnableWindow(enable);

  // Enable/disable all views
  m_viewSHA.EnableControls(enable);
  m_viewSHA3.EnableControls(enable);
  m_viewHAVAL.EnableControls(enable);
  m_viewChecksum.EnableControls(enable);

  // Enable/disable Select All and Clear All buttons
  GetDlgItem(IDC_SELECT_ALL).EnableWindow(enable);
  GetDlgItem(IDC_CLEAR_ALL).EnableWindow(enable);

  // Enable/disable Exit and About buttons
  GetDlgItem(IDC_BUTTON_EXIT).EnableWindow(enable);
  GetDlgItem(IDC_BUTTON_ABOUT).EnableWindow(enable);

  // Stay on Top checkbox always enabled (as requested)
  // IDC_CHECK_STAY_ON_TOP and IDC_BUTTON_CALCULATE are always enabled
}

void CHashCalcDialog::DisableControlsForCalculation() {
  EnableControls(false);
}

DWORD WINAPI CHashCalcDialog::CalculateHashThread(LPVOID lpParam) {
  CHashCalcDialog* pDlg = reinterpret_cast<CHashCalcDialog*>(lpParam);
  pDlg->PerformHashCalculation();
  return 0;
}

void CHashCalcDialog::PerformHashCalculation() {
  // Determine input source
  if (IsDlgButtonChecked(IDC_RADIO_TEXT)) {
    // Get text from edit control
    CString wText = GetDlgItemText(IDC_EDIT_TEXT);

    // Convert to UTF-8 for hashing
    std::wstring wstr = wText.GetString();
    int size_needed =
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string inputData(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &inputData[0], size_needed,
                        NULL, NULL);

    std::wstringstream output;
    auto start_time = std::chrono::high_resolution_clock::now();

    output << L"Calculating hash of " << inputData.size() << L" bytes string `" << wstr << L"`...\r\n\r\n";
    
    bool anyComputed = false;

    try {
      // Use the refactored method for text mode
      ComputeHashAlgorithmsForText(output, anyComputed, inputData);

      auto end_time = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed = end_time - start_time;

      // Check if cancelled
      if (m_bCancelCalculation.load()) {
        output << L"\r\n\r\nCalculation cancelled by user.";
      } else {
        output << L"\r\nCalculation took " << std::fixed << std::setprecision(3) << elapsed.count() << L" seconds";
      }

    } catch (const std::exception &e) {
      // Cancellation or other error occurred
      std::string errorMsg = e.what();
      if (errorMsg.find("cancelled") != std::string::npos) {
        output << L"\r\n\r\nCalculation cancelled by user.";
      } else {
        std::wstring wError(errorMsg.begin(), errorMsg.end());
        output << L"\r\n\r\nError: " << wError;
      }
    }

    std::wstring* pFinalOutput = new std::wstring();
    if (!anyComputed) {
      *pFinalOutput = L"Please select at least one hash algorithm.";
    } else {
      *pFinalOutput = output.str();
    }
    
    // Send result to UI thread
    PostMessage(WM_HASH_COMPLETE, reinterpret_cast<WPARAM>(pFinalOutput), 0);

  } else {
    // File mode - get file path
    CString wFilePath = GetDlgItemText(IDC_EDIT_FILE);
    std::wstring filePath = wFilePath.GetString();

    // Get file size
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    LARGE_INTEGER fileSize = {0};
    if (hFile != INVALID_HANDLE_VALUE) {
      GetFileSizeEx(hFile, &fileSize);
      CloseHandle(hFile);
    }

    std::wstringstream output;
    auto start_time = std::chrono::high_resolution_clock::now();

    // Extract just the filename from full path
    std::wstring fileName = filePath;
    size_t lastSlash = filePath.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
      fileName = filePath.substr(lastSlash + 1);
    }

    output << L"Calculating hash of file: " << fileName << L"\r\n";
    output << L"File size: " << fileSize.QuadPart << L" bytes\r\n\r\n";
    
    bool anyComputed = false;

    try {
      // Use the refactored method for file mode
      ComputeHashAlgorithmsForFile(output, anyComputed, filePath);

      auto end_time = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed = end_time - start_time;

      // Check if cancelled
      if (m_bCancelCalculation.load()) {
        output << L"\r\n\r\nCalculation cancelled by user.";
      } else {
        output << L"\r\nCalculation took " << std::fixed << std::setprecision(3) << elapsed.count() << L" seconds";
      }

    } catch (const std::exception &e) {
      // Cancellation or other error occurred
      std::string errorMsg = e.what();
      if (errorMsg.find("cancelled") != std::string::npos) {
        output << L"\r\n\r\nCalculation cancelled by user.";
      } else {
        std::wstring wError(errorMsg.begin(), errorMsg.end());
        output << L"\r\n\r\nError: " << wError;
      }
    }

    std::wstring* pFinalOutput = new std::wstring();
    if (!anyComputed) {
      *pFinalOutput = L"Please select at least one hash algorithm.";
    } else {
      *pFinalOutput = output.str();
    }
    
    // Send result to UI thread
    PostMessage(WM_HASH_COMPLETE, reinterpret_cast<WPARAM>(pFinalOutput), 0);
  }
}

void CHashCalcDialog::OnCalculate() {
  if (m_bIsCalculating) {
    // Stop the calculation
    m_bCancelCalculation.store(true);
    SetDlgItemText(IDC_EDIT_RESULT, L"Stopping calculation...");
    return;
  }
  
  // Validate input before starting calculation
  if (IsDlgButtonChecked(IDC_RADIO_TEXT)) {
    // Check if text input is empty
    CString wText = GetDlgItemText(IDC_EDIT_TEXT);
    if (wText.IsEmpty()) {
      SetDlgItemText(IDC_EDIT_RESULT, L"Please enter text to hash.");
      return;
    }
  } else {
    // Check if file path is empty
    CString wFilePath = GetDlgItemText(IDC_EDIT_FILE);
    if (wFilePath.IsEmpty()) {
      SetDlgItemText(IDC_EDIT_RESULT, L"Please enter or select a file to hash.");
      return;
    }
    
    // Validate file path
    std::wstring filePath = wFilePath.GetString();
    std::wstring errorMsg;
    if (!ValidateFilePath(filePath, errorMsg)) {
      SetDlgItemText(IDC_EDIT_RESULT, errorMsg.c_str());
      return;
    }
  }

  // Check if at least one algorithm is selected
  bool anySelected = HasAnyAlgorithmSelected();

  if (!anySelected) {
    SetDlgItemText(IDC_EDIT_RESULT, L"Please select at least one hash algorithm.");
    return;
  }
  
  // Start calculation
  m_bIsCalculating = true;
  m_bCancelCalculation.store(false);
  
  // Disable controls except Stay on Top and Calculate button
  DisableControlsForCalculation();
  
  // Change button text to "Stop"
  SetDlgItemText(IDC_BUTTON_CALCULATE, L"Stop");
  ::InvalidateRect(GetDlgItem(IDC_BUTTON_CALCULATE), NULL, TRUE);
  
  // Show progress bar
  ShowProgressBar(true);
  
  // Set taskbar progress to indeterminate state
  SetTaskbarProgress(TBPF_INDETERMINATE);
  
  // Create calculation thread
  m_hCalcThread = CreateThread(
    NULL,
    0,
    CalculateHashThread,
    this,
    0,
    NULL
  );
  
  if (m_hCalcThread == NULL) {
    // Failed to create thread
    MessageBox(L"Failed to create calculation thread.", L"Error", MB_OK | MB_ICONERROR);
    EnableControls(true);
    SetDlgItemText(IDC_BUTTON_CALCULATE, L"Calculate");
    ::InvalidateRect(GetDlgItem(IDC_BUTTON_CALCULATE), NULL, TRUE);
    m_bIsCalculating = false;
    ShowProgressBar(false);
    SetTaskbarProgress(TBPF_NOPROGRESS);
  }
}

void CHashCalcDialog::OnAbout() {
  MessageBox(L"Hash Calculator\nVersion 1.0\n\nA simple hash calculator for "
             L"multiple algorithms.",
             L"About", MB_OK | MB_ICONINFORMATION);
}

void CHashCalcDialog::OnBrowse() {
  OPENFILENAME ofn = {0};
  WCHAR szFile[MAX_PATH] = {0};

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = *this;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrFilter = L"All Files\0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (GetOpenFileName(&ofn) == TRUE) {
    SetDlgItemText(IDC_EDIT_FILE, szFile);
    // Automatically calculate hash after file selection
    OnCalculate();
  }
}

void CHashCalcDialog::OnStayOnTop() {
  BOOL checked = IsDlgButtonChecked(IDC_CHECK_STAY_ON_TOP);
  HWND hwnd = *this;

  if (checked) {
    ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  } else {
    ::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }
}

void CHashCalcDialog::OnDropFiles(HDROP hDrop) {
  WCHAR szFile[MAX_PATH] = {0};
  
  // Get the first dropped file
  UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
  if (fileCount > 0) {
    // Get the file path of the first file
    if (DragQueryFile(hDrop, 0, szFile, MAX_PATH) > 0) {
      // Switch to file mode
      CheckRadioButton(IDC_RADIO_TEXT, IDC_RADIO_FILE, IDC_RADIO_FILE);
      GetDlgItem(IDC_EDIT_TEXT).EnableWindow(FALSE);
      GetDlgItem(IDC_EDIT_FILE).EnableWindow(TRUE);
      GetDlgItem(IDC_BUTTON_BROWSE).EnableWindow(TRUE);
      
      // Set the file path
      SetDlgItemText(IDC_EDIT_FILE, szFile);
      
      // Automatically calculate hash
      OnCalculate();
    }
  }
  
  // Release the drop handle
  DragFinish(hDrop);
}

void CHashCalcDialog::ShowProgressBar(bool show) {
  HWND hProgress = GetDlgItem(IDC_PROGRESS_CALC);
  if (show) {
    ::ShowWindow(hProgress, SW_SHOW);
    ::SendMessage(hProgress, PBM_SETMARQUEE, TRUE, 30); // Start marquee animation
  } else {
    ::SendMessage(hProgress, PBM_SETMARQUEE, FALSE, 0); // Stop marquee animation
    ::ShowWindow(hProgress, SW_HIDE);
  }
}

void CHashCalcDialog::SetTaskbarProgress(TBPFLAG state) {
  if (m_pTaskbarList) {
    m_pTaskbarList->SetProgressState(*this, state);
  }
}

void CHashCalcDialog::OnFilePathEnter() {
  // Triggered when user presses Enter in file path edit box
  if (IsDlgButtonChecked(IDC_RADIO_FILE)) {
    // Automatically trigger calculation
    OnCalculate();
  }
}

bool CHashCalcDialog::ValidateFilePath(const std::wstring& filePath, std::wstring& errorMsg) {
  // Trim leading and trailing whitespace
  std::wstring trimmedPath = filePath;
  size_t start = trimmedPath.find_first_not_of(L" \t\r\n");
  size_t end = trimmedPath.find_last_not_of(L" \t\r\n");
  
  if (start == std::wstring::npos) {
    errorMsg = L"Error: File path is empty or contains only whitespace.";
    return false;
  }
  
  trimmedPath = trimmedPath.substr(start, end - start + 1);
  
  // Remove surrounding quotes if present
  if (trimmedPath.length() >= 2 && 
      ((trimmedPath.front() == L'\"' && trimmedPath.back() == L'\"') ||
       (trimmedPath.front() == L'\'' && trimmedPath.back() == L'\''))) {
    trimmedPath = trimmedPath.substr(1, trimmedPath.length() - 2);
  }
  
  // Update the edit box with trimmed path if different
  if (trimmedPath != filePath) {
    SetDlgItemText(IDC_EDIT_FILE, trimmedPath.c_str());
  }
  
  // Check if path is too long
  if (trimmedPath.length() >= MAX_PATH) {
    errorMsg = L"Error: File path is too long (max " + std::to_wstring(MAX_PATH) + L" characters).";
    return false;
  }
  
  // Check for invalid characters in path
  const std::wstring invalidChars = L"<>|";
  for (wchar_t ch : invalidChars) {
    if (trimmedPath.find(ch) != std::wstring::npos) {
      errorMsg = L"Error: File path contains invalid character: ";
      errorMsg += ch;
      return false;
    }
  }
  
  // Check if file exists
  DWORD dwAttrib = GetFileAttributes(trimmedPath.c_str());
  if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
    DWORD error = GetLastError();
    switch (error) {
      case ERROR_FILE_NOT_FOUND:
        errorMsg = L"Error: File not found.\nPath: " + trimmedPath;
        break;
      case ERROR_PATH_NOT_FOUND:
        errorMsg = L"Error: Path not found. Please check if the directory exists.\nPath: " + trimmedPath;
        break;
      case ERROR_ACCESS_DENIED:
        errorMsg = L"Error: Access denied. Please check file permissions.\nPath: " + trimmedPath;
        break;
      case ERROR_INVALID_NAME:
        errorMsg = L"Error: Invalid file name or path.\nPath: " + trimmedPath;
        break;
      default:
        errorMsg = L"Error: Cannot access file (Error code: " + std::to_wstring(error) + L").\nPath: " + trimmedPath;
        break;
    }
    return false;
  }
  
  // Check if it's a directory
  if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {
    errorMsg = L"Error: The path points to a directory, not a file.\nPath: " + trimmedPath;
    return false;
  }
  
  // Check if file is readable
  HANDLE hFile = CreateFile(trimmedPath.c_str(), GENERIC_READ, 
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    if (error == ERROR_SHARING_VIOLATION) {
      errorMsg = L"Warning: File is currently in use by another program.\nPath: " + trimmedPath + 
                 L"\n\nAttempting to read anyway...";
      // Continue anyway - sometimes we can still read files that are open
    } else {
      errorMsg = L"Error: Cannot open file for reading (Error code: " + std::to_wstring(error) + L").\nPath: " + trimmedPath;
      return false;
    }
  } else {
    CloseHandle(hFile);
  }
  
  return true;
}

bool CHashCalcDialog::HasAnyAlgorithmSelected() {
  // Check if views are created
  if (!m_viewSHA.GetHwnd() || !m_viewSHA3.GetHwnd() ||
      !m_viewHAVAL.GetHwnd() || !m_viewChecksum.GetHwnd()) {
    return false;
  }

  // Check if any view has selected algorithms
  if (m_viewSHA.CountSelectedAlgorithms() > 0) return true;
  if (m_viewSHA3.CountSelectedAlgorithms() > 0) return true;
  if (m_viewHAVAL.CountSelectedAlgorithms() > 0) return true;
  if (m_viewChecksum.CountSelectedAlgorithms() > 0) return true;

  // Check HAVAL passes
  bool pass3, pass4, pass5;
  m_viewHAVAL.GetHavalPassStates(pass3, pass4, pass5);
  if (pass3 || pass4 || pass5) return true;

  return false;
}

bool CHashCalcDialog::HasAllAlgorithmsSelected() {
  // Collect all algorithm states from all views
  std::map<int, bool> allStates;
  auto states1 = m_viewSHA.GetAlgorithmStates();
  auto states2 = m_viewSHA3.GetAlgorithmStates();
  auto states3 = m_viewHAVAL.GetAlgorithmStates();
  auto states4 = m_viewChecksum.GetAlgorithmStates();

  allStates.insert(states1.begin(), states1.end());
  allStates.insert(states2.begin(), states2.end());
  allStates.insert(states3.begin(), states3.end());
  allStates.insert(states4.begin(), states4.end());

  // Check if all algorithms are selected
  for (const auto& pair : allStates) {
    if (!pair.second) {
      return false;
    }
  }

  // Check HAVAL passes
  bool pass3, pass4, pass5;
  m_viewHAVAL.GetHavalPassStates(pass3, pass4, pass5);
  if (!pass3 || !pass4 || !pass5) {
    return false;
  }

  return true;
}

bool CHashCalcDialog::HasValidInput() {
  if (IsDlgButtonChecked(IDC_RADIO_TEXT)) {
    // Check if text input is not empty
    CString wText = GetDlgItemText(IDC_EDIT_TEXT);
    return !wText.IsEmpty();
  } else {
    // Check if file path is not empty
    CString wFilePath = GetDlgItemText(IDC_EDIT_FILE);
    return !wFilePath.IsEmpty();
  }
}

void CHashCalcDialog::UpdateButtonStates() {
  // Don't update button states while calculating
  if (m_bIsCalculating) {
    return;
  }
  
  bool hasAnyAlgorithm = HasAnyAlgorithmSelected();
  bool hasAllAlgorithms = HasAllAlgorithmsSelected();
  bool hasValidInput = HasValidInput();
  
  // Rule 1: Clear All button - disabled when no algorithm is selected
  GetDlgItem(IDC_CLEAR_ALL).EnableWindow(hasAnyAlgorithm);
  
  // Rule 2: Select All button - disabled when all algorithms are selected
  GetDlgItem(IDC_SELECT_ALL).EnableWindow(!hasAllAlgorithms);
  
  // Rule 3: Calculate button - disabled when no input, or no algorithm selected
  GetDlgItem(IDC_BUTTON_CALCULATE).EnableWindow(hasValidInput && hasAnyAlgorithm);
}

void CHashCalcDialog::LoadConfiguration() {
  // Initialize configuration manager
  m_configManager.Initialize();

  // Load configuration from file
  m_configManager.LoadConfig();

  // Apply input mode (Text or File)
  bool isFileMode = m_configManager.GetInputMode();
  if (isFileMode) {
    CheckRadioButton(IDC_RADIO_TEXT, IDC_RADIO_FILE, IDC_RADIO_FILE);
    GetDlgItem(IDC_EDIT_TEXT).EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT_FILE).EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_BROWSE).EnableWindow(TRUE);
  } else {
    CheckRadioButton(IDC_RADIO_TEXT, IDC_RADIO_FILE, IDC_RADIO_TEXT);
    GetDlgItem(IDC_EDIT_TEXT).EnableWindow(TRUE);
    GetDlgItem(IDC_EDIT_FILE).EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_BROWSE).EnableWindow(FALSE);
  }

  // Apply Stay on Top setting
  bool stayOnTop = m_configManager.GetStayOnTop();
  CheckDlgButton(IDC_CHECK_STAY_ON_TOP, stayOnTop ? BST_CHECKED : BST_UNCHECKED);
  if (stayOnTop) {
    HWND hwnd = *this;
    ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }

  // Apply algorithm selections to views
  auto algorithms = m_configManager.GetAllAlgorithms();
  m_viewSHA.SetAlgorithmStates(algorithms);
  m_viewSHA3.SetAlgorithmStates(algorithms);
  m_viewHAVAL.SetAlgorithmStates(algorithms);
  m_viewChecksum.SetAlgorithmStates(algorithms);

  // Apply HAVAL pass selections (can be multiple)
  m_viewHAVAL.SetHavalPassStates(
    m_configManager.GetHavalPass3(),
    m_configManager.GetHavalPass4(),
    m_configManager.GetHavalPass5()
  );

  // Update tab display with loaded configuration
  UpdateTabNames();
}

void CHashCalcDialog::SaveConfiguration() {
  // Save input mode
  bool isFileMode = IsDlgButtonChecked(IDC_RADIO_FILE) == BST_CHECKED;
  m_configManager.SetInputMode(isFileMode);

  // Save Stay on Top setting
  bool stayOnTop = IsDlgButtonChecked(IDC_CHECK_STAY_ON_TOP) == BST_CHECKED;
  m_configManager.SetStayOnTop(stayOnTop);

  // Collect algorithm states from all views
  std::map<int, bool> allStates;
  auto states1 = m_viewSHA.GetAlgorithmStates();
  auto states2 = m_viewSHA3.GetAlgorithmStates();
  auto states3 = m_viewHAVAL.GetAlgorithmStates();
  auto states4 = m_viewChecksum.GetAlgorithmStates();

  allStates.insert(states1.begin(), states1.end());
  allStates.insert(states2.begin(), states2.end());
  allStates.insert(states3.begin(), states3.end());
  allStates.insert(states4.begin(), states4.end());

  // Save algorithm selections
  for (const auto& pair : allStates) {
    m_configManager.SetAlgorithmEnabled(pair.first, pair.second);
  }

  // Save HAVAL pass selections (can be multiple)
  bool pass3, pass4, pass5;
  m_viewHAVAL.GetHavalPassStates(pass3, pass4, pass5);
  m_configManager.SetHavalPass3(pass3);
  m_configManager.SetHavalPass4(pass4);
  m_configManager.SetHavalPass5(pass5);

  // Write to INI file
  m_configManager.SaveConfig();
}

// ============================================================================
// Refactored Helper Methods
// ============================================================================

void CHashCalcDialog::SetCheckboxStates(const int* ids, size_t count, bool checked) {
  int state = checked ? BST_CHECKED : BST_UNCHECKED;
  for (size_t i = 0; i < count; ++i) {
    CheckDlgButton(ids[i], state);
  }
}

void CHashCalcDialog::EnableControlsById(const int* ids, size_t count, bool enable) {
  for (size_t i = 0; i < count; ++i) {
    GetDlgItem(ids[i]).EnableWindow(enable);
  }
}

bool CHashCalcDialog::IsAlgorithmSelected(int algorithmId) {
  // Collect all algorithm states from all views
  std::map<int, bool> allStates;
  auto states1 = m_viewSHA.GetAlgorithmStates();
  auto states2 = m_viewSHA3.GetAlgorithmStates();
  auto states3 = m_viewHAVAL.GetAlgorithmStates();
  auto states4 = m_viewChecksum.GetAlgorithmStates();

  allStates.insert(states1.begin(), states1.end());
  allStates.insert(states2.begin(), states2.end());
  allStates.insert(states3.begin(), states3.end());
  allStates.insert(states4.begin(), states4.end());

  auto it = allStates.find(algorithmId);
  return (it != allStates.end() && it->second);
}

void CHashCalcDialog::ComputeHashAlgorithmsForText(
    std::wstringstream& output, 
    bool& anyComputed,
    const std::string& inputData) {
  
  // Helper to compute a specific algorithm by name
  auto computeAlgo = [&](const std::string &algoName, const std::string &displayName) {
    // Check for cancellation
    if (m_bCancelCalculation.load()) {
      return;
    }
    
    try {
      if (core::HashAlgorithmFactory::isAvailable(algoName)) {
        auto algo = core::HashAlgorithmFactory::create(algoName);
        // Set cancel callback
        algo->setCancelCallback([this]() { return m_bCancelCalculation.load(); });
        auto digest = algo->computeString(inputData);
        
        // Format: Algorithm Name (padded) : Hash Value
        std::wstring wDisplayName(displayName.begin(), displayName.end());
        
        output << std::left << std::setw(12) << wDisplayName << L": " 
               << core::IHashAlgorithm::toHexWString(digest, true) << L"\r\n";
        anyComputed = true;
      } else {
        // For unavailable algorithms, show "Not Available"
        std::wstring wDisplayName(displayName.begin(), displayName.end());
        output << std::left << std::setw(12) << wDisplayName << L": Not Available\r\n";
      }
    } catch (const std::exception &e) {
      std::string errorMsg = e.what();
      // Check if this is a cancellation
      if (errorMsg.find("cancelled") != std::string::npos) {
        throw; // Re-throw to stop all calculations
      }
      std::wstring wDisplayName(displayName.begin(), displayName.end());
      std::string error = e.what();
      std::wstring wError(error.begin(), error.end());
      output << std::left << std::setw(12) << wDisplayName << L": Error";
      if (!wError.empty()) {
        output << L" - " << wError;
      }
      output << L"\r\n";
    }
  };

  // Helper to check ID and compute
  auto checkAndCompute = [&](int id, const std::string &algoName, const std::string& displayName) {
    if (IsAlgorithmSelected(id)) {
      computeAlgo(algoName, displayName);
    }
  };

  // ========== Tab 1: SHA && MD (left to right order as shown in UI) ==========
  // SHA Family
  checkAndCompute(IDC_SHA_160, "SHA-1", "SHA-160");
  checkAndCompute(IDC_SHA_224, "SHA-224", "SHA-224");
  checkAndCompute(IDC_SHA_256, "SHA-256", "SHA-256");
  checkAndCompute(IDC_SHA_384, "SHA-384", "SHA-384");
  checkAndCompute(IDC_SHA_512, "SHA-512", "SHA-512");

  // MD Family
  checkAndCompute(IDC_MD2, "MD2", "MD2");
  checkAndCompute(IDC_MD4, "MD4", "MD4");
  checkAndCompute(IDC_MD5, "MD5", "MD5");

  // MD6 Family
  checkAndCompute(IDC_MD6_128, "MD6-128", "MD6-128");
  checkAndCompute(IDC_MD6_160, "MD6-160", "MD6-160");
  checkAndCompute(IDC_MD6_192, "MD6-192", "MD6-192");
  checkAndCompute(IDC_MD6_224, "MD6-224", "MD6-224");
  checkAndCompute(IDC_MD6_256, "MD6-256", "MD6-256");
  checkAndCompute(IDC_MD6_384, "MD6-384", "MD6-384");
  checkAndCompute(IDC_MD6_512, "MD6-512", "MD6-512");

  // ========== Tab 2: SHA-3 && Modern (left to right order as shown in UI) ==========
  // SHA-3
  checkAndCompute(IDC_SHA3_224, "SHA3-224", "SHA3-224");
  checkAndCompute(IDC_SHA3_256, "SHA3-256", "SHA3-256");
  checkAndCompute(IDC_SHA3_384, "SHA3-384", "SHA3-384");
  checkAndCompute(IDC_SHA3_512, "SHA3-512", "SHA3-512");

  // Keccak
  checkAndCompute(IDC_KECCAK_224, "Keccak-224", "Keccak-224");
  checkAndCompute(IDC_KECCAK_256, "Keccak-256", "Keccak-256");
  checkAndCompute(IDC_KECCAK_384, "Keccak-384", "Keccak-384");
  checkAndCompute(IDC_KECCAK_512, "Keccak-512", "Keccak-512");

  // SHAKE
  checkAndCompute(IDC_SHAKE_128, "SHAKE128", "SHAKE-128");
  checkAndCompute(IDC_SHAKE_256, "SHAKE256", "SHAKE-256");

  // BLAKE2
  checkAndCompute(IDC_BLAKE2B, "BLAKE2b", "BLAKE2b");
  checkAndCompute(IDC_BLAKE2S, "BLAKE2s", "BLAKE2s");

  // Asian (LSH and SM3)
  checkAndCompute(IDC_LSH_256, "LSH-256", "LSH-256");
  checkAndCompute(IDC_LSH_512, "LSH-512", "LSH-512");
  checkAndCompute(IDC_SM3, "SM3", "SM3");

  // ========== Tab 3: HAVAL && RIPEMD (left to right order as shown in UI) ==========
  // HAVAL Family
  // Compute for each selected pass count
  auto checkAndComputeHaval = [&](int id, int bits, int passes) {
    if (IsDlgButtonChecked(id)) {
      std::stringstream algoName, displayName;
      algoName << "HAVAL-" << bits << "/Pass" << passes;
      displayName << "HAVAL-" << bits << "/" << passes;
      computeAlgo(algoName.str(), displayName.str());
    }
  };

  // Check which passes are selected and compute for each
  std::vector<int> selectedPasses;
  if (IsDlgButtonChecked(IDC_HAVAL_PASS3)) selectedPasses.push_back(3);
  if (IsDlgButtonChecked(IDC_HAVAL_PASS4)) selectedPasses.push_back(4);
  if (IsDlgButtonChecked(IDC_HAVAL_PASS5)) selectedPasses.push_back(5);
  
  // If no pass is selected, default to 3-pass
  if (selectedPasses.empty()) {
    selectedPasses.push_back(3);
  }

  // Compute HAVAL for each selected bit size and pass combination
  for (int passes : selectedPasses) {
    checkAndComputeHaval(IDC_HAVAL_128, 128, passes);
    checkAndComputeHaval(IDC_HAVAL_160, 160, passes);
    checkAndComputeHaval(IDC_HAVAL_192, 192, passes);
    checkAndComputeHaval(IDC_HAVAL_224, 224, passes);
    checkAndComputeHaval(IDC_HAVAL_256, 256, passes);
  }

  // RIPEMD
  checkAndCompute(IDC_RIPEMD_128, "RIPEMD-128", "RIPEMD-128");
  checkAndCompute(IDC_RIPEMD_160, "RIPEMD-160", "RIPEMD-160");
  checkAndCompute(IDC_RIPEMD_256, "RIPEMD-256", "RIPEMD-256");
  checkAndCompute(IDC_RIPEMD_320, "RIPEMD-320", "RIPEMD-320");

  // ========== Tab 4: Checksum && Others (left to right order as shown in UI) ==========
  // Checksum
  checkAndCompute(IDC_CRC8, "CRC-8", "CRC-8");
  checkAndCompute(IDC_CRC16, "CRC-16", "CRC-16");
  checkAndCompute(IDC_CRC32, "CRC32", "CRC-32");
  checkAndCompute(IDC_CRC32C, "CRC-32C", "CRC-32C");
  checkAndCompute(IDC_CRC64, "CRC-64", "CRC-64");
  checkAndCompute(IDC_ADLER32, "Adler32", "Adler-32");

  // Others
  checkAndCompute(IDC_TIGER, "Tiger", "Tiger");
  checkAndCompute(IDC_WHIRLPOOL, "Whirlpool", "Whirlpool");
}

void CHashCalcDialog::ComputeHashAlgorithmsForFile(
    std::wstringstream& output, 
    bool& anyComputed,
    const std::wstring& filePath) {
  
  // Helper to compute a specific algorithm by name for file
  auto computeAlgo = [&](const std::string &algoName, const std::string &displayName) {
    // Check for cancellation
    if (m_bCancelCalculation.load()) {
      return;
    }
    
    try {
      if (core::HashAlgorithmFactory::isAvailable(algoName)) {
        auto algo = core::HashAlgorithmFactory::create(algoName);
        // Set cancel callback
        algo->setCancelCallback([this]() { return m_bCancelCalculation.load(); });
        auto digest = algo->computeFile(filePath);
        
        // Format: Algorithm Name (padded) : Hash Value
        std::wstring wDisplayName(displayName.begin(), displayName.end());
        
        output << std::left << std::setw(12) << wDisplayName << L": " 
               << core::IHashAlgorithm::toHexWString(digest, true) << L"\r\n";
        anyComputed = true;
      } else {
        // For unavailable algorithms, show "Not Available"
        std::wstring wDisplayName(displayName.begin(), displayName.end());
        output << std::left << std::setw(12) << wDisplayName << L": Not Available\r\n";
      }
    } catch (const std::exception &e) {
      std::string errorMsg = e.what();
      // Check if this is a cancellation
      if (errorMsg.find("cancelled") != std::string::npos) {
        throw; // Re-throw to stop all calculations
      }
      std::wstring wDisplayName(displayName.begin(), displayName.end());
      std::string error = e.what();
      std::wstring wError(error.begin(), error.end());
      output << std::left << std::setw(12) << wDisplayName << L": Error";
      if (!wError.empty()) {
        output << L" - " << wError;
      }
      output << L"\r\n";
    }
  };

  // Helper to check ID and compute
  auto checkAndCompute = [&](int id, const std::string &algoName, const std::string& displayName) {
    if (IsAlgorithmSelected(id)) {
      computeAlgo(algoName, displayName);
    }
  };

  // ========== Tab 1: SHA && MD (left to right order as shown in UI) ==========
  // SHA Family
  checkAndCompute(IDC_SHA_160, "SHA-1", "SHA-160");
  checkAndCompute(IDC_SHA_224, "SHA-224", "SHA-224");
  checkAndCompute(IDC_SHA_256, "SHA-256", "SHA-256");
  checkAndCompute(IDC_SHA_384, "SHA-384", "SHA-384");
  checkAndCompute(IDC_SHA_512, "SHA-512", "SHA-512");

  // MD Family
  checkAndCompute(IDC_MD2, "MD2", "MD2");
  checkAndCompute(IDC_MD4, "MD4", "MD4");
  checkAndCompute(IDC_MD5, "MD5", "MD5");

  // MD6 Family
  checkAndCompute(IDC_MD6_128, "MD6-128", "MD6-128");
  checkAndCompute(IDC_MD6_160, "MD6-160", "MD6-160");
  checkAndCompute(IDC_MD6_192, "MD6-192", "MD6-192");
  checkAndCompute(IDC_MD6_224, "MD6-224", "MD6-224");
  checkAndCompute(IDC_MD6_256, "MD6-256", "MD6-256");
  checkAndCompute(IDC_MD6_384, "MD6-384", "MD6-384");
  checkAndCompute(IDC_MD6_512, "MD6-512", "MD6-512");

  // ========== Tab 2: SHA-3 && Modern (left to right order as shown in UI) ==========
  // SHA-3
  checkAndCompute(IDC_SHA3_224, "SHA3-224", "SHA3-224");
  checkAndCompute(IDC_SHA3_256, "SHA3-256", "SHA3-256");
  checkAndCompute(IDC_SHA3_384, "SHA3-384", "SHA3-384");
  checkAndCompute(IDC_SHA3_512, "SHA3-512", "SHA3-512");

  // Keccak
  checkAndCompute(IDC_KECCAK_224, "Keccak-224", "Keccak-224");
  checkAndCompute(IDC_KECCAK_256, "Keccak-256", "Keccak-256");
  checkAndCompute(IDC_KECCAK_384, "Keccak-384", "Keccak-384");
  checkAndCompute(IDC_KECCAK_512, "Keccak-512", "Keccak-512");

  // SHAKE
  checkAndCompute(IDC_SHAKE_128, "SHAKE128", "SHAKE-128");
  checkAndCompute(IDC_SHAKE_256, "SHAKE256", "SHAKE-256");

  // BLAKE2
  checkAndCompute(IDC_BLAKE2B, "BLAKE2b", "BLAKE2b");
  checkAndCompute(IDC_BLAKE2S, "BLAKE2s", "BLAKE2s");

  // Asian (LSH and SM3)
  checkAndCompute(IDC_LSH_256, "LSH-256", "LSH-256");
  checkAndCompute(IDC_LSH_512, "LSH-512", "LSH-512");
  checkAndCompute(IDC_SM3, "SM3", "SM3");

  // ========== Tab 3: HAVAL && RIPEMD (left to right order as shown in UI) ==========
  // HAVAL Family
  // Compute for each selected pass count
  auto checkAndComputeHaval = [&](int id, int bits, int passes) {
    if (IsDlgButtonChecked(id)) {
      std::stringstream algoName, displayName;
      algoName << "HAVAL-" << bits << "/Pass" << passes;
      displayName << "HAVAL-" << bits << "/" << passes;
      computeAlgo(algoName.str(), displayName.str());
    }
  };

  // Check which passes are selected and compute for each
  std::vector<int> selectedPasses;
  if (IsDlgButtonChecked(IDC_HAVAL_PASS3)) selectedPasses.push_back(3);
  if (IsDlgButtonChecked(IDC_HAVAL_PASS4)) selectedPasses.push_back(4);
  if (IsDlgButtonChecked(IDC_HAVAL_PASS5)) selectedPasses.push_back(5);
  
  // If no pass is selected, default to 3-pass
  if (selectedPasses.empty()) {
    selectedPasses.push_back(3);
  }

  // Compute HAVAL for each selected bit size and pass combination
  for (int passes : selectedPasses) {
    checkAndComputeHaval(IDC_HAVAL_128, 128, passes);
    checkAndComputeHaval(IDC_HAVAL_160, 160, passes);
    checkAndComputeHaval(IDC_HAVAL_192, 192, passes);
    checkAndComputeHaval(IDC_HAVAL_224, 224, passes);
    checkAndComputeHaval(IDC_HAVAL_256, 256, passes);
  }

  // RIPEMD
  checkAndCompute(IDC_RIPEMD_128, "RIPEMD-128", "RIPEMD-128");
  checkAndCompute(IDC_RIPEMD_160, "RIPEMD-160", "RIPEMD-160");
  checkAndCompute(IDC_RIPEMD_256, "RIPEMD-256", "RIPEMD-256");
  checkAndCompute(IDC_RIPEMD_320, "RIPEMD-320", "RIPEMD-320");

  // ========== Tab 4: Checksum && Others (left to right order as shown in UI) ==========
  // Checksum
  checkAndCompute(IDC_CRC8, "CRC-8", "CRC-8");
  checkAndCompute(IDC_CRC16, "CRC-16", "CRC-16");
  checkAndCompute(IDC_CRC32, "CRC32", "CRC-32");
  checkAndCompute(IDC_CRC32C, "CRC-32C", "CRC-32C");
  checkAndCompute(IDC_CRC64, "CRC-64", "CRC-64");
  checkAndCompute(IDC_ADLER32, "Adler32", "Adler-32");

  // Others
  checkAndCompute(IDC_TIGER, "Tiger", "Tiger");
  checkAndCompute(IDC_WHIRLPOOL, "Whirlpool", "Whirlpool");
}

// ============================================================================
// System Tray Functions
// ============================================================================

void CHashCalcDialog::CreateTrayIcon() {
  if (m_bTrayIconCreated) {
    return; // Already created
  }

  // Initialize NOTIFYICONDATA structure
  m_nid.cbSize = sizeof(NOTIFYICONDATA);
  m_nid.hWnd = *this;
  m_nid.uID = 1;
  m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  m_nid.uCallbackMessage = WM_TRAYICON;

  // Load the application icon
  m_nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));

  // Set tooltip text
  wcscpy_s(m_nid.szTip, L"Hash Calculator");

  // Add the icon to the system tray (but window is still visible)
  Shell_NotifyIcon(NIM_ADD, &m_nid);
  m_bTrayIconCreated = true;
}

void CHashCalcDialog::RemoveTrayIcon() {
  if (m_bTrayIconCreated) {
    Shell_NotifyIcon(NIM_DELETE, &m_nid);
    m_bTrayIconCreated = false;
  }
}

void CHashCalcDialog::ShowTrayMenu() {
  // Get cursor position for menu
  POINT pt;
  GetCursorPos(&pt);

  // Create popup menu
  HMENU hMenu = CreatePopupMenu();
  if (!hMenu) return;

  // Add menu items
  AppendMenu(hMenu, MF_STRING, IDM_TRAY_RESTORE, L"Restore");
  AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

  // Add Stop menu item - enabled only when calculating
  UINT stopFlags = MF_STRING;
  if (!m_bIsCalculating) {
    stopFlags |= MF_GRAYED;
  }
  AppendMenu(hMenu, stopFlags, IDM_TRAY_STOP, L"Stop");

  AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(hMenu, MF_STRING, IDM_TRAY_EXIT, L"Exit");

  // Required for popup menus to work correctly
  SetForegroundWindow();

  // Show the menu
  TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, *this, NULL);

  // Clean up
  DestroyMenu(hMenu);
}

void CHashCalcDialog::RestoreFromTray() {
  // Show the window
  ShowWindow(SW_SHOW);
  ShowWindow(SW_RESTORE);
  SetForegroundWindow();
  SetFocus();
}

void CHashCalcDialog::MinimizeToTray() {
  // Hide the window
  ShowWindow(SW_HIDE);
}
