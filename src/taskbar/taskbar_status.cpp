// The bar's reactions to what other apps do: stepping aside for a fullscreen
// window, the badges apps put on their buttons, and window previews.
#include "taskbar/app_icons.h"
#include "taskbar/bar_manager.h"
#include "taskbar/fullscreen_watch.h"
#include "taskbar/taskbar_window.h"
#include "taskbar/thumbnail_popup.h"

namespace throwback {

void TaskbarWindow::UpdateFullscreen() {
  const HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
  const bool fullscreen = IsFullscreenWindow(GetForegroundWindow(), monitor);
  if (fullscreen == fullscreen_) return;
  fullscreen_ = fullscreen;
  // Off the topmost band while a video or game fills the screen, as
  // Explorer's own taskbar does; back on top as soon as it no longer does.
  if (appBar_) appBar_->SetFullscreenActive(fullscreen);
  if (fullscreen && startMenu_) startMenu_->Close();
}

void TaskbarWindow::AddBadges(BarModel& model) {
  const shell::BadgeStore& badges = manager_.Badges();
  if (badges.Empty()) {
    appIds_.clear();
    return;
  }
  const auto& items = tracker_->Items();
  std::map<HWND, std::wstring> seen;
  for (size_t i = 0; i < model.buttons.size() && i < items.size(); ++i) {
    const HWND window = items[i].hwnd;
    auto cached = appIds_.find(window);
    // Packaged apps may not expose their identity at once; ask again later.
    std::wstring id = cached != appIds_.end() ? cached->second : AppIdOf(window);
    if (const auto badge = badges.For(id)) {
      model.buttons[i].badge = true;
      model.buttons[i].badgeText = BadgeText(*badge);
    }
    if (!id.empty()) seen[window] = std::move(id);
  }
  appIds_ = std::move(seen);
}

void TaskbarWindow::ShowThumbnail(int index) {
  const auto& items = tracker_->Items();
  if (index < 0 || static_cast<size_t>(index) >= items.size()) return;
  const TaskItem& item = items[static_cast<size_t>(index)];
  RECT anchor{};
  if (!ButtonScreenRect(item.hwnd, &anchor)) return;
  thumbnail_->Show({item.hwnd, item.title, item.icon}, anchor, settings_.edge, dpi_);
}

}  // namespace throwback
