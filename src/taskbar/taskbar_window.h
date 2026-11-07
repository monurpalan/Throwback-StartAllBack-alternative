#pragma once

#include <windows.h>

#include <memory>

#include "core/layout.h"
#include "core/settings.h"
#include "core/task_click.h"
#include "startmenu/start_menu_window.h"
#include "startmenu/win_key_hook.h"
#include "taskbar/appbar.h"
#include "taskbar/bar_view.h"
#include "taskbar/system_taskbar.h"
#include "taskbar/taskband_hook.h"
#include "taskbar/thumbnail_popup.h"
#include "taskbar/tray_host.h"
#include "taskbar/tray_overflow.h"
#include "taskbar/window_tracker.h"
#include "ui/canvas.h"
#include "ui/orb_window.h"
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
  static constexpr int kHitTrayArrow = -4;
  static constexpr int kHitNone = -3;
  static constexpr int kHitShowDesktop = -2;
  static constexpr int kHitStart = -1;
  // Notification icons on the bar hit as kHitTrayFirst - their place there.
  static constexpr int kHitTrayFirst = -100;
  static bool IsTrayHit(int hit) { return hit <= kHitTrayFirst; }
  // Posted by the Windows key hook.
  static constexpr UINT kStartMenuRequest = WM_APP + 3;

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static void CALLBACK OnWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object,
                                  LONG child, DWORD thread, DWORD time);
  LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT OnShellHook(WPARAM code, LPARAM param);
  // Shell hook, TaskbarCreated and ITaskbarList messages; true if handled.
  bool HandleShellMessage(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* result);
  void OnTaskbarCreated();
  void OnTimer(UINT_PTR timer);
  void OnDisplayChange();

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
  void CreateStartMenuForStyle();
  // The start button grows with its label in styles that have one.
  int StartButtonWidth() const;
  // Notification area (main bar only), in taskbar_tray.cpp.
  // Indices into TrayHost::VisibleIcons of the icons on the bar and of
  // those behind the arrow.
  struct TrayIcons {
    std::vector<size_t> shown;
    std::vector<size_t> hidden;
  };
  TrayIcons SplitTrayIcons() const;
  TrayAreaLayout TrayArea() const;
  int TrayWidth() const;
  int TrayHit(int x, int y) const;
  void SendTrayMouse(int hit, TrayHost::Mouse mouse);
  void PlaceTray();
  void AddTrayIcons(BarModel& model) const;
  void CreateTrayOverflow();
  void ToggleTrayOverflow();
  void MoveTrayIcon(size_t index, bool shown);
  // A press on a notification icon or the arrow; the release decides between
  // a click and dragging the icon off the bar. True if it was handled.
  void OnTrayPressed(int x, int y);
  bool OnTrayReleased(int pressed, int x, int y);

  // The WinEvent callback has no context parameter; the main bar hooks it.
  static TaskbarWindow* mainInstance_;

  BarManager& manager_;
  Settings& settings_;  // shared by every bar, owned by the manager
  HMONITOR monitor_;
  HINSTANCE module_;
  HWND hwnd_ = nullptr;
  UINT shellHookMessage_ = 0;
  UINT taskbarCreatedMessage_ = 0;
  HWND explorerTray_ = nullptr;  // to tell an Explorer restart from our own tray
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
  std::unique_ptr<ui::OrbWindow> orb_;  // null without the Windows 7 orb image
  std::unique_ptr<AppBar> appBar_;
  std::unique_ptr<SystemTaskbar> systemTaskbar_;
  std::unique_ptr<ThumbnailPopup> thumbnail_;
  std::unique_ptr<StartMenuWindow> startMenu_;
  StartStyle startStyle_ = StartStyle::Win7;  // of startMenu_
  std::unique_ptr<WinKeyHook> winKeyHook_;
  std::unique_ptr<TaskbandHook> taskbandHook_;  // main bar only
  std::unique_ptr<TrayHost> tray_;              // main bar only
  std::unique_ptr<TrayOverflow> trayOverflow_;  // main bar only
  POINT trayPress_{};
  int lastTrayClick_ = kHitNone;
  DWORD lastTrayClickTime_ = 0;
};

}  // namespace throwback
