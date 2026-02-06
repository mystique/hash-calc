#include "TabViewChecksum.h"

// Algorithm IDs for Checksum & Others tab
const int CTabViewChecksum::s_algorithmIds[] = {
  IDC_CRC8, IDC_CRC16, IDC_CRC32, IDC_CRC32C, IDC_CRC64, IDC_ADLER32,
  IDC_TIGER, IDC_WHIRLPOOL
};

const size_t CTabViewChecksum::s_algorithmCount = sizeof(s_algorithmIds) / sizeof(int);

CTabViewChecksum::CTabViewChecksum() : CDialog(IDD_TAB_VIEW_CHECKSUM), m_hParentDialog(NULL) {
}

INT_PTR CTabViewChecksum::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  return CDialog::DialogProc(uMsg, wParam, lParam);
}

BOOL CTabViewChecksum::OnInitDialog() {
  return TRUE;
}

BOOL CTabViewChecksum::OnCommand(WPARAM wparam, LPARAM lparam) {
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

int CTabViewChecksum::CountSelectedAlgorithms() const {
  int count = 0;
  for (size_t i = 0; i < s_algorithmCount; i++) {
    if (IsDlgButtonChecked(s_algorithmIds[i]) == BST_CHECKED) {
      count++;
    }
  }
  return count;
}

void CTabViewChecksum::SetAlgorithmStates(const std::map<int, bool>& states) {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    int id = s_algorithmIds[i];
    auto it = states.find(id);
    if (it != states.end()) {
      CheckDlgButton(id, it->second ? BST_CHECKED : BST_UNCHECKED);
    }
  }
}

std::map<int, bool> CTabViewChecksum::GetAlgorithmStates() const {
  std::map<int, bool> states;
  for (size_t i = 0; i < s_algorithmCount; i++) {
    int id = s_algorithmIds[i];
    states[id] = (IsDlgButtonChecked(id) == BST_CHECKED);
  }
  return states;
}

void CTabViewChecksum::SelectAll() {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    CheckDlgButton(s_algorithmIds[i], BST_CHECKED);
  }

  // Force update
  if (m_hParentDialog) {
    ::PostMessage(m_hParentDialog, WM_COMMAND, MAKEWPARAM(s_algorithmIds[0], BN_CLICKED), 0);
  }
}

void CTabViewChecksum::ClearAll() {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    CheckDlgButton(s_algorithmIds[i], BST_UNCHECKED);
  }
}

void CTabViewChecksum::EnableControls(bool enable) {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    GetDlgItem(s_algorithmIds[i]).EnableWindow(enable);
  }
}
