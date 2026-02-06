#ifndef TAB_VIEW_SHA_H
#define TAB_VIEW_SHA_H

#include "TabViewBase.h"

// Tab View for SHA & MD algorithms (Tab 0)
class CTabViewSHA : public CTabViewBase {
public:
  CTabViewSHA();
  virtual ~CTabViewSHA() = default;

protected:
  // Implement base class interface
  virtual const int* GetAlgorithmIds() const override;
  virtual size_t GetAlgorithmCount() const override;

private:
  // Algorithm IDs for this tab
  static const int s_algorithmIds[];
  static const size_t s_algorithmCount;
};

#endif // TAB_VIEW_SHA_H
