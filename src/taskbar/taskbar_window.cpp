#include "taskbar/taskbar_window.h"

#include <windowsx.h>

#include <algorithm>
#include <cwchar>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

#include "taskbar/bar_manager.h"
#include "taskbar/bar_view.h"
#include "taskbar/explorer_tray.h"
#include "taskbar/window_actions.h"
#include "ui/glass.h"
#include "ui/strings.h"

namespace throwback {
namespace {

constexpr wchar_t kClassName[] = L"ThrowbackTaskbar";
constexpr UINT kAppBarCallback = WM_APP + 1;
constexpr UINT kRefreshRequest = WM_APP + 2;
constexpr UINT_PTR kRefreshTimer = 1;
constexpr UINT_PTR kThumbnailTimer = 2;
constexpr UINT kRefreshIntervalMs = 1000;
constexpr int kBarHeight = 40;

// Undocumented but stable shell hook codes (Windows 8 and later).
constexpr WPARAM kShellWindowReplaced = 13;
constexpr WPARAM kShellWindowReplacing = 14;
constexpr WPARAM kShellFullscreenEnter = 53;
constexpr WPARAM kShellFullscreenExit = 54;

constexpr UINT kMenuExit = 1;
constexpr UINT kMenuProperties = 2;

// At 96 DPI; width and height come from the window at layout time.
constexpr BarMetrics kBaseMetrics = {
    .startButtonWidth = 56,
    .clockWidth = 80,
    .showDesktopWidth = 16,
    .minButtonWidth = 64,
    .maxButtonWidth = 160,
    .spacing = ui::kSpace2,
    .padding = ui::kSpace2,
};

std::wstring FormatTime() {
  wchar_t buffer[64] = {};
  GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, nullptr, nullptr, buffer, 64);
  return buffer;
}

std::wstring FormatDate() {
  wchar_t buffer[64] = {};
  GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, nullptr, nullptr, buffer, 64, nullptr);
  return buffer;
}

UINT HoverDelayMs() {
  UINT delay = 400;
  SystemParametersInfoW(SPI_GETMOUSEHOVERTIME, 0, &delay, 0);
  return delay;
}

}  // namespace

TaskbarWindow* TaskbarWindow::mainInstance_ = nullptr;

TaskbarWindow::TaskbarWindow(HINSTANCE instance, BarManager& manager, HMONITOR monitor)
    : manager_(manager),
      settings_(manager.CurrentSettings()),
      monitor_(monitor),
      module_(instance) {
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = WindowProc;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.lpszClassName = kClassName;
  RegisterClassExW(&windowClass);

  // Tool window: no taskbar button of its own. No-activate: clicking the bar
  // must not steal focus from the window whose button was clicked.
  CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, kClassName, L"Throwback",
                  WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, instance, this);
  if (!hwnd_) throw std::runtime_error("CreateWindowEx failed");
  if (IsMain()) mainInstance_ = this;

  dpi_ = GetDpiForWindow(hwnd_);
  tracker_ = std::make_unique<WindowTracker>(hwnd_);
  tracker_->SetMonitor(monitor_);
  canvas_ = std::make_unique<ui::Canvas>(hwnd_);
  canvas_->SetDpi(dpi_);
  orb_ = ui::OrbWindow::Create(instance, hwnd_);
  ui::EnableGlass(hwnd_, ui::GlassCorners::Square);
  thumbnail_ = std::make_unique<ThumbnailPopup>(instance, hwnd_);
  CreateStartMenuForStyle();
  ApplyWindowsKey();
  ApplyTheme();
  RegisterAppBar();
  if (IsMain()) {
    SystemTaskbar::RecoverFromCrash();
    taskbandHook_ = std::make_unique<TaskbandHook>(hwnd_);
    tray_ = std::make_unique<TrayHost>(instance, hwnd_, [this] {
      UpdateLayout();
      if (trayOverflow_) trayOverflow_->Refresh();
    });
    CreateTrayOverflow();
  }
  ApplySystemTaskbar();

  shellHookMessage_ = RegisterWindowMessageW(L"SHELLHOOK");
  taskbarCreatedMessage_ = RegisterWindowMessageW(L"TaskbarCreated");
  explorerTray_ = ExplorerTray();
  RegisterShellHookWindow(hwnd_);
  // Store apps and virtual desktop switches cloak and uncloak windows
  // without any shell hook message.
  if (IsMain()) {
    cloakHook_ = SetWinEventHook(EVENT_OBJECT_CLOAKED, EVENT_OBJECT_UNCLOAKED, nullptr, OnWinEvent,
                                 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
  }
  SetTimer(hwnd_, kRefreshTimer, kRefreshIntervalMs, nullptr);

  RefreshTasks();
  ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
}

