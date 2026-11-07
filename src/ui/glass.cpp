#include "ui/glass.h"

#include <dwmapi.h>

namespace throwback::ui {
namespace {

// SetWindowCompositionAttribute is exported by user32 but not declared in the
// SDK; these are the documented-by-use shapes of its arguments.
enum AccentState : int { kAccentEnableBlurBehind = 3 };

struct AccentPolicy {
  AccentState state;
  int flags;
  unsigned gradientColor;
  int animationId;
};

struct CompositionData {
  int attribute;  // 19: WCA_ACCENT_POLICY
  void* data;
  SIZE_T size;
};

using SetCompositionFn = BOOL(WINAPI*)(HWND, CompositionData*);

void EnableBlurBehind(HWND hwnd) {
  static const auto setComposition = reinterpret_cast<SetCompositionFn>(
      GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"));
  if (!setComposition) return;
  AccentPolicy policy{kAccentEnableBlurBehind, 0, 0, 0};
  CompositionData data{19, &policy, sizeof(policy)};
  setComposition(hwnd, &data);
}

}  // namespace

void EnableGlass(HWND hwnd, GlassCorners corners) {
  // A frame extended over the whole window makes DWM honor the alpha the
  // canvas draws with.
  const MARGINS margins{-1, -1, -1, -1};
  DwmExtendFrameIntoClientArea(hwnd, &margins);
  EnableBlurBehind(hwnd);

  DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_DONOTROUND;
  if (corners == GlassCorners::Round) preference = DWMWCP_ROUND;
  if (corners == GlassCorners::RoundSmall) preference = DWMWCP_ROUNDSMALL;
  DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
}

void EnableTransparency(HWND hwnd) {
  const MARGINS margins{-1, -1, -1, -1};
  DwmExtendFrameIntoClientArea(hwnd, &margins);
  DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_DONOTROUND;
  DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
}

}  // namespace throwback::ui
