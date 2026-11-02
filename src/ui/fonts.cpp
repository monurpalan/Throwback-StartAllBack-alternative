#include "ui/fonts.h"

#include <windows.h>

#include "ui/com_util.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {

ComPtr<IDWriteTextFormat> Fonts::CreateFormat(TextStyle style, TextAlign align,
                                              unsigned dpi) const {
  const float size = FontSize(style) * static_cast<float>(dpi) / 96.0f;

  wchar_t locale[LOCALE_NAME_MAX_LENGTH] = L"en-us";
  GetUserDefaultLocaleName(locale, LOCALE_NAME_MAX_LENGTH);

  ComPtr<IDWriteTextFormat> format;
  ThrowIfFailed(factory_->CreateTextFormat(
                    L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
                    IsItalic(style) ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL, size, locale, format.GetAddressOf()),
                "CreateTextFormat");
  format->SetTextAlignment(align == TextAlign::Center ? DWRITE_TEXT_ALIGNMENT_CENTER
                                                      : DWRITE_TEXT_ALIGNMENT_LEADING);
  format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

  const DWRITE_TRIMMING trimming{DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
  ComPtr<IDWriteInlineObject> ellipsis;
  if (SUCCEEDED(factory_->CreateEllipsisTrimmingSign(format.Get(), ellipsis.GetAddressOf()))) {
    format->SetTrimming(&trimming, ellipsis.Get());
  }
  return format;
}

}  // namespace throwback::ui
