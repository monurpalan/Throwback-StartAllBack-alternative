#include "startmenu/settings_pages.h"

#include <array>

namespace throwback {
namespace {

constexpr std::array kPages = std::to_array<SettingsPage>({
    {ui::StringId::SettingsDisplay, L"ms-settings:display",
     L"screen resolution brightness scale monitor çözünürlük parlaklık ölçek monitör"},
    {ui::StringId::SettingsNightLight, L"ms-settings:nightlight", L"blue light mavi ışık"},
    {ui::StringId::SettingsSound, L"ms-settings:sound",
     L"volume audio speaker microphone hoparlör mikrofon"},
    {ui::StringId::SettingsNotifications, L"ms-settings:notifications", L"alerts uyarılar"},
    {ui::StringId::SettingsPower, L"ms-settings:powersleep",
     L"sleep battery energy uyku pil enerji"},
    {ui::StringId::SettingsStorage, L"ms-settings:storagesense", L"disk space drive alan sürücü"},
    {ui::StringId::SettingsMultitasking, L"ms-settings:multitasking",
     L"snap windows desktops pencere masaüstü"},
    {ui::StringId::SettingsClipboard, L"ms-settings:clipboard",
     L"copy paste history kopyala yapıştır geçmiş"},
    {ui::StringId::SettingsAbout, L"ms-settings:about",
     L"pc name computer specs system bilgisayar adı sistem"},
    {ui::StringId::SettingsBluetooth, L"ms-settings:bluetooth", L"devices pair cihaz eşleştir"},
    {ui::StringId::SettingsPrinters, L"ms-settings:printers", L"print scan yazdır tara"},
    {ui::StringId::SettingsMouse, L"ms-settings:mousetouchpad",
     L"pointer cursor scroll işaretçi imleç kaydırma"},
    {ui::StringId::SettingsTouchpad, L"ms-settings:devices-touchpad",
     L"trackpad gestures hareketler"},
    {ui::StringId::SettingsTyping, L"ms-settings:typing",
     L"keyboard autocorrect klavye otomatik düzeltme"},
    {ui::StringId::SettingsNetwork, L"ms-settings:network", L"ethernet connection bağlantı"},
    {ui::StringId::SettingsWifi, L"ms-settings:network-wifi", L"wifi wireless kablosuz"},
    {ui::StringId::SettingsVpn, L"ms-settings:network-vpn", L"private network özel ağ"},
    {ui::StringId::SettingsProxy, L"ms-settings:network-proxy", L"proxy"},
    {ui::StringId::SettingsBackground, L"ms-settings:personalization-background",
     L"wallpaper desktop duvar kağıdı masaüstü"},
    {ui::StringId::SettingsColors, L"ms-settings:personalization-colors",
     L"dark light mode accent koyu açık mod vurgu tema"},
    {ui::StringId::SettingsThemes, L"ms-settings:themes", L"personalize kişiselleştir"},
    {ui::StringId::SettingsLockScreen, L"ms-settings:lockscreen", L"lock kilit"},
    {ui::StringId::SettingsTaskbar, L"ms-settings:taskbar", L"tray tepsi"},
    {ui::StringId::SettingsInstalledApps, L"ms-settings:appsfeatures",
     L"uninstall programs kaldır programlar"},
    {ui::StringId::SettingsDefaultApps, L"ms-settings:defaultapps",
     L"open with browser birlikte aç tarayıcı"},
    {ui::StringId::SettingsStartupApps, L"ms-settings:startupapps",
     L"autostart boot otomatik başlat açılış"},
    {ui::StringId::SettingsYourInfo, L"ms-settings:yourinfo", L"account picture hesap resim"},
    {ui::StringId::SettingsSignIn, L"ms-settings:signinoptions",
     L"password pin hello şifre parola"},
    {ui::StringId::SettingsDateTime, L"ms-settings:dateandtime", L"clock timezone saat dilimi"},
    {ui::StringId::SettingsLanguage, L"ms-settings:regionlanguage",
     L"locale format yerel ayar biçim"},
    {ui::StringId::SettingsGameMode, L"ms-settings:gaming-gamemode", L"games oyun"},
    {ui::StringId::SettingsAccessibility, L"ms-settings:easeofaccess",
     L"narrator magnifier contrast ekran okuyucu büyüteç karşıtlık"},
    {ui::StringId::SettingsPrivacy, L"ms-settings:privacy",
     L"permissions location camera izinler konum kamera"},
    {ui::StringId::SettingsWindowsUpdate, L"ms-settings:windowsupdate",
     L"update updates güncelleme güncelleştirme"},
    {ui::StringId::SettingsRecovery, L"ms-settings:recovery", L"reset restore sıfırla geri yükle"},
});

}  // namespace

std::span<const SettingsPage> SettingsPages() { return kPages; }

}  // namespace throwback
