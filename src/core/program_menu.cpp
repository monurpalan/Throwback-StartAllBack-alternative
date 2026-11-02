#include "core/program_menu.h"

namespace throwback {

bool IsFileTarget(std::wstring_view target) {
  const bool drivePath = target.size() > 2 && target[1] == L':';
  const bool uncPath = target.starts_with(L"\\\\");
  return drivePath || uncPath;
}

std::vector<ProgramMenuItem> ProgramMenuItems(bool fileTarget, bool pinned) {
  std::vector<ProgramMenuItem> items{{ProgramCommand::Open}};
  if (fileTarget) {
    items.push_back({ProgramCommand::RunAsAdministrator});
    items.push_back({ProgramCommand::OpenFileLocation});
  }
  items.push_back({pinned ? ProgramCommand::Unpin : ProgramCommand::Pin, true});
  if (fileTarget) items.push_back({ProgramCommand::Properties, true});
  return items;
}

}  // namespace throwback
