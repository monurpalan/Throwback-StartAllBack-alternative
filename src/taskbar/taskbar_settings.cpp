// The bar's settings: applying them live, saving them and the properties
// dialog.
#include "taskbar/settings_dialog.h"
#include "taskbar/taskbar_window.h"
#include "taskbar/window_actions.h"
#include "ui/strings.h"

namespace throwback {

void TaskbarWindow::ApplyTheme() {
  const bool light = settings_.theme == ThemeMode::Light ||
                     (settings_.theme == ThemeMode::System && ui::SystemUsesLightTheme());
  // The Aero glass looks the same either way; only native menus follow it.
  ApplyMenuTheme(!light);
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void TaskbarWindow::SavePinned() {
  // Only the pins are saved: settings_ also holds this run's command-line
  // overrides, which must not become the saved settings.
  Settings saved = store_.Load();
  saved.pinned = settings_.pinned;
  store_.Save(saved);
}

void TaskbarWindow::ApplyWindowsKey() {
  // Without the hook the Windows key and Ctrl+Esc open Windows' own menu.
  if (settings_.windowsKeyOpensMenu && !winKeyHook_) {
    winKeyHook_ = std::make_unique<WinKeyHook>(hwnd_, kStartMenuRequest);
  } else if (!settings_.windowsKeyOpensMenu) {
    winKeyHook_.reset();
  }
}

void TaskbarWindow::ApplySystemTaskbar() {
  if (settings_.hideSystemTaskbar && !systemTaskbar_) {
    systemTaskbar_ = std::make_unique<SystemTaskbar>();
  } else if (!settings_.hideSystemTaskbar) {
    systemTaskbar_.reset();
  }
}

void TaskbarWindow::OpenSettings() {
  ShowSettingsDialog(module_, hwnd_, settings_,
                     [this](const Settings& edited) { ApplySettings(edited); });
}

void TaskbarWindow::ApplySettings(const Settings& edited) {
  // Pins are changed in the start menu, not here, so they are kept.
  Settings next = edited;
  next.pinned = settings_.pinned;
  store_.Save(next);
  const bool moved = next.edge != settings_.edge;
  settings_ = next;

  ui::SetLanguage(ui::LanguageFor(settings_.language));
  ApplyTheme();
  ApplyWindowsKey();
  ApplySystemTaskbar();
  if (moved) {
    if (startMenu_) startMenu_->Close();
    HideThumbnail();
    Dock();
  }
  UpdateLayout();
}

}  // namespace throwback
