// The bar's settings: applying them live, saving them and the properties
// dialog.
#include <algorithm>
#include <optional>

#include "taskbar/bar_manager.h"
#include "taskbar/settings_dialog.h"
#include "taskbar/taskbar_window.h"
#include "taskbar/window_actions.h"
#include "ui/skin_ids.h"
#include "ui/strings.h"

namespace throwback {
namespace {

// The slot a chosen orb takes on bars whose own button is wider or narrower.
constexpr int kOrbButtonWidth = 54;

// The orb the settings ask for: a chosen one on any style, else the style's
// own orb, or none for styles whose start button is not an orb.
std::optional<ui::OrbArt> OrbArtFor(const Settings& settings) {
  using Kind = ui::OrbArt::Kind;
  const StartStyle style = settings.startStyle;
  if (settings.orb == OrbChoice::Style && style != StartStyle::Win7 &&
      style != StartStyle::Vista && style != StartStyle::Plain8) {
    return std::nullopt;
  }
  OrbChoice choice = settings.orb;
  if (choice == OrbChoice::Style) {
    choice = style == StartStyle::Vista    ? OrbChoice::Vista
             : style == StartStyle::Plain8 ? OrbChoice::Win8
                                           : OrbChoice::Win7;
  }
  switch (choice) {
    case OrbChoice::Vista:
      return ui::OrbArt{Kind::Skin, ui::SkinImage::VistaOrb};
    case OrbChoice::Clover:
      return ui::OrbArt{Kind::Logo, ui::SkinImage::Orb, THROWBACK_ORB_CLOVER};
    case OrbChoice::Win8:
      return ui::OrbArt{Kind::Logo, ui::SkinImage::Orb, THROWBACK_ORB_WIN8};
    case OrbChoice::E1even:
      return ui::OrbArt{Kind::Picture, ui::SkinImage::Orb, THROWBACK_ORB_E1EVEN};
    default:
      return ui::OrbArt{Kind::Skin, ui::SkinImage::Orb};
  }
}

}  // namespace

void TaskbarWindow::ShowOrb(ui::OrbState state) {
  if (!orb_) return;
  const std::optional<ui::OrbArt> art = OrbArtFor(settings_);
  if (!art) {
    orb_->Hide();
    return;
  }
  const Rect& button = layout_.startButton;
  POINT center{(button.left + button.right) / 2, (button.top + button.bottom) / 2};
  ClientToScreen(hwnd_, &center);
  orb_->Show(center, {button.Width(), button.Height()}, dpi_, state, *art);
}

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

int TaskbarWindow::Thickness() const {
  const BarStyle style = BarStyleOf(settings_.startStyle);
  if (!IsVertical(settings_.edge)) return ScaleForDpi(style.height, dpi_);
  return ScaleForDpi(kVerticalBarWidth, dpi_);
}

int TaskbarWindow::StartButtonWidth() const {
  // A chosen orb replaces the style's own button, whatever its width.
  if (settings_.orb != OrbChoice::Style) return ScaleForDpi(kOrbButtonWidth, dpi_);
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
  // A new style also brings its taskbar's height.
  if (settings_.startStyle != startStyle_) {
    CreateStartMenuForStyle();
    moved = true;
  }
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
