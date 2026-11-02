#pragma once

#include <d2d1.h>
#include <wincodec.h>
#include <windows.h>
#include <wrl/client.h>

#include <map>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "core/layout.h"
#include "ui/fonts.h"
#include "ui/theme.h"

namespace throwback::ui {

enum class ButtonVariant { Ghost, Selected, Attention };
enum class IconKind { Start };

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

  void Button(const Rect& rect, ButtonVariant variant, bool hovered);
  // A one-pixel vertical separator along the left edge of rect.
  void Divider(const Rect& rect);
  void Text(const Rect& rect, std::wstring_view text, TextStyle style,
            TextAlign align = TextAlign::Leading);
  void Icon(const Rect& rect, IconKind kind);
  void Icon(const Rect& rect, HICON icon);

 private:
  void CreateDeviceResources();
  void DiscardDeviceResources();
  IDWriteTextFormat* Format(TextStyle style, TextAlign align);
  ID2D1Bitmap* IconBitmap(HICON icon);
  void Fill(const Rect& rect, const Color& color);

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
};

}  // namespace throwback::ui
