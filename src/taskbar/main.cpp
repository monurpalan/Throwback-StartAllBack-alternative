#include <windows.h>
#include <objbase.h>
#include <shellapi.h>

#include <cwchar>
#include <exception>
#include <string>
#include <string_view>
#include <vector>

#include "core/options.h"
#include "core/settings.h"
#include "shell/autostart.h"
#include "shell/settings_store.h"
#include "startmenu/start_menu_window.h"
#include "taskbar/bar_manager.h"
#include "taskbar/system_taskbar.h"
#include "ui/strings.h"

// Themed common controls (version 6) for the properties dialog, as Windows
// draws its own dialogs.
#pragma comment(linker,                                                                          \
                "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "   \
                "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' " \
                "language='*'\"")

namespace {

std::vector<std::wstring> CommandLineArgs() {
  int count = 0;
  LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &count);
  std::vector<std::wstring> args;
  for (int i = 1; i < count; ++i) args.emplace_back(argv[i]);
  LocalFree(argv);
  return args;
}

int RunMessageLoop() {
  MSG message;
  while (GetMessageW(&message, nullptr, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
  return static_cast<int>(message.wParam);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
  // The watchdog that brings the Windows taskbar back after a killed run.
  const std::wstring_view watchdog = throwback::SystemTaskbar::kWatchdogArgument;
  if (const auto args = CommandLineArgs(); args.size() == 1 && args[0].starts_with(watchdog)) {
    const auto processId = std::wcstoul(args[0].c_str() + watchdog.size(), nullptr, 10);
    return throwback::SystemTaskbar::RunWatchdog(static_cast<DWORD>(processId));
  }

  // Two bars would fight over the same screen edge.
  HANDLE singleInstance = CreateMutexW(nullptr, TRUE, L"Local\\Throwback.SingleInstance");
  if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;

  using throwback::ui::StringId;
  using throwback::ui::Tr;
  const throwback::ParseResult parsed = throwback::ParseOptions(CommandLineArgs());
  const throwback::shell::SettingsStore store(parsed.options.profile);
  // The first run takes the Windows taskbar's place and opens the properties,
  // as StartAllBack opens its welcome page. Test profiles skip it.
  throwback::Settings saved = store.Load();
  const bool firstRun = !saved.welcomeShown && parsed.options.profile == L"default";
  if (firstRun) {
    saved.welcomeShown = true;
    saved.hideSystemTaskbar = true;
    saved.edge = throwback::shell::SystemTaskbarEdge();
    store.Save(saved);
  }
  const throwback::Settings settings = throwback::WithOverrides(saved, parsed.options);
  throwback::ui::SetLanguage(throwback::ui::LanguageFor(settings.language));
  if (!parsed.unknownArguments.empty()) {
    std::wstring text;
    for (const auto& arg : parsed.unknownArguments) {
      text += std::wstring(Tr(StringId::UnknownArgument)) + arg + L"\n";
    }
    text += L"\n" + std::wstring(Tr(StringId::Usage));
    MessageBoxW(nullptr, text.c_str(), L"Throwback", MB_ICONERROR);
    return 1;
  }

  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

  int exitCode = 0;
  try {
    throwback::BarManager bars(instance, settings, store);
    if (firstRun) PostMessageW(bars.MainWindow(), throwback::kOpenSettingsMessage, 0, 0);
    exitCode = RunMessageLoop();
  } catch (const std::exception& error) {
    MessageBoxA(nullptr, error.what(), "Throwback", MB_ICONERROR);
    exitCode = 1;
  }

  CoUninitialize();
  CloseHandle(singleInstance);
  return exitCode;
}
