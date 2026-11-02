#include "core/start_menu_layout.h"

namespace throwback {
namespace {

// Windows 7 proportions at 96 DPI, rounded to the 4 px grid.
constexpr int kWidth = 400;
constexpr int kPadding = 8;
constexpr int kLeftWidth = 248;
constexpr int kListRows = 10;
constexpr int kRowHeight = 40;
constexpr int kAllProgramsHeight = 40;
constexpr int kBottomHeight = 48;  // search box and power button row
constexpr int kControlHeight = 24;
constexpr int kPictureSize = 48;
constexpr int kPlaceHeight = 32;
constexpr int kPowerArrowWidth = 24;

}  // namespace

StartMenuLayout ComputeStartMenuLayout(unsigned dpi) {
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

  // Controls in the bottom row sit centered in it.
  const int controlTop = bottomTop + (s(kBottomHeight) - s(kControlHeight)) / 2;
  const int controlBottom = controlTop + s(kControlHeight);
  layout.search = {layout.leftPane.left + padding, controlTop, layout.leftPane.right - padding,
                   controlBottom};

  const int rightLeft = layout.leftPane.right + padding;
  const int rightRight = width - padding;
  const int pictureLeft = (rightLeft + rightRight - s(kPictureSize)) / 2;
  layout.picture = {pictureLeft, padding, pictureLeft + s(kPictureSize), padding + s(kPictureSize)};
  layout.places = {rightLeft, layout.picture.bottom + padding * 2, rightRight, bottomTop};
  layout.powerArrow = {rightRight - s(kPowerArrowWidth), controlTop, rightRight, controlBottom};
  layout.power = {rightLeft, controlTop, layout.powerArrow.left, controlBottom};
  return layout;
}

}  // namespace throwback
