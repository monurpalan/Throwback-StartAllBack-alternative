#include "taskbar/bar_manager.h"

#include <utility>
#include <vector>

#include "core/monitor_diff.h"
#include "taskbar/taskbar_window.h"
#include "ui/strings.h"

namespace throwback {
namespace {

struct Monitor {
  std::wstring name;
  HMONITOR handle = nullptr;
};

BOOL CALLBACK CollectMonitor(HMONITOR handle, HDC, LPRECT, LPARAM param) {
  MONITORINFOEXW info{};
  info.cbSize = sizeof(info);
  if (GetMonitorInfoW(handle, &info) && (info.dwFlags & MONITORINFOF_PRIMARY) == 0) {
    reinterpret_cast<std::vector<Monitor>*>(param)->push_back({info.szDevice, handle});
  }
  return TRUE;
}

// Every monitor except the primary one, which the main bar covers.
std::vector<Monitor> SecondaryMonitors() {
  std::vector<Monitor> monitors;
  EnumDisplayMonitors(nullptr, nullptr, CollectMonitor, reinterpret_cast<LPARAM>(&monitors));
  return monitors;
}

}  // namespace

BarManager::BarManager(HINSTANCE instance, const Settings& settings, shell::SettingsStore store)
    : instance_(instance), settings_(settings), store_(std::move(store)) {
  main_ = std::make_unique<TaskbarWindow>(instance_, *this, nullptr);
  Reconcile();
}

BarManager::~BarManager() {
  secondary_.clear();
  main_.reset();
}

HWND BarManager::MainWindow() const { return main_ ? main_->Hwnd() : nullptr; }

void BarManager::Reconcile() {
  const std::vector<Monitor> monitors = SecondaryMonitors();
  std::vector<std::wstring> current;
  for (const auto& [name, bar] : secondary_) current.push_back(name);
  std::vector<std::wstring> wanted;
  for (const auto& monitor : monitors) wanted.push_back(monitor.name);

  const MonitorDiff diff = DiffMonitors(current, wanted);
  for (const auto& name : diff.removed) secondary_.erase(name);
  for (const auto& monitor : monitors) {
    auto& bar = secondary_[monitor.name];
    if (bar) {
      bar->SetMonitor(monitor.handle);  // handles change across display changes
    } else {
      bar = std::make_unique<TaskbarWindow>(instance_, *this, monitor.handle);
    }
  }
}

void BarManager::ApplySettings(const Settings& edited) {
  // Pins are changed in the start menu, not in the dialog, so they are kept.
  Settings next = edited;
  next.pinned = settings_.pinned;
  store_.Save(next);
  const bool moved = next.edge != settings_.edge;
  settings_ = next;

  ui::SetLanguage(ui::LanguageFor(settings_.language));
  main_->OnSettingsChanged(moved);
  for (const auto& [name, bar] : secondary_) bar->OnSettingsChanged(moved);
}

void BarManager::SavePinned() {
  // settings_ also holds this run's command-line overrides, which must not
  // become the saved settings.
  Settings saved = store_.Load();
  saved.pinned = settings_.pinned;
  store_.Save(saved);
}

void BarManager::RequestRefresh() {
  if (main_) main_->RequestRefresh();
  for (const auto& [name, bar] : secondary_) bar->RequestRefresh();
}

}  // namespace throwback
