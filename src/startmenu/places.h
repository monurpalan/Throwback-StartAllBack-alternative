#pragma once

#include <string>
#include <vector>

namespace throwback {

// A link in the right column of the start menu.
struct Place {
  std::wstring name;
  std::wstring target;
  bool separatorBefore = false;
};

// The user's folder, libraries and the system places of the Windows 7 start
// menu, named by the shell in the user's language.
std::vector<Place> LoadPlaces();

struct Account {
  std::wstring name;
  std::wstring picturePath;  // empty when the user has no account picture
};

Account LoadAccount();

}  // namespace throwback
