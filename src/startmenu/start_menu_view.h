#pragma once

#include <windows.h>

#include <string>
#include <string_view>
#include <vector>

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
  };
  struct PlaceRow {
    std::wstring_view name;
    bool separatorBefore = false;
    bool hovered = false;
  };

  StartMenuLayout layout;
  std::vector<Row> rows;
  int rowHeight = 0;
  int iconSize = 0;
  bool showBack = false;  // "Back" instead of "All Programs"
  bool allProgramsHovered = false;
  std::wstring_view query;
  bool noResults = false;
  std::wstring_view accountName;
  std::wstring_view picturePath;
  std::vector<PlaceRow> places;
  bool powerHovered = false;
  bool powerArrowHovered = false;
};

// The place row rects, with separators taken into account.
std::vector<Rect> PlaceRects(const StartMenuLayout& layout, const std::vector<bool>& separators,
                             int gap);

// Draws the start menu using UI kit components only.
void DrawStartMenu(ui::Canvas& canvas, const StartMenuModel& model);

}  // namespace throwback
