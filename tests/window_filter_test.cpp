#include "core/window_filter.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

WindowTraits NormalWindow() { return {.visible = true}; }

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

TEST(WindowFilterTest, AppWindowStyleOverridesOwnerAndNoActivate) {
  WindowTraits traits = NormalWindow();
  traits.hasOwner = true;
  traits.noActivate = true;
  traits.appWindow = true;
  EXPECT_TRUE(ShouldShowOnTaskbar(traits));
}

TEST(WindowFilterTest, ToolWindowStaysHiddenEvenWithAppWindowStyle) {
  WindowTraits traits = NormalWindow();
  traits.toolWindow = true;
  traits.appWindow = true;
  EXPECT_FALSE(ShouldShowOnTaskbar(traits));
}

TEST(WindowFilterTest, AppWindowStillHiddenWhenCloaked) {
  WindowTraits traits = NormalWindow();
  traits.appWindow = true;
  traits.cloaked = true;
  EXPECT_FALSE(ShouldShowOnTaskbar(traits));
}

TEST(WindowFilterTest, HidesWindowsThatDeletedTheirTab) {
  WindowTraits traits = NormalWindow();
  traits.taskListDeleted = true;
  EXPECT_FALSE(ShouldShowOnTaskbar(traits));
}

TEST(WindowFilterTest, ImmersiveClassNeedsWindowEdge) {
  WindowTraits shellSurface = NormalWindow();
  shellSurface.immersiveClass = true;
  EXPECT_FALSE(ShouldShowOnTaskbar(shellSurface));

  WindowTraits storeApp = shellSurface;
  storeApp.windowEdge = true;
  EXPECT_TRUE(ShouldShowOnTaskbar(storeApp));
}

}  // namespace
}  // namespace throwback
