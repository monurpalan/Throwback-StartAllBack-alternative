# Throwback

A classic, no-nonsense taskbar for Windows 11, written from scratch in modern C++.

Windows 11 removed things many people relied on: never-combined taskbar buttons
with labels, moving the taskbar to the top of the screen, a predictable start
menu. Throwback brings them back as a standalone shell component, without
patching or injecting into Explorer.

> **Status:** early prototype. Expect rough edges.

## Features

**Taskbar**

- One button per window with icon and title, never combined
- Dock to the top or bottom edge (`--edge=top` / `--edge=bottom`)
- Click to activate, click the active button to minimize
- Live window previews on hover, the classic window menu on right-click
- Flashing buttons for windows that need attention
- Progress and overlay icons that programs set on their buttons
- The notification area with every program's icons, clicks passed through
- Show desktop button, clock and date in your locale
- A bar on every monitor, showing the windows on that monitor
- Optionally hides Explorer's taskbar while running, and brings it back after

**Start menu**

- The Windows 7 layout: frequent programs, All Programs, search, places and power
- Pinned programs, a right-click menu (run as administrator, open file location, ...)
  and jump lists with each program's recent documents
- Opens with the Start button, the Windows key or Ctrl+Esc; Win+shortcuts keep working
- Search that ignores case and accents, including Turkish letters, and finds
  Settings pages too

**Everywhere**

- The Windows 7 Aero look: blurred glass, the start orb, glossy buttons, Segoe UI
- Native menus follow the Windows light/dark setting or `--theme=light` / `--theme=dark`
- English and Turkish, following the Windows display language or forced with `--lang=en` / `--lang=tr`
- Taskbar and Start Menu Properties (right-click the bar) saves settings per
  profile; `--profile=<name>` picks one, so tests never touch your settings
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

To check the running app end to end (every theme and language, with screenshots in `build/smoke`):

```bash
powershell -ExecutionPolicy Bypass -File tools/smoke-test.ps1
```

## Project layout

```
src/core      Pure logic (layout, window rules, options), no Win32, unit tested
src/ui        UI kit: the Aero theme, glass and the components every screen draws with
src/taskbar   The bar window, AppBar docking, window tracking
src/startmenu The start menu: programs, places, search, Windows key hook
src/shell     Shell helpers shared by both: icons, launching
tests         GoogleTest unit tests
docs          Architecture notes and decision records
```

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for how the pieces fit together.

## License

[MIT](LICENSE).
