#include "startmenu/classic_menu.h"

#include <windowsx.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "shell/launch.h"
#include "ui/strings.h"

namespace throwback {
namespace {

constexpr wchar_t kClassName[] = L"ThrowbackClassicMenu";
constexpr UINT_PTR kSubmenuTimer = 1;

// Windows 98 metrics at 96 DPI: a 2 px 3D edge and 1 px of face around the
// rows, the 21 px banner, 32 px icons on the start menu itself and 16 px
// ones in submenus.
constexpr int kInset = 3;
constexpr int kBanner = 21;
constexpr int kLargeRow = 34;
constexpr int kLargeIcon = 32;
constexpr int kLargeIconLeft = 8;
constexpr int kLargeTextGap = 4;
constexpr int kSmallRow = 20;
constexpr int kXpRow = 22;  // Windows XP's All Programs menu
constexpr int kXpIconLeft = 8;
constexpr int kSmallIcon = 16;
constexpr int kSmallIconLeft = 2;
constexpr int kSmallTextGap = 6;
constexpr int kSeparator = 8;
constexpr int kArrowArea = 16;
constexpr int kRootMinWidth = 140;
constexpr int kSubmenuMinWidth = 100;

int SmallRowOf(ui::ClassicLook look) { return look == ui::ClassicLook::WinXp ? kXpRow : kSmallRow; }

int SmallIconLeftOf(ui::ClassicLook look) {
  return look == ui::ClassicLook::WinXp ? kXpIconLeft : kSmallIconLeft;
}

ui::TextStyle TextStyleOf(ui::ClassicLook look) {
  switch (look) {
    case ui::ClassicLook::Win98:
      return ui::TextStyle::Classic98;
    case ui::ClassicLook::Win2000:
      return ui::TextStyle::Classic2000;
    case ui::ClassicLook::WinXp:
      break;
  }
  return ui::TextStyle::XpBody;
}

RECT WorkAreaAt(POINT point) {
  MONITORINFO monitor{};
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfoW(MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST), &monitor);
  return monitor.rcWork;
}

}  // namespace

ClassicMenu::ClassicMenu(HINSTANCE instance, HWND owner, ClassicMenuHost& host, bool root)
    : instance_(instance), owner_(owner), host_(host), root_(root) {
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = WindowProc;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.lpszClassName = kClassName;
  RegisterClassExW(&windowClass);  // fails harmlessly when already registered

  // Only the start menu itself takes focus; its submenus never do, so
  // clicking them doesn't close the menu.
  const DWORD exStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | (root ? 0 : WS_EX_NOACTIVATE);
  CreateWindowExW(exStyle, kClassName, L"", WS_POPUP, 0, 0, 0, 0, owner, nullptr, instance, this);
  if (!hwnd_) throw std::runtime_error("CreateWindowEx failed for a classic menu");
  canvas_ = std::make_unique<ui::Canvas>(hwnd_);
}

ClassicMenu::~ClassicMenu() {
  child_.reset();
  if (hwnd_) DestroyWindow(hwnd_);
}

bool ClassicMenu::IsOpen() const { return hwnd_ && IsWindowVisible(hwnd_); }

void ClassicMenu::ShowRoot(std::vector<ClassicEntry> entries, const MenuAnchor& anchor) {
  child_.reset();
  submenuOf_ = -1;
  selected_ = -1;
  entries_ = std::move(entries);
  canvas_->SetDpi(host_.dpi);
  Measure();
  const POINT corner = PlaceMenuWindow(anchor, width_, height_);
  SetWindowPos(hwnd_, HWND_TOPMOST, corner.x, corner.y, width_, height_, SWP_NOACTIVATE);
  canvas_->Resize(static_cast<UINT>(width_), static_cast<UINT>(height_));
  ShowWindow(hwnd_, SW_SHOW);
  shell::BringToFront(hwnd_);
}

void ClassicMenu::ShowBeside(std::vector<ClassicEntry> entries, const RECT& row) {
  entries_ = std::move(entries);
  selected_ = -1;
  canvas_->SetDpi(host_.dpi);
  Measure();
  // The first row lines up with the parent's row; the menu opens to the
  // right unless there is no room, and stays on the screen.
  const int inset = ScaleForDpi(kInset, host_.dpi);
  const RECT work = WorkAreaAt({row.right, row.top});
  const int left = static_cast<int>(work.left);
  const int top = static_cast<int>(work.top);
  int x = static_cast<int>(row.right);
  if (x + width_ > work.right) x = std::max(left, static_cast<int>(row.left) - width_);
  const int y = std::clamp(static_cast<int>(row.top) - inset, top,
                           std::max(top, static_cast<int>(work.bottom) - height_));
  SetWindowPos(hwnd_, HWND_TOPMOST, x, y, width_, height_, SWP_NOACTIVATE);
  canvas_->Resize(static_cast<UINT>(width_), static_cast<UINT>(height_));
  ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
}

