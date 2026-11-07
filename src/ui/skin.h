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
  Arrow,
  PaneItem,
  GlassItem,
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
};

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

const SkinPart& PartOf(SkinImage image);

// The resource of a part for a DPI: the smallest file at least as large, or
// the largest one.
int ResourceFor(const SkinPart& part, unsigned dpi);

// Decodes an embedded image as premultiplied BGRA; null when it isn't
// embedded.
Microsoft::WRL::ComPtr<IWICBitmapSource> LoadSkinImage(IWICImagingFactory* wic, int resource);

}  // namespace throwback::ui
