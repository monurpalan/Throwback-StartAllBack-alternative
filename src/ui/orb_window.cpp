#include "ui/orb_window.h"

#include <d2d1_3.h>
#include <shlwapi.h>

#include <utility>

#include "core/layout.h"
#include "ui/skin.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {
namespace {

constexpr wchar_t kClassName[] = L"ThrowbackOrb";
constexpr int kFrames = 3;
// StartAllBack's logo size on a bar with normal icons, at 96 DPI.
constexpr int kLogoSize = 24;
constexpr float kLogoCorner = 3.0f;

// One frame of each orb at 96 DPI; pictures take the Windows 7 orb's size.
SIZE FrameSizeOf(const OrbArt& art) {
  return art.kind == OrbArt::Kind::Skin && art.skin == SkinImage::VistaOrb ? SIZE{60, 46}
                                                                           : SIZE{54, 54};
}

ComPtr<IStream> ResourceStream(int resource) {
  const HMODULE module = GetModuleHandleW(nullptr);
  const HRSRC info = FindResourceW(module, MAKEINTRESOURCEW(resource), RT_RCDATA);
  const HGLOBAL data = info ? LoadResource(module, info) : nullptr;
  const auto* bytes = static_cast<const BYTE*>(data ? LockResource(data) : nullptr);
  if (!bytes) return nullptr;
  return ComPtr<IStream>(SHCreateMemStream(bytes, SizeofResource(module, info)));
}

}  // namespace

std::unique_ptr<OrbWindow> OrbWindow::Create(HINSTANCE instance, HWND owner) {
  ComPtr<IWICImagingFactory> wic;
  if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(wic.GetAddressOf()))) ||
      !LoadSkinImage(wic.Get(), PartOf(SkinImage::Orb).resource)) {
    return nullptr;
  }

  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = DefWindowProcW;
  windowClass.hInstance = instance;
  windowClass.lpszClassName = kClassName;
  RegisterClassExW(&windowClass);  // fails harmlessly for the second bar

  // Layered and transparent to the mouse; owned by the bar, so it stays just
  // above it in z-order, also when a full screen window pushes the bar down.
  const HWND hwnd =
      CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                      kClassName, L"", WS_POPUP, 0, 0, 0, 0, owner, nullptr, instance, nullptr);
  if (!hwnd) return nullptr;
  return std::unique_ptr<OrbWindow>(new OrbWindow(hwnd, std::move(wic)));
}

OrbWindow::OrbWindow(HWND hwnd, ComPtr<IWICImagingFactory> wic)
    : hwnd_(hwnd), wic_(std::move(wic)) {}

OrbWindow::~OrbWindow() {
  if (IsWindow(hwnd_)) DestroyWindow(hwnd_);
  if (strip_) DeleteObject(strip_);
}

ComPtr<IWICBitmapSource> OrbWindow::ScaledStrip(int resource, int width, int height) {
  ComPtr<IWICBitmapSource> source = LoadSkinImage(wic_.Get(), resource);
  if (!source) return nullptr;
  UINT fileWidth = 0;
  UINT fileHeight = 0;
  source->GetSize(&fileWidth, &fileHeight);
  if (fileWidth == static_cast<UINT>(width) && fileHeight == static_cast<UINT>(height * kFrames)) {
    return source;
  }
  ComPtr<IWICBitmapScaler> scaler;
  if (FAILED(wic_->CreateBitmapScaler(scaler.GetAddressOf())) ||
      FAILED(scaler->Initialize(source.Get(), static_cast<UINT>(width),
                                static_cast<UINT>(height * kFrames),
                                WICBitmapInterpolationModeHighQualityCubic))) {
    return nullptr;
  }
  return scaler;
}

