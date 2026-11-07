#include "taskbar/notify_icon_settings.h"

#include <objbase.h>
#include <shlobj.h>

#include <cstring>

namespace throwback {
namespace {

constexpr wchar_t kChoicesKey[] = L"Control Panel\\NotifyIconSettings";

std::wstring Lower(std::wstring text) {
  if (!text.empty()) CharLowerBuffW(text.data(), static_cast<DWORD>(text.size()));
  return text;
}

std::wstring GuidText(const GUID& guid) {
  std::array<wchar_t, 40> text{};
  StringFromGUID2(guid, text.data(), static_cast<int>(text.size()));
  return Lower(text.data());
}

// Windows writes paths under known folders as "{folder id}\rest".
std::wstring ResolveKnownFolder(const std::wstring& path) {
  const size_t close = path.find(L'}');
  if (path.empty() || path[0] != L'{' || close == std::wstring::npos) return path;
  GUID folder{};
  if (FAILED(CLSIDFromString(path.substr(0, close + 1).c_str(), &folder))) return path;
  PWSTR base = nullptr;
  std::wstring resolved = path;
  if (SUCCEEDED(SHGetKnownFolderPath(folder, 0, nullptr, &base))) {
    resolved = std::wstring(base) + path.substr(close + 1);
  }
  CoTaskMemFree(base);
  return resolved;
}

std::wstring ReadText(HKEY key, const wchar_t* name) {
  DWORD bytes = 0;
  if (RegGetValueW(key, nullptr, name, RRF_RT_REG_SZ, nullptr, nullptr, &bytes) != ERROR_SUCCESS ||
      bytes < sizeof(wchar_t)) {
    return {};
  }
  std::wstring text(bytes / sizeof(wchar_t), L'\0');
  if (RegGetValueW(key, nullptr, name, RRF_RT_REG_SZ, nullptr, text.data(), &bytes) !=
      ERROR_SUCCESS) {
    return {};
  }
  text.resize(wcslen(text.c_str()));
  return text;
}

DWORD ReadNumber(HKEY key, const wchar_t* name, DWORD fallback) {
  DWORD value = 0;
  DWORD bytes = sizeof(value);
  if (RegGetValueW(key, nullptr, name, RRF_RT_REG_DWORD, nullptr, &value, &bytes) !=
      ERROR_SUCCESS) {
    return fallback;
  }
  return value;
}

std::wstring ProcessPath(HWND window) {
  DWORD process = 0;
  GetWindowThreadProcessId(window, &process);
  const HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process);
  if (!handle) return {};
  std::array<wchar_t, MAX_PATH * 2> path{};
  auto size = static_cast<DWORD>(path.size());
  const bool ok = QueryFullProcessImageNameW(handle, 0, path.data(), &size) != FALSE;
  CloseHandle(handle);
  return ok ? std::wstring(path.data(), size) : std::wstring();
}

}  // namespace

std::wstring IconIdentity(HWND window, std::uint32_t id, const std::array<std::uint8_t, 16>& guid,
                          bool useGuid) {
  if (useGuid) {
    GUID value{};
    std::memcpy(&value, guid.data(), sizeof(value));
    return GuidText(value);
  }
  const std::wstring path = ProcessPath(window);
  if (!path.empty()) return Lower(path) + L"|" + std::to_wstring(id);
  // Some processes don't let us read their path; their window class is the
  // next most stable name.
  std::array<wchar_t, 256> name{};
  const int length = GetClassNameW(window, name.data(), static_cast<int>(name.size()));
  if (length <= 0) return {};
  return L"class:" + Lower(std::wstring(name.data(), static_cast<size_t>(length))) + L"|" +
         std::to_wstring(id);
}

void WindowsIconChoices::Load() {
  shown_.clear();
  HKEY root = nullptr;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, kChoicesKey, 0, KEY_READ, &root) != ERROR_SUCCESS) return;
  std::array<wchar_t, 64> name{};
  for (DWORD index = 0;; ++index) {
    auto length = static_cast<DWORD>(name.size());
    if (RegEnumKeyExW(root, index, name.data(), &length, nullptr, nullptr, nullptr, nullptr) !=
        ERROR_SUCCESS) {
      break;
    }
    HKEY entry = nullptr;
    if (RegOpenKeyExW(root, name.data(), 0, KEY_READ, &entry) != ERROR_SUCCESS) continue;
    if (ReadNumber(entry, L"IsPromoted", 0) == 1) {
      const std::wstring guid = ReadText(entry, L"IconGuid");
      if (!guid.empty()) {
        shown_.insert(Lower(guid));
      } else {
        const std::wstring path = ResolveKnownFolder(ReadText(entry, L"ExecutablePath"));
        shown_.insert(Lower(path) + L"|" + std::to_wstring(ReadNumber(entry, L"UID", 0)));
      }
    }
    RegCloseKey(entry);
  }
  RegCloseKey(root);
}

}  // namespace throwback
