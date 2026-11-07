#pragma once

#include <dwrite.h>
#include <wrl/client.h>

#include "ui/theme.h"

namespace throwback::ui {

enum class TextAlign { Leading, Center };

// Creates the text formats of each style: Segoe UI, the Windows 7 system
// font, and the classic fonts of older styles.
class Fonts {
 public:
  explicit Fonts(IDWriteFactory* factory) : factory_(factory) {}

  Microsoft::WRL::ComPtr<IDWriteTextFormat> CreateFormat(TextStyle style, TextAlign align,
                                                         unsigned dpi) const;

 private:
  Microsoft::WRL::ComPtr<IDWriteFactory> factory_;
};

}  // namespace throwback::ui
