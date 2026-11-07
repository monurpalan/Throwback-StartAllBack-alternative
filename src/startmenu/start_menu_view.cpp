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
  const int inset = canvas.Scale(ui::kSpace2);
  const int padding = canvas.Scale(ui::kSpace8);
  for (size_t i = 0; i < model.rows.size(); ++i) {
    const auto& row = model.rows[i];
    const int top = list.top + static_cast<int>(i) * model.rowHeight;
    const Rect rect{list.left + inset, top, list.right - inset, top + model.rowHeight};
    canvas.Button(rect, ui::ButtonVariant::MenuItem, row.selected);
    if (row.separatorBefore) {
      canvas.Separator(HorizontalRule(list.left + padding, list.right - padding, top - 1, canvas),
                       ui::SeparatorVariant::Pane);
    }

    const int iconTop = top + (model.rowHeight - model.iconSize) / 2;
    const Rect icon{rect.left + padding / 2, iconTop, rect.left + padding / 2 + model.iconSize,
                    iconTop + model.iconSize};
    canvas.Icon(icon, row.icon);

    const bool arrow = row.folder || row.hasJumpList;
    const int chevron = arrow ? canvas.Scale(kChevronSize) : 0;
    canvas.Text({icon.right + padding, top, rect.right - padding - chevron, rect.bottom}, row.name,
                ui::TextStyle::Body, ui::TextTone::Dark);
    if (arrow) {
      canvas.Icon({rect.right - padding - chevron, top, rect.right - padding, rect.bottom},
                  ui::IconKind::ChevronRight);
    }
  }
  if (model.noResults) {
    canvas.Text({list.left + padding, list.top, list.right - padding, list.top + model.rowHeight},
                ui::Tr(ui::StringId::NoResults), ui::TextStyle::Body, ui::TextTone::Muted);
  }
}

void DrawLeftPane(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  const int padding = canvas.Scale(ui::kSpace8);
  DrawRows(canvas, model);

  canvas.Separator(HorizontalRule(layout.list.left + padding, layout.list.right - padding,
                                  layout.allPrograms.top - canvas.Scale(ui::kSpace2), canvas),
                   ui::SeparatorVariant::Pane);
  const int inset = canvas.Scale(ui::kSpace2);
  const Rect allPrograms{layout.allPrograms.left + inset, layout.allPrograms.top,
                         layout.allPrograms.right - inset, layout.allPrograms.bottom - inset};
  canvas.Button(allPrograms, ui::ButtonVariant::MenuItem, model.allProgramsHovered);
  const int chevron = canvas.Scale(kChevronSize);
  const Rect arrow{allPrograms.left + padding, allPrograms.top,
                   allPrograms.left + padding + chevron, allPrograms.bottom};
  canvas.Icon(arrow, model.showBack ? ui::IconKind::ChevronLeft : ui::IconKind::ChevronRight);
  canvas.Text({arrow.right, allPrograms.top, allPrograms.right - padding, allPrograms.bottom},
              ui::Tr(model.showBack ? ui::StringId::Back : ui::StringId::AllPrograms),
              ui::TextStyle::Body, ui::TextTone::Dark);

  canvas.TextField(layout.search, model.query, ui::Tr(ui::StringId::SearchPlaceholder), true);
}

void DrawPower(ui::Canvas& canvas, const StartMenuModel& model);

