#include "startmenu/modern_start_menu.h"

#include <windowsx.h>

#include <algorithm>
#include <array>
#include <cwctype>
#include <stdexcept>
#include <utility>

#include "core/search.h"
#include "shell/launch.h"
#include "shell/recent_items.h"
#include "startmenu/power.h"
#include "ui/glass.h"
#include "ui/strings.h"

namespace throwback {
namespace {

using Kind = ModernHit::Kind;
using Page = ModernModel::Page;

constexpr wchar_t kClassName[] = L"ThrowbackModernMenu";
constexpr UINT kIconsReady = WM_APP + 1;
constexpr ULONGLONG kCatalogMaxAgeMs = 30'000;
constexpr size_t kFrequent = 6;
constexpr size_t kRecent = 6;
constexpr size_t kMaxResults = 100;
constexpr int kWheelRows = 3;
constexpr int kWin11Lift = 12;  // Windows 11's menu floats above the bar
constexpr int kTileIcon = 48;
constexpr int kIcon = 32;
constexpr int kRowIcon = 24;

// When a document was opened, as Windows 11's Recommended list says it:
// "Today at 14:05", "Yesterday at 9:41" or the date.
std::wstring OpenedText(unsigned long long opened) {
  if (opened == 0) return {};
  FILETIME utc{static_cast<DWORD>(opened), static_cast<DWORD>(opened >> 32)};
  FILETIME local{};
  SYSTEMTIME when{};
  SYSTEMTIME now{};
  if (!FileTimeToLocalFileTime(&utc, &local) || !FileTimeToSystemTime(&local, &when)) return {};
  GetLocalTime(&now);
  std::array<wchar_t, 64> text{};
  const auto days = [](const SYSTEMTIME& t) {
    FILETIME f{};
    SystemTimeToFileTime(&t, &f);
    return ((static_cast<unsigned long long>(f.dwHighDateTime) << 32) | f.dwLowDateTime) /
           (10'000'000ULL * 60 * 60 * 24);
  };
  const unsigned long long age = days(now) - days(when);
  if (age > 1) {
    GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, &when, nullptr, text.data(),
                    static_cast<int>(text.size()), nullptr);
    return text.data();
  }
  GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, &when, nullptr, text.data(),
                  static_cast<int>(text.size()));
  std::wstring result(ui::Tr(age == 0 ? ui::StringId::ModernToday : ui::StringId::ModernYesterday));
  if (const size_t at = result.find(L"%s"); at != std::wstring::npos) {
    result.replace(at, 2, text.data());
  }
  return result;
}

bool NameLess(const Program* a, const Program* b) {
  return CompareStringEx(LOCALE_NAME_USER_DEFAULT, LINGUISTIC_IGNORECASE, a->name.c_str(), -1,
                         b->name.c_str(), -1, nullptr, nullptr, 0) == CSTR_LESS_THAN;
}

// The heading an app is listed under: its first letter, or "#".
std::wstring HeadingOf(const std::wstring& name) {
  if (name.empty() || !std::iswalpha(name[0])) return L"#";
  std::wstring letter(1, name[0]);
  CharUpperBuffW(letter.data(), 1);
  return letter;
}

}  // namespace

ModernStartMenu::ModernStartMenu(HINSTANCE instance, HWND owner, Settings& settings,
                                 std::function<void()> onChanged)
    : settings_(settings), style_(settings.startStyle), onChanged_(std::move(onChanged)) {
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = WindowProc;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.lpszClassName = kClassName;
  RegisterClassExW(&windowClass);
  CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kClassName, L"", WS_POPUP, 0, 0, 0, 0, owner,
                  nullptr, instance, this);
  if (!hwnd_) throw std::runtime_error("CreateWindowEx failed for the start menu");
  canvas_ = std::make_unique<ui::Canvas>(hwnd_);
  icons_ = std::make_unique<shell::AsyncIconCache>(hwnd_, kIconsReady);
  // Windows 10 and 11 blur what is behind their menus; 8's screen is solid.
  if (style_ == StartStyle::Win10) ui::EnableGlass(hwnd_, ui::GlassCorners::Square);
  if (style_ == StartStyle::Win11) ui::EnableGlass(hwnd_, ui::GlassCorners::Round);
}

ModernStartMenu::~ModernStartMenu() {
  icons_.reset();
  if (IsWindow(hwnd_)) DestroyWindow(hwnd_);
}

bool ModernStartMenu::IsOpen() const { return IsWindowVisible(hwnd_) != FALSE; }

