#pragma once

#include <windows.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace throwback::shell {

// Loads shell item icons on a worker thread so opening a long list never
// waits on the disk. Get returns nullptr until an icon is ready; the window
// then receives `message` and repaints.
class AsyncIconCache {
 public:
  AsyncIconCache(HWND notify, UINT message);
  ~AsyncIconCache();
  AsyncIconCache(const AsyncIconCache&) = delete;
  AsyncIconCache& operator=(const AsyncIconCache&) = delete;

  HICON Get(const std::wstring& parsingName, int size);

 private:
  struct Request {
    std::wstring parsingName;
    int size = 0;
  };

  void Run();

  HWND notify_;
  UINT message_;
  std::mutex mutex_;
  std::condition_variable wake_;
  // Newest requests first: they are the rows on screen right now.
  std::vector<Request> queue_;
  // nullptr while queued or when loading failed.
  std::unordered_map<std::wstring, HICON> icons_;
  bool stopping_ = false;
  std::thread worker_;
};

}  // namespace throwback::shell
