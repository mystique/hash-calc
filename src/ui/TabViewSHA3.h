#ifndef TAB_VIEW_SHA3_H
#define TAB_VIEW_SHA3_H

#include "TabViewBase.h"

// Tab View for SHA-3 & Modern algorithms (Tab 1)
class CTabViewSHA3 : public CTabViewBase {
public:
  CTabViewSHA3();
  virtual ~CTabViewSHA3() = default;

protected:
  // Implement base class interface
  virtual const int* GetAlgorithmIds() const override;
  virtual size_t GetAlgorithmCount() const override;

private:
  // Algorithm IDs for this tab
  static const int s_algorithmIds[];
  static const size_t s_algorithmCount;
};

#endif // TAB_VIEW_SHA3_H
