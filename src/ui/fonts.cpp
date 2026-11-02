#include "ui/fonts.h"

#include <windows.h>
#include <dwrite_3.h>

#include <filesystem>
#include <initializer_list>

#include "ui/com_util.h"

using Microsoft::WRL::ComPtr;

namespace throwback::ui {
namespace {

constexpr wchar_t kSansFile[] = L"IBMPlexSans-Regular.ttf";
constexpr wchar_t kMonoFile[] = L"IBMPlexMono-Regular.ttf";

std::filesystem::path FontDirectory() {
  wchar_t module[MAX_PATH] = {};
  GetModuleFileNameW(nullptr, module, MAX_PATH);
  return std::filesystem::path(module).parent_path() / L"fonts";
}

ComPtr<IDWriteFontCollection> LoadBundledFonts(IDWriteFactory* factory) {
  ComPtr<IDWriteFactory5> factory5;
  ComPtr<IDWriteFontSetBuilder1> builder;
  if (FAILED(factory->QueryInterface(IID_PPV_ARGS(factory5.GetAddressOf()))) ||
      FAILED(factory5->CreateFontSetBuilder(builder.GetAddressOf()))) {
    return nullptr;
  }

  const std::filesystem::path directory = FontDirectory();
  for (const wchar_t* name : {kSansFile, kMonoFile}) {
    const std::filesystem::path path = directory / name;
    std::error_code error;
    if (!std::filesystem::exists(path, error)) return nullptr;

    ComPtr<IDWriteFontFile> file;
    if (FAILED(factory5->CreateFontFileReference(path.c_str(), nullptr, file.GetAddressOf())) ||
        FAILED(builder->AddFontFile(file.Get()))) {
      return nullptr;
    }
  }

  ComPtr<IDWriteFontSet> fontSet;
  ComPtr<IDWriteFontCollection1> collection;
  if (FAILED(builder->CreateFontSet(fontSet.GetAddressOf())) ||
      FAILED(factory5->CreateFontCollectionFromFontSet(fontSet.Get(), collection.GetAddressOf()))) {
    return nullptr;
  }
  return collection;
}

}  // namespace

Fonts::Fonts(IDWriteFactory* factory) : factory_(factory), collection_(LoadBundledFonts(factory)) {}

ComPtr<IDWriteTextFormat> Fonts::CreateFormat(TextStyle style, TextAlign align,
                                              unsigned dpi) const {
  const bool bundled = collection_ != nullptr;
  const wchar_t* family = IsMono(style) ? (bundled ? L"IBM Plex Mono" : L"Consolas")
                                        : (bundled ? L"IBM Plex Sans" : L"Segoe UI");
  const float size = FontSize(style) * static_cast<float>(dpi) / 96.0f;

  wchar_t locale[LOCALE_NAME_MAX_LENGTH] = L"en-us";
  GetUserDefaultLocaleName(locale, LOCALE_NAME_MAX_LENGTH);

  ComPtr<IDWriteTextFormat> format;
  ThrowIfFailed(factory_->CreateTextFormat(family, collection_.Get(), DWRITE_FONT_WEIGHT_NORMAL,
                                           DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                           size, locale, format.GetAddressOf()),
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
