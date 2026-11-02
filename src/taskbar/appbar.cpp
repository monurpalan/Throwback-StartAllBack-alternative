#include "taskbar/appbar.h"

#include <shellapi.h>

namespace throwback {
namespace {

APPBARDATA MakeData(HWND hwnd) {
  APPBARDATA data{};
  data.cbSize = sizeof(data);
  data.hWnd = hwnd;
  return data;
}

void ApplyThickness(RECT& rect, Edge edge, int thickness) {
  if (edge == Edge::Top) {
    rect.bottom = rect.top + thickness;
  } else {
    rect.top = rect.bottom - thickness;
  }
}

}  // namespace

AppBar::AppBar(HWND hwnd, UINT callbackMessage) : hwnd_(hwnd) {
  APPBARDATA data = MakeData(hwnd_);
  data.uCallbackMessage = callbackMessage;
  SHAppBarMessage(ABM_NEW, &data);
}

AppBar::~AppBar() {
  APPBARDATA data = MakeData(hwnd_);
  SHAppBarMessage(ABM_REMOVE, &data);
}

void AppBar::Dock(Edge edge, int thickness) {
  edge_ = edge;
  thickness_ = thickness;

  MONITORINFO monitor{};
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfoW(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &monitor);

  APPBARDATA data = MakeData(hwnd_);
  data.uEdge = edge == Edge::Top ? ABE_TOP : ABE_BOTTOM;
  data.rc = monitor.rcMonitor;
  ApplyThickness(data.rc, edge, thickness);

  // The shell may move our rect out of the way of other AppBars (such as the
  // system taskbar), so ask first and then re-apply our thickness.
  SHAppBarMessage(ABM_QUERYPOS, &data);
  ApplyThickness(data.rc, edge, thickness);
  SHAppBarMessage(ABM_SETPOS, &data);

  MoveWindow(hwnd_, data.rc.left, data.rc.top, data.rc.right - data.rc.left,
             data.rc.bottom - data.rc.top, TRUE);
}

void AppBar::Redock() {
  if (thickness_ > 0) Dock(edge_, thickness_);
}

void AppBar::HandleCallback(WPARAM notification, LPARAM param) {
  switch (notification) {
    case ABN_POSCHANGED:
      Redock();
      break;
    case ABN_FULLSCREENAPP:
      // Step out of the way of fullscreen apps, come back afterwards.
      SetWindowPos(hwnd_, param ? HWND_BOTTOM : HWND_TOPMOST, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      break;
    case ABN_WINDOWARRANGE:
      // Cascade/tile in progress: hide so windows are arranged around us.
      ShowWindow(hwnd_, param ? SW_HIDE : SW_SHOWNOACTIVATE);
      break;
  }
}

void AppBar::OnActivated() {
  APPBARDATA data = MakeData(hwnd_);
  SHAppBarMessage(ABM_ACTIVATE, &data);
}

void AppBar::OnWindowPosChanged() {
  APPBARDATA data = MakeData(hwnd_);
  SHAppBarMessage(ABM_WINDOWPOSCHANGED, &data);
}

}  // namespace throwback
