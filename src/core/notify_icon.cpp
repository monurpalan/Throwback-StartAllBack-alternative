#include "core/notify_icon.h"

#include <algorithm>
#include <cstring>

namespace throwback {
namespace {

constexpr std::uint32_t kSignature = 0x34753423;

// Offsets in the copied data: a signature and the command, then the
// NOTIFYICONDATA with 32-bit handles.
constexpr size_t kHeader = 8;
constexpr size_t kSize = kHeader + 0;
constexpr size_t kWindow = kHeader + 4;
constexpr size_t kId = kHeader + 8;
constexpr size_t kFlags = kHeader + 12;
constexpr size_t kCallback = kHeader + 16;
constexpr size_t kIcon = kHeader + 20;
constexpr size_t kTip = kHeader + 24;
constexpr size_t kState = kHeader + 280;
constexpr size_t kStateMask = kHeader + 284;
constexpr size_t kVersion = kHeader + 800;
constexpr size_t kGuid = kHeader + 936;
// Version 1 of the structure had a 64 character tip and ended after it.
constexpr std::uint32_t kVersion1Size = 152;

constexpr std::uint32_t kAdd = 0;
constexpr std::uint32_t kModify = 1;
constexpr std::uint32_t kDelete = 2;
constexpr std::uint32_t kSetVersion = 4;
constexpr std::uint32_t kFlagMessage = 0x1;
constexpr std::uint32_t kFlagIcon = 0x2;
constexpr std::uint32_t kFlagTip = 0x4;
constexpr std::uint32_t kFlagState = 0x8;
constexpr std::uint32_t kFlagGuid = 0x20;
constexpr std::uint32_t kStateHidden = 0x1;

bool Has(std::span<const std::byte> data, size_t offset, size_t size) {
  return offset + size <= data.size();
}

std::uint32_t ReadU32(std::span<const std::byte> data, size_t offset) {
  std::uint32_t value = 0;
  if (Has(data, offset, sizeof(value))) std::memcpy(&value, data.data() + offset, sizeof(value));
  return value;
}

std::wstring ReadText(std::span<const std::byte> data, size_t offset, size_t maxChars) {
  std::wstring text;
  for (size_t i = 0; i < maxChars && Has(data, offset + i * 2, 2); ++i) {
    wchar_t c = 0;
    std::memcpy(&c, data.data() + offset + i * 2, 2);
    if (c == 0) break;
    text.push_back(c);
  }
  return text;
}

bool IsZero(const std::array<std::uint8_t, 16>& guid) {
  return std::ranges::all_of(guid, [](std::uint8_t b) { return b == 0; });
}

}  // namespace

std::optional<NotifyIconMessage> ParseNotifyIconData(std::span<const std::byte> data) {
  if (ReadU32(data, 0) != kSignature || !Has(data, kIcon, 4)) return std::nullopt;
  NotifyIconMessage message;
  message.command = ReadU32(data, 4);
  const std::uint32_t size = ReadU32(data, kSize);
  message.window = ReadU32(data, kWindow);
  message.id = ReadU32(data, kId);
  message.flags = ReadU32(data, kFlags);
  message.callbackMessage = ReadU32(data, kCallback);
  message.icon = ReadU32(data, kIcon);
  const bool version1 = size <= kVersion1Size;
  message.tip = ReadText(data, kTip, version1 ? 64 : 128);
  if (!version1) {
    message.state = ReadU32(data, kState);
    message.stateMask = ReadU32(data, kStateMask);
    message.version = ReadU32(data, kVersion);
    if (Has(data, kGuid, 16)) std::memcpy(message.guid.data(), data.data() + kGuid, 16);
  }
  return message;
}

std::vector<TrayIcon>::iterator TrayIconList::Find(const NotifyIconMessage& message) {
  // An icon is named by its GUID when it has one, else by window and ID.
  const bool byGuid = (message.flags & kFlagGuid) != 0 && !IsZero(message.guid);
  return std::ranges::find_if(icons_, [&](const TrayIcon& icon) {
    if (byGuid) return icon.guid == message.guid;
    return icon.window == message.window && icon.id == message.id;
  });
}

void TrayIconList::Update(TrayIcon& icon, const NotifyIconMessage& message) {
  if (message.flags & kFlagMessage) icon.callbackMessage = message.callbackMessage;
  if (message.flags & kFlagIcon) icon.icon = message.icon;
  if (message.flags & kFlagTip) icon.tip = message.tip;
  if ((message.flags & kFlagState) && (message.stateMask & kStateHidden)) {
    icon.hidden = (message.state & kStateHidden) != 0;
  }
  if (message.flags & kFlagGuid) icon.guid = message.guid;
  // A GUID icon may move to another window.
  icon.window = message.window;
  icon.id = message.id;
}

bool TrayIconList::Apply(const NotifyIconMessage& message) {
  const auto it = Find(message);
  switch (message.command) {
    case kAdd:
      if (it != icons_.end()) return false;
      icons_.emplace_back();
      Update(icons_.back(), message);
      return true;
    case kModify:
      if (it == icons_.end()) return false;
      Update(*it, message);
      return true;
    case kDelete:
      if (it == icons_.end()) return false;
      icons_.erase(it);
      return true;
    case kSetVersion:
      if (it == icons_.end()) return false;
      it->version = message.version;
      return true;
    default:
      return it != icons_.end();  // NIM_SETFOCUS and anything newer
  }
}

void TrayIconList::RemoveWindow(std::uint32_t window) {
  std::erase_if(icons_, [window](const TrayIcon& icon) { return icon.window == window; });
}

}  // namespace throwback
