#include "taskbar/tray_host.h"

#include <shlwapi.h>

#include <array>
#include <cstring>
#include <set>
#include <span>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "taskbar/explorer_tray.h"

namespace throwback {
namespace {

constexpr wchar_t kTrayClass[] = L"Shell_TrayWnd";
constexpr wchar_t kNotifyClass[] = L"TrayNotifyWnd";
constexpr ULONG_PTR kNotifyIconData = 1;
constexpr std::uint32_t kAddCommand = 0;
constexpr std::uint32_t kModifyCommand = 1;
constexpr std::uint32_t kFlagIcon = 0x2;
constexpr std::uint32_t kFlagGuid = 0x20;
constexpr UINT kSelect = WM_USER;  // NIN_SELECT
constexpr UINT kForwardTimeoutMs = 2000;

HWND ToWindow(std::uint32_t handle) {
  return reinterpret_cast<HWND>(static_cast<ULONG_PTR>(handle));
}

std::wstring KeyOf(std::uint32_t window, std::uint32_t id, const std::array<std::uint8_t, 16>& guid,
                   bool useGuid) {
  if (useGuid) {
    std::wstring key = L"g";
    for (std::uint8_t b : guid) key += std::to_wstring(b) + L".";
    return key;
  }
  return std::to_wstring(window) + L":" + std::to_wstring(id);
}

bool HasGuid(const std::array<std::uint8_t, 16>& guid) {
  for (std::uint8_t b : guid) {
    if (b != 0) return true;
  }
  return false;
}

std::wstring KeyOf(const TrayIcon& icon) {
  return KeyOf(icon.window, icon.id, icon.guid, HasGuid(icon.guid));
}

// Asks Explorer's taskbar the same thing, without hanging if it is busy.
LRESULT Forward(UINT message, WPARAM wParam, LPARAM lParam) {
  HWND explorer = ExplorerTray();
  if (!explorer) return 0;
  DWORD_PTR result = 0;
  SendMessageTimeoutW(explorer, message, wParam, lParam, SMTO_ABORTIFHUNG, kForwardTimeoutMs,
                      &result);
  return static_cast<LRESULT>(result);
}

// SHAppBarMessage sends a 64-byte block: the APPBARDATA (with 32-bit
// handles), the command, and shared memory for the reply.
constexpr ULONG_PTR kAppBarData = 0;
constexpr DWORD kAppBarBlockSize = 64;
constexpr size_t kCommandOffset = 40;
constexpr size_t kSharedOffset = 48;
constexpr size_t kProcessOffset = 56;
constexpr DWORD kReplySize = 40;  // the APPBARDATA the reply is written to
constexpr std::uint32_t kQueryPos = 2;
constexpr std::uint32_t kSetPos = 3;
constexpr std::uint32_t kGetTaskbarPos = 5;

template <typename T>
T Read(const std::byte* block, size_t offset) {
  T value{};
  std::memcpy(&value, block + offset, sizeof(value));
  return value;
}

// Requests that reply with a rect carry shared memory meant for the
// Shell_TrayWnd's process, which is now ours. Explorer gets a copy of its
// own and the reply is copied back for the caller.
LRESULT ForwardAppBar(WPARAM sender, const COPYDATASTRUCT& data) {
  std::array<std::byte, kAppBarBlockSize> block{};
  std::memcpy(block.data(), data.lpData, kAppBarBlockSize);
  const auto command = Read<std::uint32_t>(block.data(), kCommandOffset);
  const auto shared = Read<std::int64_t>(block.data(), kSharedOffset);
  const auto source = Read<std::uint32_t>(block.data(), kProcessOffset);
  const bool replies = command == kQueryPos || command == kSetPos || command == kGetTaskbarPos;
  HWND explorer = ExplorerTray();
  if (!replies || shared == 0 || !explorer) {
    return Forward(WM_COPYDATA, sender, reinterpret_cast<LPARAM>(&data));
  }

  DWORD explorerProcess = 0;
  GetWindowThreadProcessId(explorer, &explorerProcess);
  const auto original = reinterpret_cast<HANDLE>(static_cast<INT_PTR>(shared));
  void* reply = SHLockShared(original, source);
  if (!reply) return Forward(WM_COPYDATA, sender, reinterpret_cast<LPARAM>(&data));
  HANDLE forExplorer = SHAllocShared(reply, kReplySize, explorerProcess);

  const auto handle = static_cast<std::int64_t>(reinterpret_cast<INT_PTR>(forExplorer));
  const auto process = static_cast<std::uint32_t>(explorerProcess);
  std::memcpy(block.data() + kSharedOffset, &handle, sizeof(handle));
  std::memcpy(block.data() + kProcessOffset, &process, sizeof(process));
  COPYDATASTRUCT forwarded{data.dwData, kAppBarBlockSize, block.data()};
  const LRESULT result = Forward(WM_COPYDATA, sender, reinterpret_cast<LPARAM>(&forwarded));

  if (void* answer = SHLockShared(forExplorer, explorerProcess)) {
    std::memcpy(reply, answer, kReplySize);
    SHUnlockShared(answer);
  }
  SHUnlockShared(reply);
  SHFreeShared(forExplorer, explorerProcess);
  return result;
}

// The shell itself rebuilds its taskbar when one is created (and moves windows
// around while it does); its UI hosts redraw too. None of them needs to add
// icons to us, so they are left out of the announcement.
bool IsShellHost(DWORD process) {
  constexpr std::array<std::wstring_view, 5> kHosts{
      L"explorer.exe", L"SearchHost.exe", L"StartMenuExperienceHost.exe",
      L"ShellExperienceHost.exe", L"TextInputHost.exe"};
  HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process);
  if (!handle) return false;
  wchar_t path[MAX_PATH] = {};
  DWORD size = MAX_PATH;
  const bool named = QueryFullProcessImageNameW(handle, 0, path, &size) != FALSE;
  CloseHandle(handle);
  if (!named) return false;
  const std::wstring_view image(path, size);
  const std::wstring_view file = image.substr(image.find_last_of(L'\\') + 1);
  for (std::wstring_view host : kHosts) {
    if (CompareStringOrdinal(file.data(), static_cast<int>(file.size()), host.data(),
                             static_cast<int>(host.size()), TRUE) == CSTR_EQUAL) {
      return true;
    }
  }
  return false;
}

// Tells programs a taskbar was created, so they add their icons to it.
void AnnounceTaskbar(UINT message) {
  struct Context {
    UINT message;
    std::unordered_map<DWORD, bool> skip;
  } context{message, {}};
  EnumWindows(
      [](HWND window, LPARAM param) -> BOOL {
        auto& context = *reinterpret_cast<Context*>(param);
        DWORD process = 0;
        GetWindowThreadProcessId(window, &process);
        auto [it, added] = context.skip.try_emplace(process, false);
        if (added) it->second = IsShellHost(process);
        if (!it->second) PostMessageW(window, context.message, 0, 0);
        return TRUE;
      },
      reinterpret_cast<LPARAM>(&context));
}

void RegisterClasses(HINSTANCE instance, WNDPROC proc) {
  WNDCLASSEXW tray{};
  tray.cbSize = sizeof(tray);
  tray.lpfnWndProc = proc;
  tray.hInstance = instance;
  tray.lpszClassName = kTrayClass;
  RegisterClassExW(&tray);
  WNDCLASSEXW notify = tray;
  notify.lpfnWndProc = DefWindowProcW;
  notify.lpszClassName = kNotifyClass;
  RegisterClassExW(&notify);
}

}  // namespace

