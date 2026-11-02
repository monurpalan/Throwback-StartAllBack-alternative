#pragma once

#include <windows.h>

#include <cstdio>
#include <stdexcept>

namespace throwback::ui {

inline void ThrowIfFailed(HRESULT hr, const char* what) {
  if (SUCCEEDED(hr)) return;
  char message[160];
  std::snprintf(message, sizeof(message), "%s failed (0x%08lX)", what,
                static_cast<unsigned long>(hr));
  throw std::runtime_error(message);
}

}  // namespace throwback::ui
