#pragma once

#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "core/layout.h"
#include "ui/fonts.h"
#include "ui/skin.h"
#include "ui/theme.h"

namespace throwback::ui {

// The bright rim of bar glass faces the rest of the screen.
enum class GlassVariant { BarBottom, BarTop, Menu };
enum class ButtonVariant {
  MenuItem,       // a row on the white pane; drawn only while hovered
  GlassItem,      // a row on glass; drawn only while hovered
  Task,           // a taskbar button
  ActiveTask,     // the button of the foreground window
  FlashingTask,   // a window asking for attention
  Command,        // the glossy push button
  ShowDesktop,    // the slim button at the end of the bar
  TrayItem,       // a notification icon; drawn only while hovered
  TrayArrow,      // the arrow that shows hidden notification icons
  TrayArrowOpen,  // the same arrow while they are shown
};
// The parts of the start menu's background, each covering one corner area of
// the menu up to its outer edge.
enum class PanelVariant {
  Programs,    // the white program pane, top left
  Search,      // the strip under it that holds the search box
  Places,      // the right column
  JumpList,    // the right column while a jump list replaces it
  Power,       // the corner under the right column with the shut down button
  FlyoutLink,  // the light strip at the bottom of a flyout, for a link
};
enum class SeparatorVariant { Pane, Glass };
enum class ProgressVariant { Normal, Paused, Error, Indeterminate };
enum class TextTone {
  Dark,   // on the white pane and on push buttons
  Light,  // on glass: white with a dark glow
  Muted,  // prompts and messages
  Link,   // a link on a light strip
};
enum class IconKind {
  ChevronRight,
  ChevronLeft,
  Search,
  MenuArrow,  // the arrow on the glass push button
};
enum class OrbState { Normal, Hot, Pressed };
// The classic styles: Windows Me looks like Windows 98.
enum class ClassicLook { Win98, Win2000 };

// A window's drawing surface plus the UI kit components. Screens draw only
// through these calls; colors, fonts and sizes come from the theme. All
// coordinates are physical pixels.
class Canvas {
 public:
  explicit Canvas(HWND hwnd);

  void Resize(UINT width, UINT height);
  void SetDpi(unsigned dpi);
  // Converts a 96 DPI size to physical pixels.
  int Scale(int pixels) const { return ScaleForDpi(pixels, dpi_); }

  // Draw calls are only valid between BeginFrame and EndFrame. A frame starts
  // fully transparent, so glass windows show the blurred desktop.
  void BeginFrame();
  void EndFrame();

  void Glass(const Rect& rect, GlassVariant variant);
  // A part of the start menu's background; rect is its whole area.
  void Panel(const Rect& rect, PanelVariant variant);
  void Button(const Rect& rect, ButtonVariant variant, bool hovered);
  // A taskbar button's progress, filling fraction of it from the left.
  void Progress(const Rect& rect, ProgressVariant variant, float fraction);
  // A separator line; callers pass a one-pixel-thin rect.
  void Separator(const Rect& rect, SeparatorVariant variant);
  void Text(const Rect& rect, std::wstring_view text, TextStyle style, TextTone tone,
            TextAlign align = TextAlign::Leading);
  // The width of a line of text in pixels, rounded up.
  int MeasureText(std::wstring_view text, TextStyle style);
  // The search box; shows the prompt while text is empty.
  void TextField(const Rect& rect, std::wstring_view text, std::wstring_view prompt, bool focused);
  void Icon(const Rect& rect, IconKind kind);
  void Icon(const Rect& rect, HICON icon);
  void Orb(const Rect& rect, OrbState state);
  // The framed account picture; shows the initial when there is no image.
  void AccountPicture(const Rect& rect, const std::wstring& path, std::wstring_view initial);

  // Classic components (classic.cpp). A face is the raised gray surface of
  // menus and buttons inside a four-line 3D edge; sunken for a pressed
  // button.
  void ClassicFace(const Rect& rect, ClassicLook look, bool sunken = false);
  // A menu row, filled with the selection color while selected.
  void ClassicItem(const Rect& rect, ClassicLook look, bool selected);
  // An etched line across the middle of rect.
  void ClassicSeparator(const Rect& rect);
  void ClassicText(const Rect& rect, std::wstring_view text, ClassicLook look, bool selected,
                   bool bold = false);
  // The small triangle of a row with a submenu.
  void ClassicArrow(const Rect& rect, bool selected);
  // A start menu banner standing on the bottom of rect; above it, rect
  // takes the banner's top color.
  void ClassicBanner(const Rect& rect, SkinImage banner);
  // The Start button: a face with the flag and a bold label.
  void ClassicStartButton(const Rect& rect, ClassicLook look, std::wstring_view label,
                          bool pressed);

 private:
  void CreateDeviceResources();
  void DiscardDeviceResources();
  IDWriteTextFormat* Format(TextStyle style, TextAlign align);
  ID2D1Bitmap* IconBitmap(HICON icon);
  ID2D1Bitmap* PictureBitmap(const std::wstring& path);
  float TextWidth(std::wstring_view text, TextStyle style);
  void DrawTextRun(const Rect& rect, std::wstring_view text, IDWriteTextFormat* format,
                   const Color& color, float dx, float dy);
  void TaskButton(const Rect& rect, ButtonVariant variant, bool hovered);
  void TrayArrow(const Rect& rect, bool open, bool hovered);
  void DrawTriangle(const Rect& rect, bool up, const Color& color);
  void DrawChevron(const Rect& rect, bool right, const Color& color);
  void DrawSearchGlyph(const Rect& rect);
  void DrawFlag(float cx, float cy, float size);

  // The Windows 7 images (see skin.h). Skin draws a frame of one stretched
  // over rect and returns false when the image isn't embedded.
  ID2D1Bitmap* SkinBitmap(int resource);
  bool HasSkin(SkinImage image);
  bool Skin(SkinImage image, int frame, const Rect& rect);

  // Primitives behind the components.
  void Fill(const Rect& rect, const Color& color);
  void FillRound(const Rect& rect, float radius, const Color& color);
  void FillGradient(const Rect& rect, float radius, const Color& top, const Color& bottom);
  void FillGlow(const Rect& rect, const Color& color);
  void StrokeRound(const Rect& rect, float radius, const Color& color);
  ID2D1SolidColorBrush* Brush(const Color& color);

  HWND hwnd_;
  unsigned dpi_ = 96;

  Microsoft::WRL::ComPtr<ID2D1Factory> factory_;
  Microsoft::WRL::ComPtr<IDWriteFactory> dwrite_;
  Microsoft::WRL::ComPtr<IWICImagingFactory> wic_;
  std::unique_ptr<Fonts> fonts_;
  std::map<std::pair<TextStyle, TextAlign>, Microsoft::WRL::ComPtr<IDWriteTextFormat>> formats_;

  // Device resources: recreated when the GPU device is lost.
  Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> target_;
  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_;
  std::unordered_map<HICON, Microsoft::WRL::ComPtr<ID2D1Bitmap>> iconCache_;
  std::unordered_set<HICON> usedIcons_;
  std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID2D1Bitmap>> pictures_;
  std::unordered_map<int, Microsoft::WRL::ComPtr<ID2D1Bitmap>> skins_;
};

}  // namespace throwback::ui
