#include "startmenu/win_key_hook.h"

#include <atomic>
#include <future>

#include "core/win_key.h"

namespace throwback {
namespace {

// An unassigned virtual key. Windows opens Start only when the Windows key
// is released without another key in between; this one is that key.
constexpr WORD kMaskKey = 0xE8;
// Marks the key events we send ourselves ("THBK"), so the hook can let
// exactly those through. Other software (remote desktop, automation tools)
// may inject Windows key presses too, and those should open our menu.
constexpr ULONG_PTR kOwnInputMarker = 0x5448424B;

// The hook procedure has no context parameter; there is one hook per process.
std::atomic<HWND> g_target = nullptr;
std::atomic<UINT> g_message = 0;
WinKeyTracker g_tracker;  // only touched on the hook thread

void ReleaseWinWithMask(WORD winKey) {
  INPUT inputs[3] = {};
  for (INPUT& input : inputs) {
    input.type = INPUT_KEYBOARD;
    input.ki.dwExtraInfo = kOwnInputMarker;
  }
  inputs[0].ki.wVk = kMaskKey;
  inputs[1].ki.wVk = kMaskKey;
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
  inputs[2].ki.wVk = winKey;
  inputs[2].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY;
  SendInput(3, inputs, sizeof(INPUT));
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
  if (code != HC_ACTION) return CallNextHookEx(nullptr, code, wParam, lParam);
  const auto* key = reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
  // Our own SendInput comes back through the hook; let it pass untouched.
  if (key->dwExtraInfo == kOwnInputMarker) return CallNextHookEx(nullptr, code, wParam, lParam);

  const bool down = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
  switch (g_tracker.OnKey(key->vkCode, down)) {
    case WinKeyAction::PassThrough:
      break;
    case WinKeyAction::OpenMenuInsteadOfRelease:
      // Swallow the real release and replay it after the mask key.
      ReleaseWinWithMask(static_cast<WORD>(key->vkCode));
      PostMessageW(g_target, g_message, 0, 0);
      return 1;
    case WinKeyAction::OpenMenuAndSwallow:
      PostMessageW(g_target, g_message, 0, 0);
      return 1;
  }
  return CallNextHookEx(nullptr, code, wParam, lParam);
}

}  // namespace

WinKeyHook::WinKeyHook(HWND target, UINT message) {
  g_target = target;
  g_message = message;
  std::promise<DWORD> started;
  std::future<DWORD> threadId = started.get_future();
  thread_ = std::thread([&started] {
    MSG msg;
    // Creates this thread's message queue before anyone posts to it.
    PeekMessageW(&msg, nullptr, 0, 0, PM_NOREMOVE);
    HHOOK hook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandleW(nullptr), 0);
    started.set_value(GetCurrentThreadId());
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) DispatchMessageW(&msg);
    if (hook) UnhookWindowsHookEx(hook);
  });
  threadId_ = threadId.get();
}

WinKeyHook::~WinKeyHook() {
  PostThreadMessageW(threadId_, WM_QUIT, 0, 0);
  if (thread_.joinable()) thread_.join();
  g_target = nullptr;
}

}  // namespace throwback
