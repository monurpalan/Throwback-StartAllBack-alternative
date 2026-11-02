#pragma once

#include <dwrite.h>
#include <wrl/client.h>

#include "ui/theme.h"

namespace throwback::ui {

enum class TextAlign { Leading, Center };

// Creates text formats from the bundled IBM Plex fonts (the fonts folder next
// to the executable). Falls back to Segoe UI / Consolas when they are missing.
class Fonts {
 public:
  explicit Fonts(IDWriteFactory* factory);

  Microsoft::WRL::ComPtr<IDWriteTextFormat> CreateFormat(TextStyle style, TextAlign align,
                                                         unsigned dpi) const;
  bool UsingBundledFonts() const { return collection_ != nullptr; }

 private:
  Microsoft::WRL::ComPtr<IDWriteFactory> factory_;
  Microsoft::WRL::ComPtr<IDWriteFontCollection> collection_;
};

}  // namespace throwback::ui
