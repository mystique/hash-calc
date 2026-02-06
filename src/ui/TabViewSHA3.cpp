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

CTabViewSHA3::CTabViewSHA3() : CTabViewBase(IDD_TAB_VIEW_SHA3) {
}

const int* CTabViewSHA3::GetAlgorithmIds() const {
  return s_algorithmIds;
}

size_t CTabViewSHA3::GetAlgorithmCount() const {
  return s_algorithmCount;
}
