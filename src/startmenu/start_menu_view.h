#pragma once

#include <windows.h>

#include <string>
#include <string_view>
#include <vector>

#include "core/options.h"
#include "core/start_menu_layout.h"
#include "ui/canvas.h"

namespace throwback {

// Everything the start menu shows for one frame.
struct StartMenuModel {
  struct Row {
    std::wstring_view name;
    HICON icon = nullptr;
    bool folder = false;
    bool selected = false;
    bool separatorBefore = false;
    bool hasJumpList = false;
    bool bold = false;  // Windows XP: a pinned program
    bool arrowHot = false;  // the mouse is over the half that opens the jump list
    bool fresh = false;     // newly installed and not started yet
  };
  struct JumpRow {
    std::wstring_view name;
    HICON icon = nullptr;
    bool hovered = false;
  };
  struct PlaceRow {
    std::wstring_view name;
    bool separatorBefore = false;
    bool hovered = false;
    HICON icon = nullptr;  // Windows XP
    bool bold = false;
  };

  StartMenuLayout layout;
  StartStyle style = StartStyle::Win7;
  std::vector<Row> rows;
  int rowHeight = 0;
  int iconSize = 0;
  bool showBack = false;  // "Back" instead of "All Programs"
  bool allProgramsHovered = false;
  bool allProgramsFresh = false;  // some program is newly installed
  std::wstring_view query;
  bool noResults = false;
  std::wstring_view accountName;
  std::wstring_view picturePath;
  std::vector<PlaceRow> places;
  bool powerHovered = false;
  bool powerArrowHovered = false;
  bool xpProgramsOpen = false;  // Windows XP: All Programs stays lit while open
  bool lockHovered = false;     // Windows Vista
  // While open, the jump list replaces the picture and the places.
  bool jumpListOpen = false;
  std::vector<JumpRow> jumpRows;
};

// The width of a program row's jump list half, at 96 DPI.
constexpr int kJumpArrowWidth = 24;

// The place row rects, with separators taken into account.
std::vector<Rect> PlaceRects(const StartMenuLayout& layout, const std::vector<bool>& separators,
                             int gap);

// The jump list's item rects, below its title; as many as fit.
std::vector<Rect> JumpItemRects(const StartMenuLayout& layout, size_t count);

// Draws the start menu using UI kit components only.
void DrawStartMenu(ui::Canvas& canvas, const StartMenuModel& model);
// Windows XP's (start_menu_xp_view.cpp) and Vista's (start_menu_vista_view.cpp).
void DrawXpStartMenu(ui::Canvas& canvas, const StartMenuModel& model);
void DrawVistaStartMenu(ui::Canvas& canvas, const StartMenuModel& model);

}  // namespace throwback
