#pragma once

#include <windows.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/notify_icon.h"

namespace throwback {

// Hosts the notification area. A window of our own named Shell_TrayWnd sits
// above Explorer's, so programs' Shell_NotifyIcon and SHAppBarMessage calls
// reach it first. Every call is passed on to Explorer unchanged, so Explorer
// keeps working and nothing is lost when Throwback exits.
class TrayHost {
 public:
  struct Visible {
    HICON icon = nullptr;
    std::wstring_view tip;
  };
  enum class Mouse { Move, LeftClick, DoubleClick, RightClick };

  // taskband receives ITaskbarList messages sent to our Shell_TrayWnd.
  TrayHost(HINSTANCE instance, HWND taskband, std::function<void()> onChanged);
  ~TrayHost();
  TrayHost(const TrayHost&) = delete;
  TrayHost& operator=(const TrayHost&) = delete;

  // Keeps our window above Explorer's and its rect on the bar, where
  // programs look for the taskbar; area is the icons' rect in screen pixels.
  void Place(const RECT& bar, const RECT& area);
  std::vector<Visible> VisibleIcons() const;
  // Tells the program behind a visible icon what the mouse did; point is
  // the icon's anchor in screen coordinates.
  void Send(size_t visibleIndex, Mouse mouse, POINT point);
  // Forgets icons whose program exited without removing them.
  void Prune();

 private:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT OnCopyData(WPARAM sender, const COPYDATASTRUCT* data, LPARAM raw);
  void SyncIcon(const NotifyIconMessage& message);
  std::vector<const TrayIcon*> Shown() const;

  HWND hwnd_ = nullptr;
  HWND notifyArea_ = nullptr;  // "TrayNotifyWnd", where programs look for the icons
  HWND taskband_;
  std::function<void()> onChanged_;
  TrayIconList list_;
  // Our copies of the programs' icons, by icon identity.
  std::unordered_map<std::wstring, HICON> icons_;
  UINT taskbarCreated_ = 0;
};

}  // namespace throwback
