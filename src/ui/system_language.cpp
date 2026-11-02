#include <windows.h>

#include "ui/strings.h"

namespace throwback::ui {

Language SystemLanguage() {
  return PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_TURKISH ? Language::Turkish
                                                                   : Language::English;
}

}  // namespace throwback::ui
