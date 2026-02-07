#ifndef TAB_VIEW_HAVAL_H
#define TAB_VIEW_HAVAL_H

#include "TabViewBase.h"

// Tab View for HAVAL & RIPEMD algorithms (Tab 2)
class CTabViewHAVAL : public CTabViewBase {
public:
  CTabViewHAVAL();
  virtual ~CTabViewHAVAL() = default;

  // HAVAL pass management
  void SetHavalPassStates(bool pass3, bool pass4, bool pass5);
  void GetHavalPassStates(bool& pass3, bool& pass4, bool& pass5) const;

  // Override to count HAVAL algorithms correctly (considering passes)
  virtual int CountSelectedAlgorithms() const override;

protected:
  // Implement base class interface
  virtual const int* GetAlgorithmIds() const override;
  virtual size_t GetAlgorithmCount() const override;

  // Additional controls for HAVAL passes
  virtual const int* GetAdditionalControlIds() const override;
  virtual size_t GetAdditionalControlCount() const override;

  // Override hooks for HAVAL-specific behavior
  virtual void OnSelectAllExtra() override;
  virtual void OnClearAllExtra() override;
  virtual void OnEnableControlsExtra(bool enable) override;

private:
  // Algorithm IDs for this tab
  static const int s_algorithmIds[];
  static const size_t s_algorithmCount;

  // HAVAL pass IDs
  static const int s_havalPassIds[];
  static const size_t s_havalPassCount;
};

#endif // TAB_VIEW_HAVAL_H
