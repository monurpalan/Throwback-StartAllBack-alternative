#include "taskbar/settings_dialog.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "core/layout.h"
#include "taskbar/dialog_controls.h"
#include "taskbar/start_menu_customize.h"
#include "ui/strings.h"

namespace throwback {
namespace {

using ui::StringId;

constexpr wchar_t kClassName[] = L"ThrowbackSettings";
constexpr int kWidth = 420;
constexpr int kHeight = 442;
constexpr int kApply = 100;
constexpr int kEdge = 101;
constexpr int kHideTaskbar = 102;
constexpr int kWindowsKey = 103;
constexpr int kLanguage = 104;
constexpr int kTheme = 105;
constexpr int kStartStyle = 106;
constexpr int kMenuSide = 107;
constexpr int kMenuEdge = 108;
constexpr int kOrb = 109;
constexpr int kCustomize = 110;
constexpr int kStartWithWindows = 111;
constexpr const auto& kStyles = kStartStyles;

struct Dialog {
  HINSTANCE instance = nullptr;
  Settings settings;
  const std::function<void(const Settings&)>* apply = nullptr;
  DialogLook look;
  bool done = false;
};

HWND g_open = nullptr;  // one dialog at a time

HWND Control(HWND parent, const Dialog& dialog, const wchar_t* type, StringId text, DWORD style,
             int id, Rect rect) {
  return CreateDialogControl(parent, dialog.look, type,
                             text == StringId::kCount ? std::wstring_view() : ui::Tr(text), style,
                             id, rect);
}

void FillCombo(HWND combo, const std::vector<std::wstring_view>& items, int selected) {
  for (std::wstring_view item : items) {
    SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(std::wstring(item).c_str()));
  }
  SendMessageW(combo, CB_SETCURSEL, static_cast<WPARAM>(selected), 0);
}

// A label with a drop-down list beside it.
void LabeledCombo(HWND hwnd, const Dialog& dialog, StringId label, int id, int top,
                  const std::vector<std::wstring_view>& items, int selected) {
  constexpr DWORD kCombo = CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL;
  Control(hwnd, dialog, L"STATIC", label, SS_LEFT, 0, {20, top + 4, 236, top + 24});
  FillCombo(
      Control(hwnd, dialog, L"COMBOBOX", StringId::kCount, kCombo, id, {244, top, 400, top + 200}),
      items, selected);
}

int StyleIndex(StartStyle style) {
  for (size_t i = 0; i < kStyles.size(); ++i) {
    if (kStyles[i].style == style) return static_cast<int>(i);
  }
  return 0;
}

void CreateStartMenuControls(HWND hwnd, const Dialog& dialog) {
  const Settings& s = dialog.settings;
  Control(hwnd, dialog, L"BUTTON", StringId::StartMenuGroup, BS_GROUPBOX, 0, {8, 112, 412, 296});
  std::vector<std::wstring_view> styles;
  for (const StyleName& style : kStyles) styles.emplace_back(style.name);
  LabeledCombo(hwnd, dialog, StringId::StartStyleLabel, kStartStyle, 134, styles,
               StyleIndex(s.startStyle));
  LabeledCombo(hwnd, dialog, StringId::MenuPositionLabel, kMenuSide, 166,
               {ui::Tr(StringId::MenuLeft), ui::Tr(StringId::MenuRight)},
               static_cast<int>(s.menuHorizontal));
  LabeledCombo(hwnd, dialog, StringId::MenuEdgeLabel, kMenuEdge, 198,
               {ui::Tr(StringId::MenuNextToBar), ui::Tr(StringId::MenuScreenTop),
                ui::Tr(StringId::MenuScreenBottom)},
               static_cast<int>(s.menuVertical));
  LabeledCombo(hwnd, dialog, StringId::OrbLabel, kOrb, 230,
               {ui::Tr(StringId::OrbStyle), L"Windows 7", L"Windows Vista",
                ui::Tr(StringId::OrbClover), ui::Tr(StringId::OrbWin8), L"E1even"},
               static_cast<int>(s.orb));
  Control(hwnd, dialog, L"BUTTON", StringId::WindowsKeyOpensMenu, BS_AUTOCHECKBOX | WS_TABSTOP,
          kWindowsKey, {20, 266, 296, 286});
  Control(hwnd, dialog, L"BUTTON", StringId::CustomizeIcons, BS_PUSHBUTTON | WS_TABSTOP, kCustomize,
          {304, 262, 400, 286});
  CheckDlgButton(hwnd, kWindowsKey, s.windowsKeyOpensMenu ? BST_CHECKED : BST_UNCHECKED);
  EnableWindow(GetDlgItem(hwnd, kCustomize), HasCustomizableMenu(s.startStyle));
}

void CreateControls(HWND hwnd, const Dialog& dialog) {
  constexpr DWORD kGroup = BS_GROUPBOX;
  const Settings& s = dialog.settings;

  Control(hwnd, dialog, L"BUTTON", StringId::TaskbarAppearance, kGroup, 0, {8, 8, 412, 104});
  LabeledCombo(hwnd, dialog, StringId::TaskbarLocation, kEdge, 30,
               {ui::Tr(StringId::LocationTop), ui::Tr(StringId::LocationBottom),
                ui::Tr(StringId::MenuLeft), ui::Tr(StringId::MenuRight)},
               static_cast<int>(s.edge));
  Control(hwnd, dialog, L"BUTTON", StringId::HideWindowsTaskbar, BS_AUTOCHECKBOX | WS_TABSTOP,
          kHideTaskbar, {20, 62, 400, 80});
  CheckDlgButton(hwnd, kHideTaskbar, s.hideSystemTaskbar ? BST_CHECKED : BST_UNCHECKED);
  Control(hwnd, dialog, L"BUTTON", StringId::StartWithWindows, BS_AUTOCHECKBOX | WS_TABSTOP,
          kStartWithWindows, {20, 82, 400, 100});
  CheckDlgButton(hwnd, kStartWithWindows, s.startWithWindows ? BST_CHECKED : BST_UNCHECKED);

  CreateStartMenuControls(hwnd, dialog);

  Control(hwnd, dialog, L"BUTTON", StringId::LanguageAndTheme, kGroup, 0, {8, 304, 412, 394});
  LabeledCombo(hwnd, dialog, StringId::LanguageLabel, kLanguage, 326,
               {ui::Tr(StringId::LanguageSystem), ui::Tr(StringId::LanguageEnglish),
                ui::Tr(StringId::LanguageTurkish)},
               static_cast<int>(s.language));
  LabeledCombo(
      hwnd, dialog, StringId::MenuTheme, kTheme, 358,
      {ui::Tr(StringId::ThemeSystem), ui::Tr(StringId::ThemeLight), ui::Tr(StringId::ThemeDark)},
      static_cast<int>(s.theme));

  Control(hwnd, dialog, L"BUTTON", StringId::Ok, BS_DEFPUSHBUTTON | WS_TABSTOP, IDOK,
          {156, 406, 236, 430});
  Control(hwnd, dialog, L"BUTTON", StringId::Cancel, BS_PUSHBUTTON | WS_TABSTOP, IDCANCEL,
          {244, 406, 324, 430});
  Control(hwnd, dialog, L"BUTTON", StringId::Apply, BS_PUSHBUTTON | WS_TABSTOP, kApply,
          {332, 406, 412, 430});
}

int Selection(HWND hwnd, int id) {
  return static_cast<int>(SendDlgItemMessageW(hwnd, id, CB_GETCURSEL, 0, 0));
}

StartStyle SelectedStyle(HWND hwnd) {
  return kStyles.at(static_cast<size_t>(Selection(hwnd, kStartStyle))).style;
}

void ReadControls(HWND hwnd, Settings& settings) {
  settings.edge = static_cast<Edge>(Selection(hwnd, kEdge));
  settings.hideSystemTaskbar = IsDlgButtonChecked(hwnd, kHideTaskbar) == BST_CHECKED;
  settings.windowsKeyOpensMenu = IsDlgButtonChecked(hwnd, kWindowsKey) == BST_CHECKED;
  settings.startWithWindows = IsDlgButtonChecked(hwnd, kStartWithWindows) == BST_CHECKED;
  settings.language = static_cast<LanguageMode>(Selection(hwnd, kLanguage));
  settings.theme = static_cast<ThemeMode>(Selection(hwnd, kTheme));
  settings.startStyle = SelectedStyle(hwnd);
  settings.menuHorizontal = static_cast<MenuHorizontal>(Selection(hwnd, kMenuSide));
  settings.menuVertical = static_cast<MenuVertical>(Selection(hwnd, kMenuEdge));
  settings.orb = static_cast<OrbChoice>(Selection(hwnd, kOrb));
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
      if (dialog && id == kStartStyle && HIWORD(wParam) == CBN_SELCHANGE) {
        // Customize shapes the chosen style's menu, so it follows the list.
        dialog->settings.startStyle = SelectedStyle(hwnd);
        EnableWindow(GetDlgItem(hwnd, kCustomize),
                     HasCustomizableMenu(dialog->settings.startStyle));
        return 0;
      }
      if (dialog && id == kCustomize) {
        ShowCustomizeStartMenu(dialog->instance, hwnd, dialog->settings);
        return 0;
      }
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

  Dialog dialog{instance, current, &apply};
  dialog.look.dpi = GetDpiForWindow(owner);
  dialog.look.font = CreateDialogFont(dialog.look.dpi);
  HWND hwnd = CreateDialogFrame(instance, owner, kClassName, ui::Tr(StringId::SettingsTitle),
                                kWidth, kHeight, dialog.look.dpi, &dialog);
  if (!hwnd) {
    DeleteObject(dialog.look.font);
    return;
  }
  g_open = hwnd;
  CreateControls(hwnd, dialog);
  ShowWindow(hwnd, SW_SHOW);
  SetForegroundWindow(hwnd);
  // Modal: the bar ignores input until the dialog closes.
  RunModalLoop(hwnd, owner, dialog.done);
  g_open = nullptr;
  DeleteObject(dialog.look.font);
}

}  // namespace throwback
