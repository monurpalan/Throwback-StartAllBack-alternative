#pragma once

#include <windows.h>

#include <functional>
#include <memory>

#include "core/layout.h"
#include "ui/canvas.h"

namespace throwback {

// The flyout of the notification area's volume icon, as on Windows 7: the
// level in percent, a vertical slider, a mute button and a link to the
// volume mixer. It closes when the user clicks anywhere else.
class VolumeFlyout {
 public:
  struct Callbacks {
    std::function<float()> level;
    std::function<bool()> muted;
    std::function<void(float)> setLevel;
    std::function<void(bool)> setMuted;
    std::function<void()> openMixer;
    std::function<void()> closed;
  };

  VolumeFlyout(HINSTANCE instance, HWND owner, Callbacks callbacks);
  ~VolumeFlyout();
  VolumeFlyout(const VolumeFlyout&) = delete;
  VolumeFlyout& operator=(const VolumeFlyout&) = delete;

  // Opens next to the icon (screen pixels), away from the bar.
  void Show(const RECT& icon, Edge barEdge, unsigned dpi);
  void Hide();
  bool IsOpen() const;
  // The volume changed elsewhere (keys, another app): redraw.
  void Refresh();

 private:
  enum class Part { None, Slider, Mute, Mixer };
  struct Layout {
    Rect frame, percent, track, mute, mixer;
  };

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
  void Paint();
  Part HitTest(POINT point) const;
  void Slide(int y);
  void OnUp(POINT point);

  HWND hwnd_ = nullptr;
  Callbacks callbacks_;
  std::unique_ptr<ui::Canvas> canvas_;
  Layout layout_;
  unsigned dpi_ = 96;
  Part hovered_ = Part::None;
  Part pressed_ = Part::None;
  bool tracking_ = false;
};

}  // namespace throwback
