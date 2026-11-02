#pragma once

#include <dwrite.h>
#include <wrl/client.h>

#include "ui/theme.h"

namespace throwback::ui {

enum class TextAlign { Leading, Center };

// Creates Segoe UI text formats, the Windows 7 system font.
class Fonts {
 public:
  explicit Fonts(IDWriteFactory* factory) : factory_(factory) {}

  Microsoft::WRL::ComPtr<IDWriteTextFormat> CreateFormat(TextStyle style, TextAlign align,
                                                         unsigned dpi) const;

 private:
  Microsoft::WRL::ComPtr<IDWriteFactory> factory_;
};

}  // namespace throwback::ui
