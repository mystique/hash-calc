#ifndef TAB_VIEW_CHECKSUM_H
#define TAB_VIEW_CHECKSUM_H

#include "TabViewBase.h"

// Tab View for Checksum & Others algorithms (Tab 3)
class CTabViewChecksum : public CTabViewBase {
public:
  CTabViewChecksum();
  virtual ~CTabViewChecksum() = default;

protected:
  // Implement base class interface
  virtual const int* GetAlgorithmIds() const override;
  virtual size_t GetAlgorithmCount() const override;

private:
  // Algorithm IDs for this tab
  static const int s_algorithmIds[];
  static const size_t s_algorithmCount;
};

#endif // TAB_VIEW_CHECKSUM_H
