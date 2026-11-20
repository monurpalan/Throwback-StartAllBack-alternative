// The notification area's volume icon: its glyph, its flyout, its menu and
// the mouse wheel over it. Throwback draws it itself because Windows 11
// gives the volume icon only to its own taskbar.
#include <windowsx.h>

#include <string>

#include "core/volume_math.h"
#include "shell/launch.h"
#include "taskbar/taskbar_window.h"
#include "ui/strings.h"

namespace throwback {
namespace {

// Segoe Fluent Icons / MDL2 Assets: muted, then 0 to 3 waves.
constexpr wchar_t kSpeakerGlyphs[] = {0xE74F, 0xE993, 0xE994, 0xE995};
// A click on the icon right after the flyout closed (it closes as the mouse
// goes down on the icon) toggles it shut rather than opening it again.
constexpr ULONGLONG kReopenDelayMs = 300;

enum : UINT { kMixer = 1, kSoundSettings, kMute };

}  // namespace

void TaskbarWindow::CreateVolume() {
  volume_ = std::make_unique<shell::Volume>(hwnd_, kVolumeMessage);
  VolumeFlyout::Callbacks callbacks;
  callbacks.level = [this] { return volume_->Level(); };
  callbacks.muted = [this] { return volume_->Muted(); };
  callbacks.setLevel = [this](float level) { volume_->SetLevel(level); };
  callbacks.setMuted = [this](bool muted) { volume_->SetMuted(muted); };
  callbacks.openMixer = [] { shell::Launch(L"sndvol.exe"); };
  callbacks.closed = [this] {
    volumeClosedAt_ = GetTickCount64();
    InvalidateRect(hwnd_, nullptr, FALSE);
  };
  volumeFlyout_ = std::make_unique<VolumeFlyout>(module_, hwnd_, std::move(callbacks));
}

bool TaskbarWindow::HasVolume() const { return volume_ && volume_->Available(); }

wchar_t TaskbarWindow::VolumeGlyph() const {
  return kSpeakerGlyphs[VolumeGlyphStep(volume_->Level(), volume_->Muted())];
}

int TaskbarWindow::VolumeHit() const {
  return kHitTrayFirst - static_cast<int>(SplitTrayIcons().shown.size());
}

bool TaskbarWindow::HandleVolumeMessage(UINT message, WPARAM wParam, LPARAM lParam,
                                        LRESULT* result) {
  if (message == kVolumeMessage && volume_) {
    const bool had = volume_->Available();
    if (wParam == 1) volume_->Reattach();  // another default device
    if (had != volume_->Available()) {
      UpdateLayout();  // the icon comes or goes with the speakers
    } else {
      InvalidateRect(hwnd_, nullptr, FALSE);
    }
    volumeFlyout_->Refresh();
    *result = 0;
    return true;
  }
  if (message == WM_MOUSEWHEEL && HasVolume()) {
    POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ScreenToClient(hwnd_, &point);
    if (TrayHit(point.x, point.y) != VolumeHit()) return false;
    volume_->SetLevel(WheelLevel(volume_->Level(), GET_WHEEL_DELTA_WPARAM(wParam)));
    *result = 0;
    return true;
  }
  return false;
}

void TaskbarWindow::OnVolumeMouse(TrayHost::Mouse mouse, const Rect& slot) {
  POINT topLeft{slot.left, slot.top};
  POINT bottomRight{slot.right, slot.bottom};
  ClientToScreen(hwnd_, &topLeft);
  ClientToScreen(hwnd_, &bottomRight);
  if (mouse == TrayHost::Mouse::LeftClick) {
    if (volumeFlyout_->IsOpen()) {
      volumeFlyout_->Hide();
    } else if (GetTickCount64() - volumeClosedAt_ > kReopenDelayMs) {
      volumeFlyout_->Show({topLeft.x, topLeft.y, bottomRight.x, bottomRight.y}, settings_.edge,
                          dpi_);
    }
  } else if (mouse == TrayHost::Mouse::RightClick) {
    ShowVolumeMenu({(topLeft.x + bottomRight.x) / 2, (topLeft.y + bottomRight.y) / 2});
  }
}

void TaskbarWindow::ShowVolumeMenu(POINT at) {
  using ui::StringId;
  const bool muted = volume_->Muted();
  HMENU menu = CreatePopupMenu();
  AppendMenuW(menu, MF_STRING, kMixer, std::wstring(ui::Tr(StringId::OpenVolumeMixer)).c_str());
  AppendMenuW(menu, MF_STRING, kSoundSettings,
              std::wstring(ui::Tr(StringId::SoundSettings)).c_str());
  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(menu, MF_STRING, kMute,
              std::wstring(ui::Tr(muted ? StringId::UnmuteSound : StringId::MuteSound)).c_str());
  SetForegroundWindow(hwnd_);
  const auto command = static_cast<UINT>(TrackPopupMenu(
      menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, at.x, at.y, 0, hwnd_, nullptr));
  PostMessageW(hwnd_, WM_NULL, 0, 0);
  DestroyMenu(menu);
  if (command == kMixer) shell::Launch(L"sndvol.exe");
  if (command == kSoundSettings) shell::Launch(L"ms-settings:sound");
  if (command == kMute) volume_->SetMuted(!muted);
}

}  // namespace throwback
