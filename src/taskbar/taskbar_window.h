#pragma once

#include <windows.h>

#include <memory>

#include "core/layout.h"
#include "core/options.h"
#include "core/task_click.h"
#include "taskbar/appbar.h"
#include "taskbar/thumbnail_popup.h"
#include "taskbar/window_tracker.h"
#include "ui/canvas.h"
#include "ui/theme.h"

namespace throwback {

// The bar itself: owns the window and wires the tracker, AppBar and renderer together.
class TaskbarWindow {
 public:
  TaskbarWindow(HINSTANCE instance, const Options& options);
  ~TaskbarWindow();
  TaskbarWindow(const TaskbarWindow&) = delete;
  TaskbarWindow& operator=(const TaskbarWindow&) = delete;

 private:
  static constexpr int kHitNone = -3;
  static constexpr int kHitShowDesktop = -2;
  static constexpr int kHitStart = -1;

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static void CALLBACK OnWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object,
                                  LONG child, DWORD thread, DWORD time);
  LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT OnShellHook(WPARAM code, LPARAM param);
  void OnTimer(UINT_PTR timer);

  void RegisterAppBar();
  void Dock();
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

  // The WinEvent callback has no context parameter; there is one bar per process.
  static TaskbarWindow* instance_;

  Options options_;
  HINSTANCE module_;
  HWND hwnd_ = nullptr;
  UINT shellHookMessage_ = 0;
  UINT taskbarCreatedMessage_ = 0;
  HWINEVENTHOOK cloakHook_ = nullptr;
  bool refreshPending_ = false;
  unsigned dpi_ = 96;
  ui::Theme theme_ = ui::MakeTheme(false);
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
  std::unique_ptr<ThumbnailPopup> thumbnail_;
};

}  // namespace throwback
