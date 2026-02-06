#include "TabViewHAVAL.h"

// Algorithm IDs for HAVAL & RIPEMD tab
const int CTabViewHAVAL::s_algorithmIds[] = {
  IDC_HAVAL_128, IDC_HAVAL_160, IDC_HAVAL_192, IDC_HAVAL_224, IDC_HAVAL_256,
  IDC_RIPEMD_128, IDC_RIPEMD_160, IDC_RIPEMD_256, IDC_RIPEMD_320
};

const size_t CTabViewHAVAL::s_algorithmCount = sizeof(s_algorithmIds) / sizeof(int);

// HAVAL pass selection IDs
const int CTabViewHAVAL::s_havalPassIds[] = {
  IDC_HAVAL_PASS3, IDC_HAVAL_PASS4, IDC_HAVAL_PASS5
};

const size_t CTabViewHAVAL::s_havalPassCount = sizeof(s_havalPassIds) / sizeof(int);

CTabViewHAVAL::CTabViewHAVAL() : CDialog(IDD_TAB_VIEW_HAVAL), m_hParentDialog(NULL) {
}

INT_PTR CTabViewHAVAL::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  return CDialog::DialogProc(uMsg, wParam, lParam);
}

BOOL CTabViewHAVAL::OnInitDialog() {
  return TRUE;
}

BOOL CTabViewHAVAL::OnCommand(WPARAM wparam, LPARAM lparam) {
  UNREFERENCED_PARAMETER(lparam);

  UINT id = LOWORD(wparam);
  UINT code = HIWORD(wparam);

  if (code == BN_CLICKED) {
    // Check algorithm checkboxes
    for (size_t i = 0; i < s_algorithmCount; i++) {
      if (id == s_algorithmIds[i]) {
        if (m_hParentDialog) {
          ::PostMessage(m_hParentDialog, WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), (LPARAM)GetDlgItem(id).GetHwnd());
        }
        return TRUE;
      }
    }

    // Check HAVAL pass checkboxes
    for (size_t i = 0; i < s_havalPassCount; i++) {
      if (id == s_havalPassIds[i]) {
        if (m_hParentDialog) {
          ::PostMessage(m_hParentDialog, WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), (LPARAM)GetDlgItem(id).GetHwnd());
        }
        return TRUE;
      }
    }
  }

  return FALSE;
}

int CTabViewHAVAL::CountSelectedAlgorithms() const {
  int count = 0;
  for (size_t i = 0; i < s_algorithmCount; i++) {
    if (IsDlgButtonChecked(s_algorithmIds[i]) == BST_CHECKED) {
      count++;
    }
  }
  return count;
}

void CTabViewHAVAL::SetAlgorithmStates(const std::map<int, bool>& states) {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    int id = s_algorithmIds[i];
    auto it = states.find(id);
    if (it != states.end()) {
      CheckDlgButton(id, it->second ? BST_CHECKED : BST_UNCHECKED);
    }
  }
}

std::map<int, bool> CTabViewHAVAL::GetAlgorithmStates() const {
  std::map<int, bool> states;
  for (size_t i = 0; i < s_algorithmCount; i++) {
    int id = s_algorithmIds[i];
    states[id] = (IsDlgButtonChecked(id) == BST_CHECKED);
  }
  return states;
}

void CTabViewHAVAL::SelectAll() {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    CheckDlgButton(s_algorithmIds[i], BST_CHECKED);
  }
  // Also select all HAVAL passes
  for (size_t i = 0; i < s_havalPassCount; i++) {
    CheckDlgButton(s_havalPassIds[i], BST_CHECKED);
  }

  // Force update
  if (m_hParentDialog) {
    ::PostMessage(m_hParentDialog, WM_COMMAND, MAKEWPARAM(s_algorithmIds[0], BN_CLICKED), 0);
  }
}

void CTabViewHAVAL::ClearAll() {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    CheckDlgButton(s_algorithmIds[i], BST_UNCHECKED);
  }
  // Also clear all HAVAL passes
  for (size_t i = 0; i < s_havalPassCount; i++) {
    CheckDlgButton(s_havalPassIds[i], BST_UNCHECKED);
  }
}

void CTabViewHAVAL::EnableControls(bool enable) {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    GetDlgItem(s_algorithmIds[i]).EnableWindow(enable);
  }
  // Also enable/disable HAVAL pass checkboxes
  for (size_t i = 0; i < s_havalPassCount; i++) {
    GetDlgItem(s_havalPassIds[i]).EnableWindow(enable);
  }
}

void CTabViewHAVAL::SetHavalPassStates(bool pass3, bool pass4, bool pass5) {
  CheckDlgButton(IDC_HAVAL_PASS3, pass3 ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(IDC_HAVAL_PASS4, pass4 ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(IDC_HAVAL_PASS5, pass5 ? BST_CHECKED : BST_UNCHECKED);
}

void CTabViewHAVAL::GetHavalPassStates(bool& pass3, bool& pass4, bool& pass5) const {
  pass3 = (IsDlgButtonChecked(IDC_HAVAL_PASS3) == BST_CHECKED);
  pass4 = (IsDlgButtonChecked(IDC_HAVAL_PASS4) == BST_CHECKED);
  pass5 = (IsDlgButtonChecked(IDC_HAVAL_PASS5) == BST_CHECKED);
}
