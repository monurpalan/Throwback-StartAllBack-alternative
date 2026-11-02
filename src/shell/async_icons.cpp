#include "shell/async_icons.h"

#include <objbase.h>

#include <utility>

#include "shell/shell_icons.h"

namespace throwback::shell {
namespace {

std::wstring KeyOf(const std::wstring& parsingName, int size) {
  return std::to_wstring(size) + L"|" + parsingName;
}

}  // namespace

AsyncIconCache::AsyncIconCache(HWND notify, UINT message)
    : notify_(notify), message_(message), worker_([this] { Run(); }) {}

AsyncIconCache::~AsyncIconCache() {
  {
    std::lock_guard lock(mutex_);
    stopping_ = true;
  }
  wake_.notify_one();
  worker_.join();
  for (const auto& [key, icon] : icons_) {
    if (icon) DestroyIcon(icon);
  }
}

HICON AsyncIconCache::Get(const std::wstring& parsingName, int size) {
  const std::wstring key = KeyOf(parsingName, size);
  std::lock_guard lock(mutex_);
  if (const auto it = icons_.find(key); it != icons_.end()) return it->second;
  icons_.emplace(key, nullptr);  // queued once, even if loading fails
  queue_.push_back({parsingName, size});
  wake_.notify_one();
  return nullptr;
}

void AsyncIconCache::Run() {
  // Shell items need COM on this thread too.
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  for (;;) {
    Request request;
    {
      std::unique_lock lock(mutex_);
      wake_.wait(lock, [this] { return stopping_ || !queue_.empty(); });
      if (stopping_) break;
      request = std::move(queue_.back());
      queue_.pop_back();
    }
    HICON icon = LoadItemIcon(request.parsingName, request.size);
    if (!icon) continue;
    {
      std::lock_guard lock(mutex_);
      icons_[KeyOf(request.parsingName, request.size)] = icon;
    }
    PostMessageW(notify_, message_, 0, 0);
  }
  CoUninitialize();
}

}  // namespace throwback::shell
