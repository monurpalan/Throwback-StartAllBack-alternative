#pragma once

#include <cstddef>
#include <string_view>

#include "core/options.h"

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
  Run,
  OpenProgram,
  RunAsAdministrator,
  OpenFileLocation,
  PinToStartMenu,
  UnpinFromStartMenu,
  Properties,
  Recent,
  CustomizeIcons,
  StartStyleLabel,
  ClassicStart,
  ClassicPrograms,
  ClassicDocuments,
  ClassicSettings,
  ClassicFind,
  ClassicSearch,
  ClassicHelp,
  ClassicLogOff,
  ClassicShutDown,
  ClassicControlPanel,
  ClassicPrinters,
  ClassicTaskbar,
  ClassicFilesOrFolders,
  ClassicOnTheInternet,
  ClassicEmpty,
  // The properties dialog.
  SettingsTitle,
  TaskbarAppearance,
  TaskbarLocation,
  LocationTop,
  LocationBottom,
  HideWindowsTaskbar,
  StartMenuGroup,
  WindowsKeyOpensMenu,
  LanguageAndTheme,
  LanguageLabel,
  LanguageSystem,
  LanguageEnglish,
  LanguageTurkish,
  MenuTheme,
  ThemeSystem,
  ThemeLight,
  ThemeDark,
  Ok,
  Cancel,
  Apply,
  // Settings pages found by the start menu search.
  SettingsDisplay,
  SettingsNightLight,
  SettingsSound,
  SettingsNotifications,
  SettingsPower,
  SettingsStorage,
  SettingsMultitasking,
  SettingsClipboard,
  SettingsAbout,
  SettingsBluetooth,
  SettingsPrinters,
  SettingsMouse,
  SettingsTouchpad,
  SettingsTyping,
  SettingsNetwork,
  SettingsWifi,
  SettingsVpn,
  SettingsProxy,
  SettingsBackground,
  SettingsColors,
  SettingsThemes,
  SettingsLockScreen,
  SettingsTaskbar,
  SettingsInstalledApps,
  SettingsDefaultApps,
  SettingsStartupApps,
  SettingsYourInfo,
  SettingsSignIn,
  SettingsDateTime,
  SettingsLanguage,
  SettingsGameMode,
  SettingsAccessibility,
  SettingsPrivacy,
  SettingsWindowsUpdate,
  SettingsRecovery,
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

// The language a setting asks for, resolving "system".
Language LanguageFor(LanguageMode mode);

}  // namespace throwback::ui
