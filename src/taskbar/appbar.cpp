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
  switch (edge) {
    case Edge::Top:
      rect.bottom = rect.top + thickness;
      break;
    case Edge::Bottom:
      rect.top = rect.bottom - thickness;
      break;
    case Edge::Left:
      rect.right = rect.left + thickness;
      break;
    case Edge::Right:
      rect.left = rect.right - thickness;
      break;
  }
}

UINT AppBarEdge(Edge edge) {
  switch (edge) {
    case Edge::Top:
      return ABE_TOP;
    case Edge::Left:
      return ABE_LEFT;
    case Edge::Right:
      return ABE_RIGHT;
    default:
      return ABE_BOTTOM;
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

void AppBar::Dock(Edge edge, int thickness, HMONITOR target) {
  edge_ = edge;
  thickness_ = thickness;
  monitor_ = target;

  MONITORINFO monitor{};
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfoW(target ? target : MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &monitor);

  APPBARDATA data = MakeData(hwnd_);
  data.uEdge = AppBarEdge(edge);
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
  if (thickness_ > 0) Dock(edge_, thickness_, monitor_);
}

void AppBar::HandleCallback(WPARAM notification, LPARAM param) {
  switch (notification) {
    case ABN_POSCHANGED:
      Redock();
      break;
    case ABN_FULLSCREENAPP:
      SetFullscreenActive(param != 0);
      break;
    case ABN_WINDOWARRANGE:
      // Cascade/tile in progress: hide so windows are arranged around us.
      ShowWindow(hwnd_, param ? SW_HIDE : SW_SHOWNOACTIVATE);
      break;
  }
}

void AppBar::SetFullscreenActive(bool active) {
  // HWND_BOTTOM also drops the topmost flag; HWND_TOPMOST puts it back.
  SetWindowPos(hwnd_, active ? HWND_BOTTOM : HWND_TOPMOST, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
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
