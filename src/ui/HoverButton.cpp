#include "HoverButton.h"
#include <commctrl.h>

// Store old WNDPROC using a property string
static const wchar_t *PROP_OLDPROC = L"HoverBtn_OldProc";
static const wchar_t *PROP_HOVERING = L"HoverBtn_Hovering";

static LRESULT CALLBACK HoverButtonProc(HWND hwnd, UINT msg, WPARAM wParam,
                                        LPARAM lParam) {
  WNDPROC oldProc = (WNDPROC)GetProp(hwnd, PROP_OLDPROC);
  if (!oldProc)
    return DefWindowProc(hwnd, msg, wParam, lParam);

  switch (msg) {
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rect;
    GetClientRect(hwnd, &rect);

    // Double buffering
    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

    // Background - Match dialog background (usually COLOR_BTNFACE)
    HBRUSH hBrush = GetSysColorBrush(COLOR_BTNFACE);
    FillRect(hMemDC, &rect, hBrush);

    BOOL isHovering = (BOOL)(intptr_t)GetProp(hwnd, PROP_HOVERING);

    // Check if button is enabled
    BOOL isEnabled = IsWindowEnabled(hwnd);

    // Get button state
    LRESULT state = CallWindowProc(oldProc, hwnd, BM_GETSTATE, 0, 0);
    BOOL isPressed = (state & BST_PUSHED) && isEnabled;

    // Draw Edge based on state
    if (isPressed) {
      DrawEdge(hMemDC, &rect, BDR_SUNKENOUTER, BF_RECT);
    } else if (isHovering && isEnabled) {
      DrawEdge(hMemDC, &rect, BDR_RAISEDINNER, BF_RECT);
    }
    // else: completely borderless

    // Draw Text
    WCHAR text[256];
    GetWindowText(hwnd, text, 256);

    // Font
    HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    HFONT hOldFont = (HFONT)SelectObject(hMemDC, hFont);

    SetBkMode(hMemDC, TRANSPARENT);
    
    // Set text color based on enabled state
    if (isEnabled) {
      SetTextColor(hMemDC, GetSysColor(COLOR_BTNTEXT));
    } else {
      // Use gray color for disabled text
      SetTextColor(hMemDC, GetSysColor(COLOR_GRAYTEXT));
    }

    // Offset text if pressed
    if (isPressed) {
      OffsetRect(&rect, 1, 1);
    }

    DrawText(hMemDC, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if (isPressed) {
      OffsetRect(&rect, -1, -1);
    }

    // Blit
    BitBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hOldFont);
    SelectObject(hMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);

    EndPaint(hwnd, &ps);
    return 0;
  }
  case WM_MOUSEMOVE: {
    if (!(BOOL)(intptr_t)GetProp(hwnd, PROP_HOVERING)) {
      TRACKMOUSEEVENT tme;
      tme.cbSize = sizeof(tme);
      tme.dwFlags = TME_LEAVE;
      tme.hwndTrack = hwnd;
      TrackMouseEvent(&tme);

      SetProp(hwnd, PROP_HOVERING, (HANDLE)1);
      InvalidateRect(hwnd, NULL, TRUE);
    }
    break;
  }
  case WM_MOUSELEAVE: {
    SetProp(hwnd, PROP_HOVERING, (HANDLE)0);
    InvalidateRect(hwnd, NULL, TRUE);
    break;
  }
  case WM_ENABLE: {
    SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
    LRESULT res = CallWindowProc(oldProc, hwnd, msg, wParam, lParam);
    SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hwnd, NULL, TRUE);
    return res;
  }
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_CAPTURECHANGED: {
    // Pass to original first to update internal state
    LRESULT res = CallWindowProc(oldProc, hwnd, msg, wParam, lParam);
    InvalidateRect(hwnd, NULL, TRUE); // Repaint
    return res;
  }
  case WM_DESTROY: {
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldProc);
    RemoveProp(hwnd, PROP_OLDPROC);
    RemoveProp(hwnd, PROP_HOVERING);
    return CallWindowProc(oldProc, hwnd, msg, wParam, lParam);
  }
  }
  return CallWindowProc(oldProc, hwnd, msg, wParam, lParam);
}

void AttachHoverButton(HWND hButton) {
  if (!hButton || !IsWindow(hButton))
    return;

  // Prevent double attachment
  if (GetProp(hButton, PROP_OLDPROC))
    return;

  WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hButton, GWLP_WNDPROC,
                                              (LONG_PTR)HoverButtonProc);
  SetProp(hButton, PROP_OLDPROC, (HANDLE)oldProc);
  SetProp(hButton, PROP_HOVERING, (HANDLE)0);
}
