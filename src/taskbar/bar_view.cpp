#include "taskbar/bar_view.h"

#include <algorithm>

#include "ui/strings.h"

namespace throwback {
namespace {

using ui::SkinImage;

// At 96 DPI: icons on task buttons (16 px up to Vista, 24 px after), overlay
// badges, the Windows 7 orb, and the classic Start button's margins.
constexpr int kSmallIcon = 16;
constexpr int kLargeIcon = 24;
constexpr int kOrbSize = 38;
constexpr int kOverlaySize = 16;
constexpr int kClassicButtonInsetX = 2;
constexpr int kClassicButtonInsetY = 3;
constexpr int kClassicWellInset = 2;
constexpr int kTrayArrowWidth = 19;
constexpr int kTrayArrowHeight = 20;
constexpr int kW8TrayArrowWidth = 21;
constexpr int kXpTrayLead = 14;  // XP's tray image reaches left of the icons
constexpr wchar_t kChevronUp = 0xE70E;
constexpr wchar_t kChevronDown = 0xE70D;

Rect Centered(const Rect& rect, int width, int height) {
  const int left = rect.left + (rect.Width() - width) / 2;
  const int top = rect.top + (rect.Height() - height) / 2;
  return {left, top, left + width, top + height};
}

ui::ClassicLook ClassicLookOf(const BarModel& model) {
  return model.family == BarFamily::Classic2000 ? ui::ClassicLook::Win2000 : ui::ClassicLook::Win98;
}

ui::ModernLook ModernLookOf(const BarModel& model) {
  if (model.family == BarFamily::Win10) return ui::ModernLook::Win10;
  return model.light ? ui::ModernLook::Win11Light : ui::ModernLook::Win11Dark;
}

bool IsClassic(BarFamily family) {
  return family == BarFamily::Classic98 || family == BarFamily::Classic2000;
}

bool IsModern(BarFamily family) { return family == BarFamily::Win10 || family == BarFamily::Win11; }

// A state's frame in a six-frame toolbar button: normal, hot, pressed,
// disabled, checked (the active window) and hot checked.
int ToolbarFrame(const TaskButtonModel& button) {
  if (button.active) return button.hovered ? 5 : 4;
  return button.hovered ? 1 : 0;
}

// Windows 7's and 8's ten-frame task button.
int TaskbandFrame(const TaskButtonModel& button) {
  if (button.flashing && !button.active) return 6;
  if (button.active) return button.hovered ? 4 : 3;
  return button.hovered ? 1 : 0;
}

ui::ButtonVariant VariantFor(const TaskButtonModel& button) {
  if (button.flashing && !button.active) return ui::ButtonVariant::FlashingTask;
  if (button.active) return ui::ButtonVariant::ActiveTask;
  return ui::ButtonVariant::Task;
}

void DrawBackground(ui::Canvas& canvas, const BarModel& model) {
  switch (model.family) {
    case BarFamily::Classic98:
    case BarFamily::Classic2000:
      canvas.ClassicBar(model.bounds, ClassicLookOf(model), model.top);
      return;
    case BarFamily::Xp:
      canvas.Image(SkinImage::XpTaskbar, 0, model.bounds);
      return;
    case BarFamily::Vista:
      canvas.Glass(model.bounds, ui::GlassVariant::Menu);
      canvas.Image(SkinImage::VistaTaskbar, 0, model.bounds);
      return;
    case BarFamily::Win7:
      canvas.Glass(model.bounds,
                   model.top ? ui::GlassVariant::BarTop : ui::GlassVariant::BarBottom);
      return;
    case BarFamily::Win8:
      canvas.Glass(model.bounds, ui::GlassVariant::Menu);
      canvas.Image(SkinImage::W8Taskbar, 0, model.bounds);
      return;
    case BarFamily::Win10:
    case BarFamily::Win11:
      canvas.ModernTaskbar(model.bounds, ModernLookOf(model), model.top);
      return;
  }
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

void DrawTaskBackground(ui::Canvas& canvas, const BarModel& model, const TaskButtonModel& button) {
  const bool flashing = button.flashing && !button.active;
  switch (model.family) {
    case BarFamily::Classic98:
    case BarFamily::Classic2000:
      canvas.ClassicFace(button.rect, ClassicLookOf(model), button.active);
      return;
    case BarFamily::Xp:
      if (!flashing || !canvas.Image(SkinImage::XpTaskFlash, 0, button.rect)) {
        canvas.Image(SkinImage::XpTaskButton, ToolbarFrame(button), button.rect);
      }
      return;
    case BarFamily::Vista:
      if (!flashing || !canvas.Image(SkinImage::TaskButton, 6, button.rect)) {
        canvas.Image(SkinImage::VistaTaskButton, ToolbarFrame(button), button.rect);
      }
      return;
    case BarFamily::Win7:
      canvas.Button(button.rect, VariantFor(button), button.hovered);
      return;
    case BarFamily::Win8:
      canvas.Image(SkinImage::W8TaskButton, TaskbandFrame(button), button.rect);
      return;
    case BarFamily::Win10:
    case BarFamily::Win11:
      canvas.ModernTaskButton(button.rect, ModernLookOf(model), button.active, flashing,
                              button.hovered);
      return;
  }
}

void DrawTaskLabel(ui::Canvas& canvas, const BarModel& model, const TaskButtonModel& button,
                   const Rect& rect) {
  switch (model.family) {
    case BarFamily::Classic98:
    case BarFamily::Classic2000:
      // The active window's title is bold, as on Windows 98.
      canvas.ClassicText(rect, button.title, ClassicLookOf(model), false, button.active);
      return;
    case BarFamily::Xp:
      canvas.XpText(rect, button.title, ui::XpTextVariant::LogOff, false);
      return;
    default:
      canvas.Text(rect, button.title, ui::TextStyle::Body, ui::TextTone::Light);
      return;
  }
}

void DrawTaskButton(ui::Canvas& canvas, const BarModel& model, const TaskButtonModel& button) {
  DrawTaskBackground(canvas, model, button);
  DrawProgress(canvas, button);
  const bool smallIcons =
      IsClassic(model.family) || model.family == BarFamily::Xp || model.family == BarFamily::Vista;
  const int iconSize = canvas.Scale(smallIcons ? kSmallIcon : kLargeIcon);
  const int padding = canvas.Scale(smallIcons ? ui::kSpace4 + ui::kSpace2 : ui::kSpace8);
  const int iconTop = button.rect.top + (button.rect.Height() - iconSize) / 2;
  const int iconLeft = model.labels ? button.rect.left + padding
                                    : button.rect.left + (button.rect.Width() - iconSize) / 2;
  const Rect icon{iconLeft, iconTop, iconLeft + iconSize, iconTop + iconSize};
  canvas.Icon(icon, button.icon);
  if (button.overlay) {
    // Overlays sit on the icon's bottom right corner.
    const int overlay = canvas.Scale(kOverlaySize);
    canvas.Icon({icon.right - overlay + canvas.Scale(ui::kSpace4), icon.bottom - overlay,
                 icon.right + canvas.Scale(ui::kSpace4), icon.bottom},
                button.overlay);
  }
  if (!model.labels) return;
  DrawTaskLabel(
      canvas, model, button,
      {icon.right + padding, button.rect.top, button.rect.right - padding, button.rect.bottom});
}

void DrawTrayArea(ui::Canvas& canvas, const BarModel& model) {
  const int left = model.trayArrow.Width() > 0 ? model.trayArrow.left
                   : model.trayIcons.empty()   ? model.clock.left
                                               : model.trayIcons.front().slot.left;
  if (IsClassic(model.family)) {
    const int inset = canvas.Scale(kClassicWellInset);
    canvas.ClassicWell({left - inset, model.bounds.top + inset + canvas.Scale(1),
                        model.clock.right - inset, model.bounds.bottom - inset},
                       ClassicLookOf(model));
  } else if (model.family == BarFamily::Xp) {
    canvas.Image(SkinImage::XpTray, 0,
                 {left - canvas.Scale(kXpTrayLead), model.bounds.top, model.bounds.right,
                  model.bounds.bottom});
  }
}

void DrawTrayArrow(ui::Canvas& canvas, const BarModel& model) {
  const Rect& rect = model.trayArrow;
  if (rect.Width() <= 0) return;
  const int frame = model.trayArrowHovered ? 1 : 0;
  const Rect arrow = Centered(rect, canvas.Scale(kTrayArrowWidth), canvas.Scale(kTrayArrowHeight));
  switch (model.family) {
    case BarFamily::Classic98:
    case BarFamily::Classic2000:
      canvas.ClassicFace(arrow, ClassicLookOf(model), model.trayOpen);
      canvas.Icon(arrow, model.trayOpen ? ui::IconKind::ChevronRight : ui::IconKind::ChevronLeft);
      return;
    case BarFamily::Xp:
      canvas.Image(model.trayOpen ? SkinImage::XpTrayArrowOpen : SkinImage::XpTrayArrow, frame,
                   arrow);
      return;
    case BarFamily::Vista:
      canvas.Image(model.trayOpen ? SkinImage::VistaTrayArrowOpen : SkinImage::VistaTrayArrow,
                   frame, arrow);
      return;
    case BarFamily::Win7:
      canvas.Button(
          rect, model.trayOpen ? ui::ButtonVariant::TrayArrowOpen : ui::ButtonVariant::TrayArrow,
          model.trayArrowHovered);
      return;
    case BarFamily::Win8:
      canvas.Image(model.trayOpen ? SkinImage::W8TrayArrowOpen : SkinImage::W8TrayArrow, frame,
                   Centered(rect, canvas.Scale(kW8TrayArrowWidth), canvas.Scale(kTrayArrowHeight)));
      return;
    case BarFamily::Win10:
    case BarFamily::Win11:
      canvas.ModernHover(rect, ModernLookOf(model), model.trayArrowHovered);
      canvas.ModernGlyph(rect, model.trayOpen ? kChevronDown : kChevronUp, ModernLookOf(model),
                         ui::ModernTone::Primary);
      return;
  }
}

void DrawTrayIcons(ui::Canvas& canvas, const BarModel& model) {
  for (const auto& icon : model.trayIcons) {
    switch (model.family) {
      case BarFamily::Win7:
        canvas.Button(icon.slot, ui::ButtonVariant::TrayItem, icon.hovered);
        break;
      case BarFamily::Win8:
        if (icon.hovered) canvas.Image(SkinImage::W8TrayItem, 1, icon.slot);
        break;
      case BarFamily::Win10:
      case BarFamily::Win11:
        canvas.ModernHover(icon.slot, ModernLookOf(model), icon.hovered);
        break;
      default:
        break;  // the older notification areas had no hover
    }
    canvas.Icon(icon.icon, icon.handle);
  }
}

// A single centered line for clocks that show the time only.
void DrawTimeOnly(ui::Canvas& canvas, const BarModel& model) {
  const Rect& clock = model.clock;
  if (IsClassic(model.family)) {
    const ui::TextStyle style = model.family == BarFamily::Classic2000 ? ui::TextStyle::Classic2000
                                                                       : ui::TextStyle::Classic98;
    const int width = canvas.MeasureText(model.time, style);
    const int left = clock.left + (clock.Width() - width) / 2;
    canvas.ClassicText({left, clock.top, clock.right, clock.bottom}, model.time,
                       ClassicLookOf(model), false);
  } else if (model.family == BarFamily::Xp) {
    const int width = canvas.MeasureText(model.time, ui::TextStyle::XpBody);
    const int left = clock.left + (clock.Width() - width) / 2;
    canvas.XpText({left, clock.top, clock.right, clock.bottom}, model.time,
                  ui::XpTextVariant::LogOff, false);
  } else {
    canvas.Text(clock, model.time, ui::TextStyle::Body, ui::TextTone::Light, ui::TextAlign::Center);
  }
}

void DrawClock(ui::Canvas& canvas, const BarModel& model) {
  if (!model.dateLine) {
    DrawTimeOnly(canvas, model);
    return;
  }
  const Rect& clock = model.clock;
  const int middle = (clock.top + clock.bottom) / 2;
  const Rect upper{clock.left, clock.top, clock.right, middle + canvas.Scale(ui::kSpace2)};
  const Rect lower{clock.left, middle - canvas.Scale(ui::kSpace2), clock.right, clock.bottom};
  if (IsModern(model.family)) {
    const ui::ModernLook look = ModernLookOf(model);
    const ui::TextStyle style =
        model.family == BarFamily::Win10 ? ui::TextStyle::Win10Small : ui::TextStyle::Win11Small;
    canvas.ModernText(upper, model.time, style, look, ui::ModernTone::Primary,
                      ui::TextAlign::Center);
    canvas.ModernText(lower, model.date, style, look, ui::ModernTone::Primary,
                      ui::TextAlign::Center);
    return;
  }
  canvas.Text(upper, model.time, ui::TextStyle::Body, ui::TextTone::Light, ui::TextAlign::Center);
  canvas.Text(lower, model.date, ui::TextStyle::Body, ui::TextTone::Light, ui::TextAlign::Center);
}

void DrawShowDesktop(ui::Canvas& canvas, const BarModel& model) {
  if (model.showDesktop.Width() <= 0) return;
  switch (model.family) {
    case BarFamily::Win7:
      canvas.Button(model.showDesktop, ui::ButtonVariant::ShowDesktop, model.showDesktopHovered);
      return;
    case BarFamily::Win8:
      canvas.Image(SkinImage::W8ShowDesktop, model.showDesktopHovered ? 2 : 0, model.showDesktop);
      return;
    case BarFamily::Win10:
    case BarFamily::Win11:
      canvas.ModernHover(model.showDesktop, ModernLookOf(model), model.showDesktopHovered);
      return;
    default:
      return;
  }
}

}  // namespace

ui::OrbState OrbStateFor(const BarModel& model) {
  if (model.startOpen) return ui::OrbState::Pressed;
  return model.startHovered ? ui::OrbState::Hot : ui::OrbState::Normal;
}

void DrawStartButton(ui::Canvas& canvas, const BarModel& model) {
  switch (model.startStyle) {
    case StartStyle::Win98:
    case StartStyle::WinMe:
    case StartStyle::Win2000: {
      const Rect& area = model.startButton;
      const int dx = canvas.Scale(kClassicButtonInsetX);
      const int dy = canvas.Scale(kClassicButtonInsetY);
      canvas.ClassicStartButton({area.left + dx, area.top + dy, area.right - dx, area.bottom - dy},
                                ClassicLookOf(model), ui::Tr(ui::StringId::ClassicStart),
                                model.startOpen);
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

void DrawBar(ui::Canvas& canvas, const BarModel& model) {
  DrawBackground(canvas, model);
  DrawStartButton(canvas, model);
  for (const auto& button : model.buttons) DrawTaskButton(canvas, model, button);
  DrawTrayArea(canvas, model);
  DrawTrayArrow(canvas, model);
  DrawTrayIcons(canvas, model);
  DrawClock(canvas, model);
  DrawShowDesktop(canvas, model);
}

}  // namespace throwback
