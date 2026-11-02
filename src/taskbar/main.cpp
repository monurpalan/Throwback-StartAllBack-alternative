#include <objbase.h>
#include <shellapi.h>
#include <windows.h>

#include <exception>
#include <string>
#include <vector>

#include "core/options.h"
#include "taskbar/taskbar_window.h"

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
  // Two bars would fight over the same screen edge.
  HANDLE singleInstance = CreateMutexW(nullptr, TRUE, L"Local\\Throwback.SingleInstance");
  if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;

  const throwback::ParseResult parsed = throwback::ParseOptions(CommandLineArgs());
  if (!parsed.errors.empty()) {
    std::wstring text;
    for (const auto& error : parsed.errors) text += error + L"\n";
    text += L"\nUsage: throwback.exe [--edge=top|bottom] [--theme=system|light|dark]";
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
