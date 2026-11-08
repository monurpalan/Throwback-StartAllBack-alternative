// The bar's notification area: which icons sit on the bar and which behind
// the arrow, layout, drawing data and mouse input for the icons TrayHost
// collects.
#include <cstdlib>

#include "taskbar/bar_manager.h"
#include "taskbar/taskbar_window.h"

namespace throwback {
namespace {

constexpr int kIcon = 16;

}  // namespace

TaskbarWindow::TrayIcons TaskbarWindow::SplitTrayIcons() const {
  TrayIcons split;
  if (!tray_) return split;
  const auto icons = tray_->VisibleIcons();
  for (size_t i = 0; i < icons.size(); ++i) {
    const bool shown =
        IsTrayIconShown(settings_, std::wstring(icons[i].identity), icons[i].windowsShows);
    (shown ? split.shown : split.hidden).push_back(i);
  }
  return split;
}

TrayAreaLayout TaskbarWindow::TrayArea() const {
  const TrayIcons split = SplitTrayIcons();
  if (IsVertical(settings_.edge)) {
    return ComputeVerticalTrayArea(layout_.tray, split.shown.size(), !split.hidden.empty(), dpi_);
  }
  return ComputeTrayArea(layout_.tray, split.shown.size(), !split.hidden.empty(), dpi_);
}

int TaskbarWindow::TrayWidth() const {
  if (!tray_) return 0;
  const TrayIcons split = SplitTrayIcons();
  return TrayAreaWidth(split.shown.size(), !split.hidden.empty(), dpi_);
}

int TaskbarWindow::TrayHeight(int barWidth) const {
  if (!tray_) return 0;
  const TrayIcons split = SplitTrayIcons();
  return VerticalTrayAreaHeight(split.shown.size(), !split.hidden.empty(), barWidth, dpi_);
}

int TaskbarWindow::TrayHit(int x, int y) const {
  if (!tray_ || !layout_.tray.Contains(x, y)) return kHitNone;
  const TrayAreaLayout area = TrayArea();
  if (area.arrow.Contains(x, y)) return kHitTrayArrow;
  for (size_t i = 0; i < area.icons.size(); ++i) {
    if (area.icons[i].Contains(x, y)) return kHitTrayFirst - static_cast<int>(i);
  }
  return kHitNone;
}

void TaskbarWindow::SendTrayMouse(int hit, TrayHost::Mouse mouse) {
  if (!tray_ || !IsTrayHit(hit)) return;
  const auto place = static_cast<size_t>(kHitTrayFirst - hit);
  const TrayIcons split = SplitTrayIcons();
  const TrayAreaLayout area = TrayArea();
  if (place >= split.shown.size() || place >= area.icons.size()) return;
  // Programs place their menus and balloons at the icon.
  const Rect& rect = area.icons[place];
  POINT anchor{(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2};
  ClientToScreen(hwnd_, &anchor);
  if (mouse == TrayHost::Mouse::LeftClick) {
    // The bar has no CS_DBLCLKS (task buttons must see every click), so a
    // second click on the same icon in time is the double click.
    const DWORD now = GetMessageTime();
    const bool twice = hit == lastTrayClick_ && now - lastTrayClickTime_ <= GetDoubleClickTime();
    lastTrayClick_ = twice ? kHitNone : hit;
    lastTrayClickTime_ = now;
    if (twice) mouse = TrayHost::Mouse::DoubleClick;
  }
  tray_->Send(split.shown[place], mouse, anchor);
}

void TaskbarWindow::PlaceTray() {
  if (!tray_) return;
  RECT bar{};
  GetWindowRect(hwnd_, &bar);
  POINT topLeft{layout_.tray.left, layout_.tray.top};
  POINT bottomRight{layout_.tray.right, layout_.tray.bottom};
  ClientToScreen(hwnd_, &topLeft);
  ClientToScreen(hwnd_, &bottomRight);
  tray_->Place(bar, {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y});
}

void TaskbarWindow::AddTrayIcons(BarModel& model) const {
  if (!tray_) return;
  const auto icons = tray_->VisibleIcons();
  const TrayIcons split = SplitTrayIcons();
  const TrayAreaLayout area = TrayArea();
  const int size = ScaleForDpi(kIcon, dpi_);
  for (size_t i = 0; i < split.shown.size() && i < area.icons.size(); ++i) {
    const Rect& slot = area.icons[i];
    model.trayIcons.push_back({slot, CenteredSquare(slot, size), icons[split.shown[i]].icon,
                               hovered_ == kHitTrayFirst - static_cast<int>(i)});
  }
  model.trayArrow = area.arrow;
  model.trayArrowHovered = hovered_ == kHitTrayArrow;
  model.trayOpen = trayOverflow_ && trayOverflow_->IsOpen();
}

void TaskbarWindow::CreateTrayOverflow() {
  TrayOverflow::Callbacks callbacks;
  callbacks.items = [this] {
    std::vector<TrayOverflow::Item> items;
    if (!tray_) return items;
    const auto icons = tray_->VisibleIcons();
    for (size_t index : SplitTrayIcons().hidden) items.push_back({index, icons[index].icon});
    return items;
  };
  callbacks.send = [this](size_t index, TrayHost::Mouse mouse, POINT anchor) {
    if (tray_) tray_->Send(index, mouse, anchor);
  };
  callbacks.dropped = [this](size_t index, POINT point) {
    // Dropped on the bar: the icon moves onto it, as on Windows 7.
    RECT bar{};
    GetWindowRect(hwnd_, &bar);
    if (PtInRect(&bar, point)) MoveTrayIcon(index, true);
  };
  callbacks.closed = [this] { InvalidateRect(hwnd_, nullptr, FALSE); };
  trayOverflow_ = std::make_unique<TrayOverflow>(module_, hwnd_, std::move(callbacks));
}

void TaskbarWindow::ToggleTrayOverflow() {
  if (!trayOverflow_) return;
  if (trayOverflow_->IsOpen()) {
    trayOverflow_->Hide();
    return;
  }
  // The user may have changed Windows' choices in Settings meanwhile.
  tray_->ReloadChoices();
  UpdateLayout();
  const Rect arrow = TrayArea().arrow;
  POINT topLeft{arrow.left, arrow.top};
  POINT bottomRight{arrow.right, arrow.bottom};
  ClientToScreen(hwnd_, &topLeft);
  ClientToScreen(hwnd_, &bottomRight);
  trayOverflow_->Show({topLeft.x, topLeft.y, bottomRight.x, bottomRight.y}, settings_.edge, dpi_);
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void TaskbarWindow::MoveTrayIcon(size_t index, bool shown) {
  if (!tray_) return;
  const auto icons = tray_->VisibleIcons();
  if (index >= icons.size() || icons[index].identity.empty()) return;
  SetTrayIconShown(settings_, std::wstring(icons[index].identity), shown);
  manager_.SaveTrayChoices();
  UpdateLayout();
  if (trayOverflow_) trayOverflow_->Refresh();
}

void TaskbarWindow::OnTrayPressed(int x, int y) {
  if (!IsTrayHit(pressedHit_)) return;
  // Capture, so a release off the bar still arrives: that drags the icon off.
  trayPress_ = {x, y};
  SetCapture(hwnd_);
}

bool TaskbarWindow::OnTrayReleased(int pressed, int x, int y) {
  if (pressed == kHitTrayArrow && TrayHit(x, y) == kHitTrayArrow) {
    ToggleTrayOverflow();
    return true;
  }
  if (!IsTrayHit(pressed)) return false;
  if (GetCapture() == hwnd_) ReleaseCapture();
  const bool dragged = std::abs(x - trayPress_.x) > GetSystemMetrics(SM_CXDRAG) ||
                       std::abs(y - trayPress_.y) > GetSystemMetrics(SM_CYDRAG);
  if (!dragged || layout_.tray.Contains(x, y)) return false;
  // Dragged off the bar: the icon goes behind the arrow, as on Windows 7.
  const TrayIcons split = SplitTrayIcons();
  const auto place = static_cast<size_t>(kHitTrayFirst - pressed);
  if (place < split.shown.size()) MoveTrayIcon(split.shown[place], false);
  return true;
}

}  // namespace throwback
