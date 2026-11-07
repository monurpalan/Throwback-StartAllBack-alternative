// Draws the Windows 11 start menu: the search box, the pinned grid, the
// recommended documents and the account bar; or the list of all apps.
#include <algorithm>

#include "startmenu/modern_view.h"
#include "ui/strings.h"

namespace throwback {
namespace {

using Kind = ModernHit::Kind;

// At 96 DPI.
constexpr int kSide = 32;
constexpr int kSearchTop = 24;
constexpr int kSearchHeight = 32;
constexpr int kHeaderTop = 72;
constexpr int kHeaderHeight = 32;
constexpr int kButtonWidth = 112;  // the narrowest it gets
constexpr int kButtonHeight = 26;
constexpr int kGridTop = 108;
constexpr int kColumns = 6;
constexpr int kRows = 3;
constexpr int kCell = 96;
constexpr int kCellHeight = 84;
constexpr int kIcon = 32;
constexpr int kLabel = 32;
constexpr int kRecommendedTop = 372;
constexpr int kRecentTop = 408;
constexpr int kRecentWidth = 280;
constexpr int kRecentHeight = 52;
constexpr int kRecentGap = 8;
constexpr int kListBottom = 620;
constexpr int kRow = 36;
constexpr int kRowIcon = 24;
constexpr int kBar = 64;
constexpr int kPicture = 32;
constexpr int kPadding = 12;
constexpr wchar_t kNextGlyph = 0xE76C;
constexpr wchar_t kPreviousGlyph = 0xE76B;
constexpr wchar_t kPowerGlyph = 0xE7E8;

// A header with a small button on its right: "All apps >" or "< Back".
void DrawHeader(ui::Canvas& canvas, const ModernModel& model, ui::StringId title,
                ui::StringId button, bool back, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  const Rect& frame = model.frame;
  canvas.ModernText({s(kSide + 24), s(kHeaderTop), frame.right / 2, s(kHeaderTop + kHeaderHeight)},
                    ui::Tr(title), ui::TextStyle::Win11Header, model.look, ui::ModernTone::Primary);
  const int top = s(kHeaderTop) + (s(kHeaderHeight) - s(kButtonHeight)) / 2;
  // The button grows with its label and the chevron beside it.
  const int width =
      std::max(s(kButtonWidth), canvas.MeasureText(ui::Tr(button), ui::TextStyle::Win11Small) +
                                    s(kButtonHeight) + s(16));
  const Rect rect{frame.right - s(kSide) - width, top, frame.right - s(kSide),
                  top + s(kButtonHeight)};
  const ModernHit hit{back ? Kind::Back : Kind::AllApps};
  // A quiet filled button, lighter while pointed at.
  canvas.ModernHover(rect, model.look, true);
  canvas.ModernHover(rect, model.look, model.hover == hit);
  const int glyph = s(kButtonHeight);
  const Rect glyphRect = back ? Rect{rect.left, rect.top, rect.left + glyph, rect.bottom}
                              : Rect{rect.right - glyph, rect.top, rect.right, rect.bottom};
  canvas.ModernGlyph(glyphRect, back ? kPreviousGlyph : kNextGlyph, model.look,
                     ui::ModernTone::Primary);
  canvas.ModernText({back ? glyphRect.right : rect.left + s(8), rect.top,
                     back ? rect.right : glyphRect.left, rect.bottom},
                    ui::Tr(button), ui::TextStyle::Win11Small, model.look, ui::ModernTone::Primary,
                    back ? ui::TextAlign::Leading : ui::TextAlign::Center);
  regions.push_back({rect, hit});
}

void DrawGrid(ui::Canvas& canvas, const ModernModel& model, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  const int left0 = (model.frame.Width() - kColumns * s(kCell)) / 2;
  for (size_t i = 0; i < model.tiles.size() && i < static_cast<size_t>(kColumns * kRows); ++i) {
    const int left = left0 + (static_cast<int>(i) % kColumns) * s(kCell);
    const int top = s(kGridTop) + (static_cast<int>(i) / kColumns) * s(kCellHeight);
    const Rect cell{left + s(2), top, left + s(kCell) - s(2), top + s(kCellHeight) - s(4)};
    const ModernHit hit{Kind::Tile, static_cast<int>(i)};
    canvas.ModernHover(cell, model.look, model.hover == hit);
    const int iconLeft = left + (s(kCell) - s(kIcon)) / 2;
    canvas.Icon({iconLeft, top + s(12), iconLeft + s(kIcon), top + s(12) + s(kIcon)},
                model.tiles[i].icon);
    canvas.ModernText(
        {cell.left + s(4), top + s(12 + kIcon), cell.right - s(4), top + s(12 + kIcon + kLabel)},
        model.tiles[i].name, ui::TextStyle::Win11Small, model.look, ui::ModernTone::Primary,
        ui::TextAlign::Center);
    regions.push_back({cell, hit});
  }
}

void DrawRecommended(ui::Canvas& canvas, const ModernModel& model, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  canvas.ModernText({s(kSide + 24), s(kRecommendedTop), model.frame.right / 2,
                     s(kRecommendedTop + kHeaderHeight)},
                    ui::Tr(ui::StringId::ModernRecommended), ui::TextStyle::Win11Header, model.look,
                    ui::ModernTone::Primary);
  const int left0 = (model.frame.Width() - 2 * s(kRecentWidth) - s(kRecentGap)) / 2;
  for (size_t i = 0; i < model.recent.size() && i < 6; ++i) {
    const int left = left0 + (static_cast<int>(i) % 2) * s(kRecentWidth + kRecentGap);
    const int top = s(kRecentTop) + (static_cast<int>(i) / 2) * s(kRecentHeight + 4);
    const Rect rect{left, top, left + s(kRecentWidth), top + s(kRecentHeight)};
    const ModernHit hit{Kind::Recent, static_cast<int>(i)};
    canvas.ModernHover(rect, model.look, model.hover == hit);
    const int iconTop = top + (s(kRecentHeight) - s(kIcon)) / 2;
    canvas.Icon({left + s(kPadding), iconTop, left + s(kPadding + kIcon), iconTop + s(kIcon)},
                model.recent[i].icon);
    const int textLeft = left + s(kPadding * 2 + kIcon);
    const int middle = top + s(kRecentHeight) / 2;
    canvas.ModernText({textLeft, top + s(6), rect.right - s(8), middle + s(2)},
                      model.recent[i].name, ui::TextStyle::Win11Body, model.look,
                      ui::ModernTone::Primary);
    canvas.ModernText({textLeft, middle, rect.right - s(8), rect.bottom - s(6)},
                      model.recent[i].caption, ui::TextStyle::Win11Small, model.look,
                      ui::ModernTone::Secondary);
    regions.push_back({rect, hit});
  }
}

void DrawList(ui::Canvas& canvas, const ModernModel& model, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  int top = s(kGridTop);
  const int left = s(kSide + 16);
  const int right = model.frame.right - s(kSide + 16);
  for (size_t i = static_cast<size_t>(std::max(0, model.scroll)); i < model.apps.size(); ++i) {
    if (top + s(kRow) > s(kListBottom)) break;
    const Rect rect{left, top, right, top + s(kRow)};
    top += s(kRow);
    const auto& item = model.apps[i];
    if (item.header) {
      canvas.ModernText({rect.left + s(kPadding), rect.top, rect.right, rect.bottom}, item.name,
                        ui::TextStyle::Win11Header, model.look, ui::ModernTone::Accent);
      continue;
    }
    const ModernHit hit{Kind::App, static_cast<int>(i)};
    canvas.ModernHover(rect, model.look, model.hover == hit);
    const int iconTop = rect.top + (s(kRow) - s(kRowIcon)) / 2;
    canvas.Icon({rect.left + s(kPadding), iconTop, rect.left + s(kPadding + kRowIcon),
                 iconTop + s(kRowIcon)},
                item.icon);
    canvas.ModernText({rect.left + s(kPadding * 2 + kRowIcon), rect.top, rect.right, rect.bottom},
                      item.name, ui::TextStyle::Win11Body, model.look, ui::ModernTone::Primary);
    regions.push_back({rect, hit});
  }
}

void DrawBar(ui::Canvas& canvas, const ModernModel& model, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  const Rect& frame = model.frame;
  const Rect bar{frame.left + s(1), frame.bottom - s(kBar), frame.right - s(1),
                 frame.bottom - s(8)};
  canvas.ModernBar(bar, model.look);
  const int middle = (bar.top + bar.bottom) / 2;
  const Rect user{s(kSide + 16), middle - s(20), frame.right / 2, middle + s(20)};
  canvas.ModernHover(user, model.look, model.hover == ModernHit{Kind::User});
  const Rect picture{user.left + s(8), middle - s(kPicture) / 2, user.left + s(8 + kPicture),
                     middle + s(kPicture) / 2};
  canvas.ModernPicture(picture, std::wstring(model.picturePath), model.accountName.substr(0, 1),
                       model.look);
  canvas.ModernText({picture.right + s(kPadding), user.top, user.right, user.bottom},
                    model.accountName, ui::TextStyle::Win11Small, model.look,
                    ui::ModernTone::Primary);
  regions.push_back({user, {Kind::User}});
  const Rect power{frame.right - s(kSide + 16) - s(40), middle - s(20), frame.right - s(kSide + 16),
                   middle + s(20)};
  canvas.ModernHover(power, model.look, model.hover == ModernHit{Kind::Power});
  canvas.ModernGlyph(power, kPowerGlyph, model.look, ui::ModernTone::Primary);
  regions.push_back({power, {Kind::Power}});
}

}  // namespace

ModernRegions DrawWin11StartMenu(ui::Canvas& canvas, const ModernModel& model) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  ModernRegions regions;
  canvas.ModernSurface(model.frame, model.look);
  canvas.ModernSearchBox(
      {s(kSide), s(kSearchTop), model.frame.right - s(kSide), s(kSearchTop + kSearchHeight)},
      model.look, model.query, ui::Tr(ui::StringId::ModernSearchPrompt));
  switch (model.page) {
    case ModernModel::Page::Home:
      DrawHeader(canvas, model, ui::StringId::ModernPinned, ui::StringId::ModernAllApps, false,
                 regions);
      DrawGrid(canvas, model, regions);
      DrawRecommended(canvas, model, regions);
      break;
    case ModernModel::Page::AllApps:
      DrawHeader(canvas, model, ui::StringId::ModernAllApps, ui::StringId::Back, true, regions);
      DrawList(canvas, model, regions);
      break;
    case ModernModel::Page::Search:
      canvas.ModernText(
          {s(kSide + 24), s(kHeaderTop), model.frame.right / 2, s(kHeaderTop + kHeaderHeight)},
          ui::Tr(ui::StringId::ModernBestMatch), ui::TextStyle::Win11Header, model.look,
          ui::ModernTone::Primary);
      DrawList(canvas, model, regions);
      break;
  }
  DrawBar(canvas, model, regions);
  return regions;
}

}  // namespace throwback
