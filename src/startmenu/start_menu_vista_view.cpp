// Draws the Windows Vista start menu with UI kit components only: Windows
// 7's arrangement in Vista's own images.
#include <algorithm>

#include "startmenu/start_menu_view.h"
#include "ui/strings.h"

namespace throwback {
namespace {

using ui::SkinImage;

// At 96 DPI.
constexpr int kPadding = 8;
constexpr int kChevron = 16;
constexpr int kMoreArrow = 18;
constexpr int kArrowWidth = 7;
constexpr int kArrowHeight = 9;
constexpr int kSeparatorHeight = 2;
constexpr int kPictureInset = 4;

Rect Centered(const Rect& rect, int width, int height) {
  const int left = rect.left + (rect.Width() - width) / 2;
  const int top = rect.top + (rect.Height() - height) / 2;
  return {left, top, left + width, top + height};
}

void DrawRows(ui::Canvas& canvas, const StartMenuModel& model) {
  const Rect& list = model.layout.list;
  const int padding = canvas.Scale(kPadding);
  for (size_t i = 0; i < model.rows.size(); ++i) {
    const auto& row = model.rows[i];
    const int top = list.top + static_cast<int>(i) * model.rowHeight;
    const Rect rect{list.left + canvas.Scale(2), top, list.right - canvas.Scale(2),
                    top + model.rowHeight};
    if (row.selected) canvas.Image(SkinImage::VistaProgramHover, 0, rect);
    if (row.separatorBefore) {
      canvas.Image(SkinImage::VistaProgramSeparator, 0,
                   {list.left + padding, top - 1, list.right - padding,
                    top - 1 + canvas.Scale(kSeparatorHeight)});
    }
    const int iconTop = top + (model.rowHeight - model.iconSize) / 2;
    const Rect icon{rect.left + padding / 2, iconTop, rect.left + padding / 2 + model.iconSize,
                    iconTop + model.iconSize};
    canvas.Icon(icon, row.icon);
    const int chevron = row.folder ? canvas.Scale(kChevron) : 0;
    canvas.Text({icon.right + padding, top, rect.right - padding - chevron, rect.bottom}, row.name,
                ui::TextStyle::Body, ui::TextTone::Dark);
    if (row.folder) {
      canvas.Icon({rect.right - padding - chevron, top, rect.right - padding, rect.bottom},
                  ui::IconKind::ChevronRight);
    }
  }
  if (model.noResults) {
    canvas.Text({list.left + padding, list.top, list.right - padding, list.top + model.rowHeight},
                ui::Tr(ui::StringId::NoResults), ui::TextStyle::Body, ui::TextTone::Muted);
  }
}

void DrawAllPrograms(ui::Canvas& canvas, const StartMenuModel& model) {
  const Rect& area = model.layout.allPrograms;
  const int padding = canvas.Scale(kPadding);
  const bool hot = model.allProgramsHovered;
  if (hot) canvas.Image(SkinImage::VistaProgramHover, 0, area);
  const int arrow = canvas.Scale(kMoreArrow);
  const Rect arrowRect = Centered(
      {area.left + padding, area.top, area.left + padding + arrow, area.bottom}, arrow, arrow);
  const SkinImage image = model.showBack
                              ? (hot ? SkinImage::VistaBackArrowHot : SkinImage::VistaBackArrow)
                              : (hot ? SkinImage::VistaMoreArrowHot : SkinImage::VistaMoreArrow);
  canvas.Image(image, 0, arrowRect);
  canvas.Text({arrowRect.right + padding / 2, area.top, area.right - padding, area.bottom},
              ui::Tr(model.showBack ? ui::StringId::Back : ui::StringId::AllPrograms),
              ui::TextStyle::Body, ui::TextTone::Dark);
}

void DrawPlaces(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  const int padding = canvas.Scale(kPadding);
  canvas.AccountPicture(layout.picture, std::wstring(model.picturePath),
                        model.accountName.substr(0, 1), SkinImage::VistaPictureFrame,
                        kPictureInset);
  std::vector<bool> separators;
  for (const auto& place : model.places) separators.push_back(place.separatorBefore);
  const std::vector<Rect> rects = PlaceRects(layout, separators, padding);
  for (size_t i = 0; i < rects.size(); ++i) {
    const auto& place = model.places[i];
    if (place.separatorBefore) {
      const int y = rects[i].top - padding / 2;
      canvas.Image(SkinImage::VistaPlaceSeparator, 0,
                   {rects[i].left + padding, y, rects[i].right - padding,
                    y + canvas.Scale(kSeparatorHeight)});
    }
    if (place.hovered) canvas.Image(SkinImage::VistaPlaceHover, 0, rects[i]);
    canvas.Text({rects[i].left + padding, rects[i].top, rects[i].right - padding, rects[i].bottom},
                place.name, ui::TextStyle::Body, ui::TextTone::Light);
  }
}

void DrawPowerButtons(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  // The theme's strip holds undock, lock and power, each a finished button.
  canvas.ImageCell(model.powerHovered ? SkinImage::VistaPowerHot : SkinImage::VistaPower, 2, 3,
                   layout.power);
  canvas.ImageCell(model.lockHovered ? SkinImage::VistaPowerHot : SkinImage::VistaPower, 1, 3,
                   layout.lock);
  const int frame = model.powerArrowHovered ? 1 : 0;
  canvas.Image(SkinImage::VistaArrowButton, frame, layout.powerArrow);
  canvas.Image(SkinImage::VistaArrow, frame,
               Centered(layout.powerArrow, canvas.Scale(kArrowWidth), canvas.Scale(kArrowHeight)));
}

}  // namespace

void DrawVistaStartMenu(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  canvas.Glass(layout.frame, ui::GlassVariant::Menu);
  canvas.Image(SkinImage::VistaPane, 0, layout.programsArea);
  canvas.Image(SkinImage::VistaMorePrograms, 0, layout.searchArea);
  canvas.Image(SkinImage::VistaPlaces, 0, layout.placesArea);
  DrawRows(canvas, model);
  DrawAllPrograms(canvas, model);
  // Vista's strip is a dark frame; the white box sits inside it.
  canvas.Image(SkinImage::VistaSearch, 0, layout.search);
  const int frame = canvas.Scale(2);
  canvas.TextField({layout.search.left + frame, layout.search.top + frame,
                    layout.search.right - frame, layout.search.bottom - frame},
                   model.query, ui::Tr(ui::StringId::VistaSearchPlaceholder), true, true);
  DrawPlaces(canvas, model);
  DrawPowerButtons(canvas, model);
}

}  // namespace throwback
