#include "taskbar/taskbar_window.h"

#include <windowsx.h>

#include <algorithm>
#include <cwchar>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

#include "taskbar/bar_view.h"
#include "ui/theme.h"

namespace throwback {
namespace {

constexpr wchar_t kClassName[] = L"ThrowbackTaskbar";
constexpr UINT kAppBarCallback = WM_APP + 1;
constexpr UINT kRefreshRequest = WM_APP + 2;
constexpr UINT_PTR kRefreshTimer = 1;
constexpr UINT kRefreshIntervalMs = 1000;
constexpr int kBarHeight = 40;

constexpr UINT kMenuCloseWindow = 1;
constexpr UINT kMenuExit = 2;

// At 96 DPI; width and height come from the window at layout time.
constexpr BarMetrics kBaseMetrics = {
    .startButtonWidth = 48,
    .clockWidth = 88,
    .minButtonWidth = 64,
    .maxButtonWidth = 200,
    .spacing = ui::kSpace4,
    .padding = ui::kSpace4,
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

void SendKeyPress(WORD key) {
  INPUT inputs[2] = {};
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = key;
  inputs[1] = inputs[0];
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(2, inputs, sizeof(INPUT));
}

void OpenSystemStartMenu() { SendKeyPress(VK_LWIN); }

void BringToFront(HWND window) {
  if (SetForegroundWindow(window)) return;
  // Only the process that received the last input may change the foreground
  // window. Pressing an unassigned key gives that right back to us.
  SendKeyPress(0xE8);
  SetForegroundWindow(window);
}

TaskClickState ReadClickState(HWND task) {
  const LONG_PTR style = GetWindowLongPtrW(task, GWL_STYLE);
  return {
      .wasActive = task == GetForegroundWindow(),
      .minimized = IsIconic(task) != FALSE,
      // A disabled window has a modal dialog open and must not be minimized.
      .canMinimize = (style & WS_MINIMIZEBOX) != 0 && IsWindowEnabled(task),
  };
}

}  // namespace

TaskbarWindow* TaskbarWindow::instance_ = nullptr;

TaskbarWindow::TaskbarWindow(HINSTANCE instance, const Options& options) : options_(options) {
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
  instance_ = this;

  dpi_ = GetDpiForWindow(hwnd_);
  tracker_ = std::make_unique<WindowTracker>(hwnd_);
  canvas_ = std::make_unique<ui::Canvas>(hwnd_);
  canvas_->SetDpi(dpi_);
  ApplyTheme();
  RegisterAppBar();

  shellHookMessage_ = RegisterWindowMessageW(L"SHELLHOOK");
  taskbarCreatedMessage_ = RegisterWindowMessageW(L"TaskbarCreated");
  RegisterShellHookWindow(hwnd_);
  // Store apps and virtual desktop switches cloak and uncloak windows
  // without any shell hook message.
  cloakHook_ = SetWinEventHook(EVENT_OBJECT_CLOAKED, EVENT_OBJECT_UNCLOAKED, nullptr, OnWinEvent, 0,
                               0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
  SetTimer(hwnd_, kRefreshTimer, kRefreshIntervalMs, nullptr);

  RefreshTasks();
  ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
}

TaskbarWindow::~TaskbarWindow() {
  if (hwnd_) DestroyWindow(hwnd_);
  instance_ = nullptr;
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
  TaskbarWindow* self = instance_;
  if (!self || !self->hwnd_ || !hwnd || object != OBJID_WINDOW || child != CHILDID_SELF) return;
  // Events arrive in bursts; one refresh covers all of them.
  if (!self->refreshPending_) {
    self->refreshPending_ = true;
    PostMessageW(self->hwnd_, kRefreshRequest, 0, 0);
  }
}

LRESULT TaskbarWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  if (shellHookMessage_ != 0 && message == shellHookMessage_) return OnShellHook(wParam, lParam);
  if (taskbarCreatedMessage_ != 0 && message == taskbarCreatedMessage_) {
    // Explorer restarted and forgot every AppBar.
    RegisterAppBar();
    RefreshTasks();
    return 0;
  }

  switch (message) {
    case kAppBarCallback:
      appBar_->HandleCallback(wParam, lParam);
      return 0;
    case kRefreshRequest:
      refreshPending_ = false;
      RefreshTasks();
      return 0;
    case WM_TIMER:
      // Fallback for changes no event reports; also ticks the clock.
      RefreshTasks();
      return 0;
    case WM_ACTIVATE:
      if (appBar_) appBar_->OnActivated();
      break;
    case WM_WINDOWPOSCHANGED:
      if (appBar_) appBar_->OnWindowPosChanged();
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
      Dock();
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
      trackingMouse_ = false;
      hovered_ = kHitNone;
      InvalidateRect(hwnd_, nullptr, FALSE);
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
    case WM_DESTROY:
      KillTimer(hwnd_, kRefreshTimer);
      if (cloakHook_) UnhookWinEvent(cloakHook_);
      DeregisterShellHookWindow(hwnd_);
      appBar_.reset();  // gives the reserved screen space back
      PostQuitMessage(0);
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
  if (code == HSHELL_GETMINRECT) {
    // Aim the minimize and restore animations at our button.
    auto* info = reinterpret_cast<SHELLHOOKINFO*>(param);
    return info && ButtonScreenRect(info->hwnd, &info->rc) ? 1 : 0;
  }

  auto window = reinterpret_cast<HWND>(param);
  if (code == HSHELL_REDRAW) tracker_->InvalidateIcon(window);
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

void TaskbarWindow::RegisterAppBar() {
  // Unregister first: a new registration followed by the old one's removal
  // would leave the window without a reservation.
  appBar_.reset();
  appBar_ = std::make_unique<AppBar>(hwnd_, kAppBarCallback);
  Dock();
}

void TaskbarWindow::Dock() {
  if (appBar_) appBar_->Dock(options_.edge, ScaleForDpi(kBarHeight, dpi_));
}

void TaskbarWindow::ApplyTheme() {
  const bool light = options_.theme == ThemeMode::Light ||
                     (options_.theme == ThemeMode::System && ui::SystemUsesLightTheme());
  canvas_->SetTheme(ui::MakeTheme(light));
  InvalidateRect(hwnd_, nullptr, FALSE);
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
  layout_ = ComputeLayout(metrics, tracker_->Items().size());
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void TaskbarWindow::Paint() {
  PAINTSTRUCT paint;
  BeginPaint(hwnd_, &paint);

  const std::wstring time = FormatTime();
  const std::wstring date = FormatDate();
  const HWND foreground = GetForegroundWindow();
  const auto& items = tracker_->Items();

  BarModel model{
      .startButton = layout_.startButton,
      .startHovered = hovered_ == kHitStart,
      .clock = layout_.clock,
      .time = time,
      .date = date,
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
  }
  canvas_->BeginFrame();
  DrawBar(*canvas_, model);
  canvas_->EndFrame();

  EndPaint(hwnd_, &paint);
}

int TaskbarWindow::HitTest(int x, int y) const {
  if (layout_.startButton.Contains(x, y)) return kHitStart;
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
  if (hit != hovered_) {
    hovered_ = hit;
    InvalidateRect(hwnd_, nullptr, FALSE);
  }
}

void TaskbarWindow::OnButtonDown(int x, int y) {
  pressedHit_ = HitTest(x, y);
  pressedTask_ = nullptr;
  if (pressedHit_ >= 0) {
    pressedTask_ = tracker_->Items()[static_cast<size_t>(pressedHit_)].hwnd;
    pressedState_ = ReadClickState(pressedTask_);
  }
}

void TaskbarWindow::OnButtonUp(int x, int y) {
  const int hit = HitTest(x, y);
  const int pressed = std::exchange(pressedHit_, kHitNone);
  if (hit != pressed) return;  // released somewhere else: cancel

  if (hit == kHitStart) {
    OpenSystemStartMenu();
  } else if (hit >= 0 && tracker_->Items()[static_cast<size_t>(hit)].hwnd == pressedTask_) {
    ActivateTask(pressedTask_, pressedState_);
  }
}

void TaskbarWindow::OnContextMenu(int x, int y) {
  const int hit = HitTest(x, y);
  HMENU menu = CreatePopupMenu();
  if (hit >= 0) {
    AppendMenuW(menu, MF_STRING, kMenuCloseWindow, L"Close window");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  }
  AppendMenuW(menu, MF_STRING, kMenuExit, L"Exit Throwback");

  POINT point{x, y};
  ClientToScreen(hwnd_, &point);
  // Required for the menu to close when the user clicks elsewhere.
  SetForegroundWindow(hwnd_);
  const UINT command = static_cast<UINT>(TrackPopupMenu(
      menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, point.x, point.y, 0, hwnd_, nullptr));
  PostMessageW(hwnd_, WM_NULL, 0, 0);
  DestroyMenu(menu);

  if (command == kMenuCloseWindow && hit >= 0 &&
      static_cast<size_t>(hit) < tracker_->Items().size()) {
    PostMessageW(tracker_->Items()[static_cast<size_t>(hit)].hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
  } else if (command == kMenuExit) {
    DestroyWindow(hwnd_);
  }
}

void TaskbarWindow::ActivateTask(HWND task, const TaskClickState& state) {
  switch (DecideTaskClick(state)) {
    case TaskClickAction::Minimize: {
      DWORD processId = 0;
      GetWindowThreadProcessId(task, &processId);
      AllowSetForegroundWindow(processId);
      PostMessageW(task, WM_SYSCOMMAND, SC_MINIMIZE, 0);
      return;
    }
    case TaskClickAction::Restore: {
      // The app's own restore keeps maximized windows maximized; the timeout
      // keeps a hung app from freezing the bar.
      DWORD_PTR result = 0;
      SendMessageTimeoutW(task, WM_SYSCOMMAND, SC_RESTORE, 0, SMTO_ABORTIFHUNG, 200, &result);
      break;
    }
    case TaskClickAction::Activate:
      break;
    case TaskClickAction::None:
      return;
  }
  // Activate the open dialog, if any, rather than the window it blocks.
  BringToFront(GetLastActivePopup(task));
}

}  // namespace throwback
