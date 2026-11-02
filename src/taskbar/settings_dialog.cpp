#include "taskbar/settings_dialog.h"

#include <array>
#include <string>

#include "core/layout.h"
#include "ui/strings.h"

namespace throwback {
namespace {

using ui::StringId;

constexpr wchar_t kClassName[] = L"ThrowbackSettings";
constexpr int kWidth = 420;
constexpr int kHeight = 312;
constexpr int kApply = 100;
constexpr int kEdge = 101;
constexpr int kHideTaskbar = 102;
constexpr int kWindowsKey = 103;
constexpr int kLanguage = 104;
constexpr int kTheme = 105;

struct Dialog {
  Settings settings;
  const std::function<void(const Settings&)>* apply = nullptr;
  HFONT font = nullptr;
  unsigned dpi = 96;
  bool done = false;
};

HWND g_open = nullptr;  // one dialog at a time

HWND Control(HWND parent, const Dialog& dialog, const wchar_t* type, StringId text, DWORD style,
             int id, Rect rect) {
  const auto s = [&](int value) { return ScaleForDpi(value, dialog.dpi); };
  HWND control = CreateWindowExW(
      0, type, text == StringId::kCount ? L"" : std::wstring(ui::Tr(text)).c_str(),
      WS_CHILD | WS_VISIBLE | style, s(rect.left), s(rect.top), s(rect.Width()), s(rect.Height()),
      parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
  SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(dialog.font), TRUE);
  return control;
}

template <size_t N>
void FillCombo(HWND combo, const std::array<StringId, N>& items, int selected) {
  for (StringId item : items) {
    SendMessageW(combo, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>(std::wstring(ui::Tr(item)).c_str()));
  }
  SendMessageW(combo, CB_SETCURSEL, static_cast<WPARAM>(selected), 0);
}

void CreateControls(HWND hwnd, const Dialog& dialog) {
  constexpr DWORD kGroup = BS_GROUPBOX;
  constexpr DWORD kCheck = BS_AUTOCHECKBOX | WS_TABSTOP;
  constexpr DWORD kCombo = CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL;
  const Settings& s = dialog.settings;

  Control(hwnd, dialog, L"BUTTON", StringId::TaskbarAppearance, kGroup, 0, {8, 8, 412, 104});
  Control(hwnd, dialog, L"STATIC", StringId::TaskbarLocation, SS_LEFT, 0, {20, 34, 236, 54});
  FillCombo(
      Control(hwnd, dialog, L"COMBOBOX", StringId::kCount, kCombo, kEdge, {244, 30, 400, 130}),
      std::array{StringId::LocationTop, StringId::LocationBottom}, s.edge == Edge::Top ? 0 : 1);
  Control(hwnd, dialog, L"BUTTON", StringId::HideWindowsTaskbar, kCheck, kHideTaskbar,
          {20, 66, 400, 86});

  Control(hwnd, dialog, L"BUTTON", StringId::StartMenuGroup, kGroup, 0, {8, 112, 412, 166});
  Control(hwnd, dialog, L"BUTTON", StringId::WindowsKeyOpensMenu, kCheck, kWindowsKey,
          {20, 134, 400, 154});

  Control(hwnd, dialog, L"BUTTON", StringId::LanguageAndTheme, kGroup, 0, {8, 174, 412, 264});
  Control(hwnd, dialog, L"STATIC", StringId::LanguageLabel, SS_LEFT, 0, {20, 200, 236, 220});
  FillCombo(
      Control(hwnd, dialog, L"COMBOBOX", StringId::kCount, kCombo, kLanguage, {244, 196, 400, 296}),
      std::array{StringId::LanguageSystem, StringId::LanguageEnglish, StringId::LanguageTurkish},
      static_cast<int>(s.language));
  Control(hwnd, dialog, L"STATIC", StringId::MenuTheme, SS_LEFT, 0, {20, 232, 236, 252});
  FillCombo(
      Control(hwnd, dialog, L"COMBOBOX", StringId::kCount, kCombo, kTheme, {244, 228, 400, 328}),
      std::array{StringId::ThemeSystem, StringId::ThemeLight, StringId::ThemeDark},
      static_cast<int>(s.theme));

  Control(hwnd, dialog, L"BUTTON", StringId::Ok, BS_DEFPUSHBUTTON | WS_TABSTOP, IDOK,
          {156, 276, 236, 300});
  Control(hwnd, dialog, L"BUTTON", StringId::Cancel, BS_PUSHBUTTON | WS_TABSTOP, IDCANCEL,
          {244, 276, 324, 300});
  Control(hwnd, dialog, L"BUTTON", StringId::Apply, BS_PUSHBUTTON | WS_TABSTOP, kApply,
          {332, 276, 412, 300});

  CheckDlgButton(hwnd, kHideTaskbar, s.hideSystemTaskbar ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hwnd, kWindowsKey, s.windowsKeyOpensMenu ? BST_CHECKED : BST_UNCHECKED);
}

int Selection(HWND hwnd, int id) {
  return static_cast<int>(SendDlgItemMessageW(hwnd, id, CB_GETCURSEL, 0, 0));
}

void ReadControls(HWND hwnd, Settings& settings) {
  settings.edge = Selection(hwnd, kEdge) == 0 ? Edge::Top : Edge::Bottom;
  settings.hideSystemTaskbar = IsDlgButtonChecked(hwnd, kHideTaskbar) == BST_CHECKED;
  settings.windowsKeyOpensMenu = IsDlgButtonChecked(hwnd, kWindowsKey) == BST_CHECKED;
  settings.language = static_cast<LanguageMode>(Selection(hwnd, kLanguage));
  settings.theme = static_cast<ThemeMode>(Selection(hwnd, kTheme));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_NCCREATE) {
    SetWindowLongPtrW(
        hwnd, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams));
  }
  auto* dialog = reinterpret_cast<Dialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  switch (message) {
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
      // Labels and check boxes sit on the dialog face, not on white.
      SetBkColor(reinterpret_cast<HDC>(wParam), GetSysColor(COLOR_BTNFACE));
      return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_BTNFACE));
    case WM_COMMAND: {
      const int id = LOWORD(wParam);
      if (!dialog || (id != IDOK && id != IDCANCEL && id != kApply)) break;
      if (id != IDCANCEL) {
        ReadControls(hwnd, dialog->settings);
        (*dialog->apply)(dialog->settings);
      }
      if (id != kApply) DestroyWindow(hwnd);
      return 0;
    }
    case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;
    case WM_DESTROY:
      if (dialog) dialog->done = true;
      return 0;
  }
  return DefWindowProcW(hwnd, message, wParam, lParam);
}

