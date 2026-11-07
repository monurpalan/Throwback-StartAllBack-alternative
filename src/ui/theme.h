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

// The classic look of Windows 98, Me and 2000: raised gray faces with a
// four-line 3D edge, and a blue selection.
namespace classic {

inline constexpr Color kFace98 = FromRgb(0xC0C0C0);
inline constexpr Color kFace2000 = FromRgb(0xD4D0C8);
inline constexpr Color kHighlight = FromRgb(0xFFFFFF);
inline constexpr Color kShadow = FromRgb(0x808080);
inline constexpr Color kDarkShadow98 = FromRgb(0x000000);
inline constexpr Color kDarkShadow2000 = FromRgb(0x404040);
inline constexpr Color kSelection98 = FromRgb(0x000080);
inline constexpr Color kSelection2000 = FromRgb(0x0A246A);
inline constexpr Color kText = FromRgb(0x000000);
inline constexpr Color kSelectedText = FromRgb(0xFFFFFF);
inline constexpr Color kDisabledText = FromRgb(0x808080);
// The start menu banners' top colors, which fill the strip above them.
inline constexpr Color kBannerTop98 = FromRgb(0x000080);
inline constexpr Color kBannerTop = FromRgb(0x000000);

}  // namespace classic

// Windows XP's Luna (Blue), from the theme's NormalBlue.ini.
namespace xp {

inline constexpr Color kProgramText = FromRgb(0x373738);
inline constexpr Color kProgramHot = FromRgb(0x3070D0);
inline constexpr Color kPlaceText = FromRgb(0x0A246A);
inline constexpr Color kPlaceHot = FromRgb(0x5096F8);
inline constexpr Color kLightText = FromRgb(0xFFFFFF);
inline constexpr Color kUserShadow = FromRgb(0x09428B);
inline constexpr Color kStartShadow = FromRgb(0x454C10);
inline constexpr Color kMenuText = FromRgb(0x000000);

}  // namespace xp

// The flat looks of Windows 8, 8.1, 10 and 11, from their default settings.
namespace modern {

// Windows 8 and 8.1: the start screen and its tiles.
inline constexpr Color kScreen = FromRgb(0x180052);
inline constexpr Color kScreenText = FromRgb(0xFFFFFF);
inline constexpr Color kTileGreen = FromRgb(0x00A300);
inline constexpr Color kTileBlue = FromRgb(0x2D89EF);
inline constexpr Color kTileOrange = FromRgb(0xDA532C);
inline constexpr Color kTilePurple = FromRgb(0x603CBA);
inline constexpr Color kTileTeal = FromRgb(0x00ABA9);
inline constexpr Color kTileRed = FromRgb(0xB91D47);
inline constexpr Color kTileHover = FromRgb(0xFFFFFF, 0.18f);

// Windows 10, dark.
inline constexpr Color kWin10Surface = FromRgb(0x1F1F1F, 0.94f);
inline constexpr Color kWin10Hover = FromRgb(0xFFFFFF, 0.12f);
inline constexpr Color kWin10Accent = FromRgb(0x0078D7);
inline constexpr Color kWin10Text = FromRgb(0xFFFFFF);
inline constexpr Color kWin10Secondary = FromRgb(0xC8C8C8);

// Windows 11, dark and light.
inline constexpr Color kWin11DarkSurface = FromRgb(0x202020, 0.97f);
inline constexpr Color kWin11DarkBorder = FromRgb(0x3C3C3C);
inline constexpr Color kWin11DarkBar = FromRgb(0x1C1C1C, 0.97f);
inline constexpr Color kWin11DarkHover = FromRgb(0xFFFFFF, 0.07f);
inline constexpr Color kWin11DarkField = FromRgb(0x2C2C2C);
inline constexpr Color kWin11DarkText = FromRgb(0xFFFFFF);
inline constexpr Color kWin11DarkSecondary = FromRgb(0xC5C5C5);
inline constexpr Color kWin11DarkAccent = FromRgb(0x4CC2FF);
inline constexpr Color kWin11LightSurface = FromRgb(0xF3F3F3, 0.97f);
inline constexpr Color kWin11LightBorder = FromRgb(0xDDDDDD);
inline constexpr Color kWin11LightBar = FromRgb(0xEBEBEB, 0.97f);
inline constexpr Color kWin11LightHover = FromRgb(0x000000, 0.05f);
inline constexpr Color kWin11LightField = FromRgb(0xFFFFFF);
inline constexpr Color kWin11LightText = FromRgb(0x1B1B1B);
inline constexpr Color kWin11LightSecondary = FromRgb(0x5F5F5F);
inline constexpr Color kWin11LightAccent = FromRgb(0x0067C0);

}  // namespace modern

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
  // 8 pt MS Sans Serif of Windows 98 and Me, 8 pt Tahoma of Windows 2000.
  Classic98,
  Classic98Bold,
  Classic2000,
  Classic2000Bold,
  // Windows XP: 8 pt Tahoma, and 14 pt Franklin Gothic Medium for the user
  // name and the Start button.
  XpBody,
  XpBold,
  XpUser,
  XpStart,
  // Windows 8 to 11: Segoe UI (Light for the start screen's titles, the
  // Variable font on Windows 11) and the icon fonts.
  Win8Title,
  Win8Name,
  Win8Tile,
  Win10Body,
  Win10Small,
  Win11Body,
  Win11Small,
  Win11Header,
  GlyphMdl2,
  GlyphFluent,
};

float FontSize(TextStyle style);
bool IsItalic(TextStyle style);
bool IsBold(TextStyle style);
// The DirectWrite weight: light, normal, semibold or bold.
int FontWeightOf(TextStyle style);
const wchar_t* FontFamily(TextStyle style);
// Classic text is drawn without smoothing, as those systems did.
bool IsClassic(TextStyle style);

// Reads the Windows "taskbar and Start" light/dark setting.
bool SystemUsesLightTheme();

}  // namespace throwback::ui
