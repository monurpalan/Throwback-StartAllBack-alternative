#include "ui/strings.h"

#include <array>

namespace throwback::ui {
namespace {

constexpr std::array<Translation, kStringCount> kTable = {{
    {StringId::ExitThrowback, L"Exit Throwback", L"Throwback'ten çık"},
    {StringId::UnknownArgument, L"Unknown argument: ", L"Bilinmeyen argüman: "},
    {StringId::Usage,
     L"Usage: throwback.exe [--edge=top|bottom] [--theme=system|light|dark] "
     L"[--lang=system|en|tr] [--profile=<name>]",
     L"Kullanım: throwback.exe [--edge=top|bottom] [--theme=system|light|dark] "
     L"[--lang=system|en|tr] [--profile=<ad>]"},
    {StringId::AllPrograms, L"All Programs", L"Tüm Programlar"},
    {StringId::Back, L"Back", L"Geri"},
    {StringId::SearchPlaceholder, L"Search programs", L"Programlarda ara"},
    {StringId::NoResults, L"No items match your search.", L"Aramanızla eşleşen öğe yok."},
    {StringId::ShutDown, L"Shut down", L"Bilgisayarı kapat"},
    {StringId::Restart, L"Restart", L"Yeniden başlat"},
    {StringId::Sleep, L"Sleep", L"Uyku"},
    {StringId::SignOut, L"Sign out", L"Oturumu kapat"},
    {StringId::Lock, L"Lock", L"Kilitle"},
    {StringId::PowerOptions, L"Power options", L"Güç seçenekleri"},
    // The shell names the Run dialog in English on every Windows language.
    {StringId::Run, L"Run...", L"Çalıştır..."},
}};

// A missing or misplaced translation fails the build, not the user.
constexpr bool TableIsComplete() {
  for (std::size_t i = 0; i < kTable.size(); ++i) {
    if (static_cast<std::size_t>(kTable[i].id) != i) return false;
    if (kTable[i].english.empty() || kTable[i].turkish.empty()) return false;
  }
  return true;
}
static_assert(TableIsComplete(), "every string needs an English and a Turkish text, in order");

Language g_language = Language::English;

}  // namespace

const Translation* Translations() { return kTable.data(); }

void SetLanguage(Language language) { g_language = language; }

Language CurrentLanguage() { return g_language; }

std::wstring_view Tr(StringId id) {
  const Translation& entry = kTable[static_cast<std::size_t>(id)];
  return g_language == Language::Turkish ? entry.turkish : entry.english;
}

}  // namespace throwback::ui
