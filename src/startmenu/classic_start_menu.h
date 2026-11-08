#pragma once

#include <windows.h>

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "shell/async_icons.h"
#include "startmenu/classic_menu.h"
#include "startmenu/places.h"
#include "startmenu/programs.h"
#include "startmenu/start_menu_window.h"

namespace throwback {

// A program folder as menu rows: subfolders (with folderIcon, the style's
// own shell32 icon) first, then programs, each in name order.
std::vector<ClassicEntry> ProgramMenuEntries(const Program& folder, int folderIcon);

// The cascading start menu of Windows 98, Me and 2000: a banner and large
// icons, with Programs, Documents, Settings and Find (Search) opening
// submenus beside it.
class ClassicStartMenu : public StartMenuWindow {
 public:
  ClassicStartMenu(HINSTANCE instance, HWND owner, StartStyle style,
                   std::function<void()> onChanged);
  ~ClassicStartMenu() override;
  ClassicStartMenu(const ClassicStartMenu&) = delete;
  ClassicStartMenu& operator=(const ClassicStartMenu&) = delete;

  void Open(const MenuAnchor& anchor) override;
  void Close() override;
  bool IsOpen() const override;

 private:
  std::vector<ClassicEntry> RootEntries();
  std::vector<ClassicEntry> DocumentEntries();
  std::vector<ClassicEntry> SettingsEntries();
  std::vector<ClassicEntry> FindEntries();
  HICON Icon(const ClassicEntry& entry, int size);

  HWND owner_;
  StartStyle style_;
  std::function<void()> onChanged_;
  ClassicMenuHost host_;
  std::unique_ptr<ClassicMenu> root_;
  std::unique_ptr<shell::AsyncIconCache> icons_;
  std::unique_ptr<ProgramCatalog> catalog_;
  ULONGLONG catalogLoadedAt_ = 0;
  Account account_;
  // The style's own icons, by shell32 ID and size.
  std::map<std::pair<int, int>, HICON> classicIcons_;
};

}  // namespace throwback
