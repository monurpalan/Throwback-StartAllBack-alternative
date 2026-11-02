// The start menu's pinned programs and right-click menu.
#include <algorithm>
#include <string_view>
#include <utility>

#include "startmenu/program_menu.h"
#include "startmenu/start_menu.h"

namespace throwback {
namespace {

// The app ID to show a jump list for, when the program has recent items.
std::wstring JumpListId(const std::wstring& target) {
  std::wstring appId = shell::AppIdForTarget(target);
  if (shell::LoadRecentItems(appId, 1).empty()) appId.clear();
  return appId;
}

}  // namespace

std::wstring StartMenu::ProgramName(const std::wstring& target) const {
  // Programs from All Programs are shortcuts; frequent ones are app IDs.
  const auto it = std::ranges::find_if(
      catalog_->all, [&](const Program* program) { return program->target == target; });
  if (it != catalog_->all.end()) return (*it)->name;
  constexpr std::wstring_view kAppsFolder = L"shell:AppsFolder\\";
  if (!target.starts_with(kAppsFolder)) return {};
  const auto app = catalog_->appNames.find(target.substr(kAppsFolder.size()));
  return app == catalog_->appNames.end() ? std::wstring() : app->second;
}

void StartMenu::AddHomeRows() {
  // Pinned programs on top, then a separator and the most used ones, as on
  // Windows 7. Pins of uninstalled programs stay saved but aren't shown.
  const auto limit = static_cast<size_t>(layout_.listRows);
  for (const auto& target : settings_.pinned) {
    if (rows_.size() == limit) return;
    if (std::wstring name = ProgramName(target); !name.empty()) {
      rows_.push_back({std::move(name), target, nullptr, {}, false, true, JumpListId(target)});
    }
  }
  const size_t pinnedCount = rows_.size();
  for (const auto& program : frequent_) {
    if (rows_.size() == limit) break;
    if (IsPinned(settings_, program.target)) continue;
    rows_.push_back(
        {program.name, program.target, nullptr, {}, false, true, JumpListId(program.target)});
  }
  if (pinnedCount > 0 && rows_.size() > pinnedCount) rows_[pinnedCount].separatorBefore = true;
}

void StartMenu::ShowRowMenu(int index, POINT point) {
  if (index < 0 || index >= static_cast<int>(rows_.size())) return;
  const Row row = rows_[static_cast<size_t>(index)];
  if (!row.pinnable) return;
  selected_ = index;
  InvalidateRect(hwnd_, nullptr, FALSE);

  const bool file = IsFileTarget(row.target);
  keepOpen_ = true;
  const auto command =
      ShowProgramMenu(hwnd_, point, ProgramMenuItems(file, IsPinned(settings_, row.target)));
  keepOpen_ = false;
  if (!command) return;
  switch (*command) {
    case ProgramCommand::Open:
    case ProgramCommand::RunAsAdministrator:
      ActivateRow(index, *command == ProgramCommand::RunAsAdministrator);
      return;
    case ProgramCommand::OpenFileLocation:
      Hide(/*restoreFocus=*/false);
      OpenFileLocation(row.target);
      return;
    case ProgramCommand::Properties:
      Hide(/*restoreFocus=*/false);
      ShowProperties(GetWindow(hwnd_, GW_OWNER), row.target);
      return;
    case ProgramCommand::Pin:
      Pin(settings_, row.target);
      break;
    case ProgramCommand::Unpin:
      Unpin(settings_, row.target);
      break;
  }
  if (onPinnedChanged_) onPinnedChanged_();
  RebuildRows();
  InvalidateRect(hwnd_, nullptr, FALSE);
}

}  // namespace throwback
