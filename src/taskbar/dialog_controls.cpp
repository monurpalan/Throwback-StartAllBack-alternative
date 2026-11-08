#include "taskbar/dialog_controls.h"

#include <string>

namespace throwback {

HFONT CreateDialogFont(unsigned dpi) {
  NONCLIENTMETRICSW metrics{sizeof(metrics)};
  SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, dpi);
  return CreateFontIndirectW(&metrics.lfMessageFont);
}

HWND CreateDialogControl(HWND parent, const DialogLook& look, const wchar_t* type,
                         std::wstring_view text, DWORD style, int id, const Rect& rect) {
  const auto s = [&](int value) { return ScaleForDpi(value, look.dpi); };
  HWND control =
      CreateWindowExW(0, type, std::wstring(text).c_str(), WS_CHILD | WS_VISIBLE | style,
                      s(rect.left), s(rect.top), s(rect.Width()), s(rect.Height()), parent,
                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
  SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(look.font), TRUE);
  return control;
}

HWND CreateDialogFrame(HINSTANCE instance, HWND owner, const wchar_t* className,
                       std::wstring_view title, int width, int height, unsigned dpi,
                       void* params) {
  constexpr DWORD kStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU;
  RECT frame{0, 0, ScaleForDpi(width, dpi), ScaleForDpi(height, dpi)};
  AdjustWindowRectExForDpi(&frame, kStyle, FALSE, WS_EX_DLGMODALFRAME, dpi);
  MONITORINFO monitor{sizeof(monitor)};
  GetMonitorInfoW(MonitorFromWindow(owner, MONITOR_DEFAULTTONEAREST), &monitor);
  const RECT& work = monitor.rcWork;
  const int frameWidth = frame.right - frame.left;
  const int frameHeight = frame.bottom - frame.top;
  return CreateWindowExW(WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT, className,
                         std::wstring(title).c_str(), kStyle,
                         (work.left + work.right - frameWidth) / 2,
                         (work.top + work.bottom - frameHeight) / 2, frameWidth, frameHeight,
                         owner, nullptr, instance, params);
}

void RunModalLoop(HWND hwnd, HWND owner, const bool& done) {
  EnableWindow(owner, FALSE);
  MSG message{};
  bool quit = false;
  while (!done) {
    if (GetMessageW(&message, nullptr, 0, 0) <= 0) {
      quit = true;
      break;
    }
    if (!IsDialogMessageW(hwnd, &message)) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
  }
  if (IsWindow(owner)) EnableWindow(owner, TRUE);
  if (IsWindow(hwnd)) DestroyWindow(hwnd);
  if (quit) PostQuitMessage(static_cast<int>(message.wParam));
}

}  // namespace throwback
