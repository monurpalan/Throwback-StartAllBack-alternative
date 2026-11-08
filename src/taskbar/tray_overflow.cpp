#include "taskbar/tray_overflow.h"

#include <shellapi.h>
#include <windowsx.h>

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <utility>

#include "ui/glass.h"
#include "ui/strings.h"

namespace throwback {
namespace {

constexpr wchar_t kClassName[] = L"ThrowbackTrayOverflow";
constexpr int kGap = 8;    // between the flyout and the bar, at 96 DPI
constexpr int kIcon = 16;  // notification icons are small icons

}  // namespace

TrayOverflow::TrayOverflow(HINSTANCE instance, HWND owner, Callbacks callbacks)
    : callbacks_(std::move(callbacks)) {
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = WindowProc;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.lpszClassName = kClassName;
  RegisterClassExW(&windowClass);  // fails harmlessly when already registered

  CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kClassName, L"", WS_POPUP, 0, 0, 0, 0, owner,
                  nullptr, instance, this);
  if (!hwnd_) throw std::runtime_error("CreateWindowEx failed for the tray flyout");
  canvas_ = std::make_unique<ui::Canvas>(hwnd_);
  ui::EnableGlass(hwnd_, ui::GlassCorners::Round);
}

TrayOverflow::~TrayOverflow() {
  if (hwnd_) DestroyWindow(hwnd_);
}

bool TrayOverflow::IsOpen() const { return hwnd_ && IsWindowVisible(hwnd_); }

void TrayOverflow::Show(const RECT& arrow, Edge barEdge, unsigned dpi) {
  arrow_ = arrow;
  barEdge_ = barEdge;
  dpi_ = dpi;
  items_ = callbacks_.items();
  if (items_.empty()) return;
  hovered_ = kHitNone;
  Place();
  ShowWindow(hwnd_, SW_SHOW);
  SetForegroundWindow(hwnd_);
}

void TrayOverflow::Hide() {
  if (!IsOpen()) return;
  if (GetCapture() == hwnd_) ReleaseCapture();
  pressed_ = kHitNone;
  dragging_ = false;
  ShowWindow(hwnd_, SW_HIDE);
  if (callbacks_.closed) callbacks_.closed();
}

void TrayOverflow::Refresh() {
  if (!IsOpen()) return;
  items_ = callbacks_.items();
  if (items_.empty()) {
    Hide();
    return;
  }
  Place();
}

