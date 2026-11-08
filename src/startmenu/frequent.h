#pragma once

#include <string>
#include <vector>

#include "core/user_assist.h"
#include "startmenu/programs.h"

namespace throwback {

struct FrequentProgram {
  std::wstring name;
  std::wstring target;
};

// The parsed entries of one UserAssist key ("{GUID}\\Count" under
// Explorer\\UserAssist).
std::vector<UsageEntry> ReadUserAssist(const wchar_t* guid);

// The programs the user starts most, from Windows' own usage history, so the
// list is right from the first run. Only apps that still exist are listed.
std::vector<FrequentProgram> LoadFrequentPrograms(const ProgramCatalog& catalog, size_t limit);

}  // namespace throwback
