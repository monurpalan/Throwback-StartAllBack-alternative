#pragma once

namespace throwback::ui {

struct Color {
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 1.0f;

  constexpr Color WithAlpha(float alpha) const { return {r, g, b, alpha}; }
  constexpr bool SameRgb(const Color& other) const {
    return r == other.r && g == other.g && b == other.b;
  }
  bool operator==(const Color&) const = default;
};

constexpr Color FromRgb(unsigned rgb) {
  return {static_cast<float>((rgb >> 16) & 0xFF) / 255.0f,
          static_cast<float>((rgb >> 8) & 0xFF) / 255.0f, static_cast<float>(rgb & 0xFF) / 255.0f,
          1.0f};
}

// The design has exactly three colors. Every other tone is one of them with
// transparency applied; do not add new ones.
namespace palette {
inline constexpr Color kInk = FromRgb(0x1C1C1F);
inline constexpr Color kPaper = FromRgb(0xF2F2F0);
inline constexpr Color kAccent = FromRgb(0x4D9EFF);
}  // namespace palette

// Spacing scale in pixels at 96 DPI. Only these values are used for gaps,
// padding and insets.
inline constexpr int kSpace4 = 4;
inline constexpr int kSpace8 = 8;
inline constexpr int kSpace12 = 12;
inline constexpr int kSpace16 = 16;
inline constexpr int kSpace24 = 24;
inline constexpr int kSpace32 = 32;
inline constexpr int kSpace40 = 40;
inline constexpr int kSpace56 = 56;

// Type scale in pixels at 96 DPI. Only these four sizes exist.
inline constexpr float kTitleSize = 18.0f;
inline constexpr float kSubtitleSize = 15.0f;
inline constexpr float kBodySize = 13.0f;
inline constexpr float kSmallSize = 11.0f;

enum class TextStyle {
  Title,
  Subtitle,
  Body,
  Small,
  Label,        // small, mono, uppercase
  Number,       // body size, mono
  SmallNumber,  // small size, mono
};

float FontSize(TextStyle style);
bool IsMono(TextStyle style);
bool IsUppercase(TextStyle style);

// Semantic roles; screens never pick colors directly.
struct Theme {
  Color background;
  Color foreground;
  Color hover;
  Color selected;
  Color selectedHover;
  Color accent;
};

Theme MakeTheme(bool light);

// Reads the Windows "taskbar and Start" light/dark setting.
bool SystemUsesLightTheme();

}  // namespace throwback::ui
