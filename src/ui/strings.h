#pragma once

#include <cstddef>
#include <string_view>

namespace throwback::ui {

enum class Language { English, Turkish };

// Every user-visible text Throwback writes itself. Names of folders, apps and
// window commands come from Windows in the user's language and are not here.
enum class StringId {
  ExitThrowback,
  UnknownArgument,
  Usage,
  AllPrograms,
  Back,
  SearchPlaceholder,
  NoResults,
  ShutDown,
  Restart,
  Sleep,
  SignOut,
  Lock,
  PowerOptions,
  kCount,
};

inline constexpr std::size_t kStringCount = static_cast<std::size_t>(StringId::kCount);

struct Translation {
  StringId id;
  std::wstring_view english;
  std::wstring_view turkish;
};

// The full table, exposed so tests can check it.
const Translation* Translations();

void SetLanguage(Language language);
Language CurrentLanguage();
std::wstring_view Tr(StringId id);

// The Windows display language mapped to one we support (English otherwise).
Language SystemLanguage();

}  // namespace throwback::ui
