#include "taskbar/bar_view.h"

#include "ui/strings.h"

namespace throwback {
namespace {

// The Windows 7 taskbar's sizes at 96 DPI: 24 px button icons and an orb that
// nearly fills the bar's height.
constexpr int kIconSize = 24;
constexpr int kOrbSize = 38;
constexpr int kOverlaySize = 16;
// The classic Start button keeps a margin inside its slot of the bar.
constexpr int kClassicButtonInsetX = 2;
constexpr int kClassicButtonInsetY = 5;

ui::ButtonVariant VariantFor(const TaskButtonModel& button) {
  if (button.flashing && !button.active) return ui::ButtonVariant::FlashingTask;
  if (button.active) return ui::ButtonVariant::ActiveTask;
  return ui::ButtonVariant::Task;
}

void DrawProgress(ui::Canvas& canvas, const TaskButtonModel& button) {
  switch (button.progress) {
    case ProgressState::None:
      return;
    case ProgressState::Indeterminate:
      canvas.Progress(button.rect, ui::ProgressVariant::Indeterminate, 1.0f);
      return;
    case ProgressState::Normal:
      canvas.Progress(button.rect, ui::ProgressVariant::Normal, button.progressFraction);
      return;
    case ProgressState::Paused:
      canvas.Progress(button.rect, ui::ProgressVariant::Paused, button.progressFraction);
      return;
    case ProgressState::Error:
      canvas.Progress(button.rect, ui::ProgressVariant::Error, button.progressFraction);
      return;
  }
}

void DrawTaskButton(ui::Canvas& canvas, const TaskButtonModel& button) {
  canvas.Button(button.rect, VariantFor(button), button.hovered);
  DrawProgress(canvas, button);

  const int padding = canvas.Scale(ui::kSpace8);
  const int iconSize = canvas.Scale(kIconSize);
  const int iconTop = button.rect.top + (button.rect.Height() - iconSize) / 2;
  const Rect icon{button.rect.left + padding, iconTop, button.rect.left + padding + iconSize,
                  iconTop + iconSize};
  canvas.Icon(icon, button.icon);
  if (button.overlay) {
    // Overlays sit on the icon's bottom right corner, as on Windows 7.
    const int overlay = canvas.Scale(kOverlaySize);
    canvas.Icon({icon.right - overlay + canvas.Scale(ui::kSpace4), icon.bottom - overlay,
                 icon.right + canvas.Scale(ui::kSpace4), icon.bottom},
                button.overlay);
  }

  const Rect title{icon.right + padding, button.rect.top, button.rect.right - padding,
                   button.rect.bottom};
  canvas.Text(title, button.title, ui::TextStyle::Body, ui::TextTone::Light);
}

}  // namespace

ui::OrbState OrbStateFor(const BarModel& model);

namespace {

void DrawStartButton(ui::Canvas& canvas, const BarModel& model) {
  switch (model.startStyle) {
    case StartStyle::Win98:
    case StartStyle::WinMe:
    case StartStyle::Win2000: {
      const Rect& area = model.startButton;
      const int dx = canvas.Scale(kClassicButtonInsetX);
      const int dy = canvas.Scale(kClassicButtonInsetY);
      canvas.ClassicStartButton({area.left + dx, area.top + dy, area.right - dx, area.bottom - dy},
                                model.startStyle == StartStyle::Win2000 ? ui::ClassicLook::Win2000
                                                                        : ui::ClassicLook::Win98,
                                ui::Tr(ui::StringId::ClassicStart), model.startOpen);
      return;
    }
    case StartStyle::WinXp:
      canvas.XpStartButton(model.startButton, ui::Tr(ui::StringId::XpStart), OrbStateFor(model));
      return;
    case StartStyle::Win8:
      return;  // Windows 8 had no Start button, only the corner
    case StartStyle::Win81:
      canvas.ModernStartButton(model.startButton, ui::ModernStartVariant::Win81,
                               OrbStateFor(model));
      return;
    case StartStyle::Win10:
      canvas.ModernStartButton(model.startButton, ui::ModernStartVariant::Win10,
                               OrbStateFor(model));
      return;
    case StartStyle::Win11:
      canvas.ModernStartButton(model.startButton, ui::ModernStartVariant::Win11,
                               OrbStateFor(model));
      return;
    default:
      if (model.drawOrb) {
        canvas.Orb(CenteredSquare(model.startButton, canvas.Scale(kOrbSize)), OrbStateFor(model));
      }
      return;
  }
}

}  // namespace

ui::OrbState OrbStateFor(const BarModel& model) {
  if (model.startOpen) return ui::OrbState::Pressed;
  return model.startHovered ? ui::OrbState::Hot : ui::OrbState::Normal;
}

void DrawBar(ui::Canvas& canvas, const BarModel& model) {
  canvas.Glass(model.bounds, model.top ? ui::GlassVariant::BarTop : ui::GlassVariant::BarBottom);
  DrawStartButton(canvas, model);

  for (const auto& button : model.buttons) DrawTaskButton(canvas, button);

  if (model.trayArrow.Width() > 0) {
    canvas.Button(model.trayArrow,
                  model.trayOpen ? ui::ButtonVariant::TrayArrowOpen : ui::ButtonVariant::TrayArrow,
                  model.trayArrowHovered);
  }
  for (const auto& icon : model.trayIcons) {
    canvas.Button(icon.slot, ui::ButtonVariant::TrayItem, icon.hovered);
    canvas.Icon(icon.icon, icon.handle);
  }

  const Rect& clock = model.clock;
  const int middle = (clock.top + clock.bottom) / 2;
  canvas.Text({clock.left, clock.top, clock.right, middle + canvas.Scale(ui::kSpace2)}, model.time,
              ui::TextStyle::Body, ui::TextTone::Light, ui::TextAlign::Center);
  canvas.Text({clock.left, middle - canvas.Scale(ui::kSpace2), clock.right, clock.bottom},
              model.date, ui::TextStyle::Body, ui::TextTone::Light, ui::TextAlign::Center);

  canvas.Button(model.showDesktop, ui::ButtonVariant::ShowDesktop, model.showDesktopHovered);
}

}  // namespace throwback
