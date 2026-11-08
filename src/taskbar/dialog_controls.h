#pragma once

#include <windows.h>

#include <string_view>

#include "core/layout.h"

namespace throwback {

// What the dialogs built from native controls share: their font and DPI.
struct DialogLook {
  HFONT font = nullptr;
  unsigned dpi = 96;
};

// The message font of Windows at a DPI; the caller deletes it.
HFONT CreateDialogFont(unsigned dpi);

// A child control at rect, given at 96 DPI and scaled for the look's DPI.
HWND CreateDialogControl(HWND parent, const DialogLook& look, const wchar_t* type,
                         std::wstring_view text, DWORD style, int id, const Rect& rect);

// A popup dialog frame around a client area of width x height (at 96 DPI),
// centered on the owner's monitor; params reaches WM_NCCREATE.
HWND CreateDialogFrame(HINSTANCE instance, HWND owner, const wchar_t* className,
                       std::wstring_view title, int width, int height, unsigned dpi, void* params);

// Runs a modal loop for hwnd until done turns true, with owner disabled.
// A WM_QUIT it takes is posted again for the application's main loop.
void RunModalLoop(HWND hwnd, HWND owner, const bool& done);

}  // namespace throwback
