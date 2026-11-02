// The start menu's jump lists: a program's recent documents beside it.
#include "startmenu/start_menu.h"

namespace throwback {
namespace {

constexpr UINT_PTR kJumpTimer = 1;
constexpr UINT kJumpDelayMs = 400;
constexpr size_t kMaxJumpItems = 16;
constexpr int kArrowWidth = 24;

}  // namespace

bool StartMenu::OnJumpArrow(int row, int x) const {
  if (row < 0 || row >= static_cast<int>(rows_.size())) return false;
  if (rows_[static_cast<size_t>(row)].appId.empty()) return false;
  return x >= layout_.list.right - ScaleForDpi(kArrowWidth, dpi_);
}

void StartMenu::HoverJumpList(int row) {
  const bool hasList = row >= 0 && row < static_cast<int>(rows_.size()) &&
                       !rows_[static_cast<size_t>(row)].appId.empty();
  // Pointing at a row without a list closes the open one, with the same delay.
  const int wanted = hasList ? row : -1;
  if (wanted == jumpRow_) {
    KillTimer(hwnd_, kJumpTimer);
    pendingJumpRow_ = jumpRow_;
    return;
  }
  if (wanted == pendingJumpRow_) return;
  pendingJumpRow_ = wanted;
  SetTimer(hwnd_, kJumpTimer, kJumpDelayMs, [](HWND hwnd, UINT, UINT_PTR, DWORD) {
    KillTimer(hwnd, kJumpTimer);
    auto* self = reinterpret_cast<StartMenu*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (self) self->OpenJumpList(self->pendingJumpRow_);
  });
}

void StartMenu::OpenJumpList(int row) {
  KillTimer(hwnd_, kJumpTimer);
  pendingJumpRow_ = row;
  jumpRow_ = row;
  jumpItems_.clear();
  if (row >= 0 && row < static_cast<int>(rows_.size())) {
    jumpItems_ = shell::LoadRecentItems(rows_[static_cast<size_t>(row)].appId, kMaxJumpItems);
  }
  if (jumpItems_.empty()) jumpRow_ = -1;
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void StartMenu::ResetJumpList() {
  KillTimer(hwnd_, kJumpTimer);
  jumpRow_ = -1;
  pendingJumpRow_ = -1;
  jumpItems_.clear();
}

}  // namespace throwback
