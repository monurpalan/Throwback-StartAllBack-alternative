#pragma once

#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <memory>

#include "ui/canvas.h"

namespace throwback::ui {

// What an orb shows: one of the embedded Windows orbs, a three-frame
// picture (normal, hot, pressed) of the Windows 7 orb's size, or a vector
// logo drawn into the start button the way StartAllBack draws SVG orbs.
struct OrbArt {
  enum class Kind { Skin, Picture, Logo };
  Kind kind = Kind::Skin;
  SkinImage skin = SkinImage::Orb;  // for Kind::Skin
  int resource = 0;                 // the PNG or SVG resource otherwise
  bool operator==(const OrbArt&) const = default;
};

// The orb in a window of its own, so its glow reaches past the bar the way
// it does on Windows 7. It lets every click through to the bar under it; the
// bar keeps handling the start button.
class OrbWindow {
 public:
  // Null when the Windows 7 orb image isn't embedded; the bar then draws the
  // orb itself.
  static std::unique_ptr<OrbWindow> Create(HINSTANCE instance, HWND owner);

  OrbWindow(const OrbWindow&) = delete;
  OrbWindow& operator=(const OrbWindow&) = delete;
  ~OrbWindow();

  // Centers the orb on a point in screen pixels; a logo fills a frame the
  // size of the start button.
  void Show(POINT center, SIZE button, unsigned dpi, OrbState state, const OrbArt& art);
  void Hide();

 private:
  OrbWindow(HWND hwnd, Microsoft::WRL::ComPtr<IWICImagingFactory> wic);
  // Renders the orb's frames into strip_; false when that fails.
  bool LoadStrip(SIZE button, unsigned dpi, const OrbArt& art);
  // The three frames of an embedded image, scaled to frames of width x height.
  Microsoft::WRL::ComPtr<IWICBitmapSource> ScaledStrip(int resource, int width, int height);
  // The three frames of a logo on the start button's hot and pressed looks.
  Microsoft::WRL::ComPtr<IWICBitmapSource> LogoStrip(int resource, int width, int height,
                                                     unsigned dpi);
  bool CopyStrip(IWICBitmapSource* source, int width, int height);

  HWND hwnd_;
  Microsoft::WRL::ComPtr<IWICImagingFactory> wic_;
  HBITMAP strip_ = nullptr;  // the three states stacked, premultiplied
  unsigned stripDpi_ = 0;
  SIZE stripButton_{};
  OrbArt stripArt_;
  int width_ = 0;  // one frame's size
  int height_ = 0;

  // What is on screen, so unchanged updates are skipped.
  bool shown_ = false;
  POINT center_{};
  OrbState state_ = OrbState::Normal;
};

}  // namespace throwback::ui
