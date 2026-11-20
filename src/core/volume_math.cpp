#include "core/volume_math.h"

#include <algorithm>
#include <cmath>

namespace throwback {
namespace {

float Clamp(float level) { return std::clamp(level, 0.0f, 1.0f); }

}  // namespace

int VolumeGlyphStep(float level, bool muted) {
  if (muted || level <= 0.0f) return 0;
  if (level < 1.0f / 3.0f) return 1;
  return level < 2.0f / 3.0f ? 2 : 3;
}

float LevelAtY(int y, int top, int bottom) {
  if (bottom <= top) return 0.0f;
  return Clamp(static_cast<float>(bottom - y) / static_cast<float>(bottom - top));
}

int ThumbY(float level, int top, int bottom) {
  return bottom - static_cast<int>(std::lround(Clamp(level) * static_cast<float>(bottom - top)));
}

float WheelLevel(float level, int wheelDelta) {
  constexpr float kStep = 0.02f;
  constexpr float kNotch = 120.0f;
  // Whole percents, so repeated steps land on 2, 4, 6 ... as Windows does.
  const float stepped = level + kStep * static_cast<float>(wheelDelta) / kNotch;
  return Clamp(std::round(stepped * 100.0f) / 100.0f);
}

int VolumePercent(float level) { return static_cast<int>(std::lround(Clamp(level) * 100.0f)); }

}  // namespace throwback
