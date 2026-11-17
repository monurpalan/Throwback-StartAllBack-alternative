#include "shell/badge_store.h"

#include <shlobj.h>
#include <winsqlite/winsqlite3.h>

#include <algorithm>
#include <cwctype>
#include <filesystem>

namespace throwback::shell {
namespace {

std::wstring Lower(std::wstring text) {
  std::transform(text.begin(), text.end(), text.begin(),
                 [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
  return text;
}

std::filesystem::path DatabasePath() {
  PWSTR local = nullptr;
  std::filesystem::path path;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local))) {
    path = std::filesystem::path(local) / L"Microsoft\\Windows\\Notifications\\wpndatabase.db";
  }
  CoTaskMemFree(local);
  return path;
}

ULONGLONG LastWrite(const std::filesystem::path& file) {
  WIN32_FILE_ATTRIBUTE_DATA data{};
  if (!GetFileAttributesExW(file.c_str(), GetFileExInfoStandard, &data)) return 0;
  return (static_cast<ULONGLONG>(data.ftLastWriteTime.dwHighDateTime) << 32) |
         data.ftLastWriteTime.dwLowDateTime;
}

std::wstring Wide(const unsigned char* utf8) {
  if (!utf8) return {};
  const auto* text = reinterpret_cast<const char*>(utf8);
  const int length = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
  if (length <= 1) return {};
  std::wstring wide(static_cast<size_t>(length - 1), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, text, -1, wide.data(), length);
  return wide;
}

std::map<std::wstring, Badge> ReadBadges(const std::filesystem::path& database) {
  std::map<std::wstring, Badge> badges;
  sqlite3* db = nullptr;
  const std::string path =
      database.u8string().empty()
          ? std::string()
          : std::string(reinterpret_cast<const char*>(database.u8string().c_str()));
  if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return badges;
  }
  sqlite3_busy_timeout(db, 200);  // the notification service may be writing
  constexpr const char* kQuery =
      "select h.PrimaryId, cast(n.Payload as text) from Notification n "
      "join NotificationHandler h on h.RecordId = n.HandlerId where n.Type = 'badge'";
  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(db, kQuery, -1, &statement, nullptr) == SQLITE_OK) {
    while (sqlite3_step(statement) == SQLITE_ROW) {
      const std::wstring app = Wide(sqlite3_column_text(statement, 0));
      if (const auto badge = ParseBadge(Wide(sqlite3_column_text(statement, 1)))) {
        if (!app.empty()) badges[Lower(app)] = *badge;
      }
    }
  }
  sqlite3_finalize(statement);
  sqlite3_close(db);
  return badges;
}

}  // namespace

bool BadgeStore::Refresh() {
  static const std::filesystem::path database = DatabasePath();
  if (database.empty()) return false;
  std::filesystem::path wal = database;
  wal += L"-wal";
  const ULONGLONG stamp = std::max(LastWrite(database), LastWrite(wal));
  if (stamp == stamp_) return false;
  stamp_ = stamp;
  auto badges = ReadBadges(database);
  if (badges == badges_) return false;
  badges_ = std::move(badges);
  return true;
}

std::optional<Badge> BadgeStore::For(const std::wstring& appId) const {
  if (appId.empty()) return std::nullopt;
  const auto it = badges_.find(Lower(appId));
  if (it == badges_.end()) return std::nullopt;
  return it->second;
}

}  // namespace throwback::shell