ui::ModernLook ModernStartMenu::Look() const {
  switch (style_) {
    case StartStyle::Win8:
    case StartStyle::Win81:
      return ui::ModernLook::Win8;
    case StartStyle::Win10:
      return ui::ModernLook::Win10;
    default:
      break;
  }
  const bool light = settings_.theme == ThemeMode::Light ||
                     (settings_.theme == ThemeMode::System && ui::SystemUsesLightTheme());
  return light ? ui::ModernLook::Win11Light : ui::ModernLook::Win11Dark;
}

void ModernStartMenu::Open(const RECT& anchor, bool below, unsigned dpi) {
  dpi_ = dpi;
  canvas_->SetDpi(dpi);
  LoadData();
  query_.clear();
  hover_ = {};
  SetPage(Page::Home);

  MONITORINFO monitor{};
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfoW(MonitorFromRect(&anchor, MONITOR_DEFAULTTONEAREST), &monitor);
  const RECT& screen = monitor.rcMonitor;
  frame_ = ModernFrame(style_, dpi, screen);
  int x = static_cast<int>(screen.left);
  int y = static_cast<int>(screen.top);
  if (style_ != StartStyle::Win8 && style_ != StartStyle::Win81) {
    const int lift = style_ == StartStyle::Win11 ? ScaleForDpi(kWin11Lift, dpi) : 0;
    x = std::clamp(static_cast<int>(anchor.left), static_cast<int>(screen.left),
                   static_cast<int>(screen.right) - frame_.Width());
    y = below ? static_cast<int>(anchor.bottom) + lift
              : static_cast<int>(anchor.top) - frame_.Height() - lift;
  }
  SetWindowPos(hwnd_, HWND_TOPMOST, x, y, frame_.Width(), frame_.Height(), SWP_SHOWWINDOW);
  canvas_->Resize(static_cast<UINT>(frame_.Width()), static_cast<UINT>(frame_.Height()));
  previousForeground_ = GetForegroundWindow();
  shell::BringToFront(hwnd_);
  SetFocus(hwnd_);
  InvalidateRect(hwnd_, nullptr, FALSE);
  if (onChanged_) onChanged_();
}

void ModernStartMenu::Hide(bool restoreFocus) {
  if (!IsOpen()) return;
  if (restoreFocus && IsWindow(previousForeground_)) SetForegroundWindow(previousForeground_);
  ShowWindow(hwnd_, SW_HIDE);
  previousForeground_ = nullptr;
  if (onChanged_) onChanged_();
}

void ModernStartMenu::LoadData() {
  const ULONGLONG now = GetTickCount64();
  if (!catalog_ || now - catalogLoadedAt_ > kCatalogMaxAgeMs) {
    catalog_ = LoadPrograms();
    catalogLoadedAt_ = now;
  }
  account_ = LoadAccount();
  frequent_.clear();
  for (const auto& program : LoadFrequentPrograms(*catalog_, kFrequent)) {
    frequent_.push_back({program.name, program.target});
  }
  // Pinned programs become the tiles; without pins, the most used ones.
  tiles_.clear();
  for (const auto& target : settings_.pinned) {
    const auto it = std::ranges::find_if(
        catalog_->all, [&target](const Program* program) { return program->target == target; });
    if (it != catalog_->all.end()) tiles_.push_back({(*it)->name, target});
  }
  if (tiles_.empty()) tiles_ = frequent_;
  recent_.clear();
  for (auto& item : shell::LoadRecentDocuments(kRecent)) {
    recent_.push_back({std::move(item.name), std::move(item.target), OpenedText(item.opened)});
  }
}

void ModernStartMenu::BuildApps() {
  apps_.clear();
  scroll_ = 0;
  if (page_ == Page::Search) {
    std::vector<std::pair<int, const Program*>> matches;
    for (const Program* program : catalog_->all) {
      if (const int score = MatchScore(program->name, query_)) matches.push_back({score, program});
    }
    std::stable_sort(matches.begin(), matches.end(),
                     [](const auto& a, const auto& b) { return a.first > b.first; });
    for (const auto& [score, program] : matches) {
      if (apps_.size() == kMaxResults) break;
      apps_.push_back({program->name, program->target});
    }
    return;
  }
  // Every app in name order under its letter, as Windows 8 to 11 list them.
  std::vector<const Program*> all = catalog_->all;
  std::ranges::sort(all, NameLess);
  std::wstring heading;
  for (const Program* program : all) {
    if (std::wstring next = HeadingOf(program->name); next != heading) {
      heading = next;
      apps_.push_back({heading, {}, {}, true});
    }
    apps_.push_back({program->name, program->target});
  }
}

