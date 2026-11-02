#include "startmenu/program_menu.h"

#include <shlobj.h>

#include "ui/strings.h"

namespace throwback {
namespace {

ui::StringId LabelOf(ProgramCommand command) {
  switch (command) {
    case ProgramCommand::Open:
      return ui::StringId::OpenProgram;
    case ProgramCommand::RunAsAdministrator:
      return ui::StringId::RunAsAdministrator;
    case ProgramCommand::OpenFileLocation:
      return ui::StringId::OpenFileLocation;
    case ProgramCommand::Pin:
      return ui::StringId::PinToStartMenu;
    case ProgramCommand::Unpin:
      return ui::StringId::UnpinFromStartMenu;
    case ProgramCommand::Properties:
      return ui::StringId::Properties;
  }
  return ui::StringId::OpenProgram;
}

}  // namespace

std::optional<ProgramCommand> ShowProgramMenu(HWND owner, POINT point,
                                              const std::vector<ProgramMenuItem>& items) {
  HMENU menu = CreatePopupMenu();
  for (const auto& item : items) {
    if (item.separatorBefore) AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(item.command),
                std::wstring(ui::Tr(LabelOf(item.command))).c_str());
  }
  SetMenuDefaultItem(menu, static_cast<UINT>(ProgramCommand::Open), FALSE);
  const auto chosen =
      TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, owner, nullptr);
  DestroyMenu(menu);
  if (chosen == 0) return std::nullopt;
  return static_cast<ProgramCommand>(chosen);
}

void OpenFileLocation(const std::wstring& path) {
  PIDLIST_ABSOLUTE item = ILCreateFromPathW(path.c_str());
  if (!item) return;
  SHOpenFolderAndSelectItems(item, 0, nullptr, 0);
  ILFree(item);
}

void ShowProperties(HWND owner, const std::wstring& path) {
  SHObjectProperties(owner, SHOP_FILEPATH, path.c_str(), nullptr);
}

}  // namespace throwback
