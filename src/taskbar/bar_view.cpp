#include "taskbar/bar_view.h"

namespace throwback {
namespace {

// The classic taskbar's sizes at 96 DPI: 24 px button icons, a 20 px logo.
constexpr int kIconSize = 24;
constexpr int kStartIconSize = 20;

ui::ButtonVariant VariantFor(const TaskButtonModel& button) {
  if (button.active) return ui::ButtonVariant::Selected;
  if (button.flashing) return ui::ButtonVariant::Attention;
  return ui::ButtonVariant::Ghost;
}

void DrawTaskButton(ui::Canvas& canvas, const TaskButtonModel& button) {
  canvas.Button(button.rect, VariantFor(button), button.hovered);

  const int padding = canvas.Scale(ui::kSpace8);
  const int iconSize = canvas.Scale(kIconSize);
  const int iconTop = button.rect.top + (button.rect.Height() - iconSize) / 2;
  const Rect icon{button.rect.left + padding, iconTop, button.rect.left + padding + iconSize,
                  iconTop + iconSize};
  canvas.Icon(icon, button.icon);

  const Rect title{icon.right + padding, button.rect.top, button.rect.right - padding,
                   button.rect.bottom};
  canvas.Text(title, button.title, ui::TextStyle::Body);
}

}  // namespace

void DrawBar(ui::Canvas& canvas, const BarModel& model) {
  canvas.Button(model.startButton, ui::ButtonVariant::Ghost, model.startHovered);
  canvas.Icon(CenteredSquare(model.startButton, canvas.Scale(kStartIconSize)), ui::IconKind::Start);

  for (const auto& button : model.buttons) DrawTaskButton(canvas, button);

  const Rect& clock = model.clock;
  const int middle = (clock.top + clock.bottom) / 2;
  canvas.Text({clock.left, clock.top, clock.right, middle}, model.time, ui::TextStyle::Number,
              ui::TextAlign::Center);
  canvas.Text({clock.left, middle, clock.right, clock.bottom}, model.date,
              ui::TextStyle::SmallNumber, ui::TextAlign::Center);

  canvas.Button(model.showDesktop, ui::ButtonVariant::Ghost, model.showDesktopHovered);
  canvas.Divider(model.showDesktop);
}

}  // namespace throwback
