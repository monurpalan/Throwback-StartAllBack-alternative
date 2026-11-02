// The bar's notification area: layout, drawing data and mouse input for the
// icons TrayHost collects.
#include "taskbar/taskbar_window.h"

namespace throwback {
namespace {

// Windows 7 spaces 16 px icons 24 px apart.
constexpr int kSlot = 24;
constexpr int kIcon = 16;
constexpr int kInset = 4;

}  // namespace

int TaskbarWindow::TrayWidth() const {
  if (!tray_) return 0;
  const size_t count = tray_->VisibleIcons().size();
  if (count == 0) return 0;
  return static_cast<int>(count) * ScaleForDpi(kSlot, dpi_) + 2 * ScaleForDpi(kInset, dpi_);
}

Rect TaskbarWindow::TrayIconRect(size_t index) const {
  const int slot = ScaleForDpi(kSlot, dpi_);
  const int left = layout_.tray.left + ScaleForDpi(kInset, dpi_) + static_cast<int>(index) * slot;
  const int top = layout_.tray.top + (layout_.tray.Height() - slot) / 2;
  return {left, top, left + slot, top + slot};
}

int TaskbarWindow::TrayHit(int x, int y) const {
  if (!tray_ || !layout_.tray.Contains(x, y)) return kHitNone;
  const size_t count = tray_->VisibleIcons().size();
  for (size_t i = 0; i < count; ++i) {
    if (TrayIconRect(i).Contains(x, y)) return kHitTrayFirst - static_cast<int>(i);
  }
  return kHitNone;
}

void TaskbarWindow::SendTrayMouse(int hit, TrayHost::Mouse mouse) {
  if (!tray_ || !IsTrayHit(hit)) return;
  const auto index = static_cast<size_t>(kHitTrayFirst - hit);
  // Programs place their menus and balloons at the icon.
  const Rect rect = TrayIconRect(index);
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
  tray_->Send(index, mouse, anchor);
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
  const int size = ScaleForDpi(kIcon, dpi_);
  for (size_t i = 0; i < icons.size(); ++i) {
    const Rect slot = TrayIconRect(i);
    model.trayIcons.push_back({slot, CenteredSquare(slot, size), icons[i].icon,
                               hovered_ == kHitTrayFirst - static_cast<int>(i)});
  }
}

}  // namespace throwback
