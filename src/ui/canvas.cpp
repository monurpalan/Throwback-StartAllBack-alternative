#include "ui/canvas.h"

#include <string>

#include "ui/com_util.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {
namespace {

D2D1_COLOR_F ToD2D(const Color& color) { return D2D1::ColorF(color.r, color.g, color.b, color.a); }

D2D1_RECT_F ToRectF(const Rect& rect) {
  return D2D1::RectF(static_cast<float>(rect.left), static_cast<float>(rect.top),
                     static_cast<float>(rect.right), static_cast<float>(rect.bottom));
}

std::wstring ToUpper(std::wstring_view text) {
  std::wstring result(text);
  if (!result.empty()) {
    LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE | LCMAP_LINGUISTIC_CASING, text.data(),
                  static_cast<int>(text.size()), result.data(), static_cast<int>(result.size()),
                  nullptr, nullptr, 0);
  }
  return result;
}

}  // namespace

Canvas::Canvas(HWND hwnd) : hwnd_(hwnd) {
  ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory_.GetAddressOf()),
                "D2D1CreateFactory");
  ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                                    reinterpret_cast<IUnknown**>(dwrite_.GetAddressOf())),
                "DWriteCreateFactory");
  ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARGS(wic_.GetAddressOf())),
                "CoCreateInstance(WICImagingFactory)");
  fonts_ = std::make_unique<Fonts>(dwrite_.Get());
}

void Canvas::Resize(UINT width, UINT height) {
  if (target_) target_->Resize(D2D1::SizeU(width, height));
}

void Canvas::SetDpi(unsigned dpi) {
  if (dpi == dpi_) return;
  dpi_ = dpi;
  formats_.clear();
}

void Canvas::CreateDeviceResources() {
  if (target_) return;
  RECT client{};
  GetClientRect(hwnd_, &client);
  // Fixed 96 DPI so one Direct2D unit equals one pixel; Scale() handles DPI.
  const auto properties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                                       D2D1::PixelFormat(), 96.0f, 96.0f);
  const auto hwndProperties = D2D1::HwndRenderTargetProperties(
      hwnd_, D2D1::SizeU(static_cast<UINT32>(client.right - client.left),
                         static_cast<UINT32>(client.bottom - client.top)));
  ThrowIfFailed(
      factory_->CreateHwndRenderTarget(properties, hwndProperties, target_.GetAddressOf()),
      "CreateHwndRenderTarget");
  ThrowIfFailed(target_->CreateSolidColorBrush(ToD2D(theme_.foreground), brush_.GetAddressOf()),
                "CreateSolidColorBrush");
  target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

void Canvas::DiscardDeviceResources() {
  iconCache_.clear();
  brush_.Reset();
  target_.Reset();
}

void Canvas::BeginFrame() {
  CreateDeviceResources();
  usedIcons_.clear();
  target_->BeginDraw();
  target_->Clear(ToD2D(theme_.background));
}

void Canvas::EndFrame() {
  if (target_->EndDraw() == D2DERR_RECREATE_TARGET) {
    DiscardDeviceResources();
    return;
  }
  // Drop bitmaps of windows that are gone so the cache doesn't grow forever.
  std::erase_if(iconCache_,
                [this](const auto& entry) { return !usedIcons_.contains(entry.first); });
}

IDWriteTextFormat* Canvas::Format(TextStyle style, TextAlign align) {
  auto& format = formats_[{style, align}];
  if (!format) format = fonts_->CreateFormat(style, align, dpi_);
  return format.Get();
}

void Canvas::Fill(const Rect& rect, const Color& color) {
  if (rect.Width() <= 0 || rect.Height() <= 0) return;
  brush_->SetColor(ToD2D(color));
  target_->FillRectangle(ToRectF(rect), brush_.Get());
}

void Canvas::Button(const Rect& rect, ButtonVariant variant, bool hovered) {
  if (variant == ButtonVariant::Selected) {
    Fill(rect, hovered ? theme_.selectedHover : theme_.selected);
    const int inset = Scale(kSpace8);
    Fill({rect.left + inset, rect.bottom - Scale(kSpace4), rect.right - inset, rect.bottom},
         theme_.accent);
  } else if (hovered) {
    Fill(rect, theme_.hover);
  }
}

void Canvas::Text(const Rect& rect, std::wstring_view text, TextStyle style, TextAlign align) {
  if (text.empty() || rect.Width() <= 0 || rect.Height() <= 0) return;
  const std::wstring shown = IsUppercase(style) ? ToUpper(text) : std::wstring(text);
  brush_->SetColor(ToD2D(theme_.foreground));
  target_->DrawText(shown.c_str(), static_cast<UINT32>(shown.size()), Format(style, align),
                    ToRectF(rect), brush_.Get(), D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

}  // namespace throwback::ui