HFONT CreateDialogFont(unsigned dpi) {
  NONCLIENTMETRICSW metrics{sizeof(metrics)};
  SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, dpi);
  return CreateFontIndirectW(&metrics.lfMessageFont);
}

}  // namespace

void ShowSettingsDialog(HINSTANCE instance, HWND owner, const Settings& current,
                        const std::function<void(const Settings&)>& apply) {
  if (g_open) {
    SetForegroundWindow(g_open);
    return;
  }
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = WindowProc;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
  windowClass.lpszClassName = kClassName;
  RegisterClassExW(&windowClass);

  Dialog dialog{current, &apply};
  dialog.dpi = GetDpiForWindow(owner);
  dialog.font = CreateDialogFont(dialog.dpi);

  // Size the frame around a client area of the dialog's size, centered on
  // the owner's monitor.
  constexpr DWORD kStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU;
  RECT frame{0, 0, ScaleForDpi(kWidth, dialog.dpi), ScaleForDpi(kHeight, dialog.dpi)};
  AdjustWindowRectExForDpi(&frame, kStyle, FALSE, WS_EX_DLGMODALFRAME, dialog.dpi);
  MONITORINFO monitor{sizeof(monitor)};
  GetMonitorInfoW(MonitorFromWindow(owner, MONITOR_DEFAULTTONEAREST), &monitor);
  const RECT& work = monitor.rcWork;
  const int width = frame.right - frame.left;
  const int height = frame.bottom - frame.top;
  HWND hwnd =
      CreateWindowExW(WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT, kClassName,
                      std::wstring(ui::Tr(StringId::SettingsTitle)).c_str(), kStyle,
                      (work.left + work.right - width) / 2, (work.top + work.bottom - height) / 2,
                      width, height, owner, nullptr, instance, &dialog);
  if (!hwnd) {
    DeleteObject(dialog.font);
    return;
  }
  g_open = hwnd;
  CreateControls(hwnd, dialog);
  ShowWindow(hwnd, SW_SHOW);
  SetForegroundWindow(hwnd);

  // Modal: the bar ignores input until the dialog closes.
  EnableWindow(owner, FALSE);
  MSG message{};
  bool quit = false;
  while (!dialog.done) {
    if (GetMessageW(&message, nullptr, 0, 0) <= 0) {
      quit = true;
      break;
    }
    if (!IsDialogMessageW(hwnd, &message)) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
  }
  if (IsWindow(owner)) EnableWindow(owner, TRUE);
  if (IsWindow(hwnd)) DestroyWindow(hwnd);
  g_open = nullptr;
  DeleteObject(dialog.font);
  // A WM_QUIT taken by this loop belongs to the application's main loop.
  if (quit) PostQuitMessage(static_cast<int>(message.wParam));
}

}  // namespace throwback
