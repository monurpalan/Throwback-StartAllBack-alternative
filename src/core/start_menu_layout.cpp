#include "core/start_menu_layout.h"

#include <algorithm>

namespace throwback {
namespace {

// Windows 7 proportions at 96 DPI (the pane widths are in StartMenuShape);
// rows are rounded to the 4 px grid where Windows 7 itself doesn't use exact
// sizes.
constexpr int kPadding = 8;
constexpr int kRowHeight = 40;
constexpr int kSmallRowHeight = 24;
constexpr int kAllProgramsHeight = 40;
// The row under the panes. The search box and the shut down button share
// one 24 px row where the Windows 7 images put them.
constexpr int kBottomHeight = 54;
constexpr int kSearchLeft = 21;
constexpr int kSearchRight = 18;  // from the program pane's right edge
constexpr int kControlTop = 11;
constexpr int kControlHeight = 24;
constexpr int kPowerWidth = 76;  // the narrowest it gets
constexpr int kPowerPadding = 12;
constexpr int kPictureSize = 48;
constexpr int kPlaceHeight = 32;
constexpr int kPowerArrowWidth = 24;
constexpr int kJumpRowHeight = 24;

}  // namespace

StartMenuLayout ComputeStartMenuLayout(unsigned dpi, int powerLabelWidth,
                                       const StartMenuShape& shape) {
  const auto s = [dpi](int value) { return ScaleForDpi(value, dpi); };
  const int padding = s(kPadding);
  const int width = s(shape.programPaneWidth + shape.placesWidth);
  const int rowHeight = s(shape.largeIcons ? kRowHeight : kSmallRowHeight);
  // The panes are as tall as the taller of the program list and the right
  // column (the picture, then the places and their separators).
  const int programs = shape.rows * rowHeight + 2 * padding + s(kAllProgramsHeight);
  const int places = s(kPictureSize) + padding * 2 + shape.placeRows * s(kPlaceHeight) +
                     shape.placeSeparators * padding;
  const int paneHeight = std::max(programs, places);
  const int height = padding + paneHeight + s(kBottomHeight);

  StartMenuLayout layout;
  layout.rowHeight = rowHeight;
  layout.placeHeight = s(kPlaceHeight);
  layout.listRows = shape.rows;
  layout.frame = {0, 0, width, height};

  const int bottomTop = height - s(kBottomHeight);
  layout.leftPane = {padding, padding, s(shape.programPaneWidth), bottomTop};
  layout.list = {layout.leftPane.left, layout.leftPane.top, layout.leftPane.right,
                 layout.leftPane.bottom - s(kAllProgramsHeight) - 2 * padding};
  layout.allPrograms = {layout.leftPane.left, layout.leftPane.bottom - s(kAllProgramsHeight),
                        layout.leftPane.right, layout.leftPane.bottom};

  layout.programsArea = {0, 0, layout.leftPane.right, bottomTop};
  layout.searchArea = {0, bottomTop, layout.leftPane.right, height};
  layout.placesArea = {layout.leftPane.right, 0, width, bottomTop};
  layout.powerArea = {layout.leftPane.right, bottomTop, width, height};

  const int controlTop = bottomTop + s(kControlTop);
  const int controlBottom = controlTop + s(kControlHeight);
  layout.search = {s(kSearchLeft), controlTop, layout.leftPane.right - s(kSearchRight),
                   controlBottom};

  const int rightLeft = layout.leftPane.right + padding;
  const int rightRight = width - padding;
  const int pictureLeft = (rightLeft + rightRight - s(kPictureSize)) / 2;
  layout.picture = {pictureLeft, padding, pictureLeft + s(kPictureSize), padding + s(kPictureSize)};
  layout.places = {rightLeft, layout.picture.bottom + padding * 2, rightRight, bottomTop};
  const int powerWidth = std::clamp(powerLabelWidth + 2 * s(kPowerPadding), s(kPowerWidth),
                                    rightRight - rightLeft - s(kPowerArrowWidth));
  layout.power = {rightLeft, controlTop, rightLeft + powerWidth, controlBottom};
  layout.powerArrow = {layout.power.right, controlTop, layout.power.right + s(kPowerArrowWidth),
                       controlBottom};
  layout.jumpList = {rightLeft, layout.leftPane.top, rightRight, layout.leftPane.bottom};
  layout.jumpRowHeight = s(kJumpRowHeight);
  return layout;
}

StartMenuLayout ComputeXpStartMenuLayout(unsigned dpi, int logOffLabelWidth,
                                         int turnOffLabelWidth, int rows, bool largeIcons) {
  // Luna's NormalBlue.ini at 96 DPI: a 380 x 440 panel of a 64 px banner,
  // two 190 px wide columns and a 40 px log off bar.
  constexpr int kXpWidth = 380;
  constexpr int kXpBanner = 64;
  constexpr int kXpColumn = 190;
  constexpr int kXpColumns = 336;
  constexpr int kXpMorePrograms = 30;
  constexpr int kXpBar = 40;
  constexpr int kXpLargeRow = 36;
  constexpr int kXpSmallRow = 24;
  constexpr int kXpListTop = 9;
  constexpr int kXpListBottom = 5;
  constexpr int kXpPlace = 26;
  constexpr int kXpPicture = 48;
  constexpr int kXpPictureLeft = 8;
  constexpr int kXpButtonIcon = 24;
  constexpr int kXpButtonGap = 4;
  constexpr int kXpButtonPadding = 8;
  const auto s = [dpi](int value) { return ScaleForDpi(value, dpi); };

  StartMenuLayout layout;
  const int width = s(kXpWidth);
  const int kXpRow = largeIcons ? kXpLargeRow : kXpSmallRow;
  const int kXpRows = rows;
  // Luna's columns, taller when the list needs more room.
  const int columns =
      std::max(kXpColumns, kXpListTop + rows * kXpRow + kXpListBottom + kXpMorePrograms);
  const int columnsTop = s(kXpBanner);
  const int barTop = columnsTop + s(columns);
  const int middle = s(kXpColumn);
  layout.frame = {0, 0, width, barTop + s(kXpBar)};
  layout.userPane = {0, 0, width, columnsTop};
  const int pictureTop = (columnsTop - s(kXpPicture)) / 2 + s(2);
  layout.picture = {s(kXpPictureLeft), pictureTop, s(kXpPictureLeft) + s(kXpPicture),
                    pictureTop + s(kXpPicture)};

  const int moreTop = barTop - s(kXpMorePrograms);
  layout.programsArea = {0, columnsTop, middle, moreTop};
  layout.searchArea = {0, moreTop, middle, barTop};
  layout.placesArea = {middle, columnsTop, width, barTop};
  layout.powerArea = {0, barTop, width, layout.frame.bottom};
  layout.leftPane = layout.programsArea;

  layout.rowHeight = s(kXpRow);
  layout.listRows = kXpRows;
  const int listTop = columnsTop + s(kXpListTop);
  layout.list = {s(6), listTop, middle - s(4), listTop + kXpRows * layout.rowHeight};
  layout.allPrograms = layout.searchArea;
  layout.placeHeight = s(kXpPlace);
  layout.places = {middle + s(4), listTop, width - s(6), barTop - s(5)};

  // Turn Off Computer on the right, Log Off left of it.
  const int buttonTop = barTop + (s(kXpBar) - s(kXpButtonIcon)) / 2;
  const int buttonBottom = buttonTop + s(kXpButtonIcon);
  const int turnOffWidth = s(kXpButtonIcon + kXpButtonGap + kXpButtonPadding) + turnOffLabelWidth;
  const int logOffWidth = s(kXpButtonIcon + kXpButtonGap + kXpButtonPadding) + logOffLabelWidth;
  layout.powerArrow = {width - s(kXpButtonPadding) - turnOffWidth, buttonTop,
                       width - s(kXpButtonPadding), buttonBottom};
  layout.power = {layout.powerArrow.left - logOffWidth, buttonTop, layout.powerArrow.left,
                  buttonBottom};
  return layout;
}

StartMenuLayout ComputeVistaStartMenuLayout(unsigned dpi, const StartMenuShape& shape) {
  // At 96 DPI: Windows 7's columns, a 30 px All Programs piece closing the
  // pane, a 40 px search row on the glass, and the theme's 24 px power and
  // lock buttons beside a 23 x 30 arrow button.
  constexpr int kVistaMorePrograms = 30;
  constexpr int kVistaSearchRow = 40;
  constexpr int kVistaSearchHeight = 24;
  constexpr int kVistaButton = 24;
  constexpr int kVistaButtonGap = 4;
  constexpr int kVistaArrowWidth = 23;
  constexpr int kVistaArrowHeight = 30;
  constexpr int kVistaPlace = 30;
  const auto s = [dpi](int value) { return ScaleForDpi(value, dpi); };
  StartMenuLayout layout = ComputeStartMenuLayout(dpi, 0, shape);
  const int padding = s(kPadding);
  const int paneRight = layout.leftPane.right;
  const int moreTop = layout.list.bottom + padding;
  const int searchTop = moreTop + s(kVistaMorePrograms);
  const int height = searchTop + s(kVistaSearchRow);
  layout.frame.bottom = height;
  layout.programsArea = {0, 0, paneRight, moreTop};
  layout.searchArea = {0, moreTop, paneRight, searchTop};  // the All Programs piece
  layout.placesArea = {paneRight, 0, layout.frame.right, height};
  layout.powerArea = {};
  layout.leftPane = {padding, padding, paneRight, searchTop};
  layout.allPrograms = {padding, moreTop, paneRight, searchTop - s(2)};
  const int fieldTop = searchTop + (s(kVistaSearchRow) - s(kVistaSearchHeight)) / 2;
  layout.search = {padding * 2, fieldTop, paneRight - padding, fieldTop + s(kVistaSearchHeight)};

  const int rightLeft = paneRight + padding;
  const int arrowTop = height - padding - s(kVistaArrowHeight);
  const int buttonTop = arrowTop + (s(kVistaArrowHeight) - s(kVistaButton)) / 2;
  layout.power = {rightLeft, buttonTop, rightLeft + s(kVistaButton), buttonTop + s(kVistaButton)};
  layout.lock = {layout.power.right + s(kVistaButtonGap), buttonTop,
                 layout.power.right + s(kVistaButtonGap) + s(kVistaButton),
                 buttonTop + s(kVistaButton)};
  layout.powerArrow = {layout.lock.right + s(kVistaButtonGap), arrowTop,
                       layout.lock.right + s(kVistaButtonGap) + s(kVistaArrowWidth),
                       arrowTop + s(kVistaArrowHeight)};
  layout.placeHeight = s(kVistaPlace);
  layout.places.bottom = arrowTop - padding;
  layout.jumpList = {};
  return layout;
}

MenuPosition PlaceStartMenu(const Rect& anchor, bool below, int width, int height,
                            const Rect& screen, const Rect& work, MenuHorizontal horizontal,
                            MenuVertical vertical, int gap) {
  MenuPosition position;
  // Left: from the start button, kept inside the work area so it never
  // covers a bar on the left or right edge; right: against the work area.
  position.x = horizontal == MenuHorizontal::Right
                   ? work.right - width
                   : std::clamp(anchor.left, work.left, std::max(work.left, work.right - width));
  position.x = std::clamp(position.x, screen.left, std::max(screen.left, screen.right - width));

  // Against the bar when the menu opens on the bar's side, else against the
  // work area's edge.
  const bool top = vertical == MenuVertical::Bar ? below : vertical == MenuVertical::Top;
  if (top) {
    position.y = (below ? anchor.bottom : work.top) + gap;
  } else {
    position.y = (below ? work.bottom : anchor.top) - height - gap;
  }
  position.y = std::clamp(position.y, screen.top, std::max(screen.top, screen.bottom - height));
  return position;
}

}  // namespace throwback
