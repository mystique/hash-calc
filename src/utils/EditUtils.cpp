#include "EditUtils.h"

// Property names to store the original window procedure
static const wchar_t *PROP_ORIGINAL_PROC_CENTERED = L"Edit_OriginalProc_Centered";
static const wchar_t *PROP_ORIGINAL_PROC_AUTOSCROLL = L"Edit_OriginalProc_AutoScroll";

// Custom procedure for edits that need vertical centering (single line inputs)
static LRESULT CALLBACK VerticalCenteredEditProc(HWND hwnd, UINT msg, WPARAM wParam,
                                         LPARAM lParam) {
  WNDPROC originalProc = (WNDPROC)GetProp(hwnd, PROP_ORIGINAL_PROC_CENTERED);
  if (!originalProc)
    return DefWindowProc(hwnd, msg, wParam, lParam);

  if (msg == WM_NCCALCSIZE && wParam) {
    // First let the original proc calculate the standard client area (handles
    // borders)
    LRESULT res = CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
    NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS *)lParam;

    // params->rgrc[0] now contains the client area as calculated by the
    // original proc
    RECT &rect = params->rgrc[0];
    
    // Reserve space for our custom 3D border (2 pixels on each side)
    rect.left += 2;
    rect.top += 2;
    rect.right -= 2;
    rect.bottom -= 2;

    // Only apply vertical centering for single-line edit controls
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    BOOL isMultiline = (style & ES_MULTILINE) != 0;
    
    if (!isMultiline) {
      HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
      if (!hFont)
        hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

      HDC hdc = GetDC(hwnd);
      HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
      TEXTMETRIC tm;
      if (GetTextMetrics(hdc, &tm)) {
        int textHeight = tm.tmHeight;
        int clientHeight = rect.bottom - rect.top;

        if (clientHeight > textHeight) {
          int topPadding = (clientHeight - textHeight) / 2;
          int bottomPadding = clientHeight - textHeight - topPadding;
          rect.top += topPadding;
          rect.bottom -= bottomPadding;
        }
      }
      SelectObject(hdc, hOldFont);
      ReleaseDC(hwnd, hdc);
    }

    return res;
  }

  if (msg == WM_NCPAINT) {
    // Don't call the default handler - we'll draw everything ourselves
    HDC hdc = GetWindowDC(hwnd);
    if (hdc) {
      RECT rcWin;
      GetWindowRect(hwnd, &rcWin);
      int winWidth = rcWin.right - rcWin.left;
      int winHeight = rcWin.bottom - rcWin.top;

      // First, fill the entire non-client area with the window background
      HBRUSH hBrush;
      if (!IsWindowEnabled(hwnd) ||
          (GetWindowLong(hwnd, GWL_STYLE) & ES_READONLY)) {
        hBrush = GetSysColorBrush(COLOR_3DFACE);
      } else {
        hBrush = GetSysColorBrush(COLOR_WINDOW);
      }
      
      RECT rcFill = {0, 0, winWidth, winHeight};
      FillRect(hdc, &rcFill, hBrush);
      
      // Now draw the 3D sunken border
      // Using very dark colors for shadows and bright colors for highlights
      HPEN hPenOuterDark = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));     // Very dark outer shadow
      HPEN hPenInnerDark = CreatePen(PS_SOLID, 1, RGB(140, 140, 140));  // Medium inner shadow
      HPEN hPenInnerLight = CreatePen(PS_SOLID, 1, RGB(250, 250, 250)); // Very bright inner highlight
      HPEN hPenOuterLight = CreatePen(PS_SOLID, 1, RGB(255, 255, 255)); // White outer highlight
      
      HPEN hOldPen = (HPEN)SelectObject(hdc, hPenOuterDark);
      
      // Outer dark shadow - top edge
      MoveToEx(hdc, 0, 0, NULL);
      LineTo(hdc, winWidth, 0);
      
      // Outer dark shadow - left edge
      MoveToEx(hdc, 0, 0, NULL);
      LineTo(hdc, 0, winHeight);
      
      // Inner dark shadow - top edge
      SelectObject(hdc, hPenInnerDark);
      MoveToEx(hdc, 1, 1, NULL);
      LineTo(hdc, winWidth - 1, 1);
      
      // Inner dark shadow - left edge
      MoveToEx(hdc, 1, 1, NULL);
      LineTo(hdc, 1, winHeight - 1);
      
      // Outer light highlight - bottom edge
      SelectObject(hdc, hPenOuterLight);
      MoveToEx(hdc, 0, winHeight - 1, NULL);
      LineTo(hdc, winWidth, winHeight - 1);
      
      // Outer light highlight - right edge
      MoveToEx(hdc, winWidth - 1, 0, NULL);
      LineTo(hdc, winWidth - 1, winHeight);
      
      // Inner light highlight - bottom edge
      SelectObject(hdc, hPenInnerLight);
      MoveToEx(hdc, 1, winHeight - 2, NULL);
      LineTo(hdc, winWidth - 1, winHeight - 2);
      
      // Inner light highlight - right edge
      MoveToEx(hdc, winWidth - 2, 1, NULL);
      LineTo(hdc, winWidth - 2, winHeight - 1);
      
      SelectObject(hdc, hOldPen);
      DeleteObject(hPenOuterDark);
      DeleteObject(hPenInnerDark);
      DeleteObject(hPenInnerLight);
      DeleteObject(hPenOuterLight);

      ReleaseDC(hwnd, hdc);
    }
    return 0; // We handled it completely
  }

  if (msg == WM_SETFONT) {
    LRESULT res = CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
    // Force recalculation of client area when font changes
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    return res;
  }

  if (msg == WM_DESTROY) {
    WNDPROC original = (WNDPROC)GetProp(hwnd, PROP_ORIGINAL_PROC_CENTERED);
    if (original) {
      SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)original);
      RemoveProp(hwnd, PROP_ORIGINAL_PROC_CENTERED);
    }
  }

  return CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
}

