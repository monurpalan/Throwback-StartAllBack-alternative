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
#include "ui/theme.h"

namespace throwback::ui {

enum class ButtonVariant {
  Ghost,      // flat; only hover shows
  Selected,   // the active item: filled with an accent line
  Attention,  // asking for the user's attention
  Raised,     // a standalone push button
};
enum class TextTone { Normal, Muted };
enum class IconKind { Start, ChevronRight, ChevronLeft };

// A window's drawing surface plus the UI kit components. Screens draw only
// through these calls; colors, fonts and sizes come from the theme. All
// coordinates are physical pixels.
class Canvas {
 public:
  explicit Canvas(HWND hwnd);

  void Resize(UINT width, UINT height);
  void SetTheme(const Theme& theme) { theme_ = theme; }
  void SetDpi(unsigned dpi);
  // Converts a 96 DPI size to physical pixels.
  int Scale(int pixels) const { return ScaleForDpi(pixels, dpi_); }

  // Draw calls are only valid between BeginFrame and EndFrame.
  void BeginFrame();
  void EndFrame();

  // A surface set apart from the window background, such as a menu's pane.
  void Panel(const Rect& rect);
  void Button(const Rect& rect, ButtonVariant variant, bool hovered);
  // A separator line filling rect; callers pass a one-pixel-thin rect.
  void Divider(const Rect& rect);
  void Text(const Rect& rect, std::wstring_view text, TextStyle style,
            TextAlign align = TextAlign::Leading, TextTone tone = TextTone::Normal);
  // A single-line input box; shows the placeholder while text is empty.
  void TextField(const Rect& rect, std::wstring_view text, std::wstring_view placeholder,
                 bool focused);
  void Icon(const Rect& rect, IconKind kind);
  void Icon(const Rect& rect, HICON icon);
  // An image file such as the account picture, scaled to fill rect.
  void Picture(const Rect& rect, const std::wstring& path);

 private:
  void CreateDeviceResources();
  void DiscardDeviceResources();
  IDWriteTextFormat* Format(TextStyle style, TextAlign align);
  ID2D1Bitmap* IconBitmap(HICON icon);
  ID2D1Bitmap* PictureBitmap(const std::wstring& path);
  void Fill(const Rect& rect, const Color& color);
  float TextWidth(std::wstring_view text, TextStyle style);

  HWND hwnd_;
  unsigned dpi_ = 96;
  Theme theme_ = MakeTheme(false);

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
};

}  // namespace throwback::ui
