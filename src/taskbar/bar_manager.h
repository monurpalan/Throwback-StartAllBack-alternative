#pragma once

#include <windows.h>

#include <map>
#include <memory>
#include <string>

#include "core/settings.h"
#include "shell/settings_store.h"

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
  // A window event every bar should refresh for.
  void RequestRefresh();

 private:
  HINSTANCE instance_;
  Settings settings_;
  shell::SettingsStore store_;
  std::unique_ptr<TaskbarWindow> main_;
  std::map<std::wstring, std::unique_ptr<TaskbarWindow>> secondary_;
};

}  // namespace throwback
