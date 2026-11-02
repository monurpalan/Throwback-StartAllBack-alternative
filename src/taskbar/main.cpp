#include <objbase.h>
#include <shellapi.h>
#include <windows.h>

#include <exception>
#include <string>
#include <vector>

#include "core/options.h"
#include "taskbar/taskbar_window.h"
#include "ui/strings.h"

namespace {

throwback::ui::Language ResolveLanguage(throwback::LanguageMode mode) {
  switch (mode) {
    case throwback::LanguageMode::English:
      return throwback::ui::Language::English;
    case throwback::LanguageMode::Turkish:
      return throwback::ui::Language::Turkish;
    case throwback::LanguageMode::System:
      break;
  }
  return throwback::ui::SystemLanguage();
}

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
  // Two bars would fight over the same screen edge.
  HANDLE singleInstance = CreateMutexW(nullptr, TRUE, L"Local\\Throwback.SingleInstance");
  if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;

  using throwback::ui::StringId;
  using throwback::ui::Tr;
  const throwback::ParseResult parsed = throwback::ParseOptions(CommandLineArgs());
  throwback::ui::SetLanguage(ResolveLanguage(parsed.options.language));
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
    throwback::TaskbarWindow window(instance, parsed.options);
    exitCode = RunMessageLoop();
  } catch (const std::exception& error) {
    MessageBoxA(nullptr, error.what(), "Throwback", MB_ICONERROR);
    exitCode = 1;
  }

  CoUninitialize();
  CloseHandle(singleInstance);
  return exitCode;
}