TaskbarWindow::~TaskbarWindow() {
  winKeyHook_.reset();
  startMenu_.reset();
  thumbnail_.reset();
  if (hwnd_) DestroyWindow(hwnd_);
  if (mainInstance_ == this) mainInstance_ = nullptr;
}

LRESULT CALLBACK TaskbarWindow::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_NCCREATE) {
    auto* self =
        static_cast<TaskbarWindow*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
    self->hwnd_ = hwnd;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
  }
  auto* self = reinterpret_cast<TaskbarWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return DefWindowProcW(hwnd, message, wParam, lParam);

  // Exceptions must not unwind through the system's message dispatch.
  try {
    return self->HandleMessage(message, wParam, lParam);
  } catch (const std::exception& error) {
    OutputDebugStringA(error.what());
    return DefWindowProcW(hwnd, message, wParam, lParam);
  }
}

void CALLBACK TaskbarWindow::OnWinEvent(HWINEVENTHOOK, DWORD, HWND hwnd, LONG object, LONG child,
                                        DWORD, DWORD) {
  TaskbarWindow* self = mainInstance_;
  if (!self || !self->hwnd_ || !hwnd || object != OBJID_WINDOW || child != CHILDID_SELF) return;
  self->manager_.RequestRefresh();
}

void TaskbarWindow::RequestRefresh() {
  if (!hwnd_ || refreshPending_) return;
  refreshPending_ = true;
  PostMessageW(hwnd_, kRefreshRequest, 0, 0);
}

void TaskbarWindow::OnDisplayChange() {
  Dock();
  // Monitors may have come or gone; the main bar keeps one bar on each.
  if (IsMain()) manager_.Reconcile();
}

void TaskbarWindow::SetMonitor(HMONITOR monitor) {
  if (monitor == monitor_) return;
  monitor_ = monitor;
  tracker_->SetMonitor(monitor);
  Dock();
  RefreshTasks();
}

void TaskbarWindow::OnTaskbarCreated() {
  // Our own notification area announces itself with this message too; only
  // a new Explorer taskbar means Explorer restarted and forgot every AppBar.
  const HWND explorer = ExplorerTray();
  if (explorer == explorerTray_) return;
  explorerTray_ = explorer;
  RegisterAppBar();
  if (systemTaskbar_) systemTaskbar_->Enforce();
  if (taskbandHook_) taskbandHook_->Reapply();
  PlaceTray();  // stay above the new Explorer taskbar
  RefreshTasks();
}

bool TaskbarWindow::HandleShellMessage(UINT message, WPARAM wParam, LPARAM lParam,
                                       LRESULT* result) {
  *result = 0;
  if (shellHookMessage_ != 0 && message == shellHookMessage_) {
    *result = OnShellHook(wParam, lParam);
    return true;
  }
  if (taskbarCreatedMessage_ != 0 && message == taskbarCreatedMessage_) {
    OnTaskbarCreated();
    return true;
  }
  return IsMain() && manager_.HandleTaskbarList(message, wParam, lParam);
}

