#include "taskbar/thumbnail_popup.h"

#include <algorithm>
#include <stdexcept>

namespace throwback {
namespace {

constexpr wchar_t kClassName[] = L"ThrowbackThumbnail";
// Largest preview at 96 DPI, the size the classic taskbar uses.
constexpr int kPreviewWidth = 200;
constexpr int kPreviewHeight = 120;
constexpr int kIconSize = 16;

// Fits the source into the preview box, keeping its aspect ratio and never
// scaling small windows up.
SIZE FitPreview(SIZE source, int maxWidth, int maxHeight) {
  if (source.cx <= 0 || source.cy <= 0) return {0, 0};
  const double scale = std::min(
      {static_cast<double>(maxWidth) / source.cx, static_cast<double>(maxHeight) / source.cy, 1.0});
  return {static_cast<LONG>(source.cx * scale), static_cast<LONG>(source.cy * scale)};
}

}  // namespace

ThumbnailPopup::ThumbnailPopup(HINSTANCE instance, HWND owner) {
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = WindowProc;
  windowClass.hInstance = instance;
  windowClass.lpszClassName = kClassName;
  RegisterClassExW(&windowClass);

  hwnd_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, kClassName, L"",
                          WS_POPUP, 0, 0, 0, 0, owner, nullptr, instance, this);
  if (!hwnd_) throw std::runtime_error("CreateWindowEx failed for the thumbnail popup");
  canvas_ = std::make_unique<ui::Canvas>(hwnd_);
}

ThumbnailPopup::~ThumbnailPopup() {
  Hide();
  // Already gone if the owner was destroyed first.
  if (IsWindow(hwnd_)) DestroyWindow(hwnd_);
}

LRESULT CALLBACK ThumbnailPopup::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_NCCREATE) {
    SetWindowLongPtrW(
        hwnd, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams));
  }
  auto* self = reinterpret_cast<ThumbnailPopup*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  switch (message) {
    case WM_PAINT:
      if (self) self->Paint();
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case WM_NCHITTEST:
      return HTTRANSPARENT;  // the preview is for looking only
    case WM_MOUSEACTIVATE:
      return MA_NOACTIVATE;
    case WM_CLOSE:
      // A polite close (taskkill, installers, restart manager) is meant for
      // the app; closing only the popup would leave the bar running.
      if (HWND owner = GetWindow(hwnd, GW_OWNER)) PostMessageW(owner, WM_CLOSE, 0, 0);
      return 0;
  }
  return DefWindowProcW(hwnd, message, wParam, lParam);
}

void ThumbnailPopup::Show(const Content& content, const RECT& anchor, bool below,
                          const ui::Theme& theme, unsigned dpi) {
  Hide();
  content_ = content;
  canvas_->SetTheme(theme);
  canvas_->SetDpi(dpi);

  const int padding = canvas_->Scale(ui::kSpace8);
  const int titleHeight = canvas_->Scale(ui::kSpace24);
  const int iconSize = canvas_->Scale(kIconSize);
  const int boxWidth = canvas_->Scale(kPreviewWidth);

  SIZE source{};
  if (SUCCEEDED(DwmRegisterThumbnail(hwnd_, content.task, &thumbnail_))) {
    DwmQueryThumbnailSourceSize(thumbnail_, &source);
  }
  const SIZE preview = FitPreview(source, boxWidth, canvas_->Scale(kPreviewHeight));

  const int width = boxWidth + 2 * padding;
  const int height = padding + titleHeight + (preview.cy > 0 ? padding + preview.cy : 0) + padding;
  iconRect_ = {padding, padding + (titleHeight - iconSize) / 2, padding + iconSize,
               padding + (titleHeight - iconSize) / 2 + iconSize};
  titleRect_ = {iconRect_.right + padding, padding, width - padding, padding + titleHeight};

  // Center on the button, but stay on its monitor.
  MONITORINFO monitor{};
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfoW(MonitorFromRect(&anchor, MONITOR_DEFAULTTONEAREST), &monitor);
  const int centered = (anchor.left + anchor.right - width) / 2;
  const int x = std::clamp(centered, static_cast<int>(monitor.rcMonitor.left),
                           static_cast<int>(monitor.rcMonitor.right) - width);
  const int y = below ? anchor.bottom + padding : anchor.top - padding - height;
  SetWindowPos(hwnd_, HWND_TOPMOST, x, y, width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
  canvas_->Resize(static_cast<UINT>(width), static_cast<UINT>(height));

  if (thumbnail_ && preview.cy > 0) {
    const int left = (width - preview.cx) / 2;
    const int top = padding + titleHeight + padding;
    DWM_THUMBNAIL_PROPERTIES properties{};
    properties.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_OPACITY;
    properties.rcDestination = {left, top, left + preview.cx, top + preview.cy};
    properties.fVisible = TRUE;
    properties.opacity = 255;
    DwmUpdateThumbnailProperties(thumbnail_, &properties);
  }
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void ThumbnailPopup::Hide() {
  if (thumbnail_) {
    DwmUnregisterThumbnail(thumbnail_);
    thumbnail_ = nullptr;
  }
  content_ = {};
  if (hwnd_) ShowWindow(hwnd_, SW_HIDE);
}

void ThumbnailPopup::Paint() {
  PAINTSTRUCT paint;
  BeginPaint(hwnd_, &paint);
  canvas_->BeginFrame();
  canvas_->Icon(iconRect_, content_.icon);
  canvas_->Text(titleRect_, content_.title, ui::TextStyle::Body);
  canvas_->EndFrame();
  EndPaint(hwnd_, &paint);
}

}  // namespace throwback
