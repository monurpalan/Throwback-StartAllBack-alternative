#pragma once

#include <string>

#include "core/options.h"

namespace throwback::shell {

// Starting with the user's session. StartAllBack gets this for free by
// loading inside Explorer; a standalone app uses the per-user Run key, which
// needs no administrator rights.
//
// Each profile has its own Run value, so a test profile never replaces the
// user's own entry.
void SetStartWithWindows(const std::wstring& profile, bool enabled);

// Where Explorer's taskbar sits, for the first run to take its place.
Edge SystemTaskbarEdge();

}  // namespace throwback::shell
