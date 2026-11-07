#pragma once

#include <windows.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/layout.h"
#include "ui/canvas.h"

namespace throwback {

// One row of a classic menu.
struct ClassicEntry {
  std::wstring text;
  int classicIcon = 0;      // the style's own shell32 icon, by ID; 0 for none
  std::wstring iconTarget;  // otherwise the shell item whose icon to show
  bool separator = false;
  bool disabled = false;  // "(Empty)"
  // Set for rows that open a submenu; built when it opens.
  std::function<std::vector<ClassicEntry>()> submenu;
  std::function<void()> action;
};

// What every level of one classic start menu shares.
struct ClassicMenuHost {
  ui::ClassicLook look = ui::ClassicLook::Win98;
  ui::SkinImage banner = ui::SkinImage::Banner98;
  unsigned dpi = 96;
  // The icon of a row at a size in pixels; may be null until it is loaded.
  std::function<HICON(const ClassicEntry&, int size)> icon;
  // A row without a submenu was chosen: close everything, then run it.
  std::function<void(std::function<void()> action)> invoke;
  // The menu was dismissed (Escape, or a click elsewhere).
  std::function<void()> dismiss;
};

// One level of a cascading classic menu: the start menu itself with its
// banner and large icons, or a submenu with small ones. A level opens its
// submenu as a child level to its side, as Windows 98 did. Long menus wrap
// into more columns.
class ClassicMenu {
 public:
  ClassicMenu(HINSTANCE instance, HWND owner, ClassicMenuHost& host, bool root);
  ~ClassicMenu();
  ClassicMenu(const ClassicMenu&) = delete;
  ClassicMenu& operator=(const ClassicMenu&) = delete;

  // Shows the root level with its bottom-left (or top-left, below) corner
  // at the anchor's matching corner.
  void ShowRoot(std::vector<ClassicEntry> entries, const RECT& anchor, bool below);
  // Shows a level that isn't the start menu itself next to a row (screen
  // pixels): a submenu, or Windows XP's All Programs menu.
  void ShowBeside(std::vector<ClassicEntry> entries, const RECT& row);
  void Hide();
  bool IsOpen() const;
  HWND Hwnd() const { return hwnd_; }
  // Repaints this level and its open submenus, after icons arrived.
  void Repaint();

  static constexpr UINT kIconsReady = WM_APP + 1;

 private:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

  void Measure();
  void Paint();
  int HitTest(POINT point) const;
  RECT RowScreenRect(int index) const;
  void Select(int index, bool openNow);
  void OpenSubmenu();
  void CloseSubmenu();
  void Activate(int index);
  ClassicMenu* Deepest();
  void OnKey(UINT key);
  int Step(int from, int direction) const;

  HINSTANCE instance_;
  HWND owner_;
  HWND hwnd_ = nullptr;
  ClassicMenuHost& host_;
  bool root_;
  std::unique_ptr<ui::Canvas> canvas_;
  std::vector<ClassicEntry> entries_;
  std::vector<Rect> rows_;  // one per entry
  int width_ = 0;
  int height_ = 0;
  int selected_ = -1;
  int submenuOf_ = -1;  // the row whose submenu is open
  std::unique_ptr<ClassicMenu> child_;
  ClassicMenu* parent_ = nullptr;
  bool tracking_ = false;
};

}  // namespace throwback
