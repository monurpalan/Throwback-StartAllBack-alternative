#pragma once

#include <windows.h>

#include <functional>

#include "core/settings.h"

namespace throwback {

// The "Taskbar and Start Menu Properties" window, built from native controls
// like the Windows 7 one. Shows modally over owner; apply receives the edited
// settings on OK and Apply. Showing it again while open brings it forward.
void ShowSettingsDialog(HINSTANCE instance, HWND owner, const Settings& current,
                        const std::function<void(const Settings&)>& apply);

}  // namespace throwback
