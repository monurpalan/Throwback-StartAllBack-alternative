#include "core/task_progress.h"

#include <algorithm>

namespace throwback {
namespace {

// TBPFLAG values from shobjidl.h, kept here so the logic stays Win32-free.
constexpr unsigned kIndeterminate = 0x1;
constexpr unsigned kNormal = 0x2;
constexpr unsigned kError = 0x4;
constexpr unsigned kPaused = 0x8;
constexpr long long kFullScale = 65534;

}  // namespace

ProgressState ProgressStateFromFlags(unsigned flags) {
  if (flags & kError) return ProgressState::Error;
  if (flags & kPaused) return ProgressState::Paused;
  if (flags & kNormal) return ProgressState::Normal;
  if (flags & kIndeterminate) return ProgressState::Indeterminate;
  return ProgressState::None;
}

float ProgressFraction(long long value) {
  return static_cast<float>(std::clamp(value, 0LL, kFullScale)) / static_cast<float>(kFullScale);
}

}  // namespace throwback
