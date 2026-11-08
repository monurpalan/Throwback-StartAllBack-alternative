#include "ui/strings.h"

#include <array>

namespace throwback::ui {
namespace {

constexpr std::array<Translation, kStringCount> kTable = {{
    {StringId::ExitThrowback, L"Exit Throwback", L"Throwback'ten çık"},
    {StringId::UnknownArgument, L"Unknown argument: ", L"Bilinmeyen argüman: "},
    {StringId::Usage,
     L"Usage: throwback.exe [--edge=top|bottom|left|right] [--theme=system|light|dark] "
     L"[--lang=system|en|tr] [--start-style=98|me|2000|xp|vista|7|plain8|8|8.1|10|11] "
     L"[--profile=<name>]",
     L"Kullanım: throwback.exe [--edge=top|bottom|left|right] [--theme=system|light|dark] "
     L"[--lang=system|en|tr] [--start-style=98|me|2000|xp|vista|7|plain8|8|8.1|10|11] "
     L"[--profile=<ad>]"},
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
    {StringId::OpenProgram, L"Open", L"Aç"},
    {StringId::RunAsAdministrator, L"Run as administrator", L"Yönetici olarak çalıştır"},
    {StringId::OpenFileLocation, L"Open file location", L"Dosya konumunu aç"},
    {StringId::PinToStartMenu, L"Pin to Start menu", L"Başlat menüsüne sabitle"},
    {StringId::UnpinFromStartMenu, L"Unpin from Start menu", L"Başlat menüsünden kaldır"},
    {StringId::Properties, L"Properties", L"Özellikler"},
    {StringId::Recent, L"Recent", L"Son"},
    {StringId::CustomizeIcons, L"Customize...", L"Özelleştir..."},
    {StringId::StartStyleLabel, L"Style:", L"Görünüm:"},
    {StringId::ClassicStart, L"Start", L"Başlat"},
    {StringId::ClassicPrograms, L"Programs", L"Programlar"},
    {StringId::ClassicDocuments, L"Documents", L"Belgeler"},
    {StringId::ClassicSettings, L"Settings", L"Ayarlar"},
    {StringId::ClassicFind, L"Find", L"Bul"},
    {StringId::ClassicSearch, L"Search", L"Ara"},
    {StringId::ClassicHelp, L"Help", L"Yardım"},
    {StringId::ClassicLogOff, L"Log Off %s...", L"%s Oturumunu Kapat..."},
    {StringId::ClassicShutDown, L"Shut Down...", L"Kapat..."},
    {StringId::ClassicControlPanel, L"Control Panel", L"Denetim Masası"},
    {StringId::ClassicPrinters, L"Printers", L"Yazıcılar"},
    {StringId::ClassicTaskbar, L"Taskbar & Start Menu...", L"Görev Çubuğu ve Başlat Menüsü..."},
    {StringId::ClassicFilesOrFolders, L"Files or Folders...", L"Dosya veya Klasörler..."},
    {StringId::ClassicOnTheInternet, L"On the Internet...", L"Internet'te..."},
    {StringId::ClassicEmpty, L"(Empty)", L"(Boş)"},
    {StringId::XpStart, L"start", L"başlat"},
    {StringId::XpMyDocuments, L"My Documents", L"Belgelerim"},
    {StringId::XpMyRecentDocuments, L"My Recent Documents", L"Son Kullanılan Belgelerim"},
    {StringId::XpMyPictures, L"My Pictures", L"Resimlerim"},
    {StringId::XpMyMusic, L"My Music", L"Müziğim"},
    {StringId::XpMyComputer, L"My Computer", L"Bilgisayarım"},
    {StringId::XpPrinters, L"Printers and Faxes", L"Yazıcı ve Fakslar"},
    {StringId::XpHelp, L"Help and Support", L"Yardım ve Destek"},
    {StringId::XpLogOff, L"Log Off", L"Oturumu Kapat"},
    {StringId::XpTurnOff, L"Turn Off Computer", L"Bilgisayarı Kapat"},
    {StringId::VistaRecentItems, L"Recent Items", L"Son Öğeler"},
    {StringId::VistaNetwork, L"Network", L"Ağ"},
    {StringId::VistaDefaultPrograms, L"Default Programs", L"Varsayılan Programlar"},
    {StringId::VistaSearchPlaceholder, L"Start Search", L"Aramaya Başla"},
    {StringId::ModernPinned, L"Pinned", L"Sabitlenmiş"},
    {StringId::ModernAllApps, L"All apps", L"Tüm uygulamalar"},
    {StringId::ModernRecommended, L"Recommended", L"Önerilenler"},
    {StringId::ModernMostUsed, L"Most used", L"En çok kullanılanlar"},
    {StringId::ModernSearchPrompt, L"Search for apps, settings, and documents",
     L"Uygulama, ayar ve belge arayın"},
    {StringId::ModernApps, L"Apps", L"Uygulamalar"},
    {StringId::ModernBestMatch, L"Best match", L"En iyi eşleşme"},
    {StringId::ModernToday, L"Today at %s", L"Bugün %s"},
    {StringId::ModernYesterday, L"Yesterday at %s", L"Dün %s"},
    {StringId::SettingsTitle, L"Taskbar and Start Menu Properties",
     L"Görev Çubuğu ve Başlat Menüsü Özellikleri"},
    {StringId::TaskbarAppearance, L"Taskbar appearance", L"Görev çubuğu görünümü"},
    {StringId::TaskbarLocation, L"Taskbar location on screen:",
     L"Görev çubuğunun ekrandaki konumu:"},
    {StringId::LocationTop, L"Top", L"Üst"},
    {StringId::LocationBottom, L"Bottom", L"Alt"},
    {StringId::HideWindowsTaskbar, L"Hide the Windows taskbar while Throwback runs",
     L"Throwback çalışırken Windows görev çubuğunu gizle"},
    {StringId::StartMenuGroup, L"Start menu", L"Başlat menüsü"},
    {StringId::WindowsKeyOpensMenu, L"Open this start menu with the Windows key",
     L"Bu başlat menüsünü Windows tuşuyla aç"},
    {StringId::LanguageAndTheme, L"Language and menus", L"Dil ve menüler"},
    {StringId::LanguageLabel, L"Language:", L"Dil:"},
    {StringId::LanguageSystem, L"Windows display language", L"Windows görüntü dili"},
    {StringId::LanguageEnglish, L"English", L"English"},
    {StringId::LanguageTurkish, L"Türkçe", L"Türkçe"},
    {StringId::MenuTheme, L"Menu colors:", L"Menü renkleri:"},
    {StringId::ThemeSystem, L"Same as Windows", L"Windows ile aynı"},
    {StringId::ThemeLight, L"Light", L"Açık"},
    {StringId::ThemeDark, L"Dark", L"Koyu"},
    {StringId::Ok, L"OK", L"Tamam"},
    {StringId::Cancel, L"Cancel", L"İptal"},
    {StringId::Apply, L"Apply", L"Uygula"},
    {StringId::MenuPositionLabel, L"Menu side:", L"Menü tarafı:"},
    {StringId::MenuLeft, L"Left", L"Sol"},
    {StringId::MenuRight, L"Right", L"Sağ"},
    {StringId::MenuEdgeLabel, L"Menu edge:", L"Menü kenarı:"},
    {StringId::MenuNextToBar, L"Taskbar side", L"Görev çubuğu tarafı"},
    {StringId::MenuScreenTop, L"Top of the screen", L"Ekranın üstü"},
    {StringId::MenuScreenBottom, L"Bottom of the screen", L"Ekranın altı"},
    {StringId::OrbLabel, L"Start button:", L"Başlat düğmesi:"},
    {StringId::OrbStyle, L"Match the style", L"Görünüme göre"},
    {StringId::OrbClover, L"Clover", L"Yonca"},
    {StringId::OrbWin8, L"Windows 8 logo", L"Windows 8 logosu"},
    {StringId::CustomizeTitle, L"Customize Start Menu", L"Başlat Menüsünü Özelleştir"},
    {StringId::MenuRowsLabel, L"Number of recent programs to display:",
     L"Gösterilecek son program sayısı:"},
    {StringId::LargeIcons, L"Use large icons", L"Büyük simgeler kullan"},
    {StringId::PlacesLabel, L"Show in the right column:", L"Sağ sütunda göster:"},
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
    {StringId::StartWithWindows, L"Start Throwback with Windows", L"Windows açılınca başlat"},
    {StringId::MenuStartStyle, L"Start menu style", L"Başlat menüsü görünümü"},
    {StringId::MenuStartButton, L"Start button", L"Başlat düğmesi"},
    {StringId::MenuSide, L"Start menu side", L"Başlat menüsü tarafı"},
    {StringId::MenuCustomizeStart, L"Customize Start menu...", L"Başlat menüsünü özelleştir..."},
    {StringId::ShowSearchButton, L"Show Search button", L"Arama düğmesini göster"},
    {StringId::ShowTaskViewButton, L"Show Task View button", L"Görev Görünümü düğmesini göster"},
    {StringId::ShowTheDesktop, L"Show the desktop", L"Masaüstünü göster"},
    {StringId::TaskManager, L"Task Manager", L"Görev Yöneticisi"},
    {StringId::LockTaskbar, L"Lock the taskbar", L"Görev çubuğunu kilitle"},
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
