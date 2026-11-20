#pragma once

namespace throwback {

// The speaker glyph to show: 0 for muted or silent, then 1 to 3 waves by
// thirds of the range, as Windows' volume icon does.
int VolumeGlyphStep(float level, bool muted);

// The volume flyout's slider runs from top (full) to bottom (silent).
float LevelAtY(int y, int top, int bottom);
int ThumbY(float level, int top, int bottom);

// One wheel notch (120) moves the volume 2 %, as on Windows.
float WheelLevel(float level, int wheelDelta);

// The level as the whole percent the flyout shows.
int VolumePercent(float level);

}  // namespace throwback