void DrawJumpList(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  const int padding = canvas.Scale(ui::kSpace8);
  const int inset = canvas.Scale(ui::kSpace2);
  const Rect title{layout.jumpList.left + padding, layout.jumpList.top,
                   layout.jumpList.right - padding, layout.jumpList.top + layout.jumpRowHeight};
  canvas.Text(title, ui::Tr(ui::StringId::Recent), ui::TextStyle::Body, ui::TextTone::Muted);
  canvas.Separator(HorizontalRule(title.left, title.right, title.bottom - inset, canvas),
                   ui::SeparatorVariant::Pane);

  const int iconSize = canvas.Scale(kChevronSize);
  const std::vector<Rect> rects = JumpItemRects(layout, model.jumpRows.size());
  for (size_t i = 0; i < rects.size(); ++i) {
    const auto& item = model.jumpRows[i];
    canvas.Button(rects[i], ui::ButtonVariant::MenuItem, item.hovered);
    const int iconTop = rects[i].top + (rects[i].Height() - iconSize) / 2;
    const Rect icon{rects[i].left + padding / 2, iconTop, rects[i].left + padding / 2 + iconSize,
                    iconTop + iconSize};
    canvas.Icon(icon, item.icon);
    canvas.Text({icon.right + padding / 2, rects[i].top, rects[i].right - inset, rects[i].bottom},
                item.name, ui::TextStyle::Body, ui::TextTone::Dark);
  }
}

void DrawRightColumn(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  if (model.jumpListOpen) {
    DrawJumpList(canvas, model);
    DrawPower(canvas, model);
    return;
  }
  canvas.AccountPicture(layout.picture, std::wstring(model.picturePath),
                        model.accountName.substr(0, 1));

  const int padding = canvas.Scale(ui::kSpace8);
  std::vector<bool> separators;
  for (const auto& place : model.places) separators.push_back(place.separatorBefore);
  const std::vector<Rect> rects = PlaceRects(layout, separators, padding);
  for (size_t i = 0; i < rects.size(); ++i) {
    const auto& place = model.places[i];
    if (place.separatorBefore) {
      canvas.Separator(HorizontalRule(rects[i].left + padding, rects[i].right - padding,
                                      rects[i].top - padding / 2, canvas),
                       ui::SeparatorVariant::Glass);
    }
    canvas.Button(rects[i], ui::ButtonVariant::GlassItem, place.hovered);
    canvas.Text({rects[i].left + padding, rects[i].top, rects[i].right - padding, rects[i].bottom},
                place.name, ui::TextStyle::Body, ui::TextTone::Light);
  }

  DrawPower(canvas, model);
}

void DrawPower(ui::Canvas& canvas, const StartMenuModel& model) {
  const StartMenuLayout& layout = model.layout;
  canvas.Button(layout.power, ui::ButtonVariant::Command, model.powerHovered);
  canvas.Text(layout.power, ui::Tr(ui::StringId::ShutDown), ui::TextStyle::Body,
              ui::TextTone::Light, ui::TextAlign::Center);
  canvas.Button(layout.powerArrow, ui::ButtonVariant::Command, model.powerArrowHovered);
  canvas.Icon(layout.powerArrow, ui::IconKind::MenuArrow);
}

}  // namespace

std::vector<Rect> JumpItemRects(const StartMenuLayout& layout, size_t count) {
  std::vector<Rect> rects;
  const int inset = layout.jumpRowHeight / 12;  // 2 px at 96 DPI
  int top = layout.jumpList.top + layout.jumpRowHeight;
  for (size_t i = 0; i < count && top + layout.jumpRowHeight <= layout.jumpList.bottom; ++i) {
    rects.push_back({layout.jumpList.left + inset, top, layout.jumpList.right - inset,
                     top + layout.jumpRowHeight});
    top += layout.jumpRowHeight;
  }
  return rects;
}

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
  if (model.style == StartStyle::WinXp) {
    DrawXpStartMenu(canvas, model);
    return;
  }
  if (model.style == StartStyle::Vista) {
    DrawVistaStartMenu(canvas, model);
    return;
  }
  const StartMenuLayout& layout = model.layout;
  canvas.Glass(layout.frame, ui::GlassVariant::Menu);
  canvas.Panel(layout.programsArea, ui::PanelVariant::Programs);
  canvas.Panel(layout.searchArea, ui::PanelVariant::Search);
  canvas.Panel(layout.placesArea,
               model.jumpListOpen ? ui::PanelVariant::JumpList : ui::PanelVariant::Places);
  canvas.Panel(layout.powerArea, ui::PanelVariant::Power);
  DrawLeftPane(canvas, model);
  DrawRightColumn(canvas, model);
}

}  // namespace throwback
