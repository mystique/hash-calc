#include "TabViewSHA3.h"

// Algorithm IDs for SHA-3 & Modern tab
const int CTabViewSHA3::s_algorithmIds[] = {
  IDC_SHA3_224, IDC_SHA3_256, IDC_SHA3_384, IDC_SHA3_512,
  IDC_KECCAK_224, IDC_KECCAK_256, IDC_KECCAK_384, IDC_KECCAK_512,
  IDC_SHAKE_128, IDC_SHAKE_256,
  IDC_BLAKE2B, IDC_BLAKE2S,
  IDC_LSH_256, IDC_LSH_512, IDC_SM3
};

const size_t CTabViewSHA3::s_algorithmCount = sizeof(s_algorithmIds) / sizeof(int);

CTabViewSHA3::CTabViewSHA3() : CDialog(IDD_TAB_VIEW_SHA3), m_hParentDialog(NULL) {
}

INT_PTR CTabViewSHA3::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  return CDialog::DialogProc(uMsg, wParam, lParam);
}

BOOL CTabViewSHA3::OnInitDialog() {
  return TRUE;
}

BOOL CTabViewSHA3::OnCommand(WPARAM wparam, LPARAM lparam) {
  UNREFERENCED_PARAMETER(lparam);

  UINT id = LOWORD(wparam);
  UINT code = HIWORD(wparam);

  if (code == BN_CLICKED) {
    for (size_t i = 0; i < s_algorithmCount; i++) {
      if (id == s_algorithmIds[i]) {
        if (m_hParentDialog) {
          ::PostMessage(m_hParentDialog, WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), (LPARAM)GetDlgItem(id).GetHwnd());
        }
        return TRUE;
      }
    }
  }

  return FALSE;
}

int CTabViewSHA3::CountSelectedAlgorithms() const {
  int count = 0;
  for (size_t i = 0; i < s_algorithmCount; i++) {
    if (IsDlgButtonChecked(s_algorithmIds[i]) == BST_CHECKED) {
      count++;
    }
  }
  return count;
}

void CTabViewSHA3::SetAlgorithmStates(const std::map<int, bool>& states) {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    int id = s_algorithmIds[i];
    auto it = states.find(id);
    if (it != states.end()) {
      CheckDlgButton(id, it->second ? BST_CHECKED : BST_UNCHECKED);
    }
  }
}

std::map<int, bool> CTabViewSHA3::GetAlgorithmStates() const {
  std::map<int, bool> states;
  for (size_t i = 0; i < s_algorithmCount; i++) {
    int id = s_algorithmIds[i];
    states[id] = (IsDlgButtonChecked(id) == BST_CHECKED);
  }
  return states;
}

void CTabViewSHA3::SelectAll() {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    CheckDlgButton(s_algorithmIds[i], BST_CHECKED);
  }

  // Force update
  if (m_hParentDialog) {
    ::PostMessage(m_hParentDialog, WM_COMMAND, MAKEWPARAM(s_algorithmIds[0], BN_CLICKED), 0);
  }
}

void CTabViewSHA3::ClearAll() {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    CheckDlgButton(s_algorithmIds[i], BST_UNCHECKED);
  }
}

void CTabViewSHA3::EnableControls(bool enable) {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    GetDlgItem(s_algorithmIds[i]).EnableWindow(enable);
  }
}
