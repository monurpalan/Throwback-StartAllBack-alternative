#pragma once

#include "core/layout.h"

namespace throwback {

// The Windows 7 start menu: a program list on the left with "All Programs"
// and the search box under it, a picture and shell places on the right, and
// the shut down button in the bottom right corner.
struct StartMenuLayout {
  Rect frame;
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
  Rect jumpList;      // replaces the right column while a jump list is open
  int rowHeight = 0;  // program rows
  int placeHeight = 0;
  int listRows = 0;
  int jumpRowHeight = 0;
};

// The shut down button grows with its label, as on Windows 7; powerLabelWidth
// is the label's width in pixels.
StartMenuLayout ComputeStartMenuLayout(unsigned dpi, int powerLabelWidth = 0);

}  // namespace throwback
