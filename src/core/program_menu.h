#pragma once

#include <string_view>
#include <vector>

namespace throwback {

// The commands of a program's right-click menu in the start menu.
enum class ProgramCommand {
  Open = 1,
  RunAsAdministrator,
  OpenFileLocation,
  Pin,
  Unpin,
  Properties
};

struct ProgramMenuItem {
  ProgramCommand command;
  bool separatorBefore = false;
};

// A shortcut or program on disk, as opposed to a packaged app or a link.
bool IsFileTarget(std::wstring_view target);

// The menu Windows 7 shows for a start menu program: file-only commands are
// left out for packaged apps, and pinning flips to unpinning.
std::vector<ProgramMenuItem> ProgramMenuItems(bool fileTarget, bool pinned);

}  // namespace throwback
