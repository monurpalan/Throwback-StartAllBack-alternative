#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace throwback {

// The badge an app puts on its taskbar button (WhatsApp's unread count): a
// number, or a glyph such as "newMessage" that the bar shows as a dot.
struct Badge {
  int count = 0;
  bool glyph = false;
  bool operator==(const Badge&) const = default;
};

// Reads a badge notification's XML (<badge value="3"/>). Nothing for
// "none", 0 and anything unreadable.
std::optional<Badge> ParseBadge(std::wstring_view xml);

// What the badge says: the count, capped at "99+"; empty for a glyph.
std::wstring BadgeText(const Badge& badge);

}  // namespace throwback
