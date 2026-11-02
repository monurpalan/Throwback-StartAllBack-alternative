# Throwback

A classic, no-nonsense taskbar for Windows 11, written from scratch in modern C++.

Windows 11 removed things many people relied on: never-combined taskbar buttons
with labels, moving the taskbar to the top of the screen, a predictable start
menu. Throwback brings them back as a standalone shell component, without
patching or injecting into Explorer.

> **Status:** early prototype. Expect rough edges.

## Features

- One button per window with icon and title, never combined
- Dock to the top or bottom edge (`--edge=top` / `--edge=bottom`)
- Click to activate, click the active button to minimize
- Right-click a button to close the window
- Clock and date, follows your locale
- Light and dark themes, following Windows or forced with `--theme=light` / `--theme=dark`
- Per-monitor DPI aware, reserves screen space like the real taskbar
- Steps aside for fullscreen apps

## Building

Requirements: Visual Studio 2022 (or Build Tools) with the C++ workload,
Windows SDK 10.0.22621 or newer, CMake 3.25+.

```bash
cmake --preset msvc
cmake --build --preset debug
ctest --preset debug
```

The binary ends up at `build/msvc/Debug/throwback.exe`.

## Project layout

```
src/core      Pure logic (layout, window rules, options), no Win32, unit tested
src/ui        UI kit: theme, fonts and the components every screen draws with
src/taskbar   The bar window, AppBar docking, window tracking
assets/fonts  Bundled IBM Plex fonts
tests         GoogleTest unit tests
docs          Architecture notes and decision records
```

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for how the pieces fit together.

## License

[MIT](LICENSE). The bundled IBM Plex fonts are licensed under the
[SIL Open Font License 1.1](assets/fonts/OFL.txt).
