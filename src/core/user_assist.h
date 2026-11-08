#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace throwback {

// Windows records program launches under the UserAssist registry key. Value
// names are ROT13-encoded app IDs; the data holds run counts and times.
struct UsageEntry {
  std::wstring appId;
  std::uint32_t runCount = 0;
  std::uint64_t lastRun = 0;  // FILETIME

  bool operator==(const UsageEntry&) const = default;
};

std::wstring Rot13(std::wstring_view text);

// Parses one value of a UserAssist "Count" key (the 72-byte format used
// since Windows 7). Returns nothing for bookkeeping values and bad data.
std::optional<UsageEntry> ParseUsageValue(std::wstring_view encodedName,
                                          std::span<const std::uint8_t> data);

// Installers, uninstallers, readmes and system helpers the classic start
// menu never lists as frequent programs.
bool IsJunkProgram(std::wstring_view appId);

// Whether a Start Menu shortcut counts as newly installed, as Windows 7
// highlights it: created in the last week and not started since. Times are
// FILETIMEs; lastRun is 0 for a shortcut that was never started.
bool IsNewlyInstalled(std::uint64_t created, std::uint64_t now, std::uint64_t lastRun);

// Most used first, most recent breaking ties, junk removed.
std::vector<UsageEntry> RankFrequent(std::vector<UsageEntry> entries, std::size_t limit);

}  // namespace throwback
