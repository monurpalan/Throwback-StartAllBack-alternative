#pragma once

#include "core/layout.h"
#include "core/settings.h"

namespace throwback {

// The Windows 7 start menu: a program list on the left with "All Programs"
// and the search box under it, a picture and shell places on the right, and
// the shut down button in the bottom right corner.
struct StartMenuLayout {
  Rect frame;
  Rect userPane;  // Windows XP: the account banner across the top
  // The background's four areas, split where the program pane ends.
  Rect programsArea;
  Rect searchArea;
  Rect placesArea;
  Rect powerArea;
  Rect leftPane;
  Rect list;  // programs, or search results while searching
  Rect allPrograms;
  Rect search;
  Rect picture;
  Rect places;  // first place row starts at places.top
  Rect power;
  Rect powerArrow;
  Rect lock;          // Windows Vista: the lock button between power and the arrow
  Rect jumpList;      // replaces the right column while a jump list is open
  int rowHeight = 0;  // program rows
  int placeHeight = 0;
  int listRows = 0;
  int jumpRowHeight = 0;
};

// What the user and the style decide about the Windows 7 layout, at 96 DPI.
struct StartMenuShape {
  int rows = kDefaultMenuRows;
  bool largeIcons = true;
  // Windows 7's theme gives the panes 261 and 150 px; Plain8's 311 and 195.
  int programPaneWidth = 261;
  int placesWidth = 150;
  // The right column's rows and separators, which can make the menu taller
  // than its program list.
  int placeRows = 0;
  int placeSeparators = 0;
};

// The shut down button grows with its label, as on Windows 7; powerLabelWidth
// is the label's width in pixels.
StartMenuLayout ComputeStartMenuLayout(unsigned dpi, int powerLabelWidth = 0,
                                       const StartMenuShape& shape = {});

// Windows XP's start menu, after its theme: the account banner on top, the
// program list and "All Programs" on the left, places on the right, and the
// Log Off (power) and Turn Off Computer (powerArrow) buttons in the bar at
// the bottom, sized to their labels. It has no search box.
// rows and largeIcons size the program list; the panel grows to fit it.
StartMenuLayout ComputeXpStartMenuLayout(unsigned dpi, int logOffLabelWidth, int turnOffLabelWidth,
                                         int rows = 8, bool largeIcons = true);

// Windows Vista's: Windows 7's program pane and places, with All Programs
// closing the pane, the search box on the glass under it, and the power,
// lock and arrow buttons at the bottom of the places column.
StartMenuLayout ComputeVistaStartMenuLayout(unsigned dpi, const StartMenuShape& shape = {});

struct MenuPosition {
  int x = 0;
  int y = 0;
  bool operator==(const MenuPosition&) const = default;
};

// Where a start menu of width x height opens. anchor is the start button and
// below says the bar is at the top of the screen; screen is the monitor and
// work its work area. The menu hugs the left (from the start button) or the
// right edge, and the bar's side or the chosen top or bottom edge, gap pixels
// away from the bar or edge it touches.
MenuPosition PlaceStartMenu(const Rect& anchor, bool below, int width, int height,
                            const Rect& screen, const Rect& work, MenuHorizontal horizontal,
                            MenuVertical vertical, int gap = 0);

}  // namespace throwback
