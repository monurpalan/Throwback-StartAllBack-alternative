#include "taskbar/volume_flyout.h"

#include <windowsx.h>

#include <stdexcept>
#include <string>
#include <utility>

#include "core/volume_math.h"
#include "ui/glass.h"
#include "ui/strings.h"

namespace throwback {
namespace {

constexpr wchar_t kClassName[] = L"ThrowbackVolume";
// At 96 DPI: Windows 7's narrow flyout, its slider and the mixer link.
constexpr int kWidth = 84;
constexpr int kPercentHeight = 28;
constexpr int kTrackTop = 36;
constexpr int kTrackLength = 150;
constexpr int kTrackWidth = 28;
constexpr int kMuteSize = 32;
constexpr int kLinkHeight = 28;
constexpr int kGap = 8;
// Segoe Fluent Icons / MDL2 Assets: muted, then 0 to 3 waves.
constexpr wchar_t kSpeakerGlyphs[] = {0xE74F, 0xE993, 0xE994, 0xE995};

}  // namespace

VolumeFlyout::VolumeFlyout(HINSTANCE instance, HWND owner, Callbacks callbacks)
    : callbacks_(std::move(callbacks)) {
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = WindowProc;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.lpszClassName = kClassName;
  RegisterClassExW(&windowClass);  // fails harmlessly when already registered

  CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kClassName, L"", WS_POPUP, 0, 0, 0, 0, owner,
                  nullptr, instance, this);
  if (!hwnd_) throw std::runtime_error("CreateWindowEx failed for the volume flyout");
  canvas_ = std::make_unique<ui::Canvas>(hwnd_);
  ui::EnableGlass(hwnd_, ui::GlassCorners::Round);
}

VolumeFlyout::~VolumeFlyout() {
  if (hwnd_) DestroyWindow(hwnd_);
}

bool VolumeFlyout::IsOpen() const { return hwnd_ && IsWindowVisible(hwnd_); }

