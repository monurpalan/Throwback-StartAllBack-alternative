// The bar's settings: applying them live, saving them and the properties
// dialog.
#include <algorithm>

#include "taskbar/bar_manager.h"
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

void TaskbarWindow::ApplyWindowsKey() {
  // Without the hook the Windows key and Ctrl+Esc open Windows' own menu.
  if (settings_.windowsKeyOpensMenu && IsMain() && !winKeyHook_) {
    winKeyHook_ = std::make_unique<WinKeyHook>(hwnd_, kStartMenuRequest);
  } else if (!settings_.windowsKeyOpensMenu) {
    winKeyHook_.reset();
  }
}

void TaskbarWindow::ApplySystemTaskbar() {
  if (settings_.hideSystemTaskbar && IsMain() && !systemTaskbar_) {
    systemTaskbar_ = std::make_unique<SystemTaskbar>();
  } else if (!settings_.hideSystemTaskbar) {
    systemTaskbar_.reset();
  }
}

void TaskbarWindow::OpenSettings() {
  ShowSettingsDialog(module_, hwnd_, settings_,
                     [this](const Settings& edited) { manager_.ApplySettings(edited); });
}

int TaskbarWindow::StartButtonWidth() const {
  // The classic button: its flag, label and margins (at 96 DPI).
  constexpr int kClassicChrome = 16 + 3 + 2 * 6 + 2 * 2;
  switch (settings_.startStyle) {
    case StartStyle::Win98:
    case StartStyle::WinMe:
    case StartStyle::Win2000: {
      const ui::TextStyle style = settings_.startStyle == StartStyle::Win2000
                                      ? ui::TextStyle::Classic2000Bold
                                      : ui::TextStyle::Classic98Bold;
      return canvas_->MeasureText(ui::Tr(ui::StringId::ClassicStart), style) +
             ScaleForDpi(kClassicChrome, dpi_);
    }
    case StartStyle::WinXp: {
      // XP's button: 99 px with "start", its flag and margins.
      constexpr int kXpMinWidth = 99;
      constexpr int kXpChrome = 10 + 25 + 4 + 24;
      return std::max(ScaleForDpi(kXpMinWidth, dpi_),
                      canvas_->MeasureText(ui::Tr(ui::StringId::XpStart), ui::TextStyle::XpStart) +
                          ScaleForDpi(kXpChrome, dpi_));
    }
    default:
      return 0;
  }
}

void TaskbarWindow::CreateStartMenuForStyle() {
  if (startMenu_) startMenu_->Close();
  startMenu_.reset();
  startStyle_ = settings_.startStyle;
  startMenu_ = CreateStartMenu(
      module_, hwnd_, settings_, [this] { InvalidateRect(hwnd_, nullptr, FALSE); },
      [this] { manager_.SavePinned(); });
}

void TaskbarWindow::OnSettingsChanged(bool moved) {
  if (settings_.startStyle != startStyle_) CreateStartMenuForStyle();
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
