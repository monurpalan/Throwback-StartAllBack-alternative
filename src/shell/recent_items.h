#pragma once

#include <string>
#include <vector>

namespace throwback::shell {

struct RecentItem {
  std::wstring name;
  std::wstring target;  // parsing name, opened with the shell
};

// The app ID Windows files a program's jump list under, for a start menu
// target: the ID itself for "shell:AppsFolder\<id>", or the one a shortcut
// declares. Empty when there is none.
std::wstring AppIdForTarget(const std::wstring& target);

// The program's recent documents, newest first, as its jump list shows them.
std::vector<RecentItem> LoadRecentItems(const std::wstring& appId, size_t limit);

}  // namespace throwback::shell