void ModernStartMenu::SetPage(Page page) {
  page_ = page;
  BuildApps();
  hover_ = {};
  InvalidateRect(hwnd_, nullptr, FALSE);
}

ModernModel ModernStartMenu::Model() {
  ModernModel model;
  model.style = style_;
  model.look = Look();
  model.frame = frame_;
  model.page = page_;
  model.scroll = scroll_;
  model.query = query_;
  model.accountName = account_.name;
  model.picturePath = account_.picturePath;
  model.hover = hover_;
  const bool win8 = model.look == ui::ModernLook::Win8;
  const int tileIcon = ScaleForDpi(win8 ? kTileIcon : kIcon, dpi_);
  const int rowIcon = ScaleForDpi(win8 ? kIcon : kRowIcon, dpi_);
  const auto add = [this](std::vector<ModernModel::Item>& out, const std::vector<Entry>& in,
                          int size) {
    for (const Entry& entry : in) {
      out.push_back({entry.name, entry.caption,
                     entry.header ? nullptr : icons_->Get(entry.target, size), entry.header});
    }
  };
  add(model.tiles, tiles_, tileIcon);
  add(model.frequent, frequent_, rowIcon);
  // Icons load in the background; only ask for the rows on screen, so the
  // visible ones don't wait behind hundreds of others.
  const bool listShown = style_ == StartStyle::Win10 || page_ != Page::Home;
  const int first = scroll_;
  const int last = listShown ? first + ModernPageRows(model, dpi_) + 1 : first;
  for (int i = 0; i < static_cast<int>(apps_.size()); ++i) {
    const Entry& entry = apps_[static_cast<size_t>(i)];
    const bool visible = i >= first && i < last && !entry.header;
    model.apps.push_back({entry.name, entry.caption,
                          visible ? icons_->Get(entry.target, rowIcon) : nullptr, entry.header});
  }
  add(model.recent, recent_, ScaleForDpi(kIcon, dpi_));
  return model;
}

void ModernStartMenu::Paint() {
  PAINTSTRUCT paint;
  BeginPaint(hwnd_, &paint);
  canvas_->BeginFrame();
  regions_ = DrawModernStartMenu(*canvas_, Model());
  canvas_->EndFrame();
  EndPaint(hwnd_, &paint);
}

ModernHit ModernStartMenu::HitAt(POINT point) const {
  for (const auto& [rect, hit] : regions_) {
    if (rect.Contains(point.x, point.y)) return hit;
  }
  return {};
}

void ModernStartMenu::OnMouseMove(POINT point) {
  if (!trackingMouse_) {
    TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, hwnd_, 0};
    trackingMouse_ = TrackMouseEvent(&track) != FALSE;
  }
  const ModernHit hit = HitAt(point);
  if (hit == hover_) return;
  hover_ = hit;
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void ModernStartMenu::Launch(const std::wstring& target) {
  if (target.empty()) return;
  Hide(/*restoreFocus=*/false);
  shell::Launch(target);
}

void ModernStartMenu::OnClick(POINT point) {
  const ModernHit hit = HitAt(point);
  const auto entry = [&hit](const std::vector<Entry>& entries) {
    return hit.index >= 0 && hit.index < static_cast<int>(entries.size())
               ? entries[static_cast<size_t>(hit.index)].target
               : std::wstring();
  };
  switch (hit.kind) {
    case Kind::Tile:
      Launch(entry(tiles_));
      return;
    case Kind::Frequent:
      Launch(entry(frequent_));
      return;
    case Kind::App:
      Launch(entry(apps_));
      return;
    case Kind::Recent:
      Launch(entry(recent_));
      return;
    case Kind::AllApps:
      SetPage(Page::AllApps);
      return;
    case Kind::Back:
      query_.clear();
      SetPage(Page::Home);
      return;
    case Kind::Power:
    case Kind::User: {
      POINT screen = point;
      ClientToScreen(hwnd_, &screen);
      ShowMenu(hit.kind == Kind::Power, screen);
      return;
    }
    case Kind::Documents:
      Launch(L"shell:Personal");
      return;
    case Kind::Pictures:
      Launch(L"shell:My Pictures");
      return;
    case Kind::Settings:
      Launch(L"ms-settings:");
      return;
    case Kind::None:
      return;
  }
}

