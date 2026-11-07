#pragma once

#include <string>
#include <vector>

namespace throwback {

// A link in the right column of the start menu.
struct Place {
  std::wstring name;
  std::wstring target;
  bool separatorBefore = false;
  int icon = 0;       // Windows XP: the place's shell32 icon, by ID
  bool bold = false;  // Windows XP sets the user's own places in bold
};

// The user's folder, libraries and the system places of the Windows 7 start
// menu, named by the shell in the user's language.
std::vector<Place> LoadPlaces();

// The right column of the Windows XP start menu.
std::vector<Place> LoadXpPlaces();

// The right column of the Windows Vista start menu.
std::vector<Place> LoadVistaPlaces();

struct Account {
  std::wstring name;
  std::wstring picturePath;  // empty when the user has no account picture
};

Account LoadAccount();

}  // namespace throwback