TrayHost::TrayHost(HINSTANCE instance, HWND taskband, std::function<void()> onChanged)
    : taskband_(taskband), onChanged_(std::move(onChanged)) {
  RegisterClasses(instance, WindowProc);
  hwnd_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kTrayClass, L"", WS_POPUP, 0, 0, 0, 0,
                          nullptr, nullptr, instance, this);
  if (!hwnd_) throw std::runtime_error("CreateWindowEx failed for the notification area");
  notifyArea_ = CreateWindowExW(0, kNotifyClass, L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd_,
                                nullptr, instance, nullptr);
  // ITaskbarList finds its receiver through this property of the topmost
  // Shell_TrayWnd, which is now ours.
  SetPropW(hwnd_, L"TaskbandHWND", taskband_);
  SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

  // Programs add their icons again when a taskbar is created; posted, so it
  // arrives after this window is ready.
  taskbarCreated_ = RegisterWindowMessageW(L"TaskbarCreated");
  AnnounceTaskbar(taskbarCreated_);
}

TrayHost::~TrayHost() {
  RemovePropW(hwnd_, L"TaskbandHWND");
  DestroyWindow(hwnd_);
  for (const auto& [key, icon] : icons_) DestroyIcon(icon);
  // Explorer already has every icon, but programs that missed a call while
  // we were running set theirs up again.
  AnnounceTaskbar(taskbarCreated_);
}

LRESULT CALLBACK TrayHost::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_NCCREATE) {
    SetWindowLongPtrW(
        hwnd, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams));
  }
  auto* self = reinterpret_cast<TrayHost*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  switch (message) {
    case WM_COPYDATA:
      if (self) return self->OnCopyData(wParam, reinterpret_cast<COPYDATASTRUCT*>(lParam), lParam);
      break;
    case WM_CLOSE:
      return 0;  // the bar decides when the app closes
    case WM_COMMAND:
    case WM_HOTKEY:
      // Shell commands such as "show desktop" are Explorer's to carry out.
      return Forward(message, wParam, lParam);
    default:
      if (message >= WM_USER && message < WM_APP) return Forward(message, wParam, lParam);
      break;
  }
  return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT TrayHost::OnCopyData(WPARAM sender, const COPYDATASTRUCT* data, LPARAM raw) {
  // Everything goes on to Explorer; icon messages are also kept here.
  const bool appBar =
      data && data->dwData == kAppBarData && data->cbData == kAppBarBlockSize && data->lpData;
  const LRESULT explorer =
      appBar ? ForwardAppBar(sender, *data) : Forward(WM_COPYDATA, sender, raw);
  if (!data || data->dwData != kNotifyIconData || !data->lpData) return explorer;

  const auto bytes = std::span(static_cast<const std::byte*>(data->lpData), data->cbData);
  const auto message = ParseNotifyIconData(bytes);
  if (!message) return explorer;
  const bool handled = list_.Apply(*message);
  SyncIcon(*message);
  if (onChanged_) onChanged_();
  // Icons added before we started are only known to Explorer.
  return handled || explorer ? TRUE : FALSE;
}

