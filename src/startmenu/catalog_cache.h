#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include "startmenu/programs.h"

namespace throwback {

// Keeps a start menu's program list ready, so opening the menu never waits
// for the shell: the list is loaded in the background when Throwback starts
// and reloaded in the background once it is older than a minute. A menu
// shows the list it has and picks up a fresher one the next time it opens.
class CatalogCache {
 public:
  CatalogCache() = default;
  ~CatalogCache();
  CatalogCache(const CatalogCache&) = delete;
  CatalogCache& operator=(const CatalogCache&) = delete;

  // Starts loading in the background if nothing is loaded or loading yet.
  void Preload();
  // Called as the menu opens (never while it shows the catalog): takes a
  // freshly loaded list, or loads one right away the first time only.
  void Adopt(std::unique_ptr<ProgramCatalog>& catalog);
  // Starts a background reload when the list is stale.
  void RefreshIfStale();

 private:
  void Start();

  std::mutex mutex_;
  std::unique_ptr<ProgramCatalog> ready_;  // loaded, not yet adopted
  std::thread worker_;
  std::atomic<bool> loading_ = false;
  // When the list in use (or ready_) was read; the worker sets it too.
  std::atomic<unsigned long long> loadedAt_ = 0;
};

}  // namespace throwback
