// The modern start menus' shared parts and Windows 8's start screen.
#include "startmenu/modern_view.h"

#include <algorithm>

#include "ui/strings.h"

namespace throwback {
namespace {

using Kind = ModernHit::Kind;

// Windows 8's start screen at 96 DPI: a 120 px margin, the title band, and
// 120 px tiles 8 px apart; apps in 280 px columns of 40 px rows.
constexpr int kMargin = 120;
constexpr int kTitleTop = 40;
constexpr int kTitleHeight = 60;
constexpr int kGridTop = 180;
constexpr int kTile = 120;
constexpr int kTileGap = 8;
constexpr int kTileIcon = 48;
constexpr int kTilePadding = 8;
constexpr int kLabelHeight = 20;
constexpr int kBottomBand = 120;
constexpr int kAppColumn = 280;
constexpr int kAppRow = 40;
constexpr int kAppIcon = 32;
constexpr int kPicture = 40;
constexpr int kNameWidth = 300;
constexpr int kArrow = 32;
constexpr wchar_t kDownGlyph = 0xE74B;
constexpr wchar_t kUpGlyph = 0xE74A;

// The menus' sizes at 96 DPI.
constexpr int kWin10Width = 650;
constexpr int kWin10Height = 640;
constexpr int kWin11Width = 640;
constexpr int kWin11Height = 700;
constexpr int kWin10ListTop = 8;
constexpr int kWin10Row = 36;
constexpr int kWin11ListTop = 108;
constexpr int kWin11ListBottom = 620;
constexpr int kWin11Row = 36;

int Win8RowsPerColumn(const ModernModel& model, unsigned dpi) {
  const int height = model.frame.Height() - ScaleForDpi(kGridTop + kBottomBand, dpi);
  return std::max(1, height / ScaleForDpi(kAppRow, dpi));
}

int Win8Columns(const ModernModel& model, unsigned dpi) {
  const int width = model.frame.Width() - 2 * ScaleForDpi(kMargin, dpi);
  return std::max(1, width / ScaleForDpi(kAppColumn, dpi));
}

void DrawTiles(ui::Canvas& canvas, const ModernModel& model, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  const int rows =
      std::max(1, (model.frame.Height() - s(kGridTop + kBottomBand)) / s(kTile + kTileGap));
  for (size_t i = 0; i < model.tiles.size(); ++i) {
    const int column = static_cast<int>(i) / rows;
    const int row = static_cast<int>(i) % rows;
    const int left = s(kMargin) + column * s(kTile + kTileGap);
    const int top = s(kGridTop) + row * s(kTile + kTileGap);
    const Rect tile{left, top, left + s(kTile), top + s(kTile)};
    if (tile.right > model.frame.right - s(kMargin)) break;
    const ModernHit hit{Kind::Tile, static_cast<int>(i)};
    canvas.ModernTile(tile, static_cast<int>(i), model.hover == hit);
    const int iconLeft = left + (s(kTile) - s(kTileIcon)) / 2;
    const int iconTop = top + (s(kTile) - s(kTileIcon)) / 2 - s(kTilePadding);
    canvas.Icon({iconLeft, iconTop, iconLeft + s(kTileIcon), iconTop + s(kTileIcon)},
                model.tiles[i].icon);
    canvas.ModernText({left + s(kTilePadding), tile.bottom - s(kLabelHeight + kTilePadding / 2),
                       tile.right - s(kTilePadding), tile.bottom - s(kTilePadding / 2)},
                      model.tiles[i].name, ui::TextStyle::Win8Tile, ui::ModernLook::Win8,
                      ui::ModernTone::Primary);
    regions.push_back({tile, hit});
  }
}

void DrawApps(ui::Canvas& canvas, const ModernModel& model, unsigned dpi, ModernRegions& regions) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  const int rows = Win8RowsPerColumn(model, dpi);
  const int columns = Win8Columns(model, dpi);
  const size_t first = static_cast<size_t>(std::max(0, model.scroll));
  for (size_t i = first; i < model.apps.size(); ++i) {
    const int slot = static_cast<int>(i - first);
    if (slot / rows >= columns) break;
    const int left = s(kMargin) + (slot / rows) * s(kAppColumn);
    const int top = s(kGridTop) + (slot % rows) * s(kAppRow);
    const Rect rect{left, top, left + s(kAppColumn) - s(kTileGap), top + s(kAppRow)};
    const auto& item = model.apps[i];
    if (item.header) {
      canvas.ModernText(rect, item.name, ui::TextStyle::Win8Name, ui::ModernLook::Win8,
                        ui::ModernTone::Primary);
      continue;
    }
    const ModernHit hit{Kind::App, static_cast<int>(i)};
    canvas.ModernHover(rect, ui::ModernLook::Win8, model.hover == hit);
    const int iconTop = top + (s(kAppRow) - s(kAppIcon)) / 2;
    canvas.Icon({left + s(4), iconTop, left + s(4) + s(kAppIcon), iconTop + s(kAppIcon)},
                item.icon);
    canvas.ModernText({left + s(kAppIcon + 12), top, rect.right, rect.bottom}, item.name,
                      ui::TextStyle::Win10Body, ui::ModernLook::Win8, ui::ModernTone::Primary);
    regions.push_back({rect, hit});
  }
}

}  // namespace

