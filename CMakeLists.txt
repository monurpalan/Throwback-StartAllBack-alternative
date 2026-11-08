cmake_minimum_required(VERSION 3.25)

project(Throwback
  VERSION 0.1.0
  DESCRIPTION "Classic taskbar and start menu for Windows 11"
  LANGUAGES CXX)

if(NOT WIN32)
  message(FATAL_ERROR "Throwback only builds on Windows.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(THROWBACK_BUILD_TESTS "Build unit tests" ON)

# Shared compiler settings for our own targets (not third-party code).
add_library(throwback_options INTERFACE)
target_compile_definitions(throwback_options INTERFACE
  UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(throwback_options INTERFACE /W4 /WX /permissive- /utf-8)

# Pure logic with no Win32 dependencies: easy to unit test.
add_library(throwback_core STATIC
  src/core/bar_style.cpp
  src/core/layout.cpp
  src/core/monitor_diff.cpp
  src/core/notify_icon.cpp
  src/core/options.cpp
  src/core/program_menu.cpp
  src/core/search.cpp
  src/core/settings.cpp
  src/core/start_menu_layout.cpp
  src/core/task_progress.cpp
  src/core/task_click.cpp
  src/core/tray_layout.cpp
  src/core/user_assist.cpp
  src/core/win_key.cpp
  src/core/window_filter.cpp
  src/core/window_menu.cpp)
target_include_directories(throwback_core PUBLIC src)
target_link_libraries(throwback_core PRIVATE throwback_options)

# UI kit: the Aero theme and the components every screen draws with.
add_library(throwback_ui STATIC
  src/ui/aero.cpp
  src/ui/canvas.cpp
  src/ui/classic.cpp
  src/ui/fonts.cpp
  src/ui/glass.cpp
  src/ui/icon.cpp
  src/ui/modern.cpp
  src/ui/orb_window.cpp
  src/ui/skin.cpp
  src/ui/strings.cpp
  src/ui/system_language.cpp
  src/ui/system_theme.cpp
  src/ui/theme.cpp
  src/ui/xp.cpp)
target_link_libraries(throwback_ui
  PUBLIC throwback_core d2d1 dwrite windowscodecs dwmapi shlwapi
  PRIVATE throwback_options)

add_executable(throwback WIN32
  src/shell/async_icons.cpp
  src/shell/autostart.cpp
  src/shell/launch.cpp
  src/shell/recent_items.cpp
  src/shell/settings_store.cpp
  src/shell/shell_icons.cpp
  src/startmenu/classic_menu.cpp
  src/startmenu/classic_start_menu.cpp
  src/startmenu/frequent.cpp
  src/startmenu/modern_start_menu.cpp
  src/startmenu/modern_view.cpp
  src/startmenu/modern_view_win10.cpp
  src/startmenu/modern_view_win11.cpp
  src/startmenu/places.cpp
  src/startmenu/power.cpp
  src/startmenu/program_menu.cpp
  src/startmenu/programs.cpp
  src/startmenu/settings_pages.cpp
  src/startmenu/start_menu.cpp
  src/startmenu/start_menu_actions.cpp
  src/startmenu/start_menu_factory.cpp
  src/startmenu/start_menu_jump.cpp
  src/startmenu/start_menu_view.cpp
  src/startmenu/start_menu_vista_view.cpp
  src/startmenu/start_menu_xp.cpp
  src/startmenu/start_menu_xp_view.cpp
  src/startmenu/win_key_hook.cpp
  src/taskbar/app_icons.cpp
  src/taskbar/appbar.cpp
  src/taskbar/bar_manager.cpp
  src/taskbar/bar_view.cpp
  src/taskbar/explorer_tray.cpp
  src/taskbar/main.cpp
  src/taskbar/notify_icon_settings.cpp
  src/taskbar/system_taskbar.cpp
  src/taskbar/settings_dialog.cpp
  src/taskbar/dialog_controls.cpp
  src/taskbar/start_menu_customize.cpp
  src/taskbar/taskbar_menu.cpp
  src/taskbar/task_status.cpp
  src/taskbar/taskband_hook.cpp
  src/taskbar/taskbar_settings.cpp
  src/taskbar/taskbar_tray.cpp
  src/taskbar/taskbar_window.cpp
  src/taskbar/thumbnail_popup.cpp
  src/taskbar/tray_host.cpp
  src/taskbar/tray_overflow.cpp
  src/taskbar/window_actions.cpp
  src/taskbar/window_tracker.cpp)
target_link_libraries(throwback PRIVATE
  throwback_ui throwback_options dwmapi shell32 shlwapi ole32 secur32 powrprof)

# The original Windows images (assets/<folder>) are embedded for every folder
# that is there; without one, the UI kit draws its own version of its parts.
set(THROWBACK_ASSET_DIRS "${CMAKE_SOURCE_DIR}/src")
set(THROWBACK_ASSET_DEFINES "")
foreach(folder win7 classic xp vista win8 modern orbs plain8)
  if(EXISTS ${CMAKE_SOURCE_DIR}/assets/${folder})
    list(APPEND THROWBACK_ASSET_DIRS "${CMAKE_SOURCE_DIR}/assets/${folder}")
    string(TOUPPER ${folder} upper)
    list(APPEND THROWBACK_ASSET_DEFINES "THROWBACK_ASSETS_${upper}")
  endif()
endforeach()
target_sources(throwback PRIVATE assets/throwback.rc)
set_source_files_properties(assets/throwback.rc PROPERTIES
  INCLUDE_DIRECTORIES "${THROWBACK_ASSET_DIRS}"
  COMPILE_DEFINITIONS "${THROWBACK_ASSET_DEFINES}")

if(THROWBACK_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
