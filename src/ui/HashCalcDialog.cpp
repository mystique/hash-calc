#include "HashCalcDialog.h"
#include "HoverButton.h"
#include "utils/EditUtils.h"
#include <Commdlg.h>
#include "../core/HashAlgorithmFactory.h"
#include "../core/IHashAlgorithm.h"
#include <sstream>
#include <chrono>
#include <iomanip>
#include <vector>

CHashCalcDialog::CHashCalcDialog() : CDialog(IDD_MAIN_DIALOG) {}

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

  // Enable text input by default
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

  // Initialize Tab Control
  HWND hTab = GetDlgItem(IDC_TAB_MAIN);
  TCITEM tie = {0};
  tie.mask = TCIF_TEXT; 
  tie.pszText = (LPWSTR)L"SHA && MD";
  TabCtrl_InsertItem(hTab, 0, &tie);
  tie.pszText = (LPWSTR)L"SHA-3 && Modern";
  TabCtrl_InsertItem(hTab, 1, &tie);
  tie.pszText = (LPWSTR)L"HAVAL && RIPEMD";
  TabCtrl_InsertItem(hTab, 2, &tie);
  tie.pszText = (LPWSTR)L"Checksum && Others";
  TabCtrl_InsertItem(hTab, 3, &tie);

  UpdateTabDisplay();

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
    }
    return TRUE;

  case IDC_RADIO_FILE:
    if (code == BN_CLICKED) {
      GetDlgItem(IDC_EDIT_TEXT).EnableWindow(FALSE);
      GetDlgItem(IDC_EDIT_FILE).EnableWindow(TRUE);
      GetDlgItem(IDC_BUTTON_BROWSE).EnableWindow(TRUE);
    }
    return TRUE;

  case IDC_CHECK_STAY_ON_TOP:
    OnStayOnTop();
    return TRUE;
  }

  return FALSE;
}

LRESULT CHashCalcDialog::OnNotify(WPARAM wparam, LPARAM lparam) {
  LPNMHDR pnmh = (LPNMHDR)lparam;
  if (pnmh->idFrom == IDC_TAB_MAIN && pnmh->code == TCN_SELCHANGE) {
    UpdateTabDisplay();
    return 0;
  }
  return CDialog::OnNotify(wparam, lparam);
}

INT_PTR CHashCalcDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (uMsg == WM_CTLCOLORSTATIC) {
    HDC hdc = (HDC)wParam;
    HWND hwndCtl = (HWND)lParam;
    
    // Check if the control overlaps with the tab control
    HWND hTab = GetDlgItem(IDC_TAB_MAIN);
    if (hTab) {
        RECT rcTab, rcCtl, rcInter;
        ::GetWindowRect(hTab, &rcTab);
        ::GetWindowRect(hwndCtl, &rcCtl);
        
        // If they intersect, assume the control is "on" the tab.
        // We set the background to Transparent and return a White Brush 
        // to match the tab's usually white background.
        if (::IntersectRect(&rcInter, &rcTab, &rcCtl)) {
            ::SetBkMode(hdc, TRANSPARENT);
            return (INT_PTR)::GetStockObject(WHITE_BRUSH);
        }
    }
  }
  return CDialog::DialogProc(uMsg, wParam, lParam);
}

