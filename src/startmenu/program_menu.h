#pragma once

#include <windows.h>

#include <optional>
#include <string>
#include <vector>

#include "core/program_menu.h"

namespace throwback {

// Shows a program's right-click menu at point (screen coordinates) and
// returns the chosen command. "Open" is the bold default, as in Explorer.
std::optional<ProgramCommand> ShowProgramMenu(HWND owner, POINT point,
                                              const std::vector<ProgramMenuItem>& items);

// Opens the folder holding path in Explorer, with the item selected.
void OpenFileLocation(const std::wstring& path);

// The shell's Properties dialog for a file.
void ShowProperties(HWND owner, const std::wstring& path);

}  // namespace throwback
