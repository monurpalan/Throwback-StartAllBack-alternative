#include "taskbar/window_tracker.h"

#include <dwmapi.h>

#include <algorithm>
#include <initializer_list>

#include "core/stable_order.h"
#include "core/window_filter.h"

namespace throwback {
namespace {

struct EnumContext {
  HWND self;
  std::vector<HWND> found;
};

bool IsCloaked(HWND hwnd) {
  DWORD cloaked = 0;
  return SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) &&
         cloaked != 0;
}

WindowTraits ReadTraits(HWND hwnd) {
  const LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  return {
      .visible = IsWindowVisible(hwnd) != FALSE,
      .cloaked = IsCloaked(hwnd),
      .hasOwner = GetWindow(hwnd, GW_OWNER) != nullptr,
      .hasTitle = GetWindowTextLengthW(hwnd) > 0,
      .toolWindow = (exStyle & WS_EX_TOOLWINDOW) != 0,
      .appWindow = (exStyle & WS_EX_APPWINDOW) != 0,
      .noActivate = (exStyle & WS_EX_NOACTIVATE) != 0,
  };
}

BOOL CALLBACK CollectWindow(HWND hwnd, LPARAM param) {
  auto* context = reinterpret_cast<EnumContext*>(param);
  if (hwnd != context->self && ShouldShowOnTaskbar(ReadTraits(hwnd))) {
    context->found.push_back(hwnd);
  }
  return TRUE;
}

std::wstring ReadTitle(HWND hwnd) {
  std::wstring title(static_cast<size_t>(GetWindowTextLengthW(hwnd)) + 1, L'\0');
  const int length = GetWindowTextW(hwnd, title.data(), static_cast<int>(title.size()));
  title.resize(static_cast<size_t>(std::max(length, 0)));
  return title;
}

HICON ReadIcon(HWND hwnd) {
  // Short timeout: a hung app must not freeze the taskbar.
  for (WPARAM kind : {WPARAM{ICON_SMALL2}, WPARAM{ICON_SMALL}, WPARAM{ICON_BIG}}) {
    DWORD_PTR result = 0;
    if (SendMessageTimeoutW(hwnd, WM_GETICON, kind, 0, SMTO_ABORTIFHUNG, 50, &result) && result) {
      return reinterpret_cast<HICON>(result);
    }
  }
  if (auto icon = reinterpret_cast<HICON>(GetClassLongPtrW(hwnd, GCLP_HICONSM))) return icon;
  if (auto icon = reinterpret_cast<HICON>(GetClassLongPtrW(hwnd, GCLP_HICON))) return icon;
  return LoadIconW(nullptr, IDI_APPLICATION);
}

}  // namespace

bool WindowTracker::Refresh() {
  EnumContext context{self_, {}};
  EnumWindows(CollectWindow, reinterpret_cast<LPARAM>(&context));

  std::vector<HWND> previous;
  previous.reserve(items_.size());
  for (const auto& item : items_) previous.push_back(item.hwnd);

  std::vector<TaskItem> next;
  for (HWND hwnd : MergeStableOrder(previous, context.found)) {
    const auto existing = std::find_if(items_.begin(), items_.end(),
                                       [hwnd](const TaskItem& item) { return item.hwnd == hwnd; });
    TaskItem item = existing != items_.end() ? *existing : TaskItem{.hwnd = hwnd};
    item.title = ReadTitle(hwnd);
    if (!item.icon) item.icon = ReadIcon(hwnd);
    next.push_back(std::move(item));
  }

  const bool changed = next != items_;
  items_ = std::move(next);
  return changed;
}

void WindowTracker::InvalidateIcon(HWND hwnd) {
  for (auto& item : items_) {
    if (item.hwnd == hwnd) item.icon = nullptr;
  }
}

}  // namespace throwback