void CHashCalcDialog::UpdateTabDisplay() {
  HWND hTab = GetDlgItem(IDC_TAB_MAIN);
  int sel = TabCtrl_GetCurSel(hTab);

  // Tab 0: SHA & MD
  // Includes: SHA-1/2 group, MD group
  int tab0[] = {
      IDC_GROUP_SHA, IDC_SHA_160, IDC_SHA_224, IDC_SHA_256, IDC_SHA_384, IDC_SHA_512,
      IDC_GROUP_MD, IDC_MD2, IDC_MD4, IDC_MD5
  };

  // Tab 1: SHA-3 & Modern
  // Includes: SHA-3 group, BLAKE group, LSH group, SM3 (in Exotic/Misc group originally, moving to groups)
  int tab1[] = {
      IDC_GROUP_SHA3, IDC_SHA3_224, IDC_SHA3_256, IDC_SHA3_384, IDC_SHA3_512,
      IDC_GROUP_KECCAK, IDC_KECCAK_224, IDC_KECCAK_256, IDC_KECCAK_384, IDC_KECCAK_512,
      IDC_GROUP_SHAKE, IDC_SHAKE_128, IDC_SHAKE_256,
      IDC_GROUP_BLAKE, IDC_BLAKE2B, IDC_BLAKE2S,
      IDC_GROUP_LSH, IDC_LSH_256, IDC_LSH_512
  };

  // Tab 2: HAVAL & RIPEMD
  // Includes: HAVAL group, RIPEMD group
  int tab2[] = {
      IDC_GROUP_HAVAL, IDC_HAVAL_128, IDC_HAVAL_160, IDC_HAVAL_192, IDC_HAVAL_224, IDC_HAVAL_256,
      IDC_HAVAL_PASS3, IDC_HAVAL_PASS4, IDC_HAVAL_PASS5, IDC_STATIC_HAVAL_PASS,
      IDC_GROUP_RIPEMD, IDC_RIPEMD_128, IDC_RIPEMD_160, IDC_RIPEMD_256, IDC_RIPEMD_320
  };

  // Tab 3: Checksum & Others
  // Includes: Checksum group, Exotic/Misc group (Tiger, Whirlpool, SM3 is moved to Modern but we can keep Misc group here sans SM3 or just keep the group structure)
  // Let's put SM3 in Tab 1 (Modern) visually, but we need to handle the Group Box controls. 
  // We will assume re-organization of Group Boxes in RC file.
  int tab3[] = {
      IDC_GROUP_CHECKSUM, IDC_CRC32, IDC_ADLER32,
      IDC_GROUP_MISC, IDC_TIGER, IDC_WHIRLPOOL, IDC_SM3 
  };
  
  // Note: SM3 fits better in Modern, but if it shares a GroupBox with Tiger/Whirlpool in RC, 
  // we might need to verify groupings. 
  // For now, I'll display SM3 in Tab 1 (Modern) and Tiger/Whirlpool in Tab 3.
  // This means IDC_GROUP_MISC might need to be shown in both or split?
  // Easier: Move SM3 to a 'Modern' group or standalone in Tab 1.
  // Actually, I will explicitly list SM3 in Tab 1 in this array, and remove from Tab 3 array if I want it there.
  // BUT: The group box "Exotic" (IDC_GROUP_MISC) surrounds them.
  // I will rely on the RC file update to split these or I'll just show the group box in Tab 3 
  // and show SM3 in Tab 1 without a group box (or a new one).
  // Plan: In RC, I'll move SM3 out of "Exotic" or rename "Exotic" to "Others".
  // Let's put SM3 in Tab 1. Tiger & Whirlpool in Tab 3.

  // Correcting lists based on intended RC changes:
  
  // Tab 1 (Add SM3)
  std::vector<int> t1(tab1, tab1 + sizeof(tab1)/sizeof(int));
  t1.push_back(IDC_SM3);

  // Tab 3 (Remove SM3)
  std::vector<int> t3;
  t3.push_back(IDC_GROUP_CHECKSUM); t3.push_back(IDC_CRC32); 
  t3.push_back(IDC_ADLER32);
  t3.push_back(IDC_GROUP_MISC); t3.push_back(IDC_TIGER); t3.push_back(IDC_WHIRLPOOL);

  // Helper to apply show/hide
  auto apply = [&](const std::vector<int>& ids, bool show) {
      for (int id : ids) {
          ::ShowWindow(GetDlgItem(id), show ? SW_SHOW : SW_HIDE);
      }
  };
  
  // Convert standard arrays to vectors for uniformity
  std::vector<int> t0(tab0, tab0 + sizeof(tab0)/sizeof(int));
  std::vector<int> t2(tab2, tab2 + sizeof(tab2)/sizeof(int));

  apply(t0, sel == 0);
  apply(t1, sel == 1);
  apply(t2, sel == 2);
  apply(t3, sel == 3);
}

void CHashCalcDialog::OnCancel() {
  // Handle Escape key press - close the dialog
  OnExit();
}

