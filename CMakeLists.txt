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
  src/core/layout.cpp
  src/core/options.cpp
  src/core/task_click.cpp
  src/core/window_filter.cpp
  src/core/window_menu.cpp)
target_include_directories(throwback_core PUBLIC src)
target_link_libraries(throwback_core PRIVATE throwback_options)

# UI kit: theme, fonts and the components every screen draws with.
add_library(throwback_ui STATIC
  src/ui/canvas.cpp
  src/ui/fonts.cpp
  src/ui/icon.cpp
  src/ui/system_theme.cpp
  src/ui/theme.cpp)
target_link_libraries(throwback_ui
  PUBLIC throwback_core d2d1 dwrite windowscodecs
  PRIVATE throwback_options)

add_executable(throwback WIN32
  src/taskbar/app_icons.cpp
  src/taskbar/appbar.cpp
  src/taskbar/bar_view.cpp
  src/taskbar/main.cpp
  src/taskbar/taskbar_window.cpp
  src/taskbar/thumbnail_popup.cpp
  src/taskbar/window_actions.cpp
  src/taskbar/window_tracker.cpp)
target_link_libraries(throwback PRIVATE
  throwback_ui throwback_options dwmapi shell32 ole32)

# Bundled IBM Plex fonts (and their OFL license) ship next to the executable.
add_custom_command(TARGET throwback POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets/fonts $<TARGET_FILE_DIR:throwback>/fonts)

if(THROWBACK_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
