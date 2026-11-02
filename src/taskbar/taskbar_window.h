#pragma once

#include <windows.h>

#include <memory>

#include "core/layout.h"
#include "core/options.h"
#include "taskbar/appbar.h"
#include "taskbar/window_tracker.h"
#include "ui/canvas.h"

namespace throwback {

// The bar itself: owns the window and wires the tracker, AppBar and renderer together.
class TaskbarWindow {
 public:
  TaskbarWindow(HINSTANCE instance, const Options& options);
  ~TaskbarWindow();
  TaskbarWindow(const TaskbarWindow&) = delete;
  TaskbarWindow& operator=(const TaskbarWindow&) = delete;

 private:
  static constexpr int kHitNone = -2;
  static constexpr int kHitStart = -1;

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

  void Dock();
  void ApplyTheme();
  void RefreshTasks();
  void UpdateLayout();
  void Paint();
  int HitTest(int x, int y) const;
  void OnMouseMove(int x, int y);
  void OnClick(int x, int y);
  void OnContextMenu(int x, int y);
  void ActivateTask(HWND task);

  Options options_;
  HWND hwnd_ = nullptr;
  UINT shellHookMessage_ = 0;
  unsigned dpi_ = 96;
  int hovered_ = kHitNone;
  bool trackingMouse_ = false;
  BarLayout layout_;
  std::unique_ptr<WindowTracker> tracker_;
  std::unique_ptr<ui::Canvas> canvas_;
  std::unique_ptr<AppBar> appBar_;
};

}  // namespace throwback