void CHashCalcDialog::OnExit() {
  // Close the dialog
  EndDialog(IDOK);
}

void CHashCalcDialog::OnSelectAll() {
  // Check all hash algorithm checkboxes
  CheckDlgButton(IDC_SHA_160, BST_CHECKED);
  CheckDlgButton(IDC_SHA_224, BST_CHECKED);
  CheckDlgButton(IDC_SHA_256, BST_CHECKED);
  CheckDlgButton(IDC_SHA_384, BST_CHECKED);
  CheckDlgButton(IDC_SHA_512, BST_CHECKED);

  CheckDlgButton(IDC_HAVAL_128, BST_CHECKED);
  CheckDlgButton(IDC_HAVAL_160, BST_CHECKED);
  CheckDlgButton(IDC_HAVAL_192, BST_CHECKED);
  CheckDlgButton(IDC_HAVAL_224, BST_CHECKED);
  CheckDlgButton(IDC_HAVAL_256, BST_CHECKED);

  CheckDlgButton(IDC_HAVAL_PASS3, BST_CHECKED);
  CheckDlgButton(IDC_HAVAL_PASS4, BST_CHECKED);
  CheckDlgButton(IDC_HAVAL_PASS5, BST_CHECKED);

  CheckDlgButton(IDC_RIPEMD_160, BST_CHECKED);
  CheckDlgButton(IDC_MD2, BST_CHECKED);
  CheckDlgButton(IDC_MD4, BST_CHECKED);
  CheckDlgButton(IDC_MD5, BST_CHECKED);
  CheckDlgButton(IDC_CRC32, BST_CHECKED);
  CheckDlgButton(IDC_ADLER32, BST_CHECKED);

  // SHA-3
  CheckDlgButton(IDC_SHA3_224, BST_CHECKED);
  CheckDlgButton(IDC_SHA3_256, BST_CHECKED);
  CheckDlgButton(IDC_SHA3_384, BST_CHECKED);
  CheckDlgButton(IDC_SHA3_512, BST_CHECKED);
  CheckDlgButton(IDC_KECCAK_224, BST_CHECKED);
  CheckDlgButton(IDC_KECCAK_256, BST_CHECKED);
  CheckDlgButton(IDC_KECCAK_384, BST_CHECKED);
  CheckDlgButton(IDC_KECCAK_512, BST_CHECKED);
  CheckDlgButton(IDC_SHAKE_128, BST_CHECKED);
  CheckDlgButton(IDC_SHAKE_256, BST_CHECKED);

  // RIPEMD
  CheckDlgButton(IDC_RIPEMD_128, BST_CHECKED);
  CheckDlgButton(IDC_RIPEMD_256, BST_CHECKED);
  CheckDlgButton(IDC_RIPEMD_320, BST_CHECKED);

  // Misc
  CheckDlgButton(IDC_TIGER, BST_CHECKED);
  CheckDlgButton(IDC_SM3, BST_CHECKED);
  CheckDlgButton(IDC_WHIRLPOOL, BST_CHECKED);

  // BLAKE
  CheckDlgButton(IDC_BLAKE2B, BST_CHECKED);
  CheckDlgButton(IDC_BLAKE2S, BST_CHECKED);

  // LSH
  CheckDlgButton(IDC_LSH_256, BST_CHECKED);
  CheckDlgButton(IDC_LSH_512, BST_CHECKED);
}

