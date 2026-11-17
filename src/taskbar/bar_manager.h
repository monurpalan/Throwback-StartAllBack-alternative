#pragma once

#include <windows.h>

#include <map>
#include <memory>
#include <string>

#include "core/settings.h"
#include "shell/badge_store.h"
#include "shell/settings_store.h"
#include "taskbar/fullscreen_watch.h"
#include "taskbar/task_status.h"

namespace throwback {

class TaskbarWindow;

// One bar per monitor. The bar on the primary monitor is the main one: it
// owns the Windows key and hiding Explorer's taskbar, and closing it ends the
// app. Settings are shared by every bar.
class BarManager {
 public:
  BarManager(HINSTANCE instance, const Settings& settings, shell::SettingsStore store);
  ~BarManager();
  BarManager(const BarManager&) = delete;
  BarManager& operator=(const BarManager&) = delete;

  Settings& CurrentSettings() { return settings_; }
  HWND MainWindow() const;

  // Adds and removes secondary bars to match the connected monitors.
  void Reconcile();
  // Saves edited settings (keeping the pins) and applies them to every bar.
  void ApplySettings(const Settings& edited);
  // Saves the start menu pins without this run's command-line overrides.
  void SavePinned();
  void SaveTrayChoices();
  // A window event every bar should refresh for.
  void RequestRefresh();
  // ITaskbarList3 messages, which reach the main bar only; true if handled.
  bool HandleTaskbarList(UINT message, WPARAM wParam, LPARAM lParam);
  const TaskStatus* StatusOf(HWND window) const { return status_.Find(window); }
  // App badges (WhatsApp's unread count) for the task buttons.
  const shell::BadgeStore& Badges() const { return badges_; }
  void RefreshBadges();
  void PruneStatus() { status_.Prune(); }

 private:
  HINSTANCE instance_;
  Settings settings_;
  shell::SettingsStore store_;
  TaskStatusStore status_;
  shell::BadgeStore badges_;
  std::unique_ptr<FullscreenWatch> fullscreen_;
  std::unique_ptr<TaskbarWindow> main_;
  std::map<std::wstring, std::unique_ptr<TaskbarWindow>> secondary_;
};

}  // namespace throwback
