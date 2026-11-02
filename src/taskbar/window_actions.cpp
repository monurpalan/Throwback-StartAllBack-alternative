#include "taskbar/window_actions.h"

#include <array>
#include <string>

#include "core/window_menu.h"
#include "shell/launch.h"
#include "taskbar/explorer_tray.h"

namespace throwback {
namespace {

using shell::BringToFront;

// Explorer's command for the "show desktop" button.
constexpr WPARAM kShowDesktopCommand = 407;

void PressWinD() {
  INPUT inputs[4] = {};
  const WORD keys[4] = {VK_LWIN, 'D', 'D', VK_LWIN};
  for (int i = 0; i < 4; ++i) {
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = keys[i];
    inputs[i].ki.dwFlags = i >= 2 ? KEYEVENTF_KEYUP : 0;
  }
  SendInput(4, inputs, sizeof(INPUT));
}

void AllowToActivate(HWND window) {
  DWORD processId = 0;
  GetWindowThreadProcessId(window, &processId);
  AllowSetForegroundWindow(processId);
}

UINT ToSysCommand(WindowCommand command) {
  switch (command) {
    case WindowCommand::Restore:
      return SC_RESTORE;
    case WindowCommand::Move:
      return SC_MOVE;
    case WindowCommand::Size:
      return SC_SIZE;
    case WindowCommand::Minimize:
      return SC_MINIMIZE;
    case WindowCommand::Maximize:
      return SC_MAXIMIZE;
    case WindowCommand::Close:
      return SC_CLOSE;
  }
  return SC_CLOSE;
}

HBITMAP MenuGlyph(WindowCommand command) {
  switch (command) {
    case WindowCommand::Restore:
      return HBMMENU_POPUP_RESTORE;
    case WindowCommand::Minimize:
      return HBMMENU_POPUP_MINIMIZE;
    case WindowCommand::Maximize:
      return HBMMENU_POPUP_MAXIMIZE;
    case WindowCommand::Close:
      return HBMMENU_POPUP_CLOSE;
    default:
      return nullptr;
  }
}

// The system menu labels in the user's Windows language, read once from the
// system menu of a throwaway window.
const std::array<std::wstring, kWindowCommands.size()>& MenuLabels() {
  static const auto labels = [] {
    std::array<std::wstring, kWindowCommands.size()> result = {
        L"&Restore", L"&Move", L"&Size", L"Mi&nimize", L"Ma&ximize", L"&Close\tAlt+F4"};
    HWND probe = CreateWindowExW(0, L"STATIC", L"", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr,
                                 nullptr, nullptr, nullptr);
    if (HMENU menu = probe ? GetSystemMenu(probe, FALSE) : nullptr) {
      for (size_t i = 0; i < kWindowCommands.size(); ++i) {
        wchar_t text[128] = {};
        if (GetMenuStringW(menu, ToSysCommand(kWindowCommands[i]), text, 128, MF_BYCOMMAND) > 0) {
          result[i] = text;
        }
      }
    }
    if (probe) DestroyWindow(probe);
    return result;
  }();
  return labels;
}

WindowMenuState ReadMenuState(HWND task) {
  const LONG_PTR style = GetWindowLongPtrW(task, GWL_STYLE);
  return {
      .minimized = IsIconic(task) != FALSE,
      .maximized = IsZoomed(task) != FALSE,
      .canMinimize = (style & WS_MINIMIZEBOX) != 0,
      .canMaximize = (style & WS_MAXIMIZEBOX) != 0,
      .sizable = (style & WS_THICKFRAME) != 0,
  };
}

HMENU BuildWindowMenu(const WindowMenuState& state) {
  HMENU menu = CreatePopupMenu();
  const auto& labels = MenuLabels();
  for (size_t i = 0; i < kWindowCommands.size(); ++i) {
    const WindowCommand command = kWindowCommands[i];
    if (command == WindowCommand::Close) AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    const UINT flags = MF_STRING | (IsCommandEnabled(command, state) ? MF_ENABLED : MF_GRAYED);
    AppendMenuW(menu, flags, ToSysCommand(command), labels[i].c_str());
    if (HBITMAP glyph = MenuGlyph(command)) {
      MENUITEMINFOW info{};
      info.cbSize = sizeof(info);
      info.fMask = MIIM_BITMAP;
      info.hbmpItem = glyph;
      SetMenuItemInfoW(menu, ToSysCommand(command), FALSE, &info);
    }
  }
  SetMenuDefaultItem(menu, ToSysCommand(DefaultCommand(state)), FALSE);
  return menu;
}

}  // namespace

TaskClickState ReadClickState(HWND task) {
  const LONG_PTR style = GetWindowLongPtrW(task, GWL_STYLE);
  return {
      .wasActive = task == GetForegroundWindow(),
      .minimized = IsIconic(task) != FALSE,
      // A disabled window has a modal dialog open and must not be minimized.
      .canMinimize = (style & WS_MINIMIZEBOX) != 0 && IsWindowEnabled(task),
  };
}

void ActivateTask(HWND task, const TaskClickState& state) {
  switch (DecideTaskClick(state)) {
    case TaskClickAction::Minimize:
      AllowToActivate(task);
      PostMessageW(task, WM_SYSCOMMAND, SC_MINIMIZE, 0);
      return;
    case TaskClickAction::Restore: {
      // The app's own restore keeps maximized windows maximized; the timeout
      // keeps a hung app from freezing the bar.
      DWORD_PTR result = 0;
      SendMessageTimeoutW(task, WM_SYSCOMMAND, SC_RESTORE, 0, SMTO_ABORTIFHUNG, 200, &result);
      break;
    }
    case TaskClickAction::Activate:
      break;
    case TaskClickAction::None:
      return;
  }
  // Activate the open dialog, if any, rather than the window it blocks.
  BringToFront(GetLastActivePopup(task));
}

void ShowWindowMenu(HWND owner, HWND task, POINT screenPoint) {
  HMENU menu = BuildWindowMenu(ReadMenuState(task));
  // Required for the menu to close when the user clicks elsewhere.
  SetForegroundWindow(owner);
  const auto command =
      static_cast<UINT>(TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
                                       screenPoint.x, screenPoint.y, 0, owner, nullptr));
  PostMessageW(owner, WM_NULL, 0, 0);
  DestroyMenu(menu);
  if (command == 0) return;

