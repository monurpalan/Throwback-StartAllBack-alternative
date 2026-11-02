#include "core/user_assist.h"

#include <algorithm>
#include <array>
#include <cstring>

#include "core/search.h"

namespace throwback {
namespace {

constexpr std::size_t kValueSize = 72;
constexpr std::size_t kRunCountOffset = 4;
constexpr std::size_t kLastRunOffset = 60;

template <typename T>
T ReadLittleEndian(std::span<const std::uint8_t> data, std::size_t offset) {
  T value{};
  std::memcpy(&value, data.data() + offset, sizeof(T));
  return value;
}

}  // namespace

std::wstring Rot13(std::wstring_view text) {
  std::wstring result(text);
  for (wchar_t& c : result) {
    if (c >= L'a' && c <= L'z') {
      c = static_cast<wchar_t>(L'a' + (c - L'a' + 13) % 26);
    } else if (c >= L'A' && c <= L'Z') {
      c = static_cast<wchar_t>(L'A' + (c - L'A' + 13) % 26);
    }
  }
  return result;
}

std::optional<UsageEntry> ParseUsageValue(std::wstring_view encodedName,
                                          std::span<const std::uint8_t> data) {
  if (data.size() < kValueSize) return std::nullopt;
  std::wstring appId = Rot13(encodedName);
  // Bookkeeping values such as "UEME_CTLSESSION" are not programs.
  if (appId.starts_with(L"UEME_")) return std::nullopt;
  const auto runCount = ReadLittleEndian<std::uint32_t>(data, kRunCountOffset);
  if (runCount == 0) return std::nullopt;
  return UsageEntry{std::move(appId), runCount,
                    ReadLittleEndian<std::uint64_t>(data, kLastRunOffset)};
}

bool IsJunkProgram(std::wstring_view appId) {
  static constexpr std::array<std::wstring_view, 10> kJunk = {
      L"setup",    L"install", L"readme",      L"help",        L"msiexec.exe",
      L"rundll32", L"mmc.exe", L"consent.exe", L"dllhost.exe", L"werfault.exe",
  };
  const std::wstring folded = FoldForSearch(appId);
  return std::any_of(kJunk.begin(), kJunk.end(), [&folded](std::wstring_view junk) {
    return folded.find(junk) != std::wstring::npos;
  });
}

std::vector<UsageEntry> RankFrequent(std::vector<UsageEntry> entries, std::size_t limit) {
  std::erase_if(entries, [](const UsageEntry& entry) { return IsJunkProgram(entry.appId); });
  std::sort(entries.begin(), entries.end(), [](const UsageEntry& a, const UsageEntry& b) {
    if (a.runCount != b.runCount) return a.runCount > b.runCount;
    return a.lastRun > b.lastRun;
  });
  if (entries.size() > limit) entries.resize(limit);
  return entries;
}

}  // namespace throwback