Rect ModernFrame(StartStyle style, unsigned dpi, const RECT& monitor) {
  switch (style) {
    case StartStyle::Win8:
    case StartStyle::Win81:
      return {0, 0, static_cast<int>(monitor.right - monitor.left),
              static_cast<int>(monitor.bottom - monitor.top)};
    case StartStyle::Win10:
      return {0, 0, ScaleForDpi(kWin10Width, dpi), ScaleForDpi(kWin10Height, dpi)};
    default:
      return {0, 0, ScaleForDpi(kWin11Width, dpi), ScaleForDpi(kWin11Height, dpi)};
  }
}

int ModernPageRows(const ModernModel& model, unsigned dpi) {
  switch (model.style) {
    case StartStyle::Win8:
    case StartStyle::Win81:
      return Win8RowsPerColumn(model, dpi) * Win8Columns(model, dpi);
    case StartStyle::Win10:
      return (model.frame.Height() - ScaleForDpi(kWin10ListTop, dpi)) / ScaleForDpi(kWin10Row, dpi);
    default:
      return ScaleForDpi(kWin11ListBottom - kWin11ListTop, dpi) / ScaleForDpi(kWin11Row, dpi);
  }
}

ModernRegions DrawModernStartMenu(ui::Canvas& canvas, const ModernModel& model) {
  switch (model.style) {
    case StartStyle::Win8:
    case StartStyle::Win81:
      return DrawWin8StartScreen(canvas, model);
    case StartStyle::Win10:
      return DrawWin10StartMenu(canvas, model);
    default:
      return DrawWin11StartMenu(canvas, model);
  }
}

ModernRegions DrawWin8StartScreen(ui::Canvas& canvas, const ModernModel& model) {
  const auto s = [&canvas](int value) { return canvas.Scale(value); };
  const unsigned dpi = static_cast<unsigned>(canvas.Scale(96));
  ModernRegions regions;
  const Rect& frame = model.frame;
  canvas.ModernSurface(frame, ui::ModernLook::Win8);
  const bool home = model.page == ModernModel::Page::Home;
  canvas.ModernText({s(kMargin), s(kTitleTop), frame.right / 2, s(kTitleTop + kTitleHeight)},
                    ui::Tr(home ? ui::StringId::ClassicStart : ui::StringId::ModernApps),
                    ui::TextStyle::Win8Title, ui::ModernLook::Win8, ui::ModernTone::Primary);

  // The account in the top right corner.
  const Rect picture{frame.right - s(kMargin) - s(kPicture), s(kTitleTop + 10),
                     frame.right - s(kMargin), s(kTitleTop + 10 + kPicture)};
  canvas.ModernPicture(picture, std::wstring(model.picturePath), model.accountName.substr(0, 1),
                       ui::ModernLook::Win8);
  const Rect name{picture.left - s(kNameWidth), picture.top, picture.left - s(12), picture.bottom};
  canvas.ModernText(name, model.accountName, ui::TextStyle::Win8Name, ui::ModernLook::Win8,
                    ui::ModernTone::Primary);
  regions.push_back({{name.left, name.top, picture.right, picture.bottom}, {Kind::User}});

  if (home) {
    DrawTiles(canvas, model, regions);
  } else {
    DrawApps(canvas, model, dpi, regions);
  }
  // Windows 8.1's arrow between the start screen and the apps.
  if (model.style == StartStyle::Win81) {
    const Rect arrow{s(kMargin), frame.bottom - s(kBottomBand) / 2 - s(kArrow) / 2,
                     s(kMargin) + s(kArrow), frame.bottom - s(kBottomBand) / 2 + s(kArrow) / 2};
    const ModernHit hit{home ? Kind::AllApps : Kind::Back};
    canvas.ModernHover(arrow, ui::ModernLook::Win8, model.hover == hit);
    canvas.ModernGlyph(arrow, home ? kDownGlyph : kUpGlyph, ui::ModernLook::Win8,
                       ui::ModernTone::Primary);
    regions.push_back({arrow, hit});
  }
  return regions;
}

}  // namespace throwback
