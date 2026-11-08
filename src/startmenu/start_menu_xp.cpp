// The Windows XP start menu's own parts: its layout, its cascading All
// Programs menu and the theme's icons.
#include "shell/launch.h"
#include "startmenu/classic_start_menu.h"
#include "startmenu/start_menu.h"
#include "ui/skin_ids.h"
#include "ui/strings.h"

namespace throwback {
namespace {

// Windows XP's program group icon in its shell32.dll.
constexpr int kXpFolderIcon = 20;
// Plain8's pane widths at 96 DPI, from its theme.
constexpr int kPlain8ProgramPane = 311;
constexpr int kPlain8Places = 195;

}  // namespace

void StartMenu::ComputeLayout() {
  if (IsXp()) {
    layout_ = ComputeXpStartMenuLayout(
        dpi_, canvas_->MeasureText(ui::Tr(ui::StringId::XpLogOff), ui::TextStyle::XpBody),
        canvas_->MeasureText(ui::Tr(ui::StringId::XpTurnOff), ui::TextStyle::XpBody),
        settings_.menuRows, settings_.largeIcons);
    return;
  }
  StartMenuShape shape;
  shape.rows = settings_.menuRows;
  shape.largeIcons = settings_.largeIcons;
  if (style_ == StartStyle::Plain8) {
    shape.programPaneWidth = kPlain8ProgramPane;
    shape.placesWidth = kPlain8Places;
  }
  shape.placeRows = static_cast<int>(places_.size());
  for (const Place& place : places_) shape.placeSeparators += place.separatorBefore ? 1 : 0;
  if (style_ == StartStyle::Vista) {
    layout_ = ComputeVistaStartMenuLayout(dpi_, shape);
    return;
  }
  layout_ = ComputeStartMenuLayout(
      dpi_, canvas_->MeasureText(ui::Tr(ui::StringId::ShutDown), ui::TextStyle::Body), shape);
}

void StartMenu::CreateXpPrograms(HINSTANCE instance, HWND owner) {
  xpHost_.look = ui::ClassicLook::WinXp;
  xpHost_.icon = [this](const ClassicEntry& entry, int size) {
    if (entry.classicIcon != 0) return XpIcon(entry.classicIcon, size);
    return entry.iconTarget.empty() ? nullptr : icons_->Get(entry.iconTarget, size);
  };
  xpHost_.invoke = [this](std::function<void()> action) {
    Hide(/*restoreFocus=*/false);
    if (action) action();
  };
  xpHost_.dismiss = [this] { CloseXpPrograms(); };
  xpPrograms_ = std::make_unique<ClassicMenu>(instance, owner, xpHost_, /*root=*/false);
}

void StartMenu::OpenXpPrograms() {
  if (!xpPrograms_ || xpPrograms_->IsOpen() || !catalog_) return;
  xpHost_.dpi = dpi_;
  POINT topLeft{layout_.allPrograms.left, layout_.allPrograms.top};
  POINT bottomRight{layout_.allPrograms.right, layout_.allPrograms.bottom};
  ClientToScreen(hwnd_, &topLeft);
  ClientToScreen(hwnd_, &bottomRight);
  xpPrograms_->ShowBeside(ProgramMenuEntries(catalog_->root, kXpFolderIcon),
                          {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y});
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void StartMenu::CloseXpPrograms() {
  if (!xpPrograms_ || !xpPrograms_->IsOpen()) return;
  xpPrograms_->Hide();
  InvalidateRect(hwnd_, nullptr, FALSE);
}

HICON StartMenu::XpIcon(int id, int size) {
  const auto key = std::make_pair(id, size);
  if (const auto it = xpIcons_.find(key); it != xpIcons_.end()) return it->second;
  auto* icon = static_cast<HICON>(LoadImageW(GetModuleHandleW(nullptr),
                                             MAKEINTRESOURCEW(THROWBACK_XP_ICONS + id), IMAGE_ICON,
                                             size, size, LR_DEFAULTCOLOR));
  xpIcons_[key] = icon;
  return icon;
}

}  // namespace throwback