void VolumeFlyout::Show(const RECT& icon, Edge barEdge, unsigned dpi) {
  dpi_ = dpi;
  canvas_->SetDpi(dpi);
  const auto s = [dpi](int value) { return ScaleForDpi(value, dpi); };
  const int width = s(kWidth);
  const int trackBottom = s(kTrackTop + kTrackLength);
  const int muteTop = trackBottom + s(kGap);
  const int linkTop = muteTop + s(kMuteSize) + s(kGap);
  const int height = linkTop + s(kLinkHeight);
  layout_.frame = {0, 0, width, height};
  layout_.percent = {0, s(kGap / 2), width, s(kGap / 2 + kPercentHeight)};
  layout_.track = {(width - s(kTrackWidth)) / 2, s(kTrackTop), (width + s(kTrackWidth)) / 2,
                   trackBottom};
  layout_.mute = {(width - s(kMuteSize)) / 2, muteTop, (width + s(kMuteSize)) / 2,
                  muteTop + s(kMuteSize)};
  layout_.mixer = {0, linkTop, width, height};

  MONITORINFO monitor{};
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfoW(MonitorFromRect(&icon, MONITOR_DEFAULTTONEAREST), &monitor);
  const RECT& screen = monitor.rcMonitor;
  const Point at =
      PlacePopup({icon.left, icon.top, icon.right, icon.bottom}, width, height, barEdge, s(kGap),
                 {screen.left, screen.top, screen.right, screen.bottom});
  SetWindowPos(hwnd_, HWND_TOPMOST, at.x, at.y, width, height, SWP_SHOWWINDOW);
  canvas_->Resize(static_cast<UINT>(width), static_cast<UINT>(height));
  hovered_ = pressed_ = Part::None;
  SetForegroundWindow(hwnd_);
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void VolumeFlyout::Hide() {
  if (!IsOpen()) return;
  if (GetCapture() == hwnd_) ReleaseCapture();
  pressed_ = Part::None;
  ShowWindow(hwnd_, SW_HIDE);
  if (callbacks_.closed) callbacks_.closed();
}

void VolumeFlyout::Refresh() {
  if (IsOpen()) InvalidateRect(hwnd_, nullptr, FALSE);
}

void VolumeFlyout::Paint() {
  PAINTSTRUCT paint;
  BeginPaint(hwnd_, &paint);
  const float level = callbacks_.level();
  const bool muted = callbacks_.muted();
  canvas_->BeginFrame();
  canvas_->Glass(layout_.frame, ui::GlassVariant::Menu);
  canvas_->Text(layout_.percent, std::to_wstring(VolumePercent(level)), ui::TextStyle::Body,
                ui::TextTone::Light, ui::TextAlign::Center);
  canvas_->VolumeSlider(layout_.track, ThumbY(level, layout_.track.top, layout_.track.bottom),
                        muted, hovered_ == Part::Slider || pressed_ == Part::Slider);
  if (hovered_ == Part::Mute) canvas_->TaskTile(layout_.mute, false, ui::TileState::Hot);
  canvas_->ModernGlyph(layout_.mute, kSpeakerGlyphs[VolumeGlyphStep(level, muted)],
                       ui::ModernLook::Win11Dark, ui::ModernTone::Primary);
  canvas_->Panel(layout_.mixer, ui::PanelVariant::FlyoutLink);
  canvas_->Text(layout_.mixer, ui::Tr(ui::StringId::VolumeMixerLink), ui::TextStyle::Body,
                ui::TextTone::Link, ui::TextAlign::Center);
  canvas_->EndFrame();
  EndPaint(hwnd_, &paint);
}

VolumeFlyout::Part VolumeFlyout::HitTest(POINT point) const {
  // The slider takes clicks a little beyond its ends, like Windows'.
  const int slack = ScaleForDpi(kGap, dpi_);
  const Rect slider{layout_.track.left, layout_.track.top - slack, layout_.track.right,
                    layout_.track.bottom + slack};
  if (slider.Contains(point.x, point.y)) return Part::Slider;
  if (layout_.mute.Contains(point.x, point.y)) return Part::Mute;
  if (layout_.mixer.Contains(point.x, point.y)) return Part::Mixer;
  return Part::None;
}

void VolumeFlyout::Slide(int y) {
  callbacks_.setLevel(LevelAtY(y, layout_.track.top, layout_.track.bottom));
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void VolumeFlyout::OnUp(POINT point) {
  const Part pressed = std::exchange(pressed_, Part::None);
  if (GetCapture() == hwnd_) ReleaseCapture();
  InvalidateRect(hwnd_, nullptr, FALSE);
  if (HitTest(point) != pressed) return;
  if (pressed == Part::Mute) {
    callbacks_.setMuted(!callbacks_.muted());
  } else if (pressed == Part::Mixer) {
    Hide();
    callbacks_.openMixer();
  }
}

LRESULT VolumeFlyout::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  const POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
  switch (message) {
    case WM_PAINT:
      Paint();
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case WM_ACTIVATE:
      // Clicking anywhere else closes the flyout, as on Windows 7.
      if (LOWORD(wParam) == WA_INACTIVE) Hide();
      return 0;
    case WM_KEYDOWN:
      if (wParam == VK_ESCAPE) Hide();
      if (wParam == VK_UP || wParam == VK_DOWN) {
        callbacks_.setLevel(WheelLevel(callbacks_.level(), wParam == VK_UP ? 120 : -120));
        InvalidateRect(hwnd_, nullptr, FALSE);
      }
      return 0;
    case WM_MOUSEWHEEL:
      callbacks_.setLevel(WheelLevel(callbacks_.level(), GET_WHEEL_DELTA_WPARAM(wParam)));
      InvalidateRect(hwnd_, nullptr, FALSE);
      return 0;
    case WM_MOUSEMOVE: {
      if (!tracking_) {
        TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, hwnd_, 0};
        tracking_ = TrackMouseEvent(&track) != FALSE;
      }
      if (pressed_ == Part::Slider) Slide(point.y);
      const Part hit = HitTest(point);
      if (hit != hovered_) {
        hovered_ = hit;
        InvalidateRect(hwnd_, nullptr, FALSE);
      }
      return 0;
    }
    case WM_MOUSELEAVE:
      tracking_ = false;
      hovered_ = Part::None;
      InvalidateRect(hwnd_, nullptr, FALSE);
      return 0;
    case WM_LBUTTONDOWN:
      pressed_ = HitTest(point);
      if (pressed_ != Part::None) SetCapture(hwnd_);
      if (pressed_ == Part::Slider) Slide(point.y);
      return 0;
    case WM_LBUTTONUP:
      OnUp(point);
      return 0;
  }
  return DefWindowProcW(hwnd_, message, wParam, lParam);
}

LRESULT CALLBACK VolumeFlyout::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_NCCREATE) {
    auto* self =
        static_cast<VolumeFlyout*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
    self->hwnd_ = hwnd;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
  }
  auto* self = reinterpret_cast<VolumeFlyout*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return DefWindowProcW(hwnd, message, wParam, lParam);
  if (message == WM_NCDESTROY) {
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    self->hwnd_ = nullptr;
    return DefWindowProcW(hwnd, message, wParam, lParam);
  }
  return self->HandleMessage(message, wParam, lParam);
}

}  // namespace throwback