LRESULT TaskbarWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  if (LRESULT result = 0; HandleShellMessage(message, wParam, lParam, &result)) return result;

  switch (message) {
    case kAppBarCallback:
      appBar_->HandleCallback(wParam, lParam);
      return 0;
    case kRefreshRequest:
      refreshPending_ = false;
      RefreshTasks();
      return 0;
    case kStartMenuRequest:
      ToggleStartMenu();
      return 0;
    case kOpenSettingsMessage:
      OpenSettings();
      return 0;
    case WM_TIMER:
      OnTimer(wParam);
      return 0;
    case WM_ACTIVATE:
      if (appBar_) appBar_->OnActivated();
      break;
    case WM_WINDOWPOSCHANGED:
      if (appBar_) appBar_->OnWindowPosChanged();
      // The orb follows the bar: painting moves it, hiding hides it.
      if (orb_ && (reinterpret_cast<const WINDOWPOS*>(lParam)->flags & SWP_HIDEWINDOW))
        orb_->Hide();
      InvalidateRect(hwnd_, nullptr, FALSE);
      break;  // DefWindowProc turns this into WM_SIZE
    case WM_SIZE:
      if (canvas_) canvas_->Resize(LOWORD(lParam), HIWORD(lParam));
      UpdateLayout();
      return 0;
    case WM_DPICHANGED:
      dpi_ = HIWORD(wParam);
      canvas_->SetDpi(dpi_);
      Dock();
      return 0;
    case WM_SETTINGCHANGE:
      // Sent when the user switches between light and dark mode.
      if (lParam &&
          std::wcscmp(reinterpret_cast<const wchar_t*>(lParam), L"ImmersiveColorSet") == 0) {
        ApplyTheme();
      }
      return 0;
    case WM_DISPLAYCHANGE:
      OnDisplayChange();
      return 0;
    case WM_CLOSE:
      // Closing any bar (its menu, taskkill, installers) ends the app.
      if (IsMain()) break;
      PostMessageW(manager_.MainWindow(), WM_CLOSE, 0, 0);
      return 0;
    case WM_PAINT:
      Paint();
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case WM_MOUSEACTIVATE:
      return MA_NOACTIVATE;
    case WM_MOUSEMOVE:
      OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return 0;
    case WM_MOUSELEAVE:
      OnMouseLeave();
      return 0;
    case WM_LBUTTONDOWN:
      OnButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return 0;
    case WM_LBUTTONUP:
      OnButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return 0;
    case WM_RBUTTONUP:
      OnContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return 0;
    case WM_ENDSESSION:
      // Sign-out or shutdown: give the screen space back before we are killed.
      if (wParam) DestroyWindow(hwnd_);
      return 0;
    case WM_DESTROY:
      winKeyHook_.reset();  // give the Windows key back right away
      KillTimer(hwnd_, kRefreshTimer);
      KillTimer(hwnd_, kThumbnailTimer);
      if (cloakHook_) UnhookWinEvent(cloakHook_);
      DeregisterShellHookWindow(hwnd_);
      appBar_.reset();  // gives the reserved screen space back
      orb_.reset();
      taskbandHook_.reset();
      trayOverflow_.reset();
      tray_.reset();
      systemTaskbar_.reset();
      if (IsMain()) PostQuitMessage(0);
      return 0;
    case WM_NCDESTROY: {
      const HWND hwnd = hwnd_;
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
      hwnd_ = nullptr;
      return DefWindowProcW(hwnd, message, wParam, lParam);
    }
  }
  return DefWindowProcW(hwnd_, message, wParam, lParam);
}

LRESULT TaskbarWindow::OnShellHook(WPARAM code, LPARAM param) {
  auto window = reinterpret_cast<HWND>(param);
  switch (code) {
    case HSHELL_GETMINRECT: {
      // Aim the minimize and restore animations at our button.
      auto* info = reinterpret_cast<SHELLHOOKINFO*>(param);
      return info && ButtonScreenRect(info->hwnd, &info->rc) ? 1 : 0;
    }
    case kShellFullscreenEnter:
    case kShellFullscreenExit:
      if (appBar_) appBar_->SetFullscreenActive(code == kShellFullscreenEnter);
      return 0;
    case kShellWindowReplacing:
      replacingWindow_ = window;
      return 0;
    case kShellWindowReplaced:
      tracker_->ReplaceWindow(window, std::exchange(replacingWindow_, nullptr));
      break;
    case HSHELL_REDRAW:
      tracker_->InvalidateIcon(window);
      break;
  }

  // Refresh first so a window we haven't seen yet exists before its flags change.
  RefreshTasks();
  window = tracker_->ButtonOwner(window);
  if (!window) return 0;
  switch (code) {
    case HSHELL_FLASH:
      if (window != GetForegroundWindow()) tracker_->SetFlashing(window, true);
      break;
    case HSHELL_REDRAW:
    case HSHELL_WINDOWACTIVATED:
    case HSHELL_RUDEAPPACTIVATED:
      tracker_->SetFlashing(window, false);
      break;
  }
  return 0;
}

void TaskbarWindow::OnTimer(UINT_PTR timer) {
  if (timer == kThumbnailTimer) {
    KillTimer(hwnd_, kThumbnailTimer);
    if (hovered_ >= 0) ShowThumbnail(hovered_);
    return;
  }
  // Fallback for changes no event reports; also ticks the clock.
  RefreshTasks();
  if (systemTaskbar_) systemTaskbar_->Enforce();
  if (IsMain()) manager_.PruneStatus();
  if (tray_) {
    tray_->Prune();
    PlaceTray();  // Explorer's taskbar may have come above ours
  }
  if (thumbnail_->Task() && !tracker_->ButtonOwner(thumbnail_->Task())) HideThumbnail();
}

void TaskbarWindow::RegisterAppBar() {
  // Unregister first: a new registration followed by the old one's removal
  // would leave the window without a reservation.
  appBar_.reset();
  appBar_ = std::make_unique<AppBar>(hwnd_, kAppBarCallback);
  Dock();
}

