#include "core/start_menu_layout.h"

#include <algorithm>

namespace throwback {
namespace {

// Windows 7 proportions at 96 DPI, rounded to the 4 px grid where Windows 7
// itself doesn't use exact sizes.
constexpr int kWidth = 400;
constexpr int kPadding = 8;
constexpr int kLeftWidth = 248;
constexpr int kListRows = 10;
constexpr int kRowHeight = 40;
constexpr int kAllProgramsHeight = 40;
// The row under the panes. The search box and the shut down button sit where
// the Windows 7 images put them.
constexpr int kBottomHeight = 54;
constexpr int kSearchLeft = 21;
constexpr int kSearchTop = 12;
constexpr int kSearchRight = 18;  // from the program pane's right edge
constexpr int kSearchHeight = 22;
constexpr int kControlTop = 11;
constexpr int kControlHeight = 24;
constexpr int kPowerWidth = 76;  // the narrowest it gets
constexpr int kPowerPadding = 12;
constexpr int kPictureSize = 48;
constexpr int kPlaceHeight = 32;
constexpr int kPowerArrowWidth = 24;
constexpr int kJumpRowHeight = 24;

}  // namespace

StartMenuLayout ComputeStartMenuLayout(unsigned dpi, int powerLabelWidth) {
  const auto s = [dpi](int value) { return ScaleForDpi(value, dpi); };
  const int padding = s(kPadding);
  const int width = s(kWidth);
  const int listHeight = kListRows * s(kRowHeight);
  const int height =
      padding + listHeight + padding + s(kAllProgramsHeight) + padding + s(kBottomHeight);

  StartMenuLayout layout;
  layout.rowHeight = s(kRowHeight);
  layout.placeHeight = s(kPlaceHeight);
  layout.listRows = kListRows;
  layout.frame = {0, 0, width, height};

  const int bottomTop = height - s(kBottomHeight);
  layout.leftPane = {padding, padding, padding + s(kLeftWidth), bottomTop};
  layout.list = {layout.leftPane.left, layout.leftPane.top, layout.leftPane.right,
                 layout.leftPane.top + listHeight};
  layout.allPrograms = {layout.leftPane.left, layout.leftPane.bottom - s(kAllProgramsHeight),
                        layout.leftPane.right, layout.leftPane.bottom};

  layout.programsArea = {0, 0, layout.leftPane.right, bottomTop};
  layout.searchArea = {0, bottomTop, layout.leftPane.right, height};
  layout.placesArea = {layout.leftPane.right, 0, width, bottomTop};
  layout.powerArea = {layout.leftPane.right, bottomTop, width, height};

  const int searchTop = bottomTop + s(kSearchTop);
  layout.search = {s(kSearchLeft), searchTop, layout.leftPane.right - s(kSearchRight),
                   searchTop + s(kSearchHeight)};
  const int controlTop = bottomTop + s(kControlTop);
  const int controlBottom = controlTop + s(kControlHeight);

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
                                         int turnOffLabelWidth) {
  // Luna's NormalBlue.ini at 96 DPI: a 380 x 440 panel of a 64 px banner,
  // two 190 px wide columns and a 40 px log off bar.
  constexpr int kXpWidth = 380;
  constexpr int kXpBanner = 64;
  constexpr int kXpColumn = 190;
  constexpr int kXpColumns = 336;
  constexpr int kXpMorePrograms = 30;
  constexpr int kXpBar = 40;
  constexpr int kXpRow = 36;
  constexpr int kXpRows = 8;
  constexpr int kXpPlace = 26;
  constexpr int kXpPicture = 48;
  constexpr int kXpPictureLeft = 8;
  constexpr int kXpButtonIcon = 24;
  constexpr int kXpButtonGap = 4;
  constexpr int kXpButtonPadding = 8;
  const auto s = [dpi](int value) { return ScaleForDpi(value, dpi); };

  StartMenuLayout layout;
  const int width = s(kXpWidth);
  const int columnsTop = s(kXpBanner);
  const int barTop = columnsTop + s(kXpColumns);
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
  const int listTop = columnsTop + s(9);
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

}  // namespace throwback
