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

CTabViewHAVAL::CTabViewHAVAL() : CTabViewBase(IDD_TAB_VIEW_HAVAL) {
}

const int* CTabViewHAVAL::GetAlgorithmIds() const {
  return s_algorithmIds;
}

size_t CTabViewHAVAL::GetAlgorithmCount() const {
  return s_algorithmCount;
}

const int* CTabViewHAVAL::GetAdditionalControlIds() const {
  return s_havalPassIds;
}

size_t CTabViewHAVAL::GetAdditionalControlCount() const {
  return s_havalPassCount;
}

void CTabViewHAVAL::OnSelectAllExtra() {
  // Also select all HAVAL passes
  for (size_t i = 0; i < s_havalPassCount; i++) {
    CheckDlgButton(s_havalPassIds[i], BST_CHECKED);
  }
}

void CTabViewHAVAL::OnClearAllExtra() {
  // Also clear all HAVAL passes
  for (size_t i = 0; i < s_havalPassCount; i++) {
    CheckDlgButton(s_havalPassIds[i], BST_UNCHECKED);
  }
}

void CTabViewHAVAL::OnEnableControlsExtra(bool enable) {
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