void TaskbarWindow::Dock() {
  if (appBar_) appBar_->Dock(settings_.edge, ScaleForDpi(kBarHeight, dpi_), monitor_);
}

void TaskbarWindow::RefreshTasks() {
  if (!tracker_) return;
  if (tracker_->Refresh()) UpdateLayout();
  // Repaint even without list changes: the active window and clock may differ.
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void TaskbarWindow::UpdateLayout() {
  if (!tracker_) return;
  RECT client{};
  GetClientRect(hwnd_, &client);
  BarMetrics metrics = ScaleMetrics(kBaseMetrics, dpi_);
  metrics.barWidth = client.right - client.left;
  metrics.barHeight = client.bottom - client.top;
  metrics.trayWidth = TrayWidth();
  metrics.startButtonWidth = std::max(metrics.startButtonWidth, StartButtonWidth());
  layout_ = ComputeLayout(metrics, tracker_->Items().size());
  PlaceTray();
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void TaskbarWindow::Paint() {
  PAINTSTRUCT paint;
  BeginPaint(hwnd_, &paint);

  const std::wstring time = FormatTime();
  const std::wstring date = FormatDate();
  const HWND foreground = GetForegroundWindow();
  const auto& items = tracker_->Items();

  RECT client{};
  GetClientRect(hwnd_, &client);
  BarModel model{
      .bounds = {0, 0, client.right, client.bottom},
      .top = settings_.edge == Edge::Top,
      .startButton = layout_.startButton,
      .startHovered = hovered_ == kHitStart,
      .startOpen = startMenu_ && startMenu_->IsOpen(),
      .drawOrb = !orb_,
      .startStyle = settings_.startStyle,
      .clock = layout_.clock,
      .time = time,
      .date = date,
      .showDesktop = layout_.showDesktop,
      .showDesktopHovered = hovered_ == kHitShowDesktop,
  };
  for (size_t i = 0; i < layout_.taskButtons.size() && i < items.size(); ++i) {
    model.buttons.push_back({
        .rect = layout_.taskButtons[i],
        .title = items[i].title,
        .icon = items[i].icon,
        .active = items[i].hwnd == foreground,
        .flashing = items[i].flashing,
        .hovered = hovered_ == static_cast<int>(i),
    });
    if (const TaskStatus* status = manager_.StatusOf(items[i].hwnd)) {
      model.buttons.back().progress = status->progress;
      model.buttons.back().progressFraction = status->fraction;
      model.buttons.back().overlay = status->overlay;
    }
  }
  AddTrayIcons(model);
  canvas_->BeginFrame();
  DrawBar(*canvas_, model);
  canvas_->EndFrame();
  const bool orbStyle =
      settings_.startStyle == StartStyle::Win7 || settings_.startStyle == StartStyle::Vista;
  if (orb_ && !orbStyle) orb_->Hide();
  if (orb_ && orbStyle) {
    POINT center{(layout_.startButton.left + layout_.startButton.right) / 2,
                 (layout_.startButton.top + layout_.startButton.bottom) / 2};
    ClientToScreen(hwnd_, &center);
    orb_->Show(center, dpi_, OrbStateFor(model));
  }

  EndPaint(hwnd_, &paint);
}

int TaskbarWindow::HitTest(int x, int y) const {
  if (layout_.startButton.Contains(x, y)) return kHitStart;
  if (const int tray = TrayHit(x, y); tray != kHitNone) return tray;
  if (layout_.showDesktop.Contains(x, y)) return kHitShowDesktop;
  const size_t count = std::min(layout_.taskButtons.size(), tracker_->Items().size());
  for (size_t i = 0; i < count; ++i) {
    if (layout_.taskButtons[i].Contains(x, y)) return static_cast<int>(i);
  }
  return kHitNone;
}

bool TaskbarWindow::ButtonScreenRect(HWND task, RECT* rect) const {
  const auto& items = tracker_->Items();
  const size_t count = std::min(layout_.taskButtons.size(), items.size());
  for (size_t i = 0; i < count; ++i) {
    if (items[i].hwnd != task) continue;
    const Rect& button = layout_.taskButtons[i];
    POINT topLeft{button.left, button.top};
    POINT bottomRight{button.right, button.bottom};
    ClientToScreen(hwnd_, &topLeft);
    ClientToScreen(hwnd_, &bottomRight);
    *rect = {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
    return true;
  }
  return false;
}

void TaskbarWindow::OnMouseMove(int x, int y) {
  if (!trackingMouse_) {
    TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, hwnd_, 0};
    trackingMouse_ = TrackMouseEvent(&track) != FALSE;
  }
  const int hit = HitTest(x, y);
  if (hit == hovered_) return;
  hovered_ = hit;
  InvalidateRect(hwnd_, nullptr, FALSE);

  if (IsTrayHit(hit)) SendTrayMouse(hit, TrayHost::Mouse::Move);
  if (hit < 0) {
    HideThumbnail();
  } else if (thumbnail_->Task()) {
    ShowThumbnail(hit);  // already previewing: follow the mouse at once
  } else {
    SetTimer(hwnd_, kThumbnailTimer, HoverDelayMs(), nullptr);
  }
}

void TaskbarWindow::OnMouseLeave() {
  trackingMouse_ = false;
  hovered_ = kHitNone;
  HideThumbnail();
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void TaskbarWindow::OnButtonDown(int x, int y) {
  HideThumbnail();
  pressedHit_ = HitTest(x, y);
  pressedTask_ = nullptr;
  OnTrayPressed(x, y);
  if (pressedHit_ >= 0) {
    pressedTask_ = tracker_->Items()[static_cast<size_t>(pressedHit_)].hwnd;
    pressedState_ = ReadClickState(pressedTask_);
  }
}

void TaskbarWindow::OnButtonUp(int x, int y) {
  const int hit = HitTest(x, y);
  const int pressed = std::exchange(pressedHit_, kHitNone);
  if (OnTrayReleased(pressed, x, y)) return;
  if (hit != pressed) return;  // released somewhere else: cancel

  if (IsTrayHit(hit)) {
    SendTrayMouse(hit, TrayHost::Mouse::LeftClick);
  } else if (hit == kHitStart) {
    ToggleStartMenu();
  } else if (hit == kHitShowDesktop) {
    ToggleDesktop();
  } else if (hit >= 0 && tracker_->Items()[static_cast<size_t>(hit)].hwnd == pressedTask_) {
    ActivateTask(pressedTask_, pressedState_);
  }
}

void TaskbarWindow::OnContextMenu(int x, int y) {
  HideThumbnail();
  POINT point{x, y};
  ClientToScreen(hwnd_, &point);

  const int hit = HitTest(x, y);
  if (IsTrayHit(hit)) {
    SendTrayMouse(hit, TrayHost::Mouse::RightClick);
    return;
  }
  if (hit >= 0) {
    ShowWindowMenu(hwnd_, tracker_->Items()[static_cast<size_t>(hit)].hwnd, point);
    return;
  }

  HMENU menu = CreatePopupMenu();
  AppendMenuW(menu, MF_STRING, kMenuProperties,
              std::wstring(ui::Tr(ui::StringId::Properties)).c_str());
  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(menu, MF_STRING, kMenuExit,
              std::wstring(ui::Tr(ui::StringId::ExitThrowback)).c_str());
  // Required for the menu to close when the user clicks elsewhere.
  SetForegroundWindow(hwnd_);
  const auto command = static_cast<UINT>(TrackPopupMenu(
      menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, point.x, point.y, 0, hwnd_, nullptr));
  PostMessageW(hwnd_, WM_NULL, 0, 0);
  DestroyMenu(menu);
  if (command == kMenuExit) PostMessageW(hwnd_, WM_CLOSE, 0, 0);
  if (command == kMenuProperties) OpenSettings();
}

void TaskbarWindow::ShowThumbnail(int index) {
  const auto& items = tracker_->Items();
  if (index < 0 || static_cast<size_t>(index) >= items.size()) return;
  const TaskItem& item = items[static_cast<size_t>(index)];
  RECT anchor{};
  if (!ButtonScreenRect(item.hwnd, &anchor)) return;
  thumbnail_->Show({item.hwnd, item.title, item.icon}, anchor, settings_.edge == Edge::Top, dpi_);
}

void TaskbarWindow::ToggleStartMenu() {
  HideThumbnail();
  if (startMenu_->IsOpen()) {
    startMenu_->Close();
    return;
  }
  POINT topLeft{layout_.startButton.left, layout_.startButton.top};
  POINT bottomRight{layout_.startButton.right, layout_.startButton.bottom};
  ClientToScreen(hwnd_, &topLeft);
  ClientToScreen(hwnd_, &bottomRight);
  startMenu_->Open({topLeft.x, topLeft.y, bottomRight.x, bottomRight.y},
                   settings_.edge == Edge::Top, dpi_);
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void TaskbarWindow::HideThumbnail() {
  KillTimer(hwnd_, kThumbnailTimer);
  if (thumbnail_ && thumbnail_->Task()) thumbnail_->Hide();
}

}  // namespace throwback