void TrayOverflow::Place() {
  canvas_->SetDpi(dpi_);
  const int linkWidth =
      canvas_->MeasureText(ui::Tr(ui::StringId::CustomizeIcons), ui::TextStyle::Body);
  layout_ = ComputeTrayFlyout(items_.size(), linkWidth, dpi_);
  const int width = layout_.frame.Width();
  const int height = layout_.frame.Height();
  const int gap = ScaleForDpi(kGap, dpi_);

  MONITORINFO monitor{};
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfoW(MonitorFromRect(&arrow_, MONITOR_DEFAULTTONEAREST), &monitor);
  const RECT& screen = monitor.rcMonitor;
  const Point at =
      PlacePopup({arrow_.left, arrow_.top, arrow_.right, arrow_.bottom}, width, height, barEdge_,
                 gap, {screen.left, screen.top, screen.right, screen.bottom});
  SetWindowPos(hwnd_, HWND_TOPMOST, at.x, at.y, width, height, SWP_NOACTIVATE);
  canvas_->Resize(static_cast<UINT>(width), static_cast<UINT>(height));
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void TrayOverflow::Paint() {
  PAINTSTRUCT paint;
  BeginPaint(hwnd_, &paint);
  canvas_->BeginFrame();
  canvas_->Glass(layout_.frame, ui::GlassVariant::Menu);
  canvas_->Panel(layout_.link, ui::PanelVariant::FlyoutLink);
  const int iconSize = canvas_->Scale(kIcon);
  for (size_t i = 0; i < items_.size() && i < layout_.icons.size(); ++i) {
    canvas_->Button(layout_.icons[i], ui::ButtonVariant::TrayItem,
                    hovered_ == static_cast<int>(i) && !dragging_);
    canvas_->Icon(CenteredSquare(layout_.icons[i], iconSize), items_[i].icon);
  }
  canvas_->Text(layout_.link, ui::Tr(ui::StringId::CustomizeIcons), ui::TextStyle::Body,
                ui::TextTone::Link, ui::TextAlign::Center);
  canvas_->EndFrame();
  EndPaint(hwnd_, &paint);
}

int TrayOverflow::HitTest(POINT point) const {
  for (size_t i = 0; i < items_.size() && i < layout_.icons.size(); ++i) {
    if (layout_.icons[i].Contains(point.x, point.y)) return static_cast<int>(i);
  }
  if (layout_.link.Contains(point.x, point.y)) return kHitLink;
  return kHitNone;
}

POINT TrayOverflow::Anchor(int slot) const {
  const Rect& rect = layout_.icons[static_cast<size_t>(slot)];
  POINT anchor{(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2};
  ClientToScreen(hwnd_, &anchor);
  return anchor;
}

void TrayOverflow::OnMove(POINT point) {
  if (!tracking_) {
    TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, hwnd_, 0};
    tracking_ = TrackMouseEvent(&track) != FALSE;
  }
  if (pressed_ >= 0 && !dragging_ &&
      (std::abs(point.x - pressPoint_.x) > GetSystemMetrics(SM_CXDRAG) ||
       std::abs(point.y - pressPoint_.y) > GetSystemMetrics(SM_CYDRAG))) {
    dragging_ = true;
    SetCursor(LoadCursorW(nullptr, IDC_SIZEALL));
    InvalidateRect(hwnd_, nullptr, FALSE);
  }
  const int hit = HitTest(point);
  if (hit == hovered_) return;
  hovered_ = hit;
  InvalidateRect(hwnd_, nullptr, FALSE);
  if (hit >= 0 && !dragging_) {
    callbacks_.send(items_[static_cast<size_t>(hit)].index, TrayHost::Mouse::Move, Anchor(hit));
  }
}

void TrayOverflow::OnUp(POINT point) {
  const int pressed = std::exchange(pressed_, kHitNone);
  const bool dragged = std::exchange(dragging_, false);
  if (GetCapture() == hwnd_) ReleaseCapture();
  InvalidateRect(hwnd_, nullptr, FALSE);

  if (dragged) {
    POINT screen = point;
    ClientToScreen(hwnd_, &screen);
    RECT window{};
    GetWindowRect(hwnd_, &window);
    if (pressed >= 0 && !PtInRect(&window, screen)) {
      callbacks_.dropped(items_[static_cast<size_t>(pressed)].index, screen);
    }
    return;
  }
  const int hit = HitTest(point);
  if (hit != pressed) return;
  if (hit == kHitLink) {
    // Windows' own page for which icons appear on the taskbar.
    Hide();
    ShellExecuteW(nullptr, L"open", L"ms-settings:taskbar", nullptr, nullptr, SW_SHOWNORMAL);
    return;
  }
  if (hit < 0) return;
  // No CS_DBLCLKS: a second click on the same icon in time is a double click.
  const DWORD now = GetMessageTime();
  const bool twice = hit == lastClick_ && now - lastClickTime_ <= GetDoubleClickTime();
  lastClick_ = twice ? kHitNone : hit;
  lastClickTime_ = now;
  callbacks_.send(items_[static_cast<size_t>(hit)].index,
                  twice ? TrayHost::Mouse::DoubleClick : TrayHost::Mouse::LeftClick, Anchor(hit));
}

LRESULT TrayOverflow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  const POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
  switch (message) {
    case WM_PAINT:
      Paint();
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case WM_ACTIVATE:
      // Clicking anywhere else closes the flyout, as on Windows 7.
      if (LOWORD(wParam) == WA_INACTIVE) Hide();
      return 0;
    case WM_KEYDOWN:
      if (wParam == VK_ESCAPE) Hide();
      return 0;
    case WM_MOUSEMOVE:
      OnMove(point);
      return 0;
    case WM_MOUSELEAVE:
      tracking_ = false;
      hovered_ = kHitNone;
      InvalidateRect(hwnd_, nullptr, FALSE);
      return 0;
    case WM_LBUTTONDOWN:
      pressed_ = HitTest(point);
      pressPoint_ = point;
      if (pressed_ >= 0) SetCapture(hwnd_);
      return 0;
    case WM_LBUTTONUP:
      OnUp(point);
      return 0;
    case WM_RBUTTONUP:
      if (const int hit = HitTest(point); hit >= 0) {
        callbacks_.send(items_[static_cast<size_t>(hit)].index, TrayHost::Mouse::RightClick,
                        Anchor(hit));
      }
      return 0;
  }
  return DefWindowProcW(hwnd_, message, wParam, lParam);
}

LRESULT CALLBACK TrayOverflow::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_NCCREATE) {
    auto* self =
        static_cast<TrayOverflow*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
    self->hwnd_ = hwnd;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
  }
  auto* self = reinterpret_cast<TrayOverflow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return DefWindowProcW(hwnd, message, wParam, lParam);
  if (message == WM_NCDESTROY) {
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    self->hwnd_ = nullptr;
    return DefWindowProcW(hwnd, message, wParam, lParam);
  }
  return self->HandleMessage(message, wParam, lParam);
}

}  // namespace throwback
