#pragma once

#include <windows.h>

#include <string_view>
#include <utility>
#include <vector>

#include "core/layout.h"
#include "core/options.h"
#include "ui/canvas.h"

namespace throwback {

// What the mouse can point at in a modern start menu.
struct ModernHit {
  enum class Kind {
    None,
    Tile,      // a pinned program
    Frequent,  // Windows 10: a most used program
    App,       // a row of the app list or of search results
    Recent,    // Windows 11: a recommended document
    AllApps,
    Back,
    Power,
    User,
    Documents,
    Pictures,
    Settings,
  } kind = Kind::None;
  int index = -1;

  bool operator==(const ModernHit&) const = default;
};

// Everything a modern start menu shows for one frame.
struct ModernModel {
  struct Item {
    std::wstring_view name;
    std::wstring_view caption;  // Windows 11: where a recommended document is
    HICON icon = nullptr;
    bool header = false;  // a letter heading the apps under it
  };
  enum class Page { Home, AllApps, Search };

  StartStyle style = StartStyle::Win11;
  ui::ModernLook look = ui::ModernLook::Win11Dark;
  Rect frame;
  Page page = Page::Home;
  std::vector<Item> tiles;
  std::vector<Item> frequent;
  std::vector<Item> apps;  // all apps with letter headings, or search results
  std::vector<Item> recent;
  int scroll = 0;  // apps scrolled past the top (or the left, on Windows 8)
  std::wstring_view query;
  std::wstring_view accountName;
  std::wstring_view picturePath;
  ModernHit hover;
};

using ModernRegions = std::vector<std::pair<Rect, ModernHit>>;

// The menu's size at 96 DPI scaled for dpi; Windows 8's start screen fills
// the monitor instead.
Rect ModernFrame(StartStyle style, unsigned dpi, const RECT& monitor);

// How many rows of apps fit on a page (columns on Windows 8), for scrolling.
int ModernPageRows(const ModernModel& model, unsigned dpi);

// Draws the menu and returns where each thing the mouse can hit is.
ModernRegions DrawModernStartMenu(ui::Canvas& canvas, const ModernModel& model);
ModernRegions DrawWin8StartScreen(ui::Canvas& canvas, const ModernModel& model);
ModernRegions DrawWin10StartMenu(ui::Canvas& canvas, const ModernModel& model);
ModernRegions DrawWin11StartMenu(ui::Canvas& canvas, const ModernModel& model);

}  // namespace throwback
