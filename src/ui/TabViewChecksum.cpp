#include "TabViewChecksum.h"

// Algorithm IDs for Checksum & Others tab
const int CTabViewChecksum::s_algorithmIds[] = {
  IDC_CRC8, IDC_CRC16, IDC_CRC32, IDC_CRC32C, IDC_CRC64, IDC_ADLER32,
  IDC_TIGER, IDC_WHIRLPOOL
};

const size_t CTabViewChecksum::s_algorithmCount = sizeof(s_algorithmIds) / sizeof(int);

CTabViewChecksum::CTabViewChecksum() : CTabViewBase(IDD_TAB_VIEW_CHECKSUM) {
}

const int* CTabViewChecksum::GetAlgorithmIds() const {
  return s_algorithmIds;
}

size_t CTabViewChecksum::GetAlgorithmCount() const {
  return s_algorithmCount;
}