void TrayHost::SyncIcon(const NotifyIconMessage& message) {
  const bool newIcon = (message.flags & kFlagIcon) != 0 &&
                       (message.command == kAddCommand || message.command == kModifyCommand);
  if (newIcon) {
    const bool useGuid = (message.flags & kFlagGuid) != 0 && HasGuid(message.guid);
    auto& copy = icons_[KeyOf(message.window, message.id, message.guid, useGuid)];
    if (copy) DestroyIcon(copy);
    // The program owns its icon and may destroy it right after the call.
    copy = message.icon ? CopyIcon(reinterpret_cast<HICON>(static_cast<ULONG_PTR>(message.icon)))
                        : nullptr;
  }
  // Drop the copies of icons that are gone.
  std::set<std::wstring> live;
  for (const auto& icon : list_.Icons()) live.insert(KeyOf(icon));
  for (auto it = icons_.begin(); it != icons_.end();) {
    if (live.contains(it->first)) {
      ++it;
    } else {
      if (it->second) DestroyIcon(it->second);
      it = icons_.erase(it);
    }
  }
}

std::vector<const TrayIcon*> TrayHost::Shown() const {
  std::vector<const TrayIcon*> shown;
  for (const auto& icon : list_.Icons()) {
    const auto copy = icons_.find(KeyOf(icon));
    if (!icon.hidden && copy != icons_.end() && copy->second) shown.push_back(&icon);
  }
  return shown;
}

std::vector<TrayHost::Visible> TrayHost::VisibleIcons() const {
  std::vector<Visible> visible;
  for (const TrayIcon* icon : Shown()) visible.push_back({icons_.at(KeyOf(*icon)), icon->tip});
  return visible;
}

void TrayHost::Place(const RECT& bar, const RECT& area) {
  SetWindowPos(hwnd_, HWND_TOPMOST, bar.left, bar.top, bar.right - bar.left, bar.bottom - bar.top,
               SWP_NOACTIVATE);
  MoveWindow(notifyArea_, area.left - bar.left, area.top - bar.top, area.right - area.left,
             area.bottom - area.top, FALSE);
}

void TrayHost::Send(size_t visibleIndex, Mouse mouse, POINT point) {
  const std::vector<const TrayIcon*> shown = Shown();
  if (visibleIndex >= shown.size()) return;
  const TrayIcon& icon = *shown[visibleIndex];
  const HWND target = ToWindow(icon.window);
  if (!IsWindow(target) || icon.callbackMessage == 0) return;

  // The program will want to bring its menu or window to the front.
  DWORD process = 0;
  GetWindowThreadProcessId(target, &process);
  AllowSetForegroundWindow(process);

  const auto post = [&](UINT event) {
    if (icon.version >= 4) {
      PostMessageW(target, icon.callbackMessage,
                   MAKEWPARAM(static_cast<WORD>(point.x), static_cast<WORD>(point.y)),
                   MAKELPARAM(event, icon.id));
    } else {
      PostMessageW(target, icon.callbackMessage, icon.id, event);
    }
  };
  switch (mouse) {
    case Mouse::Move:
      post(WM_MOUSEMOVE);
      break;
    case Mouse::LeftClick:
      post(WM_LBUTTONDOWN);
      post(WM_LBUTTONUP);
      if (icon.version >= 3) post(kSelect);
      break;
    case Mouse::DoubleClick:
      post(WM_LBUTTONDBLCLK);
      post(WM_LBUTTONUP);
      break;
    case Mouse::RightClick:
      post(WM_RBUTTONDOWN);
      post(WM_RBUTTONUP);
      if (icon.version >= 3) post(WM_CONTEXTMENU);
      break;
  }
}

void TrayHost::Prune() {
  std::vector<std::uint32_t> gone;
  for (const auto& icon : list_.Icons()) {
    if (!IsWindow(ToWindow(icon.window))) gone.push_back(icon.window);
  }
  if (gone.empty()) return;
  for (std::uint32_t window : gone) list_.RemoveWindow(window);
  SyncIcon({});
  if (onChanged_) onChanged_();
}

}  // namespace throwback
