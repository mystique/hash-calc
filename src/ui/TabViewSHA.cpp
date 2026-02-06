#include "TabViewSHA.h"

// Algorithm IDs for SHA & MD tab
const int CTabViewSHA::s_algorithmIds[] = {
  IDC_SHA_160, IDC_SHA_224, IDC_SHA_256, IDC_SHA_384, IDC_SHA_512,
  IDC_MD2, IDC_MD4, IDC_MD5,
  IDC_MD6_128, IDC_MD6_160, IDC_MD6_192, IDC_MD6_224, IDC_MD6_256, IDC_MD6_384, IDC_MD6_512
};

const size_t CTabViewSHA::s_algorithmCount = sizeof(s_algorithmIds) / sizeof(int);

CTabViewSHA::CTabViewSHA() : CDialog(IDD_TAB_VIEW_SHA), m_hParentDialog(NULL) {
}

INT_PTR CTabViewSHA::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  // Call base class
  return CDialog::DialogProc(uMsg, wParam, lParam);
}

BOOL CTabViewSHA::OnInitDialog() {
  // Initialize dialog
  return TRUE;
}

BOOL CTabViewSHA::OnCommand(WPARAM wparam, LPARAM lparam) {
  UNREFERENCED_PARAMETER(lparam);

  UINT id = LOWORD(wparam);
  UINT code = HIWORD(wparam);

  // Handle checkbox clicks
  if (code == BN_CLICKED) {
    // Check if it's one of our algorithm checkboxes
    for (size_t i = 0; i < s_algorithmCount; i++) {
      if (id == s_algorithmIds[i]) {
        // Notify parent dialog that selection changed
        if (m_hParentDialog) {
          ::PostMessage(m_hParentDialog, WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), (LPARAM)GetDlgItem(id).GetHwnd());
        }
        return TRUE;
      }
    }
  }

  return FALSE;
}

int CTabViewSHA::CountSelectedAlgorithms() const {
  int count = 0;
  for (size_t i = 0; i < s_algorithmCount; i++) {
    if (IsDlgButtonChecked(s_algorithmIds[i]) == BST_CHECKED) {
      count++;
    }
  }
  return count;
}

void CTabViewSHA::SetAlgorithmStates(const std::map<int, bool>& states) {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    int id = s_algorithmIds[i];
    auto it = states.find(id);
    if (it != states.end()) {
      CheckDlgButton(id, it->second ? BST_CHECKED : BST_UNCHECKED);
    }
  }
}

std::map<int, bool> CTabViewSHA::GetAlgorithmStates() const {
  std::map<int, bool> states;
  for (size_t i = 0; i < s_algorithmCount; i++) {
    int id = s_algorithmIds[i];
    states[id] = (IsDlgButtonChecked(id) == BST_CHECKED);
  }
  return states;
}

void CTabViewSHA::SelectAll() {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    CheckDlgButton(s_algorithmIds[i], BST_CHECKED);
  }

  // Force window update
  ::UpdateWindow(*this);

  // Force update
  if (m_hParentDialog) {
    ::PostMessage(m_hParentDialog, WM_COMMAND, MAKEWPARAM(s_algorithmIds[0], BN_CLICKED), 0);
  }
}

void CTabViewSHA::ClearAll() {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    CheckDlgButton(s_algorithmIds[i], BST_UNCHECKED);
  }
}

void CTabViewSHA::EnableControls(bool enable) {
  for (size_t i = 0; i < s_algorithmCount; i++) {
    GetDlgItem(s_algorithmIds[i]).EnableWindow(enable);
  }
}
