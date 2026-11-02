#include "core/window_filter.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

WindowTraits NormalWindow() { return {.visible = true, .hasTitle = true}; }

TEST(WindowFilterTest, ShowsNormalAppWindow) { EXPECT_TRUE(ShouldShowOnTaskbar(NormalWindow())); }

TEST(WindowFilterTest, HidesInvisibleAndCloakedWindows) {
  WindowTraits hidden = NormalWindow();
  hidden.visible = false;
  EXPECT_FALSE(ShouldShowOnTaskbar(hidden));

  WindowTraits cloaked = NormalWindow();
  cloaked.cloaked = true;
  EXPECT_FALSE(ShouldShowOnTaskbar(cloaked));
}

TEST(WindowFilterTest, HidesToolOwnedAndNoActivateWindows) {
  for (auto flag :
       {&WindowTraits::toolWindow, &WindowTraits::hasOwner, &WindowTraits::noActivate}) {
    WindowTraits traits = NormalWindow();
    traits.*flag = true;
    EXPECT_FALSE(ShouldShowOnTaskbar(traits));
  }
}

TEST(WindowFilterTest, AppWindowStyleOverridesOwnerAndToolStyle) {
  WindowTraits traits = NormalWindow();
  traits.hasOwner = true;
  traits.toolWindow = true;
  traits.appWindow = true;
  EXPECT_TRUE(ShouldShowOnTaskbar(traits));
}

TEST(WindowFilterTest, AppWindowStillHiddenWhenCloaked) {
  WindowTraits traits = NormalWindow();
  traits.appWindow = true;
  traits.cloaked = true;
  EXPECT_FALSE(ShouldShowOnTaskbar(traits));
}

TEST(WindowFilterTest, HidesUntitledWindows) {
  WindowTraits traits = NormalWindow();
  traits.hasTitle = false;
  EXPECT_FALSE(ShouldShowOnTaskbar(traits));
}

}  // namespace
}  // namespace throwback
