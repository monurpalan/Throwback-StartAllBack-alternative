#pragma once

#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <cstddef>

namespace throwback::ui {

// The Windows 7 images the UI kit draws with when they are embedded in the
// executable (assets/win7). Without them each component draws itself.
enum class SkinImage {
  Orb,
  PictureFrame,
  Taskbar,
  TaskButton,
  ShowDesktop,
  MenuPrograms,
  MenuSearch,
  MenuPlaces,
  MenuJumpList,
  MenuPower,
  ShutDown,
  Arrow,          // the white arrow of the shut down button's menu half
  PlaceItem,      // a hovered row of the right column
  ProgramItem,    // a program row: normal, hot, selected, newly installed
  TrayArrow,      // shows the hidden notification icons
  TrayArrowOpen,  // the same while they are shown
  TrayItem,       // a notification icon under the mouse
  FlyoutLink,     // the light strip at the bottom of a flyout
  Banner98,       // the classic start menus' vertical banners
  BannerMe,
  Banner2000,
  Flag,  // the four-color flag of the classic Start button
  XpStartButton,
  XpFlag,
  XpUserPane,
  XpUserTile,
  XpPrograms,
  XpProgramsSeparator,
  XpMorePrograms,
  XpMoreArrow,
  XpMoreArrowHot,
  XpPlaces,
  XpPlacesSeparator,
  XpLogOff,
  XpLogOffButtons,
  XpLogOffButtonsHot,
  XpMenu,
  XpMenuItem,
  XpMenuSeparator,
  VistaOrb,
  VistaPane,
  VistaMorePrograms,
  VistaPlaces,
  VistaSearch,
  VistaProgramHover,
  VistaPlaceHover,
  VistaPower,
  VistaPowerHot,
  VistaArrowButton,
  VistaArrow,
  VistaMoreArrow,
  VistaMoreArrowHot,
  VistaBackArrow,
  VistaBackArrowHot,
  VistaPictureFrame,
  VistaProgramSeparator,
  VistaPlaceSeparator,
  W81Logo,  // Windows 8.1's Start button and its hover glow
  W81Glow,
  W11Logo,  // Windows 11's Start button
  W10Logo,  // Windows 10's: the flat logo, as 8.1 drew it small
  // The taskbars of XP, Vista and 8.
  XpTaskbar,
  XpTaskButton,
  XpTaskFlash,
  XpTray,
  XpTrayArrow,
  XpTrayArrowOpen,
  VistaTaskbar,
  VistaTaskButton,
  VistaTrayArrow,
  VistaTrayArrowOpen,
  W8Taskbar,
  W8TaskButton,
  W8ShowDesktop,
  W8TrayArrow,
  W8TrayArrowOpen,
  W8TrayItem,
  // More of Windows 7's start menu.
  PowerArrow,        // the menu half of the shut down button
  ProgramSeparator,  // the line between pinned and frequent programs
  PlaceSeparator,    // the line between groups of the right column
  SearchGlyph,       // the magnifier at the end of the search box
  SearchClear,       // the button that empties the search box
  MoreArrow,         // the arrow of "All Programs"
  BackArrow,         // the arrow of "Back"
  SplitMain,         // a program row with a jump list: the program half
  SplitArrow,        // and the half with the arrow that opens the list
};

// Which theme's images to draw with. Plain8 replaces the Windows 7 start
// menu and taskbar parts it has with its flat ones; everything else stays.
enum class SkinSet { Aero, Plain8 };

// How an image is cut, as the Windows 7 theme describes it: its states are
// frames stacked top to bottom, and the edges keep their size while the
// middle stretches. Edges are in pixels of the image file; images with one
// file per DPI have none.
struct SkinPart {
  int resource = 0;  // for images with one file per DPI, the 96 DPI one
  int frames = 1;
  bool perDpi = false;  // 96, 120, 144 and 192 DPI files follow each other
  int left = 0;
  int right = 0;
  int top = 0;
  int bottom = 0;
};

const SkinPart& PartOf(SkinImage image, SkinSet set = SkinSet::Aero);

// The resource of a part for a DPI: the smallest file at least as large, or
// the largest one.
int ResourceFor(const SkinPart& part, unsigned dpi);

// Decodes an embedded image as premultiplied BGRA; null when it isn't
// embedded.
Microsoft::WRL::ComPtr<IWICBitmapSource> LoadSkinImage(IWICImagingFactory* wic, int resource);

}  // namespace throwback::ui
