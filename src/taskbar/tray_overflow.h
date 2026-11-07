#pragma once

#include <windows.h>

#include <functional>
#include <memory>
#include <vector>

#include "core/tray_layout.h"
#include "taskbar/tray_host.h"
#include "ui/canvas.h"

namespace throwback {

// The flyout that opens from the notification area's arrow, with the icons
// that don't sit on the bar. Clicks go to the icons' programs; dragging an
// icon out of the flyout asks for it to move onto the bar.
class TrayOverflow {
 public:
  struct Item {
    size_t index = 0;  // in TrayHost::VisibleIcons
    HICON icon = nullptr;
  };
  struct Callbacks {
    std::function<std::vector<Item>()> items;
    std::function<void(size_t index, TrayHost::Mouse mouse, POINT anchor)> send;
    // An icon was dropped outside the flyout at a screen point.
    std::function<void(size_t index, POINT point)> dropped;
    std::function<void()> closed;
  };

  TrayOverflow(HINSTANCE instance, HWND owner, Callbacks callbacks);
  ~TrayOverflow();
  TrayOverflow(const TrayOverflow&) = delete;
  TrayOverflow& operator=(const TrayOverflow&) = delete;

  // Opens next to the arrow (screen pixels), toward the rest of the screen.
  void Show(const RECT& arrow, bool barAtTop, unsigned dpi);
  void Hide();
  bool IsOpen() const;
  // The icons changed: resizes or closes when none are left.
  void Refresh();

 private:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
  void Place();
  void Paint();
  int HitTest(POINT point) const;  // icon slot, kHitLink or kHitNone
  void OnMove(POINT point);
  void OnUp(POINT point);
  POINT Anchor(int slot) const;

  static constexpr int kHitNone = -1;
  static constexpr int kHitLink = -2;

  HWND hwnd_ = nullptr;
  Callbacks callbacks_;
  std::unique_ptr<ui::Canvas> canvas_;
  std::vector<Item> items_;
  TrayFlyoutLayout layout_;
  RECT arrow_{};
  bool barAtTop_ = false;
  unsigned dpi_ = 96;
  int hovered_ = kHitNone;
  bool tracking_ = false;
  int pressed_ = kHitNone;
  POINT pressPoint_{};
  bool dragging_ = false;
  int lastClick_ = kHitNone;
  DWORD lastClickTime_ = 0;
};

}  // namespace throwback
