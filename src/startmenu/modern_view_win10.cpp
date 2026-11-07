// Draws the Windows 10 start menu: the rail of icons on the left, the app
// list with its most used programs and letter headings, and the tiles.
#include <algorithm>

#include "startmenu/modern_view.h"
#include "ui/strings.h"

namespace throwback {
namespace {

using Kind = ModernHit::Kind;
constexpr ui::ModernLook kLook = ui::ModernLook::Win10;

// At 96 DPI.
constexpr int kRail = 48;
constexpr int kListRight = 328;
constexpr int kListTop = 8;
constexpr int kRow = 36;
constexpr int kIcon = 24;
constexpr int kPadding = 12;
constexpr int kTilesLeft = 340;
constexpr int kTilesTop = 44;
constexpr int kTile = 100;
constexpr int kTileGap = 4;
constexpr int kTileColumns = 3;
constexpr int kTileIcon = 32;
constexpr int kTileLabel = 18;
constexpr int kPicture = 28;
constexpr wchar_t kMenuGlyph = 0xE700;
constexpr wchar_t kPowerGlyph = 0xE7E8;
constexpr wchar_t kSettingsGlyph = 0xE713;
constexpr wchar_t kPicturesGlyph = 0xEB9F;
constexpr wchar_t kDocumentsGlyph = 0xE8A5;

void DrawRail(ui::Canvas& canvas, const ModernModel& model, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  canvas.ModernGlyph({0, 0, s(kRail), s(kRail)}, kMenuGlyph, kLook, ui::ModernTone::Primary);
  // From the bottom up: power, settings, pictures, documents, the account.
  struct Button {
    Kind kind;
    wchar_t glyph;
  };
  static constexpr Button kButtons[] = {{Kind::Power, kPowerGlyph},
                                        {Kind::Settings, kSettingsGlyph},
                                        {Kind::Pictures, kPicturesGlyph},
                                        {Kind::Documents, kDocumentsGlyph},
                                        {Kind::User, 0}};
  int bottom = model.frame.bottom;
  for (const Button& button : kButtons) {
    const Rect rect{0, bottom - s(kRail), s(kRail), bottom};
    const ModernHit hit{button.kind};
    canvas.ModernHover(rect, kLook, model.hover == hit);
    if (button.glyph != 0) {
      canvas.ModernGlyph(rect, button.glyph, kLook, ui::ModernTone::Primary);
    } else {
      const int inset = (s(kRail) - s(kPicture)) / 2;
      canvas.ModernPicture(
          {rect.left + inset, rect.top + inset, rect.right - inset, rect.bottom - inset},
          std::wstring(model.picturePath), model.accountName.substr(0, 1), kLook);
    }
    regions.push_back({rect, hit});
    bottom -= s(kRail);
  }
}

// One row of the list: a heading or a program with its icon.
void DrawRow(ui::Canvas& canvas, const ModernModel& model, const ModernModel::Item& item,
             const Rect& rect, ModernHit hit, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  if (item.header) {
    canvas.ModernText({rect.left + s(kPadding), rect.top, rect.right, rect.bottom}, item.name,
                      ui::TextStyle::Win10Body, kLook, ui::ModernTone::Primary);
    return;
  }
  canvas.ModernHover(rect, kLook, model.hover == hit);
  const int iconTop = rect.top + (rect.Height() - s(kIcon)) / 2;
  const Rect icon{rect.left + s(kPadding), iconTop, rect.left + s(kPadding) + s(kIcon),
                  iconTop + s(kIcon)};
  canvas.Icon(icon, item.icon);
  canvas.ModernText({icon.right + s(kPadding), rect.top, rect.right - s(kPadding), rect.bottom},
                    item.name, ui::TextStyle::Win10Body, kLook, ui::ModernTone::Primary);
  regions.push_back({rect, hit});
}

void DrawList(ui::Canvas& canvas, const ModernModel& model, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  int top = s(kListTop);
  const auto next = [&]() {
    const Rect rect{s(kRail), top, s(kListRight), top + s(kRow)};
    top += s(kRow);
    return rect;
  };
  const bool searching = model.page == ModernModel::Page::Search;
  if (!searching && model.scroll == 0 && !model.frequent.empty()) {
    canvas.ModernText(next(), ui::Tr(ui::StringId::ModernMostUsed), ui::TextStyle::Win10Small,
                      kLook, ui::ModernTone::Secondary);
    for (size_t i = 0; i < model.frequent.size(); ++i) {
      DrawRow(canvas, model, model.frequent[i], next(), {Kind::Frequent, static_cast<int>(i)},
              regions);
    }
  }
  if (searching) {
    canvas.ModernText(next(), ui::Tr(ui::StringId::ModernBestMatch), ui::TextStyle::Win10Small,
                      kLook, ui::ModernTone::Secondary);
  }
  for (size_t i = static_cast<size_t>(std::max(0, model.scroll)); i < model.apps.size(); ++i) {
    if (top + s(kRow) > model.frame.bottom) break;
    DrawRow(canvas, model, model.apps[i], next(), {Kind::App, static_cast<int>(i)}, regions);
  }
}

void DrawTiles(ui::Canvas& canvas, const ModernModel& model, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  canvas.ModernText({s(kTilesLeft), s(kListTop), model.frame.right, s(kTilesTop)},
                    ui::Tr(ui::StringId::ModernPinned), ui::TextStyle::Win10Small, kLook,
                    ui::ModernTone::Primary);
  for (size_t i = 0; i < model.tiles.size(); ++i) {
    const int column = static_cast<int>(i) % kTileColumns;
    const int row = static_cast<int>(i) / kTileColumns;
    const int left = s(kTilesLeft) + column * s(kTile + kTileGap);
    const int top = s(kTilesTop) + row * s(kTile + kTileGap);
    const Rect tile{left, top, left + s(kTile), top + s(kTile)};
    if (tile.bottom > model.frame.bottom) break;
    const ModernHit hit{Kind::Tile, static_cast<int>(i)};
    canvas.ModernTile(tile, -1, model.hover == hit);
    const int iconLeft = left + (s(kTile) - s(kTileIcon)) / 2;
    const int iconTop = top + s(kTile) / 2 - s(kTileIcon) / 2 - s(kTileLabel) / 2;
    canvas.Icon({iconLeft, iconTop, iconLeft + s(kTileIcon), iconTop + s(kTileIcon)},
                model.tiles[i].icon);
    canvas.ModernText(
        {left + s(6), tile.bottom - s(kTileLabel + 4), tile.right - s(6), tile.bottom - s(4)},
        model.tiles[i].name, ui::TextStyle::Win10Small, kLook, ui::ModernTone::Primary);
    regions.push_back({tile, hit});
  }
}

}  // namespace

ModernRegions DrawWin10StartMenu(ui::Canvas& canvas, const ModernModel& model) {
  ModernRegions regions;
  canvas.ModernSurface(model.frame, kLook);
  DrawRail(canvas, model, regions);
  DrawList(canvas, model, regions);
  DrawTiles(canvas, model, regions);
  return regions;
}

}  // namespace throwback