void CHashCalcDialog::OnClearAll() {
  // Uncheck all hash algorithm checkboxes
  CheckDlgButton(IDC_SHA_160, BST_UNCHECKED);
  CheckDlgButton(IDC_SHA_224, BST_UNCHECKED);
  CheckDlgButton(IDC_SHA_256, BST_UNCHECKED);
  CheckDlgButton(IDC_SHA_384, BST_UNCHECKED);
  CheckDlgButton(IDC_SHA_512, BST_UNCHECKED);

  CheckDlgButton(IDC_HAVAL_128, BST_UNCHECKED);
  CheckDlgButton(IDC_HAVAL_160, BST_UNCHECKED);
  CheckDlgButton(IDC_HAVAL_192, BST_UNCHECKED);
  CheckDlgButton(IDC_HAVAL_224, BST_UNCHECKED);
  CheckDlgButton(IDC_HAVAL_256, BST_UNCHECKED);

  CheckDlgButton(IDC_HAVAL_PASS3, BST_UNCHECKED);
  CheckDlgButton(IDC_HAVAL_PASS4, BST_UNCHECKED);
  CheckDlgButton(IDC_HAVAL_PASS5, BST_UNCHECKED);

  CheckDlgButton(IDC_RIPEMD_160, BST_UNCHECKED);
  CheckDlgButton(IDC_MD2, BST_UNCHECKED);
  CheckDlgButton(IDC_MD4, BST_UNCHECKED);
  CheckDlgButton(IDC_MD5, BST_UNCHECKED);
  CheckDlgButton(IDC_CRC32, BST_UNCHECKED);
  CheckDlgButton(IDC_ADLER32, BST_UNCHECKED);

  // SHA-3
  CheckDlgButton(IDC_SHA3_224, BST_UNCHECKED);
  CheckDlgButton(IDC_SHA3_256, BST_UNCHECKED);
  CheckDlgButton(IDC_SHA3_384, BST_UNCHECKED);
  CheckDlgButton(IDC_SHA3_512, BST_UNCHECKED);
  CheckDlgButton(IDC_KECCAK_224, BST_UNCHECKED);
  CheckDlgButton(IDC_KECCAK_256, BST_UNCHECKED);
  CheckDlgButton(IDC_KECCAK_384, BST_UNCHECKED);
  CheckDlgButton(IDC_KECCAK_512, BST_UNCHECKED);
  CheckDlgButton(IDC_SHAKE_128, BST_UNCHECKED);
  CheckDlgButton(IDC_SHAKE_256, BST_UNCHECKED);

  // RIPEMD
  CheckDlgButton(IDC_RIPEMD_128, BST_UNCHECKED);
  CheckDlgButton(IDC_RIPEMD_256, BST_UNCHECKED);
  CheckDlgButton(IDC_RIPEMD_320, BST_UNCHECKED);

  // Misc
  CheckDlgButton(IDC_TIGER, BST_UNCHECKED);
  CheckDlgButton(IDC_SM3, BST_UNCHECKED);
  CheckDlgButton(IDC_WHIRLPOOL, BST_UNCHECKED);

  // BLAKE
  CheckDlgButton(IDC_BLAKE2B, BST_UNCHECKED);
  CheckDlgButton(IDC_BLAKE2S, BST_UNCHECKED);

  // LSH
  CheckDlgButton(IDC_LSH_256, BST_UNCHECKED);
  CheckDlgButton(IDC_LSH_512, BST_UNCHECKED);
}

