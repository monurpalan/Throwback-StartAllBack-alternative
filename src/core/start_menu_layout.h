#pragma once

#include "core/layout.h"

namespace throwback {

// The Windows 7 start menu: a program list on the left with "All Programs"
// and the search box under it, a picture and shell places on the right, and
// the shut down button in the bottom right corner.
struct StartMenuLayout {
  Rect frame;
  Rect leftPane;
  Rect list;  // programs, or search results while searching
  Rect allPrograms;
  Rect search;
  Rect picture;
  Rect places;  // first place row starts at places.top
  Rect power;
  Rect powerArrow;
  int rowHeight = 0;  // program rows
  int placeHeight = 0;
  int listRows = 0;
};

StartMenuLayout ComputeStartMenuLayout(unsigned dpi);

}  // namespace throwback
