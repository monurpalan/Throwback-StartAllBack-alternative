#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace throwback {

// Shell_NotifyIcon calls reach the taskbar as WM_COPYDATA. Handles travel as
// 32-bit values, the same for 32 and 64-bit programs.
struct NotifyIconMessage {
  std::uint32_t command = 0;  // NIM_ADD, NIM_MODIFY, ...
  std::uint32_t window = 0;
  std::uint32_t id = 0;
  std::uint32_t flags = 0;  // NIF_* bits saying which fields are set
  std::uint32_t callbackMessage = 0;
  std::uint32_t icon = 0;
  std::wstring tip;
  std::uint32_t state = 0;
  std::uint32_t stateMask = 0;
  std::uint32_t version = 0;
  std::array<std::uint8_t, 16> guid{};
};

// Reads the data of a WM_COPYDATA with dwData 1; nullopt for anything else.
std::optional<NotifyIconMessage> ParseNotifyIconData(std::span<const std::byte> data);

struct TrayIcon {
  std::uint32_t window = 0;
  std::uint32_t id = 0;
  std::array<std::uint8_t, 16> guid{};  // all zero when the icon has none
  std::uint32_t callbackMessage = 0;
  std::uint32_t icon = 0;
  std::wstring tip;
  bool hidden = false;
  std::uint32_t version = 0;  // NOTIFYICON_VERSION_4 changes the callback format

  bool operator==(const TrayIcon&) const = default;
};

// The notification area's icons in the order they were added. Apply returns
// what Shell_NotifyIcon should return.
class TrayIconList {
 public:
  bool Apply(const NotifyIconMessage& message);
  const std::vector<TrayIcon>& Icons() const { return icons_; }
  // Drops every icon of a window that no longer exists.
  void RemoveWindow(std::uint32_t window);

 private:
  std::vector<TrayIcon>::iterator Find(const NotifyIconMessage& message);
  void Update(TrayIcon& icon, const NotifyIconMessage& message);

  std::vector<TrayIcon> icons_;
};

}  // namespace throwback