void AttachVerticalCenteredEdit(HWND hEdit) {
  if (!hEdit || !IsWindow(hEdit))
    return;
  if (GetProp(hEdit, PROP_ORIGINAL_PROC_CENTERED))
    return; // Already attached

  WNDPROC originalProc = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC,
                                              (LONG_PTR)VerticalCenteredEditProc);
  SetProp(hEdit, PROP_ORIGINAL_PROC_CENTERED, (HANDLE)originalProc);

  // Force NCCALCSIZE to be called immediately
  SetWindowPos(hEdit, NULL, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

// Helper function to dynamically manage scrollbar visibility by adding/removing window styles
static void UpdateScrollBars(HWND hwnd) {
  RECT rcClient;
  GetClientRect(hwnd, &rcClient);
  int clientWidth = rcClient.right - rcClient.left;
  int clientHeight = rcClient.bottom - rcClient.top;
  
  // Get text metrics
  int lineCount = (int)SendMessage(hwnd, EM_GETLINECOUNT, 0, 0);
  
  HDC hdc = GetDC(hwnd);
  HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
  if (!hFont)
    hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
  
  TEXTMETRIC tm;
  GetTextMetrics(hdc, &tm);
  
  // Calculate total content height with some margin
  int lineHeight = tm.tmHeight + tm.tmExternalLeading;
  int totalHeight = lineCount * lineHeight;
  
  // Check if we need vertical scrollbar
  // Be more lenient - show scrollbar if content is close to the edge
  BOOL needVScroll = (totalHeight > clientHeight - 5);
  
  // For horizontal scrollbar, check the longest line
  BOOL needHScroll = FALSE;
  int textLength = GetWindowTextLength(hwnd);
  if (textLength > 0) {
    WCHAR *text = new WCHAR[textLength + 1];
    GetWindowText(hwnd, text, textLength + 1);
    
    // Find the longest line
    int maxWidth = 0;
    WCHAR *line = text;
    WCHAR *nextLine;
    while (line && *line) {
      nextLine = wcschr(line, L'\n');
      int lineLen = nextLine ? (int)(nextLine - line) : (int)wcslen(line);
      
      // Skip \r if present
      if (lineLen > 0 && line[lineLen - 1] == L'\r')
        lineLen--;
      
      if (lineLen > 0) {
        SIZE size;
        GetTextExtentPoint32(hdc, line, lineLen, &size);
        if (size.cx > maxWidth)
          maxWidth = size.cx;
      }
      
      line = nextLine ? nextLine + 1 : NULL;
    }
    delete[] text;
    
    // Be more lenient for horizontal scrollbar too
    needHScroll = (maxWidth > clientWidth - 5);
  }
  
  SelectObject(hdc, hOldFont);
  ReleaseDC(hwnd, hdc);
  
  // Get current window style
  LONG style = GetWindowLong(hwnd, GWL_STYLE);
  LONG newStyle = style;
  
  // Add or remove WS_VSCROLL style
  if (needVScroll) {
    newStyle |= WS_VSCROLL;
  } else {
    newStyle &= ~WS_VSCROLL;
  }
  
  // Add or remove WS_HSCROLL style
  if (needHScroll) {
    newStyle |= WS_HSCROLL;
  } else {
    newStyle &= ~WS_HSCROLL;
  }
  
  // Only update if style changed
  if (newStyle != style) {
    SetWindowLong(hwnd, GWL_STYLE, newStyle);
    // Force window to recalculate its layout
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    InvalidateRect(hwnd, NULL, TRUE);
  }
}


static LRESULT CALLBACK AutoScrollingEditProc(HWND hwnd, UINT msg, WPARAM wParam,
                                       LPARAM lParam) {
  WNDPROC originalProc = (WNDPROC)GetProp(hwnd, PROP_ORIGINAL_PROC_AUTOSCROLL);
  if (!originalProc)
    return DefWindowProc(hwnd, msg, wParam, lParam);

  // Let the edit control handle everything normally, including scrollbars
  // We only customize the border appearance
  
  if (msg == WM_NCPAINT) {
    // Let Windows handle the default non-client painting first
    LRESULT res = CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
    
    // Now draw our custom border on top
    HDC hdc = GetWindowDC(hwnd);
    if (hdc) {
      RECT rcWin;
      GetWindowRect(hwnd, &rcWin);
      OffsetRect(&rcWin, -rcWin.left, -rcWin.top);
      
      int winWidth = rcWin.right;
      int winHeight = rcWin.bottom;
      
      // Create pens for 3D border
      HPEN hPenOuterDark = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
      HPEN hPenInnerDark = CreatePen(PS_SOLID, 1, RGB(140, 140, 140));
      HPEN hPenInnerLight = CreatePen(PS_SOLID, 1, RGB(250, 250, 250));
      HPEN hPenOuterLight = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
      
      HPEN hOldPen = (HPEN)SelectObject(hdc, hPenOuterDark);
      
      // Draw outer dark shadow (top and left)
      MoveToEx(hdc, 0, 0, NULL);
      LineTo(hdc, winWidth - 1, 0);
      MoveToEx(hdc, 0, 0, NULL);
      LineTo(hdc, 0, winHeight - 1);
      
      // Draw inner dark shadow
      SelectObject(hdc, hPenInnerDark);
      MoveToEx(hdc, 1, 1, NULL);
      LineTo(hdc, winWidth - 2, 1);
      MoveToEx(hdc, 1, 1, NULL);
      LineTo(hdc, 1, winHeight - 2);
      
      // Draw outer light highlight (bottom and right)
      SelectObject(hdc, hPenOuterLight);
      MoveToEx(hdc, 0, winHeight - 1, NULL);
      LineTo(hdc, winWidth, winHeight - 1);
      MoveToEx(hdc, winWidth - 1, 0, NULL);
      LineTo(hdc, winWidth - 1, winHeight);
      
      // Draw inner light highlight
      SelectObject(hdc, hPenInnerLight);
      MoveToEx(hdc, 1, winHeight - 2, NULL);
      LineTo(hdc, winWidth - 1, winHeight - 2);
      MoveToEx(hdc, winWidth - 2, 1, NULL);
      LineTo(hdc, winWidth - 2, winHeight - 1);
      
      SelectObject(hdc, hOldPen);
      DeleteObject(hPenOuterDark);
      DeleteObject(hPenInnerDark);
      DeleteObject(hPenInnerLight);
      DeleteObject(hPenOuterLight);

      ReleaseDC(hwnd, hdc);
    }
    return res;
  }

  // Handle text changes - update scrollbar visibility
  if (msg == WM_SETTEXT || msg == EM_REPLACESEL) {
    LRESULT res = CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
    UpdateScrollBars(hwnd);
    return res;
  }

  // Handle character input
  if (msg == WM_CHAR || msg == WM_KEYDOWN) {
    LRESULT res = CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
    // Use PostMessage to update after the text has been processed
    PostMessage(hwnd, WM_USER + 100, 0, 0);
    return res;
  }

  // Handle paste
  if (msg == WM_PASTE) {
    LRESULT res = CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
    UpdateScrollBars(hwnd);
    return res;
  }

  // Custom message for delayed scrollbar update
  if (msg == WM_USER + 100) {
    UpdateScrollBars(hwnd);
    return 0;
  }

  // Handle size changes
  if (msg == WM_SIZE) {
    LRESULT res = CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
    UpdateScrollBars(hwnd);
    return res;
  }

  if (msg == WM_DESTROY) {
    WNDPROC original = (WNDPROC)GetProp(hwnd, PROP_ORIGINAL_PROC_AUTOSCROLL);
    if (original) {
      SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)original);
      RemoveProp(hwnd, PROP_ORIGINAL_PROC_AUTOSCROLL);
    }
  }

  return CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
}

void AttachAutoScrollingEdit(HWND hEdit) {
  if (!hEdit || !IsWindow(hEdit))
    return;
  if (GetProp(hEdit, PROP_ORIGINAL_PROC_AUTOSCROLL))
    return; // Already attached

  WNDPROC originalProc = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC,
                                              (LONG_PTR)AutoScrollingEditProc);
  SetProp(hEdit, PROP_ORIGINAL_PROC_AUTOSCROLL, (HANDLE)originalProc);

  // Initialize scrollbar visibility based on current content
  UpdateScrollBars(hEdit);
}
