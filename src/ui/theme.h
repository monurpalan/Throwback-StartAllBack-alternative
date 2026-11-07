#pragma once

namespace throwback::ui {

struct Color {
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 1.0f;

  constexpr Color WithAlpha(float alpha) const { return {r, g, b, alpha}; }
  bool operator==(const Color&) const = default;
};

constexpr Color FromRgb(unsigned rgb, float alpha = 1.0f) {
  return {static_cast<float>((rgb >> 16) & 0xFF) / 255.0f,
          static_cast<float>((rgb >> 8) & 0xFF) / 255.0f, static_cast<float>(rgb & 0xFF) / 255.0f,
          alpha};
}

// The Windows 7 Aero look with its default "Sky" glass color. Every color the
// UI kit draws with is defined here and nowhere else.
namespace aero {

// Glass: a dark veil over the blurred desktop, tinted sky blue, with a
// bright rim on the edge that faces the screen.
inline constexpr Color kGlassVeil = FromRgb(0x0C1622, 0.38f);
inline constexpr Color kGlassTint = FromRgb(0x74B8FC, 0.26f);
inline constexpr Color kGlassSheen = FromRgb(0xFFFFFF, 0.14f);
inline constexpr Color kGlassRim = FromRgb(0xFFFFFF, 0.40f);
inline constexpr Color kGlassEdge = FromRgb(0x000000, 0.70f);

// The white program pane of the start menu.
inline constexpr Color kPane = FromRgb(0xFFFFFF);
inline constexpr Color kPaneBorder = FromRgb(0x3C4E66, 0.85f);
inline constexpr Color kPaneSeparator = FromRgb(0xD6E5F5);

// The blue highlight of the item under the mouse on the white pane.
inline constexpr Color kItemBorder = FromRgb(0x7DA2CE);
inline constexpr Color kItemTop = FromRgb(0xDCEBFC);
inline constexpr Color kItemBottom = FromRgb(0xC1DBFC);
inline constexpr Color kItemInner = FromRgb(0xFFFFFF, 0.60f);

// Highlights drawn on glass: start menu places and taskbar buttons.
inline constexpr Color kGlassItemBorder = FromRgb(0x000000, 0.45f);
inline constexpr Color kGlassItemInner = FromRgb(0xFFFFFF, 0.45f);
inline constexpr Color kGlassItemTop = FromRgb(0xFFFFFF, 0.30f);
inline constexpr Color kGlassItemBottom = FromRgb(0xFFFFFF, 0.08f);
inline constexpr Color kTaskTop = FromRgb(0xFFFFFF, 0.14f);
inline constexpr Color kTaskBottom = FromRgb(0xFFFFFF, 0.04f);
inline constexpr Color kTaskInner = FromRgb(0xFFFFFF, 0.22f);
inline constexpr Color kActiveTaskTop = FromRgb(0xFFFFFF, 0.34f);
inline constexpr Color kActiveTaskBottom = FromRgb(0xFFFFFF, 0.16f);
inline constexpr Color kHotGlow = FromRgb(0xC2E4FF, 0.45f);
inline constexpr Color kFlashTop = FromRgb(0xFFC164, 0.95f);
inline constexpr Color kFlashBottom = FromRgb(0xE8780C, 0.90f);

// Progress fills a taskbar button from the left: green while running,
// yellow when paused, red on error.
inline constexpr Color kProgressTop = FromRgb(0x7CE37C, 0.65f);
inline constexpr Color kProgressBottom = FromRgb(0x2E9E2E, 0.65f);
inline constexpr Color kPausedTop = FromRgb(0xF7DC5C, 0.65f);
inline constexpr Color kPausedBottom = FromRgb(0xC9A40E, 0.65f);
inline constexpr Color kErrorTop = FromRgb(0xF27474, 0.65f);
inline constexpr Color kErrorBottom = FromRgb(0xB52A2A, 0.65f);

// The glossy push button (Shut down).
inline constexpr Color kCommandBorder = FromRgb(0x1B2A3D, 0.80f);
inline constexpr Color kCommandGlossTop = FromRgb(0xFBFCFD);
inline constexpr Color kCommandGlossBottom = FromRgb(0xE3E9F0);
inline constexpr Color kCommandBodyTop = FromRgb(0xCCD7E3);
inline constexpr Color kCommandBodyBottom = FromRgb(0xDDE6EF);
inline constexpr Color kCommandHotTop = FromRgb(0xC9E9FB);
inline constexpr Color kCommandHotBottom = FromRgb(0x7CC6F2);

// The search box.
inline constexpr Color kField = FromRgb(0xFFFFFF);
inline constexpr Color kFieldBorder = FromRgb(0x000000, 0.45f);

// Text.
inline constexpr Color kText = FromRgb(0x000000);
inline constexpr Color kHintText = FromRgb(0x6D6D6D);
inline constexpr Color kGlassText = FromRgb(0xFFFFFF);
inline constexpr Color kLink = FromRgb(0x0066CC);
inline constexpr Color kTextShadow = FromRgb(0x000000, 0.55f);

// The start orb: a dark glass sphere with the four-color flag.
inline constexpr Color kOrbRim = FromRgb(0x0B1A2E);
inline constexpr Color kOrbDeep = FromRgb(0x0F2D55);
inline constexpr Color kOrbMiddle = FromRgb(0x1D6BB5);
inline constexpr Color kOrbGlow = FromRgb(0x5CC1FF);
inline constexpr Color kOrbHotGlow = FromRgb(0x9BE3FF);
inline constexpr Color kOrbPressedGlow = FromRgb(0x2E8FE0);
inline constexpr Color kOrbGloss = FromRgb(0xFFFFFF, 0.55f);
inline constexpr Color kFlagRed = FromRgb(0xF35325);
inline constexpr Color kFlagGreen = FromRgb(0x81BC06);
inline constexpr Color kFlagBlue = FromRgb(0x05A6F0);
inline constexpr Color kFlagYellow = FromRgb(0xFFBA08);

}  // namespace aero

// Spacing in pixels at 96 DPI.
inline constexpr int kSpace2 = 2;
inline constexpr int kSpace4 = 4;
inline constexpr int kSpace8 = 8;
inline constexpr int kSpace12 = 12;
inline constexpr int kSpace16 = 16;
inline constexpr int kSpace24 = 24;

// Windows 7 sets nearly everything in 9 pt Segoe UI: 12 px at 96 DPI.
inline constexpr float kBodySize = 12.0f;
inline constexpr float kTitleSize = 20.0f;
inline constexpr float kCornerRadius = 3.0f;

enum class TextStyle {
  Body,
  Hint,   // italic, like the search box prompt
  Title,  // the account initial when there is no picture
};

float FontSize(TextStyle style);
bool IsItalic(TextStyle style);

// Reads the Windows "taskbar and Start" light/dark setting.
bool SystemUsesLightTheme();

}  // namespace throwback::ui
