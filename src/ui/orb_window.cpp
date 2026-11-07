#include "ui/orb_window.h"

#include "core/layout.h"
#include "ui/skin.h"

#include <utility>

using Microsoft::WRL::ComPtr;

namespace throwback::ui {
namespace {

constexpr wchar_t kClassName[] = L"ThrowbackOrb";
constexpr int kFrames = 3;

// One frame of each orb at 96 DPI.
SIZE FrameSizeOf(SkinImage image) {
  return image == SkinImage::VistaOrb ? SIZE{60, 46} : SIZE{54, 54};
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

bool OrbWindow::LoadStrip(unsigned dpi, SkinImage image) {
  if (strip_ && stripDpi_ == dpi && stripImage_ == image) return true;
  const SIZE frame = FrameSizeOf(image);
  const int width = ScaleForDpi(static_cast<int>(frame.cx), dpi);
  const int height = ScaleForDpi(static_cast<int>(frame.cy), dpi);
  ComPtr<IWICBitmapSource> source = LoadSkinImage(wic_.Get(), ResourceFor(PartOf(image), dpi));
  if (!source) return false;
  UINT fileWidth = 0;
  UINT fileHeight = 0;
  source->GetSize(&fileWidth, &fileHeight);
  if (fileWidth != static_cast<UINT>(width) || fileHeight != static_cast<UINT>(height * kFrames)) {
    ComPtr<IWICBitmapScaler> scaler;
    if (FAILED(wic_->CreateBitmapScaler(scaler.GetAddressOf())) ||
        FAILED(scaler->Initialize(source.Get(), static_cast<UINT>(width),
                                  static_cast<UINT>(height * kFrames),
                                  WICBitmapInterpolationModeHighQualityCubic))) {
      return false;
    }
    source = scaler;
  }

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
  stripDpi_ = dpi;
  stripImage_ = image;
  width_ = width;
  height_ = height;
  return true;
}

void OrbWindow::Show(POINT center, unsigned dpi, OrbState state, SkinImage image) {
  if (shown_ && stripDpi_ == dpi && stripImage_ == image && center.x == center_.x &&
      center.y == center_.y && state == state_) {
    return;
  }
  if (!LoadStrip(dpi, image)) return;

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
