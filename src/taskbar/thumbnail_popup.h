#pragma once

#include <dwmapi.h>
#include <windows.h>

#include <memory>
#include <string>

#include "core/layout.h"
#include "ui/canvas.h"
#include "ui/theme.h"

namespace throwback {

// The live preview shown when the mouse rests on a taskbar button: the
// window's icon and title above a DWM thumbnail of the window.
class ThumbnailPopup {
 public:
  // The popup is owned by the bar, so closing requests reach the bar.
  ThumbnailPopup(HINSTANCE instance, HWND owner);
  ~ThumbnailPopup();
  ThumbnailPopup(const ThumbnailPopup&) = delete;
  ThumbnailPopup& operator=(const ThumbnailPopup&) = delete;

  struct Content {
    HWND task = nullptr;
    std::wstring title;
    HICON icon = nullptr;
  };
  // anchor is the button in screen coordinates; the popup opens below it
  // for a bar at the top of the screen and above it otherwise.
  void Show(const Content& content, const RECT& anchor, bool below, const ui::Theme& theme,
            unsigned dpi);
  void Hide();
  HWND Task() const { return content_.task; }

 private:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  void Paint();

  HWND hwnd_ = nullptr;
  HTHUMBNAIL thumbnail_ = nullptr;
  Content content_;
  Rect iconRect_;
  Rect titleRect_;
  std::unique_ptr<ui::Canvas> canvas_;
};

}  // namespace throwback
