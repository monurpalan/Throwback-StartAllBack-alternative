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
    {StringId::SearchPlaceholder, L"Search programs and settings", L"Programları ve ayarları ara"},
    {StringId::NoResults, L"No items match your search.", L"Aramanızla eşleşen öğe yok."},
    {StringId::ShutDown, L"Shut down", L"Bilgisayarı kapat"},
    {StringId::Restart, L"Restart", L"Yeniden başlat"},
    {StringId::Sleep, L"Sleep", L"Uyku"},
    {StringId::SignOut, L"Sign out", L"Oturumu kapat"},
    {StringId::Lock, L"Lock", L"Kilitle"},
    {StringId::PowerOptions, L"Power options", L"Güç seçenekleri"},
    // The shell names the Run dialog in English on every Windows language.
    {StringId::Run, L"Run...", L"Çalıştır..."},
    {StringId::SettingsDisplay, L"Display", L"Ekran"},
    {StringId::SettingsNightLight, L"Night light", L"Gece ışığı"},
    {StringId::SettingsSound, L"Sound", L"Ses"},
    {StringId::SettingsNotifications, L"Notifications", L"Bildirimler"},
    {StringId::SettingsPower, L"Power & battery", L"Güç ve pil"},
    {StringId::SettingsStorage, L"Storage", L"Depolama"},
    {StringId::SettingsMultitasking, L"Multitasking", L"Çoklu görev"},
    {StringId::SettingsClipboard, L"Clipboard", L"Pano"},
    {StringId::SettingsAbout, L"About", L"Hakkında"},
    {StringId::SettingsBluetooth, L"Bluetooth & devices", L"Bluetooth ve cihazlar"},
    {StringId::SettingsPrinters, L"Printers & scanners", L"Yazıcılar ve tarayıcılar"},
    {StringId::SettingsMouse, L"Mouse", L"Fare"},
    {StringId::SettingsTouchpad, L"Touchpad", L"Dokunmatik yüzey"},
    {StringId::SettingsTyping, L"Typing", L"Yazma"},
    {StringId::SettingsNetwork, L"Network & internet", L"Ağ ve internet"},
    {StringId::SettingsWifi, L"Wi-Fi", L"Wi-Fi"},
    {StringId::SettingsVpn, L"VPN", L"VPN"},
    {StringId::SettingsProxy, L"Proxy", L"Ara sunucu"},
    {StringId::SettingsBackground, L"Background", L"Arka plan"},
    {StringId::SettingsColors, L"Colors", L"Renkler"},
    {StringId::SettingsThemes, L"Themes", L"Temalar"},
    {StringId::SettingsLockScreen, L"Lock screen", L"Kilit ekranı"},
    {StringId::SettingsTaskbar, L"Taskbar", L"Görev çubuğu"},
    {StringId::SettingsInstalledApps, L"Installed apps", L"Yüklü uygulamalar"},
    {StringId::SettingsDefaultApps, L"Default apps", L"Varsayılan uygulamalar"},
    {StringId::SettingsStartupApps, L"Startup apps", L"Başlangıç uygulamaları"},
    {StringId::SettingsYourInfo, L"Your info", L"Bilgileriniz"},
    {StringId::SettingsSignIn, L"Sign-in options", L"Oturum açma seçenekleri"},
    {StringId::SettingsDateTime, L"Date & time", L"Tarih ve saat"},
    {StringId::SettingsLanguage, L"Language & region", L"Dil ve bölge"},
    {StringId::SettingsGameMode, L"Game Mode", L"Oyun Modu"},
    {StringId::SettingsAccessibility, L"Accessibility", L"Erişilebilirlik"},
    {StringId::SettingsPrivacy, L"Privacy & security", L"Gizlilik ve güvenlik"},
    {StringId::SettingsWindowsUpdate, L"Windows Update", L"Windows Güncelleştirme"},
    {StringId::SettingsRecovery, L"Recovery", L"Kurtarma"},
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
