#include "AboutDialog.h"
#include <windowsx.h>
#include <sstream>
#include <iomanip>
#include <commctrl.h>

CAboutDialog::CAboutDialog() : CDialog(IDD_ABOUT_DIALOG) {
}

BOOL CAboutDialog::OnInitDialog() {
  CDialog::OnInitDialog();

  // Get version from resource
  std::wstring version = GetVersionFromResource();
  std::wstring buildDate = GetBuildDate();
  std::wstring copyright = GetCopyrightFromResource();

  // Set application name and version
  std::wstring appNameText = L"Hash Calculator " + version;
  SetDlgItemText(IDC_ABOUT_APPNAME, appNameText.c_str());

  // Set build date (right-aligned)
  std::wstring buildDateText = L"Build: " + buildDate;
  SetDlgItemText(IDC_ABOUT_VERSION, buildDateText.c_str());

  // Set copyright
  SetDlgItemText(IDC_ABOUT_COPYRIGHT, copyright.c_str());

  // Set technology info (right-aligned)
  SetDlgItemText(IDC_ABOUT_TECH_INFO, L"[ Win32++, Crypto++, BLAKE3, MD6, HAVAL ]");

  // Apply hover effect to OK button
  AttachHoverButton(GetDlgItem(IDC_ABOUT_OK));

  return TRUE;
}

BOOL CAboutDialog::OnCommand(WPARAM wparam, LPARAM lparam) {
  UNREFERENCED_PARAMETER(lparam);

  switch (LOWORD(wparam)) {
    case IDC_ABOUT_OK:
    case IDOK:
    case IDCANCEL:
      EndDialog(IDOK);
      return TRUE;
  }

  return FALSE;
}

INT_PTR CAboutDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_NOTIFY: {
      LPNMHDR pnmhdr = (LPNMHDR)lParam;
      if (pnmhdr->idFrom == IDC_ABOUT_WEB_LINK && (pnmhdr->code == NM_CLICK || pnmhdr->code == NM_RETURN)) {
        // SysLink control handles the link opening automatically
        PNMLINK pNMLink = (PNMLINK)lParam;
        ShellExecute(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
        return TRUE;
      }
      break;
    }

    case WM_DESTROY: {
      // Clean up resources (no longer needed for SysLink)
      break;
    }
  }

  return CDialog::DialogProc(uMsg, wParam, lParam);
}

std::wstring CAboutDialog::GetVersionFromResource() {
  HMODULE hModule = GetModuleHandle(NULL);
  HRSRC hResInfo = FindResource(hModule, MAKEINTRESOURCE(1), RT_VERSION);
  if (!hResInfo) return L"v1.0.0";

  HGLOBAL hResData = LoadResource(hModule, hResInfo);
  if (!hResData) return L"v1.0.0";

  LPVOID pRes = LockResource(hResData);
  if (!pRes) return L"v1.0.0";

  UINT uLen;
  VS_FIXEDFILEINFO* pFileInfo = nullptr;
  if (VerQueryValue(pRes, L"\\", (LPVOID*)&pFileInfo, &uLen)) {
    WORD major = HIWORD(pFileInfo->dwProductVersionMS);
    WORD minor = LOWORD(pFileInfo->dwProductVersionMS);
    WORD build = HIWORD(pFileInfo->dwProductVersionLS);

    std::wstringstream ss;
    ss << L"v" << major << L"." << minor << L"." << build;
    return ss.str();
  }

  return L"v1.0.0";
}

std::wstring CAboutDialog::GetBuildDate() {
  // Use compile-time date
  const char* date = __DATE__;  // Format: "Feb  7 2026"
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  char month[4] = {0};
  int day = 0, year = 0;
  sscanf_s(date, "%s %d %d", month, (unsigned)sizeof(month), &day, &year);

  int monthNum = 1;
  for (int i = 0; i < 12; i++) {
    if (strcmp(month, months[i]) == 0) {
      monthNum = i + 1;
      break;
    }
  }

  std::wstringstream ss;
  ss << year << L"-" << std::setfill(L'0') << std::setw(2) << monthNum
     << L"-" << std::setfill(L'0') << std::setw(2) << day;
  return ss.str();
}

std::wstring CAboutDialog::GetCopyrightFromResource() {
  HMODULE hModule = GetModuleHandle(NULL);
  HRSRC hResInfo = FindResource(hModule, MAKEINTRESOURCE(1), RT_VERSION);
  if (!hResInfo) return L"(c) 2026 gog.one";

  HGLOBAL hResData = LoadResource(hModule, hResInfo);
  if (!hResData) return L"(c) 2026 gog.one";

  LPVOID pRes = LockResource(hResData);
  if (!pRes) return L"(c) 2026 gog.one";

  UINT uLen;
  LPWSTR pCopyright = nullptr;
  if (VerQueryValue(pRes, L"\\StringFileInfo\\040904b0\\LegalCopyright", (LPVOID*)&pCopyright, &uLen)) {
    return std::wstring(pCopyright);
  }

  return L"(c) 2026 gog.one";
}
