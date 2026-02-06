#include "TabViewBase.h"

CTabViewBase::CTabViewBase(UINT nResID) 
  : CDialog(nResID), m_hParentDialog(NULL) {
}

INT_PTR CTabViewBase::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  return CDialog::DialogProc(uMsg, wParam, lParam);
}

BOOL CTabViewBase::OnInitDialog() {
  return TRUE;
}

BOOL CTabViewBase::OnCommand(WPARAM wparam, LPARAM lparam) {
  UNREFERENCED_PARAMETER(lparam);

  UINT id = LOWORD(wparam);
  UINT code = HIWORD(wparam);

  if (code == BN_CLICKED) {
    // Check algorithm checkboxes
    const int* algorithmIds = GetAlgorithmIds();
    size_t algorithmCount = GetAlgorithmCount();
    
    for (size_t i = 0; i < algorithmCount; i++) {
      if (id == algorithmIds[i]) {
        if (m_hParentDialog) {
          ::PostMessage(m_hParentDialog, WM_COMMAND, 
                       MAKEWPARAM(id, BN_CLICKED), 
                       (LPARAM)GetDlgItem(id).GetHwnd());
        }
        return TRUE;
      }
    }

    // Check additional control checkboxes (if any)
    const int* additionalIds = GetAdditionalControlIds();
    size_t additionalCount = GetAdditionalControlCount();
    
    if (additionalIds && additionalCount > 0) {
      for (size_t i = 0; i < additionalCount; i++) {
        if (id == additionalIds[i]) {
          if (m_hParentDialog) {
            ::PostMessage(m_hParentDialog, WM_COMMAND, 
                         MAKEWPARAM(id, BN_CLICKED), 
                         (LPARAM)GetDlgItem(id).GetHwnd());
          }
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

int CTabViewBase::CountSelectedAlgorithms() const {
  int count = 0;
  const int* algorithmIds = GetAlgorithmIds();
  size_t algorithmCount = GetAlgorithmCount();
  
  for (size_t i = 0; i < algorithmCount; i++) {
    if (IsDlgButtonChecked(algorithmIds[i]) == BST_CHECKED) {
      count++;
    }
  }
  return count;
}

void CTabViewBase::SetAlgorithmStates(const std::map<int, bool>& states) {
  const int* algorithmIds = GetAlgorithmIds();
  size_t algorithmCount = GetAlgorithmCount();
  
  for (size_t i = 0; i < algorithmCount; i++) {
    int id = algorithmIds[i];
    auto it = states.find(id);
    if (it != states.end()) {
      CheckDlgButton(id, it->second ? BST_CHECKED : BST_UNCHECKED);
    }
  }
}

std::map<int, bool> CTabViewBase::GetAlgorithmStates() const {
  std::map<int, bool> states;
  const int* algorithmIds = GetAlgorithmIds();
  size_t algorithmCount = GetAlgorithmCount();
  
  for (size_t i = 0; i < algorithmCount; i++) {
    int id = algorithmIds[i];
    states[id] = (IsDlgButtonChecked(id) == BST_CHECKED);
  }
  return states;
}

void CTabViewBase::SelectAll() {
  const int* algorithmIds = GetAlgorithmIds();
  size_t algorithmCount = GetAlgorithmCount();
  
  for (size_t i = 0; i < algorithmCount; i++) {
    CheckDlgButton(algorithmIds[i], BST_CHECKED);
  }

  // Allow derived classes to add extra behavior
  OnSelectAllExtra();

  // Force update
  if (m_hParentDialog && algorithmCount > 0) {
    ::PostMessage(m_hParentDialog, WM_COMMAND, 
                 MAKEWPARAM(algorithmIds[0], BN_CLICKED), 0);
  }
}

void CTabViewBase::ClearAll() {
  const int* algorithmIds = GetAlgorithmIds();
  size_t algorithmCount = GetAlgorithmCount();
  
  for (size_t i = 0; i < algorithmCount; i++) {
    CheckDlgButton(algorithmIds[i], BST_UNCHECKED);
  }

  // Allow derived classes to add extra behavior
  OnClearAllExtra();
}

void CTabViewBase::EnableControls(bool enable) {
  const int* algorithmIds = GetAlgorithmIds();
  size_t algorithmCount = GetAlgorithmCount();
  
  for (size_t i = 0; i < algorithmCount; i++) {
    GetDlgItem(algorithmIds[i]).EnableWindow(enable);
  }

  // Allow derived classes to add extra behavior
  OnEnableControlsExtra(enable);
}

