#ifndef HASH_CALC_DIALOG_H
#define HASH_CALC_DIALOG_H

#include "resource.h"
#include "wxx_dialog.h"

class CHashCalcDialog : public Win32xx::CDialog {
public:
  CHashCalcDialog();
  virtual ~CHashCalcDialog() override = default;

protected:
  // Virtual function overrides
  virtual BOOL OnInitDialog() override;
  virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;
  virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
  virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override; // Added
  virtual void OnCancel() override;

private:
  void UpdateTabDisplay(); // Added

private:
  void OnExit();
  void OnSelectAll();
  void OnClearAll();
  void OnCalculate();
  void OnAbout();
  void OnBrowse();
  void OnStayOnTop();

  CFont m_fontResult;
};

#endif // HASH_CALC_DIALOG_H
