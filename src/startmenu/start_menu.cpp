#include "startmenu/start_menu.h"

#include <windowsx.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "core/search.h"
#include "shell/launch.h"
#include "startmenu/power.h"
#include "startmenu/settings_pages.h"
#include "startmenu/start_menu_view.h"
#include "ui/glass.h"
#include "ui/strings.h"

namespace throwback {
namespace {

constexpr wchar_t kClassName[] = L"ThrowbackStartMenu";
constexpr UINT kIconsReady = WM_APP + 1;
// The program list is read again when the menu opens after this long, so
// newly installed programs show up.
constexpr ULONGLONG kCatalogMaxAgeMs = 30'000;
constexpr int kCompactRowHeight = ui::kSpace24;
constexpr int kLargeIcon = 32;
constexpr int kSmallIcon = 16;
constexpr size_t kMaxSearchResults = 200;
constexpr size_t kMaxSettingsResults = 8;
constexpr int kWheelRows = 3;

bool KeyDown(int key) { return (GetKeyState(key) & 0x8000) != 0; }

}  // namespace

StartMenu::StartMenu(HINSTANCE instance, HWND owner, Settings& settings,
                     std::function<void()> onClosed, std::function<void()> onPinnedChanged)
    : settings_(settings),
      onClosed_(std::move(onClosed)),
      onPinnedChanged_(std::move(onPinnedChanged)) {
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = WindowProc;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.lpszClassName = kClassName;
  RegisterClassExW(&windowClass);

  hwnd_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kClassName, L"", WS_POPUP, 0, 0, 0, 0,
                          owner, nullptr, instance, this);
  if (!hwnd_) throw std::runtime_error("CreateWindowEx failed for the start menu");
  canvas_ = std::make_unique<ui::Canvas>(hwnd_);
  icons_ = std::make_unique<shell::AsyncIconCache>(hwnd_, kIconsReady);
  ui::EnableGlass(hwnd_, ui::GlassCorners::Round);
}

StartMenu::~StartMenu() {
  if (IsWindow(hwnd_)) DestroyWindow(hwnd_);
}

LRESULT CALLBACK StartMenu::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_NCCREATE) {
    auto* created =
        static_cast<StartMenu*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
    created->hwnd_ = hwnd;  // HandleMessage needs it before CreateWindowEx returns
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(created));
  }
  auto* self = reinterpret_cast<StartMenu*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return DefWindowProcW(hwnd, message, wParam, lParam);
  return self->HandleMessage(message, wParam, lParam);
}

LRESULT StartMenu::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
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
      // Clicking anywhere else closes the menu, like the real one.
      if (LOWORD(wParam) == WA_INACTIVE && !keepOpen_) Hide(/*restoreFocus=*/false);
      return 0;
    case WM_MOUSEMOVE:
      OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return 0;
    case WM_MOUSELEAVE:
      trackingMouse_ = false;
      hover_ = {};
      if (mode_ != Mode::Search) selected_ = -1;
      InvalidateRect(hwnd_, nullptr, FALSE);
      return 0;
    case WM_LBUTTONUP:
      OnClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return 0;
    case WM_RBUTTONUP: {
      const Hit hit = HitTest(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      ClientToScreen(hwnd_, &point);
      if (hit.kind == Hit::Kind::Row) ShowRowMenu(hit.index, point);
      return 0;
    }
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
      // A polite close (taskkill, installers) is meant for the whole app.
      if (HWND owner = GetWindow(hwnd_, GW_OWNER)) PostMessageW(owner, WM_CLOSE, 0, 0);
      return 0;
  }
  return DefWindowProcW(hwnd_, message, wParam, lParam);
}

