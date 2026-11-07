#pragma once

#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <memory>

#include "ui/canvas.h"

namespace throwback::ui {

// The Windows 7 orb in a window of its own, so its glow reaches past the bar
// the way it does on Windows 7. It lets every click through to the bar under
// it; the bar keeps handling the start button.
class OrbWindow {
 public:
  // Null when the Windows 7 orb image isn't embedded; the bar then draws the
  // orb itself.
  static std::unique_ptr<OrbWindow> Create(HINSTANCE instance, HWND owner);

  OrbWindow(const OrbWindow&) = delete;
  OrbWindow& operator=(const OrbWindow&) = delete;
  ~OrbWindow();

  // Centers the orb on a point in screen pixels.
  void Show(POINT center, unsigned dpi, OrbState state);
  void Hide();

 private:
  OrbWindow(HWND hwnd, Microsoft::WRL::ComPtr<IWICImagingFactory> wic);
  // Renders the orb's frames for a DPI into strip_; false when that fails.
  bool LoadStrip(unsigned dpi);

  HWND hwnd_;
  Microsoft::WRL::ComPtr<IWICImagingFactory> wic_;
  HBITMAP strip_ = nullptr;  // the three states stacked, premultiplied
  unsigned stripDpi_ = 0;
  int size_ = 0;  // one frame's width and height

  // What is on screen, so unchanged updates are skipped.
  bool shown_ = false;
  POINT center_{};
  OrbState state_ = OrbState::Normal;
};

}  // namespace throwback::ui
