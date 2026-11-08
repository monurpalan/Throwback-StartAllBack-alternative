#pragma once

#include <string>
#include <vector>

#include "core/settings.h"

namespace throwback {

// A link in the right column of the start menu.
struct Place {
  std::wstring name;
  std::wstring target;
  bool separatorBefore = false;
  int icon = 0;       // Windows XP: the place's shell32 icon, by ID
  bool bold = false;  // Windows XP sets the user's own places in bold
};

// A place the user can turn on or off, as the Customize Start Menu dialog
// lists it: its settings key and its name. Styles share keys, so a place
// turned off stays off in every style that has it.
struct PlaceChoice {
  std::wstring key;
  std::wstring name;
  bool shown = false;
};

// Every place of the settings' style (Windows 7's for the styles without a
// list of their own) in menu order, shown or not as settings say.
std::vector<PlaceChoice> LoadPlaceChoices(const Settings& settings);

// The user's folder, libraries and the system places of the Windows 7 start
// menu that settings show, named by the shell in the user's language.
// Groups are separated by a line.
std::vector<Place> LoadPlaces(const Settings& settings);

// The right columns of the Windows XP and Vista start menus, without the
// places the user turned off.
std::vector<Place> LoadXpPlaces(const Settings& settings);
std::vector<Place> LoadVistaPlaces(const Settings& settings);

struct Account {
  std::wstring name;
  std::wstring picturePath;  // empty when the user has no account picture
};

Account LoadAccount();

}  // namespace throwback
