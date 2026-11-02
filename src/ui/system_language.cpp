#include <windows.h>

#include "ui/strings.h"

namespace throwback::ui {

Language SystemLanguage() {
  return PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_TURKISH ? Language::Turkish
                                                                   : Language::English;
}

Language LanguageFor(LanguageMode mode) {
  switch (mode) {
    case LanguageMode::English:
      return Language::English;
    case LanguageMode::Turkish:
      return Language::Turkish;
    case LanguageMode::System:
      break;
  }
  return SystemLanguage();
}

}  // namespace throwback::ui
