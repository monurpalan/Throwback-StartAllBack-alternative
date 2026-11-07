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

}  // namespace throwback
