#pragma once

#include <string>
#include <vector>

#include "startmenu/programs.h"

namespace throwback {

struct FrequentProgram {
  std::wstring name;
  std::wstring target;
};

// The programs the user starts most, from Windows' own usage history, so the
// list is right from the first run. Only apps that still exist are listed.
std::vector<FrequentProgram> LoadFrequentPrograms(const ProgramCatalog& catalog, size_t limit);

}  // namespace throwback