void ModernStartMenu::ShowMenu(bool power, POINT point) {
  using ui::StringId;
  struct Item {
    StringId label;
    PowerAction action;
  };
  static constexpr Item kPower[] = {{StringId::Sleep, PowerAction::Sleep},
                                    {StringId::ShutDown, PowerAction::ShutDown},
                                    {StringId::Restart, PowerAction::Restart}};
  static constexpr Item kUser[] = {{StringId::Lock, PowerAction::Lock},
                                   {StringId::SignOut, PowerAction::SignOut}};
  const Item* items = power ? kPower : kUser;
  const UINT count = power ? 3 : 2;
  HMENU menu = CreatePopupMenu();
  for (UINT i = 0; i < count; ++i) {
    AppendMenuW(menu, MF_STRING, i + 1, std::wstring(ui::Tr(items[i].label)).c_str());
  }
  keepOpen_ = true;
  const auto command =
      static_cast<UINT>(TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN,
                                       point.x, point.y, 0, hwnd_, nullptr));
  keepOpen_ = false;
  DestroyMenu(menu);
  if (command == 0) return;
  Hide(/*restoreFocus=*/false);
  RunPowerAction(items[command - 1].action);
}

void ModernStartMenu::OnWheel(int delta) {
  const ModernModel model = Model();
  const int maxScroll = std::max(0, static_cast<int>(apps_.size()) - ModernPageRows(model, dpi_));
  scroll_ = std::clamp(scroll_ - delta / WHEEL_DELTA * kWheelRows, 0, maxScroll);
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void ModernStartMenu::OnChar(wchar_t c) {
  // Typing searches in Windows 10 and 11; the start screens had no box.
  if (style_ != StartStyle::Win10 && style_ != StartStyle::Win11) return;
  if (c == L'\b') {
    if (query_.empty()) return;
    query_.pop_back();
  } else if (c >= L' ' && c != 0x7F) {
    query_.push_back(c);
  } else {
    return;
  }
  SetPage(query_.empty() ? Page::Home : Page::Search);
}

void ModernStartMenu::OnKey(UINT key) {
  if (key == VK_ESCAPE) {
    if (!query_.empty() || page_ != Page::Home) {
      query_.clear();
      SetPage(Page::Home);
    } else {
      Close();
    }
  } else if (key == VK_RETURN && page_ == Page::Search) {
    const auto first = std::ranges::find_if(apps_, [](const Entry& e) { return !e.header; });
    if (first != apps_.end()) Launch(first->target);
  }
}

LRESULT ModernStartMenu::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  const POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
  switch (message) {
    case WM_PAINT:
      Paint();
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case kIconsReady:
      if (IsOpen()) InvalidateRect(hwnd_, nullptr, FALSE);
      return 0;
    case WM_ACTIVATE:
      if (LOWORD(wParam) == WA_INACTIVE && !keepOpen_) Hide(/*restoreFocus=*/false);
      return 0;
    case WM_MOUSEMOVE:
      OnMouseMove(point);
      return 0;
    case WM_MOUSELEAVE:
      trackingMouse_ = false;
      hover_ = {};
      InvalidateRect(hwnd_, nullptr, FALSE);
      return 0;
    case WM_LBUTTONUP:
      OnClick(point);
      return 0;
    case WM_RBUTTONUP:
      // Windows 8 showed all apps from the start screen's app bar.
      if (style_ == StartStyle::Win8) SetPage(page_ == Page::Home ? Page::AllApps : Page::Home);
      return 0;
    case WM_MOUSEWHEEL:
      OnWheel(GET_WHEEL_DELTA_WPARAM(wParam));
      return 0;
    case WM_CHAR:
      OnChar(static_cast<wchar_t>(wParam));
      return 0;
    case WM_KEYDOWN:
      OnKey(static_cast<UINT>(wParam));
      return 0;
    case WM_CLOSE:
      if (HWND owner = GetWindow(hwnd_, GW_OWNER)) PostMessageW(owner, WM_CLOSE, 0, 0);
      return 0;
  }
  return DefWindowProcW(hwnd_, message, wParam, lParam);
}

LRESULT CALLBACK ModernStartMenu::WindowProc(HWND hwnd, UINT message, WPARAM wParam,
                                             LPARAM lParam) {
  if (message == WM_NCCREATE) {
    auto* self =
        static_cast<ModernStartMenu*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
    self->hwnd_ = hwnd;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
  }
  auto* self = reinterpret_cast<ModernStartMenu*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return DefWindowProcW(hwnd, message, wParam, lParam);
  return self->HandleMessage(message, wParam, lParam);
}

}  // namespace throwback