void StartMenu::Open(const RECT& anchor, bool below, unsigned dpi) {
  folders_.clear();  // may point into the catalog that LoadData replaces
  dpi_ = dpi;
  canvas_->SetDpi(dpi);
  layout_ = ComputeStartMenuLayout(
      dpi, canvas_->MeasureText(ui::Tr(ui::StringId::ShutDown), ui::TextStyle::Body));
  LoadData();
  query_.clear();
  hover_ = {};
  SetMode(Mode::Home);
  const int width = layout_.frame.Width();
  const int height = layout_.frame.Height();
  MONITORINFO monitor{};
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfoW(MonitorFromRect(&anchor, MONITOR_DEFAULTTONEAREST), &monitor);
  const RECT& screen = monitor.rcMonitor;
  const int x = std::clamp(static_cast<int>(anchor.left), static_cast<int>(screen.left),
                           static_cast<int>(screen.right) - width);
  const int y =
      std::clamp(below ? static_cast<int>(anchor.bottom) : static_cast<int>(anchor.top) - height,
                 static_cast<int>(screen.top), static_cast<int>(screen.bottom) - height);
  SetWindowPos(hwnd_, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW);
  canvas_->Resize(static_cast<UINT>(width), static_cast<UINT>(height));
  previousForeground_ = GetForegroundWindow();
  shell::BringToFront(hwnd_);
  SetFocus(hwnd_);
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void StartMenu::Hide(bool restoreFocus) {
  if (!IsOpen()) return;
  // Hand focus back before hiding; otherwise Windows activates our owner,
  // the bar, and the user's window loses the keyboard.
  if (restoreFocus && IsWindow(previousForeground_)) SetForegroundWindow(previousForeground_);
  ShowWindow(hwnd_, SW_HIDE);
  previousForeground_ = nullptr;
  if (onClosed_) onClosed_();
}

bool StartMenu::IsOpen() const { return IsWindowVisible(hwnd_) != FALSE; }

void StartMenu::LoadData() {
  const ULONGLONG now = GetTickCount64();
  if (!catalog_ || now - catalogLoadedAt_ > kCatalogMaxAgeMs) {
    catalog_ = LoadPrograms();
    catalogLoadedAt_ = now;
  }
  frequent_ = LoadFrequentPrograms(*catalog_, static_cast<size_t>(layout_.listRows));
  if (places_.empty()) {
    places_ = LoadPlaces();
    account_ = LoadAccount();
  }
}

void StartMenu::SetMode(Mode mode) {
  ResetJumpList();
  mode_ = mode;
  if (mode == Mode::Home) {
    folders_.clear();
    query_.clear();
  }
  RebuildRows();
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void StartMenu::RebuildRows() {
  rows_.clear();
  switch (mode_) {
    case Mode::Home:
      AddHomeRows();
      break;
    case Mode::AllPrograms: {
      const Program& folder = folders_.empty() ? catalog_->root : *folders_.back();
      for (const auto& child : folder.children) {
        rows_.push_back({child->name,
                         child->target,
                         child->folder ? child.get() : nullptr,
                         {},
                         false,
                         !child->folder});
      }
      break;
    }
    case Mode::Search: {
      std::vector<std::pair<int, const Program*>> matches;
      for (const Program* program : catalog_->all) {
        if (const int score = MatchScore(program->name, query_))
          matches.push_back({score, program});
      }
      // The catalog is already alphabetical, so a stable sort keeps names in order.
      std::stable_sort(matches.begin(), matches.end(),
                       [](const auto& a, const auto& b) { return a.first > b.first; });
      for (const auto& [score, program] : matches) {
        if (rows_.size() == kMaxSearchResults) break;
        rows_.push_back({program->name, program->target, nullptr, {}, false, true});
      }
      AddSettingsResults();
      break;
    }
  }
  scroll_ = 0;
  selected_ = mode_ == Mode::Search && !rows_.empty() ? 0 : -1;
}

void StartMenu::AddSettingsResults() {
  // Settings pages follow the programs, like the Control Panel group of the
  // Windows 7 search. Keywords only count when a query word starts one.
  std::vector<std::pair<int, const SettingsPage*>> matches;
  for (const SettingsPage& page : SettingsPages()) {
    const int byName = MatchScore(ui::Tr(page.name), query_);
    const int byKeyword = MatchScore(page.keywords, query_) >= 2 ? 1 : 0;
    if (const int score = std::max(byName, byKeyword)) matches.push_back({score, &page});
  }
  std::stable_sort(matches.begin(), matches.end(),
                   [](const auto& a, const auto& b) { return a.first > b.first; });
  for (size_t i = 0; i < matches.size() && i < kMaxSettingsResults; ++i) {
    const SettingsPage& page = *matches[i].second;
    rows_.push_back({std::wstring(ui::Tr(page.name)), std::wstring(page.uri), nullptr,
                     std::wstring(kSettingsApp)});
  }
}

int StartMenu::RowHeight() const {
  return mode_ == Mode::Home ? layout_.rowHeight : ScaleForDpi(kCompactRowHeight, dpi_);
}

int StartMenu::IconSize() const {
  return ScaleForDpi(mode_ == Mode::Home ? kLargeIcon : kSmallIcon, dpi_);
}

int StartMenu::VisibleRows() const { return layout_.list.Height() / RowHeight(); }

void StartMenu::EnsureVisible(int row) {
  if (row < scroll_) scroll_ = row;
  if (row >= scroll_ + VisibleRows()) scroll_ = row - VisibleRows() + 1;
  scroll_ = std::max(scroll_, 0);
}

std::vector<Rect> StartMenu::PlaceRowRects() const {
  std::vector<bool> separators;
  for (const auto& place : places_) separators.push_back(place.separatorBefore);
  return PlaceRects(layout_, separators, ScaleForDpi(ui::kSpace8, dpi_));
}

StartMenu::Hit StartMenu::HitTest(int x, int y) const {
  using Kind = Hit::Kind;
  if (layout_.list.Contains(x, y)) {
    const int row = (y - layout_.list.top) / RowHeight();
    const int index = scroll_ + row;
    if (row < VisibleRows() && index < static_cast<int>(rows_.size())) return {Kind::Row, index};
    return {};
  }
  if (layout_.allPrograms.Contains(x, y)) return {Kind::AllPrograms};
  if (layout_.power.Contains(x, y)) return {Kind::Power};
  if (layout_.powerArrow.Contains(x, y)) return {Kind::PowerArrow};
  if (jumpRow_ >= 0) {
    // The open jump list covers the places.
    const std::vector<Rect> items = JumpItemRects(layout_, jumpItems_.size());
    for (size_t i = 0; i < items.size(); ++i) {
      if (items[i].Contains(x, y)) return {Kind::JumpItem, static_cast<int>(i)};
    }
    return {};
  }
  const std::vector<Rect> places = PlaceRowRects();
  for (size_t i = 0; i < places.size(); ++i) {
    if (places[i].Contains(x, y)) return {Kind::Place, static_cast<int>(i)};
  }
  return {};
}

void StartMenu::Paint() {
  PAINTSTRUCT paint;
  BeginPaint(hwnd_, &paint);

  StartMenuModel model;
  model.layout = layout_;
  model.rowHeight = RowHeight();
  model.iconSize = IconSize();
  const int last = std::min(static_cast<int>(rows_.size()), scroll_ + VisibleRows());
  for (int i = scroll_; i < last; ++i) {
    const Row& row = rows_[static_cast<size_t>(i)];
    model.rows.push_back(
        {row.name,
         icons_->Get(row.iconTarget.empty() ? row.target : row.iconTarget, model.iconSize),
         row.folder != nullptr, i == selected_ || i == jumpRow_, row.separatorBefore,
         !row.appId.empty()});
  }
  model.showBack = mode_ != Mode::Home;
  model.allProgramsHovered = hover_.kind == Hit::Kind::AllPrograms;
  model.query = query_;
  model.noResults = mode_ == Mode::Search && rows_.empty();
  model.accountName = account_.name;
  model.picturePath = account_.picturePath;
  for (size_t i = 0; i < places_.size(); ++i) {
    model.places.push_back(
        {places_[i].name, places_[i].separatorBefore,
         hover_.kind == Hit::Kind::Place && hover_.index == static_cast<int>(i)});
  }
  model.powerHovered = hover_.kind == Hit::Kind::Power;
  model.powerArrowHovered = hover_.kind == Hit::Kind::PowerArrow;
  model.jumpListOpen = jumpRow_ >= 0;
  for (size_t i = 0; i < jumpItems_.size(); ++i) {
    model.jumpRows.push_back(
        {jumpItems_[i].name, icons_->Get(jumpItems_[i].target, ScaleForDpi(kSmallIcon, dpi_)),
         hover_.kind == Hit::Kind::JumpItem && hover_.index == static_cast<int>(i)});
  }

  canvas_->BeginFrame();
  DrawStartMenu(*canvas_, model);
  canvas_->EndFrame();
  EndPaint(hwnd_, &paint);
}

void StartMenu::OnMouseMove(int x, int y) {
  if (!trackingMouse_) {
    TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, hwnd_, 0};
    trackingMouse_ = TrackMouseEvent(&track) != FALSE;
  }
  // Reaching the open jump list keeps it, even after crossing another row
  // on the way there.
  if (jumpRow_ >= 0 && layout_.jumpList.Contains(x, y)) {
    HoverJumpList(jumpRow_);
    selected_ = jumpRow_;
  }
  const Hit hit = HitTest(x, y);
  if (hit.kind == hover_.kind && hit.index == hover_.index) return;
  hover_ = hit;
  // Like the classic menu, pointing at a program selects it.
  if (hit.kind == Hit::Kind::Row) selected_ = hit.index;
  if (mode_ == Mode::Home && hit.kind == Hit::Kind::Row) HoverJumpList(hit.index);
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void StartMenu::OnClick(int x, int y) {
  const Hit hit = HitTest(x, y);
  switch (hit.kind) {
    case Hit::Kind::Row:
      if (mode_ == Mode::Home && OnJumpArrow(hit.index, x)) {
        OpenJumpList(hit.index);
        break;
      }
      ActivateRow(hit.index, KeyDown(VK_CONTROL) && KeyDown(VK_SHIFT));
      break;
    case Hit::Kind::AllPrograms:
      if (mode_ == Mode::Home) {
        SetMode(Mode::AllPrograms);
      } else {
        GoBack();
      }
      break;
    case Hit::Kind::Place:
      Hide(/*restoreFocus=*/false);
      shell::Launch(places_[static_cast<size_t>(hit.index)].target);
      break;
    case Hit::Kind::Power:
      Hide(/*restoreFocus=*/false);
      RunPowerAction(PowerAction::ShutDown);
      break;
    case Hit::Kind::PowerArrow:
      ShowPowerMenu();
      break;
    case Hit::Kind::JumpItem: {
      const std::wstring target = jumpItems_[static_cast<size_t>(hit.index)].target;
      Hide(/*restoreFocus=*/false);
      shell::Launch(target);
      break;
    }
    case Hit::Kind::None:
      break;
  }
}

void StartMenu::OnWheel(int delta) {
  if (mode_ == Mode::Home) return;
  const int maxScroll = std::max(0, static_cast<int>(rows_.size()) - VisibleRows());
  scroll_ = std::clamp(scroll_ - delta / WHEEL_DELTA * kWheelRows, 0, maxScroll);
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void StartMenu::OnChar(wchar_t c) {
  if (c == L'\b') {
    if (query_.empty()) return;
    query_.pop_back();
  } else if (c >= L' ' && c != 0x7F) {
    query_.push_back(c);
  } else {
    return;  // Enter, Escape and Tab are handled as keys
  }
  if (query_.empty()) {
    SetMode(Mode::Home);
  } else {
    folders_.clear();
    SetMode(Mode::Search);
  }
}

void StartMenu::OnKey(UINT key) {
  const int count = static_cast<int>(rows_.size());
  switch (key) {
    case VK_ESCAPE:
      if (mode_ == Mode::Search) {
        SetMode(Mode::Home);
      } else {
        Close();
      }
      return;
    case VK_DOWN:
      if (count > 0) selected_ = std::min(selected_ + 1, count - 1);
      break;
    case VK_UP:
      if (count > 0) selected_ = std::max(selected_ - 1, 0);
      break;
    case VK_APPS:
      if (selected_ >= 0 && selected_ < count) {
        const int top = layout_.list.top + (selected_ - scroll_) * RowHeight();
        POINT point{layout_.list.left + RowHeight(), top + RowHeight()};
        ClientToScreen(hwnd_, &point);
        ShowRowMenu(selected_, point);
      }
      return;
    case VK_RETURN:
      if (count > 0) ActivateRow(std::max(selected_, 0), KeyDown(VK_CONTROL) && KeyDown(VK_SHIFT));
      return;
    case VK_RIGHT:
      if (selected_ >= 0 && selected_ < count && rows_[static_cast<size_t>(selected_)].folder) {
        ActivateRow(selected_, false);
      }
      return;
    case VK_LEFT:
      if (mode_ == Mode::AllPrograms) GoBack();
      return;
    default:
      return;
  }
  if (selected_ >= 0) EnsureVisible(selected_);
  InvalidateRect(hwnd_, nullptr, FALSE);
}

void StartMenu::ActivateRow(int index, bool elevated) {
  if (index < 0 || index >= static_cast<int>(rows_.size())) return;
  const Row row = rows_[static_cast<size_t>(index)];
  if (row.folder) {
    // Folders open in place, like the Windows 7 All Programs list.
    folders_.push_back(row.folder);
    mode_ = Mode::AllPrograms;
    RebuildRows();
    InvalidateRect(hwnd_, nullptr, FALSE);
    return;
  }
  Hide(/*restoreFocus=*/false);
  shell::Launch(row.target, elevated);
}

void StartMenu::GoBack() {
  if (mode_ == Mode::AllPrograms && !folders_.empty()) {
    folders_.pop_back();
    RebuildRows();
    InvalidateRect(hwnd_, nullptr, FALSE);
  } else {
    SetMode(Mode::Home);
  }
}

void StartMenu::ShowPowerMenu() {
  using ui::StringId;
  struct Item {
    StringId label;
    PowerAction action;
  };
  static constexpr Item kItems[] = {
      {StringId::SignOut, PowerAction::SignOut},
      {StringId::Lock, PowerAction::Lock},
      {StringId::Sleep, PowerAction::Sleep},
      {StringId::Restart, PowerAction::Restart},
  };
  HMENU menu = CreatePopupMenu();
  for (UINT i = 0; i < std::size(kItems); ++i) {
    AppendMenuW(menu, MF_STRING, i + 1, std::wstring(ui::Tr(kItems[i].label)).c_str());
  }
  POINT point{layout_.powerArrow.right, layout_.powerArrow.top};
  ClientToScreen(hwnd_, &point);
  keepOpen_ = true;
  const auto command =
      static_cast<UINT>(TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN,
                                       point.x, point.y, 0, hwnd_, nullptr));
  keepOpen_ = false;
  DestroyMenu(menu);
  if (command == 0) return;
  Hide(/*restoreFocus=*/false);
  RunPowerAction(kItems[command - 1].action);
}

}  // namespace throwback
