#include "taskbar/taskbar_window.h"

#include <windowsx.h>

#include <algorithm>
#include <cwchar>
#include <exception>
#include <stdexcept>
#include <string>

#include "taskbar/bar_view.h"
#include "ui/theme.h"

namespace throwback {
namespace {

constexpr wchar_t kClassName[] = L"ThrowbackTaskbar";
constexpr UINT kAppBarCallback = WM_APP + 1;
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

void OpenSystemStartMenu() {
  INPUT inputs[2] = {};
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = VK_LWIN;
  inputs[1] = inputs[0];
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(2, inputs, sizeof(INPUT));
}

}  // namespace

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

  dpi_ = GetDpiForWindow(hwnd_);
  tracker_ = std::make_unique<WindowTracker>(hwnd_);
  canvas_ = std::make_unique<ui::Canvas>(hwnd_);
  canvas_->SetDpi(dpi_);
  ApplyTheme();
  appBar_ = std::make_unique<AppBar>(hwnd_, kAppBarCallback);
  Dock();

  shellHookMessage_ = RegisterWindowMessageW(L"SHELLHOOK");
  RegisterShellHookWindow(hwnd_);
  SetTimer(hwnd_, kRefreshTimer, kRefreshIntervalMs, nullptr);

  RefreshTasks();
  ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
}

TaskbarWindow::~TaskbarWindow() {
  if (hwnd_) DestroyWindow(hwnd_);
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

LRESULT TaskbarWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  if (shellHookMessage_ != 0 && message == shellHookMessage_) {
    if ((wParam & 0x7FFF) == HSHELL_REDRAW)
      tracker_->InvalidateIcon(reinterpret_cast<HWND>(lParam));
    RefreshTasks();
    return 0;
  }

  switch (message) {
    case kAppBarCallback:
      appBar_->HandleCallback(wParam, lParam);
      return 0;
    case WM_TIMER:
      // Fallback for changes the shell hook doesn't report; also ticks the clock.
      RefreshTasks();
      return 0;
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
    case WM_LBUTTONUP:
      OnClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return 0;
    case WM_RBUTTONUP:
      OnContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return 0;
    case WM_DESTROY:
      KillTimer(hwnd_, kRefreshTimer);
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

void TaskbarWindow::OnClick(int x, int y) {
  const int hit = HitTest(x, y);
  if (hit == kHitStart) {
    OpenSystemStartMenu();
  } else if (hit >= 0) {
    ActivateTask(tracker_->Items()[static_cast<size_t>(hit)].hwnd);
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

void TaskbarWindow::ActivateTask(HWND task) {
  // Classic behaviour: clicking the active window's button minimizes it.
  if (task == GetForegroundWindow() && !IsIconic(task)) {
    ShowWindowAsync(task, SW_MINIMIZE);
    return;
  }
  if (IsIconic(task)) ShowWindowAsync(task, SW_RESTORE);
  SetForegroundWindow(task);
}

}  // namespace throwback