void ClassicMenu::Hide() {
  KillTimer(hwnd_, kSubmenuTimer);
  if (child_) child_->Hide();
  ShowWindow(hwnd_, SW_HIDE);
}

void ClassicMenu::Repaint() {
  InvalidateRect(hwnd_, nullptr, FALSE);
  if (child_) child_->Repaint();
}

void ClassicMenu::Measure() {
  const auto s = [this](int value) { return ScaleForDpi(value, host_.dpi); };
  const ui::TextStyle style = TextStyleOf(host_.look);
  int textWidth = 0;
  for (const auto& entry : entries_) {
    if (!entry.separator) textWidth = std::max(textWidth, canvas_->MeasureText(entry.text, style));
  }
  const int rowHeight = s(root_ ? kLargeRow : SmallRowOf(host_.look));
  const int iconArea = root_ ? s(kLargeIconLeft + kLargeIcon + kLargeTextGap)
                             : s(SmallIconLeftOf(host_.look) + kSmallIcon + kSmallTextGap);
  const int left = s(kInset) + (root_ ? s(kBanner) : 0);
  const int columnWidth =
      std::max(iconArea + textWidth + s(kArrowArea), s(root_ ? kRootMinWidth : kSubmenuMinWidth));

  // Long submenus wrap into columns that fit the screen's height.
  const RECT work = WorkAreaAt({0, 0});
  const int maxBottom = root_ ? INT_MAX : static_cast<int>(work.bottom - work.top) - s(kInset);
  rows_.clear();
  int x = left;
  int y = s(kInset);
  int bottom = y;
  for (const auto& entry : entries_) {
    const int height = entry.separator ? s(kSeparator) : rowHeight;
    if (y + height > maxBottom && y > s(kInset)) {
      x += columnWidth;
      y = s(kInset);
    }
    rows_.push_back({x, y, x + columnWidth, y + height});
    y += height;
    bottom = std::max(bottom, y);
  }
  width_ = x + columnWidth + s(kInset);
  height_ = bottom + s(kInset);
}

void ClassicMenu::Paint() {
  PAINTSTRUCT paint;
  BeginPaint(hwnd_, &paint);
  canvas_->BeginFrame();
  const auto s = [this](int value) { return ScaleForDpi(value, host_.dpi); };
  canvas_->ClassicFace({0, 0, width_, height_}, host_.look);
  if (root_) {
    canvas_->ClassicBanner({s(kInset), s(kInset), s(kInset + kBanner), height_ - s(kInset)},
                           host_.banner);
  }
  const int iconSize = s(root_ ? kLargeIcon : kSmallIcon);
  const int iconLeft = s(root_ ? kLargeIconLeft : SmallIconLeftOf(host_.look));
  const int textGap = s(root_ ? kLargeTextGap : kSmallTextGap);
  for (size_t i = 0; i < entries_.size(); ++i) {
    const ClassicEntry& entry = entries_[i];
    const Rect& row = rows_[i];
    if (entry.separator) {
      canvas_->ClassicSeparator({row.left + s(1), row.top, row.right - s(1), row.bottom},
                                host_.look);
      continue;
    }
    const bool selected = static_cast<int>(i) == selected_ && !entry.disabled;
    canvas_->ClassicItem(row, host_.look, selected);
    const int iconTop = row.top + (row.Height() - iconSize) / 2;
    const Rect icon{row.left + iconLeft, iconTop, row.left + iconLeft + iconSize,
                    iconTop + iconSize};
    if (HICON handle = host_.icon(entry, iconSize)) canvas_->Icon(icon, handle);
    canvas_->ClassicText({icon.right + textGap, row.top, row.right - s(kArrowArea), row.bottom},
                         entry.text, host_.look, selected);
    if (entry.submenu) {
      canvas_->ClassicArrow({row.right - s(kArrowArea), row.top, row.right, row.bottom}, selected);
    }
  }
  canvas_->EndFrame();
  EndPaint(hwnd_, &paint);
}

int ClassicMenu::HitTest(POINT point) const {
  for (size_t i = 0; i < rows_.size(); ++i) {
    if (rows_[i].Contains(point.x, point.y)) return static_cast<int>(i);
  }
  return -1;
}

