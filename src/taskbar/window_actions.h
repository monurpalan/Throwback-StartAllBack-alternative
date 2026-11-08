#pragma once

#include <windows.h>

#include "core/task_click.h"

namespace throwback {

// Captures what a click on the window's button should do, at mouse-down time.
TaskClickState ReadClickState(HWND task);
void ActivateTask(HWND task, const TaskClickState& state);

// Shows the window's classic menu (Restore, Move, Size, Minimize, Maximize,
// Close) at a screen point, labelled with Windows' own localized strings,
// and runs the chosen command.
void ShowWindowMenu(HWND owner, HWND task, POINT screenPoint);

// Toggles "show desktop" the way the system taskbar does.
void ToggleDesktop();
// Windows' calendar flyout (Win+Alt+D) and notification center (Win+N).
void OpenClockFlyout();
void OpenNotifications();
// Windows' search (Win+S) and Task View (Win+Tab).
void OpenSearch();
void OpenTaskView();

// Makes native popup menus of this process follow the bar's theme.
void ApplyMenuTheme(bool dark);

}  // namespace throwback
