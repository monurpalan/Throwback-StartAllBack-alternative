#pragma once

#include <windows.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/settings.h"
#include "shell/async_icons.h"
#include "startmenu/frequent.h"
#include "startmenu/modern_view.h"
#include "startmenu/places.h"
#include "startmenu/programs.h"
#include "startmenu/start_menu_window.h"

namespace throwback {

// The flat start menus: Windows 8's and 8.1's start screen, Windows 10's
// menu with tiles, and Windows 11's menu with its pinned grid. Typing
// searches the programs in Windows 10 and 11.
class ModernStartMenu : public StartMenuWindow {
 public:
  ModernStartMenu(HINSTANCE instance, HWND owner, Settings& settings,
                  std::function<void()> onChanged);
  ~ModernStartMenu() override;
  ModernStartMenu(const ModernStartMenu&) = delete;
  ModernStartMenu& operator=(const ModernStartMenu&) = delete;

  void Open(const MenuAnchor& anchor) override;
  void Close() override { Hide(/*restoreFocus=*/true); }
  bool IsOpen() const override;

 private:
  struct Entry {
    std::wstring name;
    std::wstring target;
    std::wstring caption;
    bool header = false;
  };

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
  void Hide(bool restoreFocus);
  void LoadData();
  void BuildApps();
  void SetPage(ModernModel::Page page);
  ModernModel Model();
  void Paint();
  ModernHit HitAt(POINT point) const;
  void OnMouseMove(POINT point);
  void OnClick(POINT point);
  void OnWheel(int delta);
  void OnChar(wchar_t c);
  void OnKey(UINT key);
  void Launch(const std::wstring& target);
  void ShowMenu(bool power, POINT point);
  ui::ModernLook Look() const;

  HWND hwnd_ = nullptr;
  Settings& settings_;
  StartStyle style_;
  std::function<void()> onChanged_;
  std::unique_ptr<ui::Canvas> canvas_;
  std::unique_ptr<shell::AsyncIconCache> icons_;
  unsigned dpi_ = 96;
  Rect frame_;

  std::unique_ptr<ProgramCatalog> catalog_;
  ULONGLONG catalogLoadedAt_ = 0;
  Account account_;
  std::vector<Entry> tiles_;
  std::vector<Entry> frequent_;
  std::vector<Entry> apps_;
  std::vector<Entry> recent_;

  ModernModel::Page page_ = ModernModel::Page::Home;
  std::wstring query_;
  int scroll_ = 0;
  ModernHit hover_;
  ModernRegions regions_;
  bool trackingMouse_ = false;
  bool keepOpen_ = false;  // a popup menu of ours is showing
  HWND previousForeground_ = nullptr;
};

}  // namespace throwback
