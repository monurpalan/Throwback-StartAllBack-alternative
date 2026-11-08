#include "taskbar/start_menu_customize.h"

#include <string>
#include <vector>

#include "startmenu/places.h"
#include "taskbar/dialog_controls.h"
#include "ui/strings.h"

namespace throwback {
namespace {

using ui::StringId;

constexpr wchar_t kClassName[] = L"ThrowbackCustomizeStartMenu";
constexpr int kWidth = 380;
constexpr int kRows = 200;
constexpr int kLargeIcons = 201;
constexpr int kFirstPlace = 300;
// Places in two columns of check boxes.
constexpr int kPlaceTop = 100;
constexpr int kPlaceRow = 22;
constexpr int kColumnWidth = 170;

struct Customize {
  Settings* settings = nullptr;
  std::vector<PlaceChoice> places;
  DialogLook look;
  bool done = false;
  bool accepted = false;
};

int PlaceRows(const Customize& customize) {
  return static_cast<int>((customize.places.size() + 1) / 2);
}

int Height(const Customize& customize) { return kPlaceTop + PlaceRows(customize) * kPlaceRow + 48; }

void CreateControls(HWND hwnd, const Customize& customize) {
  constexpr DWORD kCheck = BS_AUTOCHECKBOX | WS_TABSTOP;
  constexpr DWORD kCombo = CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL;
  const DialogLook& look = customize.look;
  const Settings& settings = *customize.settings;

  CreateDialogControl(hwnd, look, L"STATIC", ui::Tr(StringId::MenuRowsLabel), SS_LEFT, 0,
                      {12, 16, 268, 36});
  HWND rows = CreateDialogControl(hwnd, look, L"COMBOBOX", L"", kCombo, kRows, {276, 12, 368, 212});
  for (int count = kMinMenuRows; count <= kMaxMenuRows; ++count) {
    SendMessageW(rows, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(std::to_wstring(count).c_str()));
  }
  SendMessageW(rows, CB_SETCURSEL, static_cast<WPARAM>(settings.menuRows - kMinMenuRows), 0);
  CreateDialogControl(hwnd, look, L"BUTTON", ui::Tr(StringId::LargeIcons), kCheck, kLargeIcons,
                      {12, 44, 368, 64});
  CheckDlgButton(hwnd, kLargeIcons, settings.largeIcons ? BST_CHECKED : BST_UNCHECKED);

  CreateDialogControl(hwnd, look, L"STATIC", ui::Tr(StringId::PlacesLabel), SS_LEFT, 0,
                      {12, 76, 368, 96});
  const int rowsPerColumn = PlaceRows(customize);
  for (size_t i = 0; i < customize.places.size(); ++i) {
    const int column = static_cast<int>(i) / rowsPerColumn;
    const int row = static_cast<int>(i) % rowsPerColumn;
    const int left = 24 + column * kColumnWidth;
    const int top = kPlaceTop + row * kPlaceRow;
    const int id = kFirstPlace + static_cast<int>(i);
    CreateDialogControl(hwnd, look, L"BUTTON", customize.places[i].name, kCheck, id,
                        {left, top, left + kColumnWidth - 8, top + 20});
    CheckDlgButton(hwnd, id, customize.places[i].shown ? BST_CHECKED : BST_UNCHECKED);
  }

  const int buttonTop = Height(customize) - 36;
  CreateDialogControl(hwnd, look, L"BUTTON", ui::Tr(StringId::Ok), BS_DEFPUSHBUTTON | WS_TABSTOP,
                      IDOK, {200, buttonTop, 280, buttonTop + 24});
  CreateDialogControl(hwnd, look, L"BUTTON", ui::Tr(StringId::Cancel), BS_PUSHBUTTON | WS_TABSTOP,
                      IDCANCEL, {288, buttonTop, 368, buttonTop + 24});
}

void ReadControls(HWND hwnd, const Customize& customize) {
  Settings& settings = *customize.settings;
  const auto rows = static_cast<int>(SendDlgItemMessageW(hwnd, kRows, CB_GETCURSEL, 0, 0));
  if (rows >= 0) settings.menuRows = kMinMenuRows + rows;
  settings.largeIcons = IsDlgButtonChecked(hwnd, kLargeIcons) == BST_CHECKED;
  for (size_t i = 0; i < customize.places.size(); ++i) {
    const bool shown = IsDlgButtonChecked(hwnd, kFirstPlace + static_cast<int>(i)) == BST_CHECKED;
    // Only changes are saved, so untouched places keep following the
    // defaults.
    if (shown != customize.places[i].shown) {
      SetPlaceShown(settings, customize.places[i].key, shown);
    }
  }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_NCCREATE) {
    SetWindowLongPtrW(
        hwnd, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams));
  }
  auto* customize = reinterpret_cast<Customize*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  switch (message) {
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
      SetBkColor(reinterpret_cast<HDC>(wParam), GetSysColor(COLOR_BTNFACE));
      return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_BTNFACE));
    case WM_COMMAND: {
      const int id = LOWORD(wParam);
      if (!customize || (id != IDOK && id != IDCANCEL)) break;
      if (id == IDOK) {
        ReadControls(hwnd, *customize);
        customize->accepted = true;
      }
      DestroyWindow(hwnd);
      return 0;
    }
    case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;
    case WM_DESTROY:
      if (customize) customize->done = true;
      return 0;
  }
  return DefWindowProcW(hwnd, message, wParam, lParam);
}

}  // namespace

bool ShowCustomizeStartMenu(HINSTANCE instance, HWND owner, Settings& settings) {
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = WindowProc;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
  windowClass.lpszClassName = kClassName;
  RegisterClassExW(&windowClass);

  Customize customize;
  customize.settings = &settings;
  customize.places = LoadPlaceChoices(settings);
  customize.look.dpi = GetDpiForWindow(owner);
  customize.look.font = CreateDialogFont(customize.look.dpi);
  HWND hwnd = CreateDialogFrame(instance, owner, kClassName, ui::Tr(StringId::CustomizeTitle),
                                kWidth, Height(customize), customize.look.dpi, &customize);
  if (hwnd) {
    CreateControls(hwnd, customize);
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    RunModalLoop(hwnd, owner, customize.done);
    if (IsWindow(owner)) SetForegroundWindow(owner);
  }
  DeleteObject(customize.look.font);
  return customize.accepted;
}

}  // namespace throwback
