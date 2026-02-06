#include "TabViewSHA.h"

// Algorithm IDs for SHA & MD tab
const int CTabViewSHA::s_algorithmIds[] = {
  IDC_SHA_160, IDC_SHA_224, IDC_SHA_256, IDC_SHA_384, IDC_SHA_512,
  IDC_MD2, IDC_MD4, IDC_MD5,
  IDC_MD6_128, IDC_MD6_160, IDC_MD6_192, IDC_MD6_224, IDC_MD6_256, IDC_MD6_384, IDC_MD6_512
};

const size_t CTabViewSHA::s_algorithmCount = sizeof(s_algorithmIds) / sizeof(int);

CTabViewSHA::CTabViewSHA() : CTabViewBase(IDD_TAB_VIEW_SHA) {
}

const int* CTabViewSHA::GetAlgorithmIds() const {
  return s_algorithmIds;
}

size_t CTabViewSHA::GetAlgorithmCount() const {
  return s_algorithmCount;
}