  // Moving and sizing happen with the keyboard, so the window must be active.
  if (command == SC_MOVE || command == SC_SIZE) BringToFront(task);
  AllowToActivate(task);
  PostMessageW(task, WM_SYSCOMMAND, command, 0);
}

void ToggleDesktop() {
  if (HWND tray = ExplorerTray()) {
    PostMessageW(tray, WM_COMMAND, kShowDesktopCommand, 0);
  } else {
    PressWinD();
  }
}

void ApplyMenuTheme(bool dark) {
  // Undocumented uxtheme exports (ordinals 135 and 136) that every dark-mode
  // Win32 app relies on; missing on old builds, in which case menus stay light.
  using SetPreferredAppModeFn = int(WINAPI*)(int);
  using FlushMenuThemesFn = void(WINAPI*)();
  static const HMODULE uxtheme =
      LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
  if (!uxtheme) return;
  const auto setMode = reinterpret_cast<SetPreferredAppModeFn>(
      reinterpret_cast<void*>(GetProcAddress(uxtheme, MAKEINTRESOURCEA(135))));
  const auto flush = reinterpret_cast<FlushMenuThemesFn>(
      reinterpret_cast<void*>(GetProcAddress(uxtheme, MAKEINTRESOURCEA(136))));
  constexpr int kForceDark = 2;
  constexpr int kForceLight = 3;
  if (setMode) setMode(dark ? kForceDark : kForceLight);
  if (flush) flush();
}

}  // namespace throwback
