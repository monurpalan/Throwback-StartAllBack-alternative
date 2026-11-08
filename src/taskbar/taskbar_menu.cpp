// The bar's right-click menu. Besides Properties and Exit it offers the
// start menu choices people change most, so they need not open the dialog.
#include <string>

#include "taskbar/bar_manager.h"
#include "taskbar/settings_dialog.h"
#include "taskbar/start_menu_customize.h"
#include "taskbar/taskbar_window.h"
#include "ui/strings.h"

namespace throwback {
namespace {

using ui::StringId;

enum : UINT {
  kExit = 1,
  kProperties,
  kCustomize,
  kHideTaskbar,
  kStartWithWindows,
  kStyleFirst = 100,  // + index in kStartStyles
  kOrbFirst = 200,    // + OrbChoice
  kSideFirst = 300,   // + MenuHorizontal
};

void Append(HMENU menu, UINT id, std::wstring_view text, bool checked = false) {
  AppendMenuW(menu, MF_STRING | (checked ? MF_CHECKED : 0), id, std::wstring(text).c_str());
}

// A submenu of choices with a dot on the current one.
void AppendChoices(HMENU menu, StringId label, UINT first,
                   std::initializer_list<std::wstring_view> names, size_t current) {
  HMENU sub = CreatePopupMenu();
  size_t index = 0;
  for (std::wstring_view name : names) Append(sub, first + static_cast<UINT>(index++), name);
  CheckMenuRadioItem(sub, first, first + static_cast<UINT>(names.size()) - 1,
                     first + static_cast<UINT>(current), MF_BYCOMMAND);
  AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(sub), std::wstring(ui::Tr(label)).c_str());
}

HMENU BuildMenu(const Settings& s) {
  HMENU menu = CreatePopupMenu();

  HMENU styles = CreatePopupMenu();
  for (size_t i = 0; i < kStartStyles.size(); ++i) {
    Append(styles, kStyleFirst + static_cast<UINT>(i), kStartStyles[i].name);
    if (kStartStyles[i].style == s.startStyle) {
      CheckMenuRadioItem(styles, kStyleFirst,
                         kStyleFirst + static_cast<UINT>(kStartStyles.size()) - 1,
                         kStyleFirst + static_cast<UINT>(i), MF_BYCOMMAND);
    }
  }
  AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(styles),
              std::wstring(ui::Tr(StringId::MenuStartStyle)).c_str());
  AppendChoices(menu, StringId::MenuStartButton, kOrbFirst,
                {ui::Tr(StringId::OrbStyle), L"Windows 7", L"Windows Vista",
                 ui::Tr(StringId::OrbClover), ui::Tr(StringId::OrbWin8), L"E1even"},
                static_cast<size_t>(s.orb));
  AppendChoices(menu, StringId::MenuSide, kSideFirst,
                {ui::Tr(StringId::MenuLeft), ui::Tr(StringId::MenuRight)},
                static_cast<size_t>(s.menuHorizontal));
  Append(menu, kCustomize, ui::Tr(StringId::MenuCustomizeStart));
  if (!HasCustomizableMenu(s.startStyle)) EnableMenuItem(menu, kCustomize, MF_GRAYED);

  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  Append(menu, kHideTaskbar, ui::Tr(StringId::HideWindowsTaskbar), s.hideSystemTaskbar);
  Append(menu, kStartWithWindows, ui::Tr(StringId::StartWithWindows), s.startWithWindows);
  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  Append(menu, kProperties, ui::Tr(StringId::Properties));
  Append(menu, kExit, ui::Tr(StringId::ExitThrowback));
  return menu;
}

// Applies a choice to settings; false when the command changes none.
bool ApplyChoice(UINT command, Settings& s) {
  if (command >= kStyleFirst && command < kStyleFirst + kStartStyles.size()) {
    s.startStyle = kStartStyles[command - kStyleFirst].style;
  } else if (command >= kOrbFirst && command <= kOrbFirst + static_cast<UINT>(OrbChoice::E1even)) {
    s.orb = static_cast<OrbChoice>(command - kOrbFirst);
  } else if (command >= kSideFirst && command <= kSideFirst + 1) {
    s.menuHorizontal = static_cast<MenuHorizontal>(command - kSideFirst);
  } else if (command == kHideTaskbar) {
    s.hideSystemTaskbar = !s.hideSystemTaskbar;
  } else if (command == kStartWithWindows) {
    s.startWithWindows = !s.startWithWindows;
  } else {
    return false;
  }
  return true;
}

}  // namespace

void TaskbarWindow::ToggleStartMenu() {
  HideThumbnail();
  if (startMenu_->IsOpen()) {
    startMenu_->Close();
    return;
  }
  POINT topLeft{layout_.startButton.left, layout_.startButton.top};
  POINT bottomRight{layout_.startButton.right, layout_.startButton.bottom};
  ClientToScreen(hwnd_, &topLeft);
  ClientToScreen(hwnd_, &bottomRight);
  // On an upright bar the start button is at the top, and the menu opens
  // beside the bar from the button's top edge down.
  const bool vertical = IsVertical(settings_.edge);
  if (vertical) bottomRight.y = topLeft.y;
  startMenu_->Open({.button = {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y},
                    .below = settings_.edge == Edge::Top || vertical,
                    .dpi = dpi_,
                    .horizontal = settings_.menuHorizontal,
                    .vertical = settings_.menuVertical});
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void TaskbarWindow::ShowBarMenu(POINT screenPoint) {
  HMENU menu = BuildMenu(settings_);
  // Required for the menu to close when the user clicks elsewhere.
  SetForegroundWindow(hwnd_);
  const auto command =
      static_cast<UINT>(TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
                                       screenPoint.x, screenPoint.y, 0, hwnd_, nullptr));
  PostMessageW(hwnd_, WM_NULL, 0, 0);
  DestroyMenu(menu);

  Settings edited = settings_;
  if (command == kExit) {
    PostMessageW(hwnd_, WM_CLOSE, 0, 0);
  } else if (command == kProperties) {
    OpenSettings();
  } else if (command == kCustomize) {
    if (ShowCustomizeStartMenu(module_, hwnd_, edited)) manager_.ApplySettings(edited);
  } else if (ApplyChoice(command, edited)) {
    manager_.ApplySettings(edited);
  }
}

}  // namespace throwback