ComPtr<IWICBitmapSource> OrbWindow::LogoStrip(int resource, int width, int height, unsigned dpi) {
  ComPtr<ID2D1Factory> factory;
  ComPtr<IWICBitmap> bitmap;
  ComPtr<ID2D1RenderTarget> target;
  ComPtr<ID2D1DeviceContext5> context;
  const ComPtr<IStream> stream = ResourceStream(resource);
  if (!stream ||
      FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.GetAddressOf())) ||
      FAILED(wic_->CreateBitmap(static_cast<UINT>(width), static_cast<UINT>(height * kFrames),
                                GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnDemand,
                                bitmap.GetAddressOf())) ||
      FAILED(factory->CreateWicBitmapRenderTarget(
          bitmap.Get(),
          D2D1::RenderTargetProperties(
              D2D1_RENDER_TARGET_TYPE_DEFAULT,
              D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
          target.GetAddressOf())) ||
      FAILED(target.As(&context))) {
    return nullptr;
  }
  // StartAllBack draws logos smaller than 192 DPI at odd sizes, so they sit
  // on whole pixels in the middle of the button.
  int logo = ScaleForDpi(kLogoSize, dpi);
  if (dpi < 192 && logo % 2 == 0) --logo;
  ComPtr<ID2D1SvgDocument> document;
  if (FAILED(context->CreateSvgDocument(
          stream.Get(), D2D1::SizeF(static_cast<float>(logo), static_cast<float>(logo)),
          document.GetAddressOf()))) {
    return nullptr;
  }
  ComPtr<ID2D1SolidColorBrush> hot;
  ComPtr<ID2D1SolidColorBrush> pressed;
  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.18f), hot.GetAddressOf());
  context->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.10f), pressed.GetAddressOf());

  context->BeginDraw();
  context->Clear(D2D1::ColorF(0, 0.0f));
  const float corner = kLogoCorner * static_cast<float>(dpi) / 96.0f;
  for (int frame = 0; frame < kFrames; ++frame) {
    const float top = static_cast<float>(frame * height);
    // The hot and pressed frames light up the button behind the logo.
    if (frame > 0) {
      const D2D1_ROUNDED_RECT box{
          D2D1::RectF(0.0f, top, static_cast<float>(width), top + static_cast<float>(height)),
          corner, corner};
      context->FillRoundedRectangle(box, frame == 1 ? hot.Get() : pressed.Get());
    }
    context->SetTransform(D2D1::Matrix3x2F::Translation(
        static_cast<float>((width - logo) / 2), top + static_cast<float>((height - logo) / 2)));
    context->DrawSvgDocument(document.Get());
    context->SetTransform(D2D1::Matrix3x2F::Identity());
  }
  if (FAILED(context->EndDraw())) return nullptr;
  return bitmap;
}

bool OrbWindow::CopyStrip(IWICBitmapSource* source, int width, int height) {
  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(info.bmiHeader);
  info.bmiHeader.biWidth = width;
  info.bmiHeader.biHeight = -height * kFrames;  // top-down, like the image
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  void* bits = nullptr;
  const HBITMAP strip = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
  if (!strip) return false;
  const UINT stride = static_cast<UINT>(width) * 4;
  if (FAILED(source->CopyPixels(nullptr, stride, stride * static_cast<UINT>(height * kFrames),
                                static_cast<BYTE*>(bits)))) {
    DeleteObject(strip);
    return false;
  }
  if (strip_) DeleteObject(strip_);
  strip_ = strip;
  width_ = width;
  height_ = height;
  return true;
}

bool OrbWindow::LoadStrip(SIZE button, unsigned dpi, const OrbArt& art) {
  if (strip_ && stripDpi_ == dpi && stripArt_ == art && stripButton_.cx == button.cx &&
      stripButton_.cy == button.cy) {
    return true;
  }
  ComPtr<IWICBitmapSource> source;
  int width = static_cast<int>(button.cx);
  int height = static_cast<int>(button.cy);
  if (art.kind == OrbArt::Kind::Logo) {
    source = LogoStrip(art.resource, width, height, dpi);
  } else {
    const SIZE frame = FrameSizeOf(art);
    width = ScaleForDpi(static_cast<int>(frame.cx), dpi);
    height = ScaleForDpi(static_cast<int>(frame.cy), dpi);
    const int resource =
        art.kind == OrbArt::Kind::Skin ? ResourceFor(PartOf(art.skin), dpi) : art.resource;
    source = ScaledStrip(resource, width, height);
  }
  if (!source || width <= 0 || height <= 0 || !CopyStrip(source.Get(), width, height)) {
    return false;
  }
  stripDpi_ = dpi;
  stripArt_ = art;
  stripButton_ = button;
  return true;
}

void OrbWindow::Show(POINT center, SIZE button, unsigned dpi, OrbState state, const OrbArt& art) {
  if (shown_ && stripDpi_ == dpi && stripArt_ == art && stripButton_.cx == button.cx &&
      stripButton_.cy == button.cy && center.x == center_.x && center.y == center_.y &&
      state == state_) {
    return;
  }
  if (!LoadStrip(button, dpi, art)) return;

  const HDC screen = GetDC(nullptr);
  const HDC memory = CreateCompatibleDC(screen);
  const HGDIOBJ previous = SelectObject(memory, strip_);
  POINT position{center.x - width_ / 2, center.y - height_ / 2};
  SIZE size{width_, height_};
  POINT source{0, height_ * static_cast<int>(state)};
  BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  UpdateLayeredWindow(hwnd_, screen, &position, &size, memory, &source, 0, &blend, ULW_ALPHA);
  SelectObject(memory, previous);
  DeleteDC(memory);
  ReleaseDC(nullptr, screen);

  if (!shown_) ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
  shown_ = true;
  center_ = center;
  state_ = state;
}

void OrbWindow::Hide() {
  if (!shown_) return;
  ShowWindow(hwnd_, SW_HIDE);
  shown_ = false;
}

}  // namespace throwback::ui
