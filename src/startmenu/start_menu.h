#pragma once

#include <windows.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/settings.h"
#include "core/start_menu_layout.h"
#include "shell/async_icons.h"
#include "startmenu/frequent.h"
#include "startmenu/places.h"
#include "startmenu/programs.h"
#include "ui/canvas.h"
#include "ui/theme.h"

namespace throwback {

// The classic Windows 7 start menu: frequent programs, All Programs and
// search on the left; the account picture, places and power on the right.
class StartMenu {
 public:
  // The menu is owned by the bar, so closing requests reach the bar.
  // Pinning changes settings.pinned and then calls onPinnedChanged to save.
  StartMenu(HINSTANCE instance, HWND owner, Settings& settings, std::function<void()> onClosed,
            std::function<void()> onPinnedChanged);
  ~StartMenu();
  StartMenu(const StartMenu&) = delete;
  StartMenu& operator=(const StartMenu&) = delete;

  // anchor is the start button in screen coordinates; the menu opens below
  // it for a bar at the top of the screen and above it otherwise.
  void Open(const RECT& anchor, bool below, unsigned dpi);
  // Dismisses the menu and gives focus back to the window that had it.
  void Close() { Hide(/*restoreFocus=*/true); }
  bool IsOpen() const;

 private:
  // restoreFocus is false when something else is taking focus anyway: the
  // user clicked another window, or a program is being launched.
  void Hide(bool restoreFocus);

  enum class Mode { Home, AllPrograms, Search };
  struct Row {
    std::wstring name;
    std::wstring target;
    const Program* folder = nullptr;
    std::wstring iconTarget;  // when the icon comes from another item than target
    bool separatorBefore = false;
    bool pinnable = false;  // a program, not a folder or a settings page
  };
  // What the mouse is over.
  struct Hit {
    enum class Kind { None, Row, AllPrograms, Place, Power, PowerArrow } kind = Kind::None;
    int index = -1;
  };

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

  void LoadData();
  void AddHomeRows();
  // The display name of a launchable target; empty once it is uninstalled.
  std::wstring ProgramName(const std::wstring& target) const;
  void ShowRowMenu(int index, POINT point);
  void AddSettingsResults();
  void SetMode(Mode mode);
  void RebuildRows();
  int RowHeight() const;
  int IconSize() const;
  int VisibleRows() const;
  void EnsureVisible(int row);
  Hit HitTest(int x, int y) const;
  std::vector<Rect> PlaceRowRects() const;

  void Paint();
  void OnMouseMove(int x, int y);
  void OnClick(int x, int y);
  void OnWheel(int delta);
  void OnChar(wchar_t c);
  void OnKey(UINT key);
  void ActivateRow(int index, bool elevated);
  void GoBack();
  void ShowPowerMenu();

  HWND hwnd_ = nullptr;
  Settings& settings_;
  std::function<void()> onClosed_;
  std::function<void()> onPinnedChanged_;
  std::unique_ptr<ui::Canvas> canvas_;
  StartMenuLayout layout_;
  unsigned dpi_ = 96;

  std::unique_ptr<ProgramCatalog> catalog_;
  ULONGLONG catalogLoadedAt_ = 0;
  std::vector<FrequentProgram> frequent_;
  std::vector<Place> places_;
  Account account_;
  std::unique_ptr<shell::AsyncIconCache> icons_;

  Mode mode_ = Mode::Home;
  std::vector<const Program*> folders_;  // All Programs: the open folder path
  std::wstring query_;
  std::vector<Row> rows_;
  int scroll_ = 0;
  int selected_ = -1;
  Hit hover_;
  bool trackingMouse_ = false;
  bool keepOpen_ = false;  // a popup menu of ours is showing
  HWND previousForeground_ = nullptr;
};

}  // namespace throwback