void CHashCalcDialog::OnCalculate() {
  // Determine input source
  if (IsDlgButtonChecked(IDC_RADIO_TEXT)) {
    // Get text from edit control
    CString wText = GetDlgItemText(IDC_EDIT_TEXT);

    if (wText.IsEmpty()) {
      SetDlgItemText(IDC_EDIT_RESULT, L"Please enter text to hash.");
      return;
    }

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

    // Helper to compute a specific algorithm by name
    auto computeAlgo = [&](const std::string &algoName, const std::string &displayName) {
      try {
        if (core::HashAlgorithmFactory::isAvailable(algoName)) {
          auto algo = core::HashAlgorithmFactory::create(algoName);
          auto digest = algo->computeString(inputData);
          
          // Format: Algorithm Name (padded) : Hash Value
          std::wstring wDisplayName(displayName.begin(), displayName.end());
          
          output << std::left << std::setw(12) << wDisplayName << L": " 
                 << core::IHashAlgorithm::toHexWString(digest) << L"\r\n";
          anyComputed = true;
        } else {
             // For unavailable algorithms, we might want to skip or show not available.
             // Given the format, skipping might be cleaner or showing "N/A"
             // checkAndCompute logic decides if we call this.
             // If user selected it, we should probably show it.
             std::wstring wDisplayName(displayName.begin(), displayName.end());
             output << std::left << std::setw(12) << wDisplayName << L": Not Available\r\n";
        }
      } catch (const std::exception &e) {
             std::wstring wDisplayName(displayName.begin(), displayName.end());
             output << std::left << std::setw(12) << wDisplayName << L": Error\r\n";
      }
    };

    // Helper to check ID and compute
    auto checkAndCompute = [&](int id, const std::string &algoName, const std::string& displayName) {
        if (IsDlgButtonChecked(id)) {
            computeAlgo(algoName, displayName);
        }
    };

    // SHA Family
    // User requested SHA-160 for SHA-1
    checkAndCompute(IDC_SHA_160, "SHA-1", "SHA-160");
    checkAndCompute(IDC_SHA_224, "SHA-224", "SHA-224");
    checkAndCompute(IDC_SHA_256, "SHA-256", "SHA-256");
    checkAndCompute(IDC_SHA_384, "SHA-384", "SHA-384");
    checkAndCompute(IDC_SHA_512, "SHA-512", "SHA-512");

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

    // MD Family
    checkAndCompute(IDC_MD2, "MD2", "MD2");
    checkAndCompute(IDC_MD4, "MD4", "MD4");
    checkAndCompute(IDC_MD5, "MD5", "MD5");

    // Checksum
    checkAndCompute(IDC_CRC32, "CRC32", "CRC-32");
    checkAndCompute(IDC_ADLER32, "Adler32", "Adler-32");

    // SHA-3
    checkAndCompute(IDC_SHA3_224, "SHA3-224", "SHA3-224");
    checkAndCompute(IDC_SHA3_256, "SHA3-256", "SHA3-256");
    checkAndCompute(IDC_SHA3_384, "SHA3-384", "SHA3-384");
    checkAndCompute(IDC_SHA3_512, "SHA3-512", "SHA3-512");

    checkAndCompute(IDC_KECCAK_224, "Keccak-224", "Keccak-224");
    checkAndCompute(IDC_KECCAK_256, "Keccak-256", "Keccak-256");
    checkAndCompute(IDC_KECCAK_384, "Keccak-384", "Keccak-384");
    checkAndCompute(IDC_KECCAK_512, "Keccak-512", "Keccak-512");

    checkAndCompute(IDC_SHAKE_128, "SHAKE128", "SHAKE-128");
    checkAndCompute(IDC_SHAKE_256, "SHAKE256", "SHAKE-256");

    // RIPEMD
    checkAndCompute(IDC_RIPEMD_128, "RIPEMD-128", "RIPEMD-128");
    checkAndCompute(IDC_RIPEMD_160, "RIPEMD-160", "RIPEMD-160");
    checkAndCompute(IDC_RIPEMD_256, "RIPEMD-256", "RIPEMD-256");
    checkAndCompute(IDC_RIPEMD_320, "RIPEMD-320", "RIPEMD-320");

    // Misc
    checkAndCompute(IDC_TIGER, "Tiger", "Tiger");
    checkAndCompute(IDC_SM3, "SM3", "SM3");
    checkAndCompute(IDC_WHIRLPOOL, "Whirlpool", "Whirlpool");

    // BLAKE2
    checkAndCompute(IDC_BLAKE2B, "BLAKE2b", "BLAKE2b");
    checkAndCompute(IDC_BLAKE2S, "BLAKE2s", "BLAKE2s");

    // LSH
    checkAndCompute(IDC_LSH_256, "LSH-256", "LSH-256");
    checkAndCompute(IDC_LSH_512, "LSH-512", "LSH-512");

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    output << L"\r\nCalculation took " << std::fixed << std::setprecision(3) << elapsed.count() << L" seconds";

    std::wstring finalOutput = output.str();
    if (!anyComputed) {
         SetDlgItemText(IDC_EDIT_RESULT, L"Please select at least one hash algorithm.");
    } else {
      SetDlgItemText(IDC_EDIT_RESULT, finalOutput.c_str());
    }

  } else {
    SetDlgItemText(IDC_EDIT_RESULT, L"File calculation is not implemented yet.");
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
