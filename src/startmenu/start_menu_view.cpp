#include "startmenu/start_menu_view.h"

#include <algorithm>

#include "ui/strings.h"

namespace throwback {
namespace {

constexpr int kChevronSize = 16;

Rect HorizontalRule(int left, int right, int y, ui::Canvas& canvas) {
  return {left, y, right, y + std::max(1, canvas.Scale(1))};
}

void DrawRows(ui::Canvas& canvas, const StartMenuModel& model) {
  const Rect& list = model.layout.list;
  const int padding = canvas.Scale(ui::kSpace8);
  for (size_t i = 0; i < model.rows.size(); ++i) {
    const auto& row = model.rows[i];
    const int top = list.top + static_cast<int>(i) * model.rowHeight;
    const Rect rect{list.left, top, list.right, top + model.rowHeight};
    canvas.Button(rect, ui::ButtonVariant::Ghost, row.selected);

    const int iconTop = top + (model.rowHeight - model.iconSize) / 2;
    const Rect icon{rect.left + padding, iconTop, rect.left + padding + model.iconSize,
                    iconTop + model.iconSize};
    canvas.Icon(icon, row.icon);

    const int chevron = row.folder ? canvas.Scale(kChevronSize) : 0;
    canvas.Text({icon.right + padding, top, rect.right - padding - chevron, rect.bottom}, row.name,
                ui::TextStyle::Body);
    if (row.folder) {
      canvas.Icon({rect.right - padding - chevron, top, rect.right - padding, rect.bottom},
                  ui::IconKind::ChevronRight);
    }
  }
  if (model.noResults) {
    canvas.Text({list.left + padding, list.top, list.right - padding, list.top + model.rowHeight},
                ui::Tr(ui::StringId::NoResults), ui::TextStyle::Body, ui::TextAlign::Leading,
                ui::TextTone::Muted);
  }
}

void DrawLeftPane(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  const int padding = canvas.Scale(ui::kSpace8);
  canvas.Panel(layout.leftPane);
  DrawRows(canvas, model);

  canvas.Divider(HorizontalRule(layout.list.left + padding, layout.list.right - padding,
                                layout.allPrograms.top - padding, canvas));
  canvas.Button(layout.allPrograms, ui::ButtonVariant::Ghost, model.allProgramsHovered);
  const int chevron = canvas.Scale(kChevronSize);
  const Rect arrow{layout.allPrograms.left + padding, layout.allPrograms.top,
                   layout.allPrograms.left + padding + chevron, layout.allPrograms.bottom};
  canvas.Icon(arrow, model.showBack ? ui::IconKind::ChevronLeft : ui::IconKind::ChevronRight);
  canvas.Text({arrow.right + padding, layout.allPrograms.top, layout.allPrograms.right - padding,
               layout.allPrograms.bottom},
              ui::Tr(model.showBack ? ui::StringId::Back : ui::StringId::AllPrograms),
              ui::TextStyle::Body);

  canvas.TextField(layout.search, model.query, ui::Tr(ui::StringId::SearchPlaceholder), true);
}

void DrawRightColumn(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  if (!model.picturePath.empty()) {
    canvas.Picture(layout.picture, std::wstring(model.picturePath));
  } else {
    // No account picture: the initial on a tile, as Windows shows it.
    canvas.Panel(layout.picture);
    canvas.Text(layout.picture, model.accountName.substr(0, 1), ui::TextStyle::Title,
                ui::TextAlign::Center);
  }

  const int padding = canvas.Scale(ui::kSpace8);
  std::vector<bool> separators;
  for (const auto& place : model.places) separators.push_back(place.separatorBefore);
  const std::vector<Rect> rects = PlaceRects(layout, separators, padding);
  for (size_t i = 0; i < rects.size(); ++i) {
    const auto& place = model.places[i];
    if (place.separatorBefore) {
      canvas.Divider(HorizontalRule(rects[i].left + padding, rects[i].right - padding,
                                    rects[i].top - padding / 2, canvas));
    }
    canvas.Button(rects[i], ui::ButtonVariant::Ghost, place.hovered);
    canvas.Text({rects[i].left + padding, rects[i].top, rects[i].right - padding, rects[i].bottom},
                place.name, ui::TextStyle::Body);
  }

  canvas.Button(layout.power, ui::ButtonVariant::Raised, model.powerHovered);
  canvas.Text(layout.power, ui::Tr(ui::StringId::ShutDown), ui::TextStyle::Body,
              ui::TextAlign::Center);
  canvas.Button(layout.powerArrow, ui::ButtonVariant::Raised, model.powerArrowHovered);
  canvas.Icon(layout.powerArrow, ui::IconKind::ChevronRight);
  canvas.Divider({layout.powerArrow.left, layout.powerArrow.top,
                  layout.powerArrow.left + std::max(1, canvas.Scale(1)), layout.powerArrow.bottom});
}

}  // namespace

std::vector<Rect> PlaceRects(const StartMenuLayout& layout, const std::vector<bool>& separators,
                             int gap) {
  std::vector<Rect> rects;
  int top = layout.places.top;
  for (bool separator : separators) {
    if (separator) top += gap;
    if (top + layout.placeHeight > layout.places.bottom) break;
    rects.push_back({layout.places.left, top, layout.places.right, top + layout.placeHeight});
    top += layout.placeHeight;
  }
  return rects;
}

void DrawStartMenu(ui::Canvas& canvas, const StartMenuModel& model) {
  DrawLeftPane(canvas, model);
  DrawRightColumn(canvas, model);
}

}  // namespace throwback
