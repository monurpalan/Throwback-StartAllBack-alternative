#include "taskbar/bar_view.h"

namespace throwback {
namespace {

// The Windows 7 taskbar's sizes at 96 DPI: 24 px button icons and an orb that
// nearly fills the bar's height.
constexpr int kIconSize = 24;
constexpr int kOrbSize = 38;
constexpr int kOverlaySize = 16;

ui::ButtonVariant VariantFor(const TaskButtonModel& button) {
  if (button.flashing && !button.active) return ui::ButtonVariant::FlashingTask;
  if (button.active) return ui::ButtonVariant::ActiveTask;
  return ui::ButtonVariant::Task;
}

ui::OrbState OrbStateFor(const BarModel& model) {
  if (model.startOpen) return ui::OrbState::Pressed;
  return model.startHovered ? ui::OrbState::Hot : ui::OrbState::Normal;
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

void DrawBar(ui::Canvas& canvas, const BarModel& model) {
  canvas.Glass(model.bounds, model.top ? ui::GlassVariant::BarTop : ui::GlassVariant::BarBottom);
  canvas.Orb(CenteredSquare(model.startButton, canvas.Scale(kOrbSize)), OrbStateFor(model));

  for (const auto& button : model.buttons) DrawTaskButton(canvas, button);

  for (const auto& icon : model.trayIcons) {
    canvas.Button(icon.slot, ui::ButtonVariant::GlassItem, icon.hovered);
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
