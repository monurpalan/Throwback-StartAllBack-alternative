#pragma once

#include <windows.h>

#include <functional>

namespace throwback {

// Tells when the foreground window may have gone fullscreen or back: when
// another window comes to the front, and when the front window moves or
// resizes (a video player's fullscreen button). The location events are
// taken only from the front window's thread, so an idle desktop costs
// nothing.
class FullscreenWatch {
 public:
  explicit FullscreenWatch(std::function<void()> changed);
  ~FullscreenWatch();
  FullscreenWatch(const FullscreenWatch&) = delete;
  FullscreenWatch& operator=(const FullscreenWatch&) = delete;

 private:
  static void CALLBACK OnEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object, LONG child,
                               DWORD thread, DWORD time);
  void Follow(HWND foreground);

  std::function<void()> changed_;
  HWINEVENTHOOK foregroundHook_ = nullptr;
  HWINEVENTHOOK locationHook_ = nullptr;
  DWORD followedThread_ = 0;
  static FullscreenWatch* instance_;
};

// Whether window fills its monitor and is an app's (not the desktop's or a
// taskbar's), so the bar on that monitor should step aside.
bool IsFullscreenWindow(HWND window, HMONITOR monitor);

}  // namespace throwback
