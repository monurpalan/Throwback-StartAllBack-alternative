# Architecture

Throwback is a single process that draws its own bar. It does not modify or
inject into `explorer.exe` (see [ADR 0001](adr/0001-standalone-appbar.md)).

## Components

```
            shell hook / timer
 Windows ───────────────────────► WindowTracker ──► TaskItem list
    ▲                                                   │
    │ SHAppBarMessage                                   ▼
  AppBar ◄──── TaskbarWindow ────► ComputeLayout ──► BarLayout
                    │                                   │
                    └──────────► DrawBar ──► ui::Canvas (Direct2D)
```

| Component | Responsibility |
|---|---|
| `TaskbarWindow` | Owns the window and message loop handling; wires the rest together. |
| `AppBar` | Registers the window as a shell AppBar so maximized windows leave room for it. |
| `WindowTracker` | Enumerates top-level windows, keeps a stable button order, caches icons. |
| `DrawBar` | Turns a `BarModel` into UI kit calls; contains no drawing code of its own. |
| `ui::Canvas` | The UI kit: buttons, text and icons on a Direct2D surface; recovers from device loss. |
| `ui::Theme` | Three-color palette, spacing scale, type scale, light and dark themes. |
| `ui::Fonts` | Loads the bundled IBM Plex fonts with DirectWrite, falls back to system fonts. |
| `core/*` | Pure, Win32-free logic: layout math, taskbar eligibility rules, options. |

## Design system

- **Color:** three colors only: ink `#1C1C1F`, paper `#F2F2F0`, accent `#4D9EFF`.
  Dark mode puts paper on ink, light mode ink on paper. Hover and selection
  states are the foreground color with transparency.
- **Spacing:** multiples of 4 px (4, 8, 12, 16, 24, 32, 40, 56) at 96 DPI.
- **Type:** 18 / 15 / 13 / 11 px. IBM Plex Sans for text, IBM Plex Mono for
  numbers and labels. Square corners.
- Screens only draw through `ui::Canvas`, whose components accept fixed
  variants (`Ghost`, `Selected`), never raw colors or sizes.

## Design principles

- **Logic is testable without a desktop.** Anything that is a decision rather
  than a system call lives in `src/core` and has unit tests. The Win32 layer
  only collects facts (`WindowTraits`) and applies results (`BarLayout`).
- **Never hang on other apps.** Calls into other processes (`WM_GETICON`) use
  `SendMessageTimeout` with `SMTO_ABORTIFHUNG`.
- **Leave the system as we found it.** The AppBar is removed on exit, so the
  reserved screen space is returned even when the user just kills the process
  politely.
- **Event driven with a safety net.** The shell hook (`RegisterShellHookWindow`)
  reports window changes immediately; a 1 s timer catches anything it misses
  and drives the clock.

## Known limitations

- Primary monitor only.
- UWP apps hosted in `ApplicationFrameWindow` show a generic icon.
- The system taskbar stays visible; Throwback docks next to it.