RECT ClassicMenu::RowScreenRect(int index) const {
  const Rect& row = rows_[static_cast<size_t>(index)];
  POINT topLeft{row.left, row.top};
  POINT bottomRight{row.right, row.bottom};
  ClientToScreen(hwnd_, &topLeft);
  ClientToScreen(hwnd_, &bottomRight);
  return {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
}

void ClassicMenu::Select(int index, bool openNow) {
  if (index == selected_ && (!openNow || submenuOf_ == index)) return;
  selected_ = index;
  KillTimer(hwnd_, kSubmenuTimer);
  if (submenuOf_ != index) CloseSubmenu();
  InvalidateRect(hwnd_, nullptr, FALSE);
  if (index < 0 || !entries_[static_cast<size_t>(index)].submenu || submenuOf_ == index) return;
  if (openNow) {
    OpenSubmenu();
  } else {
    UINT delay = 400;
    SystemParametersInfoW(SPI_GETMENUSHOWDELAY, 0, &delay, 0);
    SetTimer(hwnd_, kSubmenuTimer, delay, nullptr);
  }
}

void ClassicMenu::OpenSubmenu() {
  KillTimer(hwnd_, kSubmenuTimer);
  if (selected_ < 0) return;
  const ClassicEntry& entry = entries_[static_cast<size_t>(selected_)];
  if (!entry.submenu) return;
  std::vector<ClassicEntry> children = entry.submenu();
  if (children.empty()) {
    ClassicEntry empty;
    empty.text = ui::Tr(ui::StringId::ClassicEmpty);
    empty.disabled = true;
    children.push_back(std::move(empty));
  }
  child_ = std::make_unique<ClassicMenu>(instance_, owner_, host_, /*root=*/false);
  child_->parent_ = this;
  submenuOf_ = selected_;
  child_->ShowBeside(std::move(children), RowScreenRect(selected_));
}

void ClassicMenu::CloseSubmenu() {
  child_.reset();
  submenuOf_ = -1;
}

void ClassicMenu::Activate(int index) {
  if (index < 0) return;
  const ClassicEntry& entry = entries_[static_cast<size_t>(index)];
  if (entry.separator || entry.disabled) return;
  if (entry.submenu) {
    Select(index, /*openNow=*/true);
    return;
  }
  // Copied: invoking hides the menus, and the host may rebuild the entries.
  std::function<void()> action = entry.action;
  host_.invoke(std::move(action));
}

ClassicMenu* ClassicMenu::Deepest() { return child_ ? child_->Deepest() : this; }

int ClassicMenu::Step(int from, int direction) const {
  const int count = static_cast<int>(entries_.size());
  for (int step = 1; step <= count; ++step) {
    const int index =
        ((from < 0 && direction < 0 ? count : from) + direction * step + count * 2) % count;
    const ClassicEntry& entry = entries_[static_cast<size_t>(index)];
    if (!entry.separator && !entry.disabled) return index;
  }
  return from;
}

void ClassicMenu::OnKey(UINT key) {
  switch (key) {
    case VK_DOWN:
    case VK_UP:
      Select(Step(selected_, key == VK_DOWN ? 1 : -1), false);
      KillTimer(hwnd_, kSubmenuTimer);
      return;
    case VK_RIGHT:
      if (selected_ >= 0 && entries_[static_cast<size_t>(selected_)].submenu) {
        Select(selected_, true);
        if (child_) child_->Select(child_->Step(-1, 1), false);
      }
      return;
    case VK_LEFT:
    case VK_ESCAPE:
      if (ClassicMenu* parent = parent_) {
        // Closing the submenu destroys this level: touch nothing after.
        parent->CloseSubmenu();
        parent->Repaint();
      } else if (key == VK_ESCAPE) {
        host_.dismiss();
      }
      return;
    case VK_RETURN:
      Activate(selected_);
      return;
    default:
      return;
  }
}

LRESULT ClassicMenu::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  const POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
  switch (message) {
    case WM_PAINT:
      Paint();
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case WM_MOUSEACTIVATE:
      return root_ ? MA_ACTIVATE : MA_NOACTIVATE;
    case WM_ACTIVATE:
      if (root_ && LOWORD(wParam) == WA_INACTIVE && IsOpen()) host_.dismiss();
      return 0;
    case WM_KEYDOWN:
      Deepest()->OnKey(static_cast<UINT>(wParam));
      return 0;
    case WM_TIMER:
      if (wParam == kSubmenuTimer) OpenSubmenu();
      return 0;
    case WM_MOUSEMOVE: {
      if (!tracking_) {
        TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, hwnd_, 0};
        tracking_ = TrackMouseEvent(&track) != FALSE;
      }
      const int hit = HitTest(point);
      if (hit >= 0 && !entries_[static_cast<size_t>(hit)].separator) Select(hit, false);
      return 0;
    }
    case WM_MOUSELEAVE:
      tracking_ = false;
      // The row of an open submenu stays selected.
      if (!child_) Select(-1, false);
      return 0;
    case WM_LBUTTONUP:
      Activate(HitTest(point));
      return 0;
    case kIconsReady:
      Repaint();
      return 0;
  }
  return DefWindowProcW(hwnd_, message, wParam, lParam);
}

LRESULT CALLBACK ClassicMenu::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_NCCREATE) {
    auto* self =
        static_cast<ClassicMenu*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
    self->hwnd_ = hwnd;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
  }
  auto* self = reinterpret_cast<ClassicMenu*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return DefWindowProcW(hwnd, message, wParam, lParam);
  if (message == WM_NCDESTROY) {
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    self->hwnd_ = nullptr;
    return DefWindowProcW(hwnd, message, wParam, lParam);
  }
  return self->HandleMessage(message, wParam, lParam);
}

}  // namespace throwback
