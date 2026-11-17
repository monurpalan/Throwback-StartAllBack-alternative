#include "startmenu/catalog_cache.h"

#include <windows.h>
#include <objbase.h>

#include <utility>

namespace throwback {
namespace {

// A new program shows up in the menu at most this long after it is installed.
constexpr ULONGLONG kMaxAgeMs = 60'000;

}  // namespace

CatalogCache::~CatalogCache() {
  if (worker_.joinable()) worker_.join();
}

void CatalogCache::Start() {
  if (loading_.exchange(true)) return;
  if (worker_.joinable()) worker_.join();  // the last load is done
  worker_ = std::thread([this] {
    // Shell items need COM on this thread too.
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    auto catalog = LoadPrograms();
    CoUninitialize();
    {
      std::lock_guard lock(mutex_);
      ready_ = std::move(catalog);
      loadedAt_ = GetTickCount64();
    }
    loading_ = false;
  });
}

void CatalogCache::Preload() {
  if (loadedAt_ == 0) Start();
}

void CatalogCache::Adopt(std::unique_ptr<ProgramCatalog>& catalog) {
  {
    std::lock_guard lock(mutex_);
    if (ready_) catalog = std::move(ready_);
  }
  if (catalog) return;
  // Opened before the first background load finished: wait for it rather
  // than reading the shell twice.
  if (worker_.joinable()) {
    worker_.join();
    std::lock_guard lock(mutex_);
    if (ready_) catalog = std::move(ready_);
  }
  if (!catalog) {
    catalog = LoadPrograms();
    loadedAt_ = GetTickCount64();
  }
}

void CatalogCache::RefreshIfStale() {
  if (loadedAt_ != 0 && GetTickCount64() - loadedAt_ > kMaxAgeMs) Start();
}

}  // namespace throwback
