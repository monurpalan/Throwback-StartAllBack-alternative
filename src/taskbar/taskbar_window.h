#pragma once

#include <windows.h>

#include <memory>

#include "core/layout.h"
#include "core/settings.h"
#include "core/task_click.h"
#include "startmenu/start_menu.h"
#include "startmenu/win_key_hook.h"
#include "taskbar/appbar.h"
#include "taskbar/system_taskbar.h"
#include "taskbar/thumbnail_popup.h"
#include "taskbar/window_tracker.h"
#include "ui/canvas.h"
#include "ui/theme.h"

namespace throwback {

class BarManager;

// The bar itself: owns the window and wires the tracker, AppBar and renderer
// together. monitor is nullptr for the main bar on the primary monitor.
class TaskbarWindow {
 public:
  TaskbarWindow(HINSTANCE instance, BarManager& manager, HMONITOR monitor);
  ~TaskbarWindow();
  TaskbarWindow(const TaskbarWindow&) = delete;
  TaskbarWindow& operator=(const TaskbarWindow&) = delete;

  HWND Hwnd() const { return hwnd_; }
  void SetMonitor(HMONITOR monitor);
  void OnSettingsChanged(bool moved);
  // Refreshes the buttons soon; events arrive in bursts and share one refresh.
  void RequestRefresh();

 private:
  static constexpr int kHitNone = -3;
  static constexpr int kHitShowDesktop = -2;
  static constexpr int kHitStart = -1;
  // Posted by the Windows key hook.
  static constexpr UINT kStartMenuRequest = WM_APP + 3;

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static void CALLBACK OnWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object,
                                  LONG child, DWORD thread, DWORD time);
  LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT OnShellHook(WPARAM code, LPARAM param);
  void OnTimer(UINT_PTR timer);

  void RegisterAppBar();
  void Dock();
  void ApplySystemTaskbar();
  void ApplyWindowsKey();
  void OpenSettings();
  bool IsMain() const { return monitor_ == nullptr; }
  void ApplyTheme();
  void RefreshTasks();
  void UpdateLayout();
  void Paint();
  int HitTest(int x, int y) const;
  bool ButtonScreenRect(HWND task, RECT* rect) const;
  void OnMouseMove(int x, int y);
  void OnMouseLeave();
  void OnButtonDown(int x, int y);
  void OnButtonUp(int x, int y);
  void OnContextMenu(int x, int y);
  void ShowThumbnail(int index);
  void HideThumbnail();
  void ToggleStartMenu();

  // The WinEvent callback has no context parameter; the main bar hooks it.
  static TaskbarWindow* mainInstance_;

  BarManager& manager_;
  Settings& settings_;  // shared by every bar, owned by the manager
  HMONITOR monitor_;
  HINSTANCE module_;
  HWND hwnd_ = nullptr;
  UINT shellHookMessage_ = 0;
  UINT taskbarCreatedMessage_ = 0;
  HWINEVENTHOOK cloakHook_ = nullptr;
  bool refreshPending_ = false;
  unsigned dpi_ = 96;
  int hovered_ = kHitNone;
  bool trackingMouse_ = false;
  int pressedHit_ = kHitNone;
  HWND pressedTask_ = nullptr;
  TaskClickState pressedState_;
  // Set while Windows swaps a window: HSHELL_WINDOWREPLACING names the new
  // window, HSHELL_WINDOWREPLACED the old one.
  HWND replacingWindow_ = nullptr;
  BarLayout layout_;
  std::unique_ptr<WindowTracker> tracker_;
  std::unique_ptr<ui::Canvas> canvas_;
  std::unique_ptr<AppBar> appBar_;
  std::unique_ptr<SystemTaskbar> systemTaskbar_;
  std::unique_ptr<ThumbnailPopup> thumbnail_;
  std::unique_ptr<StartMenu> startMenu_;
  std::unique_ptr<WinKeyHook> winKeyHook_;
};

}  // namespace throwback
