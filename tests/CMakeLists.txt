include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
  DOWNLOAD_EXTRACT_TIMESTAMP ON)
# Match the MSVC runtime of our own targets.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(throwback_tests
  layout_test.cpp
  monitor_diff_test.cpp
  notify_icon_test.cpp
  options_test.cpp
  program_menu_test.cpp
  search_test.cpp
  settings_test.cpp
  skin_test.cpp
  stable_order_test.cpp
  start_menu_layout_test.cpp
  strings_test.cpp
  task_click_test.cpp
  task_progress_test.cpp
  theme_test.cpp
  tray_layout_test.cpp
  user_assist_test.cpp
  win_key_test.cpp
  window_filter_test.cpp
  window_menu_test.cpp)
target_link_libraries(throwback_tests PRIVATE throwback_ui throwback_options GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(throwback_tests)
