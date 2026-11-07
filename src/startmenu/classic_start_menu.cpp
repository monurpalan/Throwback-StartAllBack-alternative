#include "startmenu/classic_start_menu.h"

#include <algorithm>
#include <string>

#include "shell/launch.h"
#include "shell/recent_items.h"
#include "startmenu/power.h"
#include "ui/skin_ids.h"
#include "ui/strings.h"

namespace throwback {
namespace {

using ui::StringId;

// The classic shell32 icons, by their ID in that version's shell32.dll.
constexpr int kProgramsIcon = 20;
constexpr int kDocumentsIcon = 21;
constexpr int kSettingsIcon = 22;
constexpr int kFindIcon = 23;
constexpr int kHelpIcon = 24;
constexpr int kRunIcon = 25;
constexpr int kShutDownIcon = 28;
constexpr int kControlPanelIcon = 36;
constexpr int kPrintersIcon = 38;
constexpr int kTaskbarIcon = 40;
constexpr int kLogOffIcon = 45;
constexpr int kUpdateIcon = 47;

constexpr ULONGLONG kCatalogMaxAgeMs = 30'000;
constexpr size_t kRecentDocuments = 15;

std::wstring Text(StringId id) { return std::wstring(ui::Tr(id)); }

ClassicEntry Separator() {
  ClassicEntry entry;
  entry.separator = true;
  return entry;
}

ClassicEntry Launcher(StringId text, int icon, std::wstring target) {
  ClassicEntry entry;
  entry.text = Text(text);
  entry.classicIcon = icon;
  entry.action = [target = std::move(target)] { shell::Launch(target); };
  return entry;
}

bool NameLess(const Program* a, const Program* b) {
  return CompareStringEx(LOCALE_NAME_USER_DEFAULT, LINGUISTIC_IGNORECASE, a->name.c_str(), -1,
                         b->name.c_str(), -1, nullptr, nullptr, 0) == CSTR_LESS_THAN;
}

}  // namespace

std::vector<ClassicEntry> ProgramMenuEntries(const Program& folder, int folderIcon) {
  // Folders first, then programs, each in name order, as Windows 98 sorted.
  std::vector<const Program*> folders;
  std::vector<const Program*> programs;
  for (const auto& child : folder.children) {
    (child->folder ? folders : programs).push_back(child.get());
  }
  std::ranges::sort(folders, NameLess);
  std::ranges::sort(programs, NameLess);

  std::vector<ClassicEntry> entries;
  for (const Program* sub : folders) {
    ClassicEntry entry;
    entry.text = sub->name;
    entry.classicIcon = folderIcon;
    entry.submenu = [sub, folderIcon] { return ProgramMenuEntries(*sub, folderIcon); };
    entries.push_back(std::move(entry));
  }
  for (const Program* program : programs) {
    ClassicEntry entry;
    entry.text = program->name;
    entry.iconTarget = program->target;
    entry.action = [target = program->target] { shell::Launch(target); };
    entries.push_back(std::move(entry));
  }
  return entries;
}

ClassicStartMenu::ClassicStartMenu(HINSTANCE instance, HWND owner, StartStyle style,
                                   std::function<void()> onChanged)
    : owner_(owner), style_(style), onChanged_(std::move(onChanged)) {
  host_.look = style == StartStyle::Win2000 ? ui::ClassicLook::Win2000 : ui::ClassicLook::Win98;
  host_.banner = style == StartStyle::Win98   ? ui::SkinImage::Banner98
                 : style == StartStyle::WinMe ? ui::SkinImage::BannerMe
                                              : ui::SkinImage::Banner2000;
  host_.icon = [this](const ClassicEntry& entry, int size) { return Icon(entry, size); };
  host_.invoke = [this](std::function<void()> action) {
    Close();
    if (action) action();
  };
  host_.dismiss = [this] { Close(); };
  root_ = std::make_unique<ClassicMenu>(instance, owner, host_, /*root=*/true);
  icons_ = std::make_unique<shell::AsyncIconCache>(root_->Hwnd(), ClassicMenu::kIconsReady);
}

ClassicStartMenu::~ClassicStartMenu() {
  icons_.reset();
  root_.reset();
  for (const auto& [key, icon] : classicIcons_) DestroyIcon(icon);
}

void ClassicStartMenu::Open(const RECT& anchor, bool below, unsigned dpi) {
  const ULONGLONG now = GetTickCount64();
  if (!catalog_ || now - catalogLoadedAt_ > kCatalogMaxAgeMs) {
    catalog_ = LoadPrograms();
    catalogLoadedAt_ = now;
  }
  account_ = LoadAccount();
  host_.dpi = dpi;
  root_->ShowRoot(RootEntries(), anchor, below);
  if (onChanged_) onChanged_();
}

void ClassicStartMenu::Close() {
  if (!root_->IsOpen()) return;
  root_->Hide();
  if (onChanged_) onChanged_();
}

bool ClassicStartMenu::IsOpen() const { return root_->IsOpen(); }

std::vector<ClassicEntry> ClassicStartMenu::RootEntries() {
  std::vector<ClassicEntry> entries;
  entries.push_back(
      Launcher(StringId::SettingsWindowsUpdate, kUpdateIcon, L"ms-settings:windowsupdate"));
  entries.push_back(Separator());

  ClassicEntry programs;
  programs.text = Text(StringId::ClassicPrograms);
  programs.classicIcon = kProgramsIcon;
  programs.submenu = [this] { return ProgramMenuEntries(catalog_->root, kProgramsIcon); };
  entries.push_back(std::move(programs));

  ClassicEntry documents;
  documents.text = Text(StringId::ClassicDocuments);
  documents.classicIcon = kDocumentsIcon;
  documents.submenu = [this] { return DocumentEntries(); };
  entries.push_back(std::move(documents));

  ClassicEntry settings;
  settings.text = Text(StringId::ClassicSettings);
  settings.classicIcon = kSettingsIcon;
  settings.submenu = [this] { return SettingsEntries(); };
  entries.push_back(std::move(settings));

  // Windows 98 finds; Me and 2000 search.
  ClassicEntry find;
  find.text = Text(style_ == StartStyle::Win98 ? StringId::ClassicFind : StringId::ClassicSearch);
  find.classicIcon = kFindIcon;
  find.submenu = [this] { return FindEntries(); };
  entries.push_back(std::move(find));

  entries.push_back(Launcher(StringId::ClassicHelp, kHelpIcon, L"ms-contact-support:"));
  entries.push_back(
      Launcher(StringId::Run, kRunIcon, L"shell:::{2559A1F3-21D7-11D4-BDAF-00C04F60B9F0}"));
  entries.push_back(Separator());

  // Windows 2000 logs off from its Shut Down dialog.
  if (style_ != StartStyle::Win2000) {
    ClassicEntry logOff;
    logOff.text = Text(StringId::ClassicLogOff);
    if (const size_t at = logOff.text.find(L"%s"); at != std::wstring::npos) {
      logOff.text.replace(at, 2, account_.name);
    }
    logOff.classicIcon = kLogOffIcon;
    logOff.action = [] { RunPowerAction(PowerAction::SignOut); };
    entries.push_back(std::move(logOff));
  }
  ClassicEntry shutDown;
  shutDown.text = Text(StringId::ClassicShutDown);
  shutDown.classicIcon = kShutDownIcon;
  shutDown.action = [] { ShowShutDownDialog(); };
  entries.push_back(std::move(shutDown));
  return entries;
}

std::vector<ClassicEntry> ClassicStartMenu::DocumentEntries() {
  std::vector<ClassicEntry> entries;
  for (auto& document : shell::LoadRecentDocuments(kRecentDocuments)) {
    ClassicEntry entry;
    entry.text = std::move(document.name);
    entry.iconTarget = document.target;
    entry.action = [target = std::move(document.target)] { shell::Launch(target); };
    entries.push_back(std::move(entry));
  }
  return entries;
}

std::vector<ClassicEntry> ClassicStartMenu::SettingsEntries() {
  std::vector<ClassicEntry> entries;
  entries.push_back(Launcher(StringId::ClassicControlPanel, kControlPanelIcon,
                             L"shell:::{26EE0668-A00A-44D7-9371-BEB064C98683}"));
  entries.push_back(Launcher(StringId::ClassicPrinters, kPrintersIcon, L"ms-settings:printers"));
  ClassicEntry taskbar;
  taskbar.text = Text(StringId::ClassicTaskbar);
  taskbar.classicIcon = kTaskbarIcon;
  taskbar.action = [owner = owner_] { PostMessageW(owner, kOpenSettingsMessage, 0, 0); };
  entries.push_back(std::move(taskbar));
  return entries;
}

std::vector<ClassicEntry> ClassicStartMenu::FindEntries() {
  std::vector<ClassicEntry> entries;
  entries.push_back(Launcher(StringId::ClassicFilesOrFolders, kFindIcon, L"search-ms:"));
  entries.push_back(Launcher(StringId::ClassicOnTheInternet, kFindIcon, L"https://www.bing.com/"));
  return entries;
}

HICON ClassicStartMenu::Icon(const ClassicEntry& entry, int size) {
  if (entry.classicIcon == 0) {
    return entry.iconTarget.empty() ? nullptr : icons_->Get(entry.iconTarget, size);
  }
  const auto key = std::make_pair(entry.classicIcon, size);
  if (const auto it = classicIcons_.find(key); it != classicIcons_.end()) return it->second;
  const int base =
      style_ == StartStyle::Win2000 ? THROWBACK_CLASSIC_ICONS_2000 : THROWBACK_CLASSIC_ICONS_98;
  auto* icon = static_cast<HICON>(LoadImageW(GetModuleHandleW(nullptr),
                                             MAKEINTRESOURCEW(base + entry.classicIcon), IMAGE_ICON,
                                             size, size, LR_DEFAULTCOLOR));
  classicIcons_[key] = icon;
  return icon;
}

}  // namespace throwback
