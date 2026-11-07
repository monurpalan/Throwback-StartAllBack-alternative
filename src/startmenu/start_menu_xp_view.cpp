// Draws the Windows XP start menu with UI kit components only.
#include <algorithm>

#include "startmenu/start_menu_view.h"
#include "ui/strings.h"

namespace throwback {
namespace {

// At 96 DPI.
constexpr int kNameGap = 10;     // between the picture and the user name
constexpr int kRowPadding = 4;   // icon inset in a row
constexpr int kTextGap = 6;      // between a row's icon and its text
constexpr int kBand = 6;         // the band a separator line sits in
constexpr int kArrowWidth = 16;  // the green arrow of All Programs
constexpr int kArrowHeight = 24;

void DrawPrograms(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  const int padding = canvas.Scale(kRowPadding);
  const int band = canvas.Scale(kBand);
  for (size_t i = 0; i < model.rows.size(); ++i) {
    const auto& row = model.rows[i];
    const int top = layout.list.top + static_cast<int>(i) * model.rowHeight;
    const Rect rect{layout.list.left, top, layout.list.right, top + model.rowHeight};
    if (row.separatorBefore) {
      canvas.XpSeparator({rect.left, top - band / 2, rect.right, top + band / 2},
                         ui::XpList::Programs);
    }
    canvas.XpItem(rect, ui::XpList::Programs, row.selected);
    const int iconTop = top + (model.rowHeight - model.iconSize) / 2;
    const Rect icon{rect.left + padding, iconTop, rect.left + padding + model.iconSize,
                    iconTop + model.iconSize};
    canvas.Icon(icon, row.icon);
    canvas.XpText({icon.right + canvas.Scale(kTextGap), top, rect.right - padding, rect.bottom},
                  row.name, row.bold ? ui::XpTextVariant::ProgramBold : ui::XpTextVariant::Program,
                  row.selected);
  }
}

void DrawMorePrograms(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  const Rect& area = layout.allPrograms;
  canvas.XpBackground(area, ui::XpPanel::MorePrograms);
  canvas.XpSeparator({area.left, area.top - canvas.Scale(kBand), area.right, area.top},
                     ui::XpList::Programs);
  const bool hot = model.allProgramsHovered || model.xpProgramsOpen;
  const std::wstring_view label = ui::Tr(ui::StringId::AllPrograms);
  const int textWidth = canvas.MeasureText(label, ui::TextStyle::XpBold);
  const int arrowWidth = canvas.Scale(kArrowWidth);
  const int gap = canvas.Scale(kTextGap);
  const int width = textWidth + gap + arrowWidth;
  const int left = area.left + (area.Width() - width) / 2;
  const int inset = canvas.Scale(kRowPadding);
  canvas.XpItem({left - inset, area.top + inset / 2, left + width + inset, area.bottom - inset / 2},
                ui::XpList::Programs, hot);
  canvas.XpText({left, area.top, left + textWidth + gap, area.bottom}, label,
                ui::XpTextVariant::MorePrograms, hot);
  const int arrowTop = area.top + (area.Height() - canvas.Scale(kArrowHeight)) / 2;
  canvas.XpMoreArrow(
      {left + textWidth + gap, arrowTop, left + width, arrowTop + canvas.Scale(kArrowHeight)}, hot);
}

void DrawPlaces(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  const int gap = canvas.Scale(ui::kSpace8);
  std::vector<bool> separators;
  for (const auto& place : model.places) separators.push_back(place.separatorBefore);
  const std::vector<Rect> rects = PlaceRects(layout, separators, gap);
  const int padding = canvas.Scale(ui::kSpace2);
  for (size_t i = 0; i < rects.size(); ++i) {
    const auto& place = model.places[i];
    const Rect& rect = rects[i];
    if (place.separatorBefore) {
      canvas.XpSeparator({rect.left, rect.top - gap, rect.right, rect.top}, ui::XpList::Places);
    }
    canvas.XpItem(rect, ui::XpList::Places, place.hovered);
    const int size = rect.Height() - 2 * padding;
    const Rect icon{rect.left + padding, rect.top + padding, rect.left + padding + size,
                    rect.top + padding + size};
    canvas.Icon(icon, place.icon);
    canvas.XpText({icon.right + canvas.Scale(kTextGap), rect.top, rect.right, rect.bottom},
                  place.name, place.bold ? ui::XpTextVariant::PlaceBold : ui::XpTextVariant::Place,
                  place.hovered);
  }
}

}  // namespace

void DrawXpStartMenu(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  canvas.XpBackground(layout.userPane, ui::XpPanel::UserPane);
  canvas.XpUserPicture(layout.picture, std::wstring(model.picturePath),
                       model.accountName.substr(0, 1));
  canvas.XpText({layout.picture.right + canvas.Scale(kNameGap), layout.userPane.top,
                 layout.userPane.right, layout.userPane.bottom},
                model.accountName, ui::XpTextVariant::User, false);

  canvas.XpBackground(layout.programsArea, ui::XpPanel::Programs);
  DrawPrograms(canvas, model);
  DrawMorePrograms(canvas, model);

  canvas.XpBackground(layout.placesArea, ui::XpPanel::Places);
  DrawPlaces(canvas, model);

  canvas.XpBackground(layout.powerArea, ui::XpPanel::LogOff);
  canvas.XpLogOffButton(layout.power, ui::XpPowerButton::LogOff, ui::Tr(ui::StringId::XpLogOff),
                        model.powerHovered);
  canvas.XpLogOffButton(layout.powerArrow, ui::XpPowerButton::TurnOff,
                        ui::Tr(ui::StringId::XpTurnOff), model.powerArrowHovered);
}

}  // namespace throwback
