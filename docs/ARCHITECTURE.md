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
  only collects facts (`WindowTraits`, `TaskClickState`) and applies results
  (`BarLayout`, `TaskClickAction`).
- **Never hang on other apps.** Calls into other processes (`WM_GETICON`,
  `SC_RESTORE`) use `SendMessageTimeout` with `SMTO_ABORTIFHUNG`; minimizing is
  posted, never sent.
- **Leave the system as we found it.** The AppBar is removed on exit, so the
  reserved screen space is returned even when the user just kills the process
  politely.
- **Event driven with a safety net.** The shell hook (`RegisterShellHookWindow`)
  reports created, activated and flashing windows; a WinEvent hook on
  cloak/uncloak catches store apps and virtual desktop switches. A 1 s timer
  catches anything both miss and drives the clock.
- **Survive Explorer restarts.** On `TaskbarCreated` the AppBar registers again,
  since Explorer forgets every AppBar when it restarts.

## Window behaviour

- **Which windows get a button:** visible, not cloaked, not a tool window, no
  owner (unless `WS_EX_APPWINDOW`), not removed through
  `ITaskbarList::DeleteTab`. Store app frames need `WS_EX_WINDOWEDGE`, which
  separates them from shell surfaces of the same class.
- **Clicks** are decided from the window state at mouse-down: activate, restore
  a minimized window through the app's own `SC_RESTORE`, or minimize the active
  one. An open modal dialog is activated instead of the window it blocks.
- **Minimize animation** targets our button by answering `HSHELL_GETMINRECT`.
- **Store app icons** come from the window's AppUserModelID via
  `shell:AppsFolder`, because their frame windows carry a generic icon.
- **Previews:** resting on a button shows a live DWM thumbnail
  (`DwmRegisterThumbnail`) after the system hover delay.
- **Window menu:** right-clicking a button shows the classic system menu. Its
  labels are read from a system menu Windows builds for a throwaway window,
  so they are always in the user's Windows language. Native menus follow the
  bar's light or dark theme.
- **Fullscreen apps:** the bar drops below them on `ABN_FULLSCREENAPP` and the
  shell's fullscreen enter/exit notifications, and comes back afterwards.
- **Hung windows:** when Windows swaps a window for its "not responding" ghost,
  the button keeps its place.

## Start menu

A Windows 7 style menu (`src/startmenu`), drawn with the same UI kit:

- **Left:** the most used programs, read from Windows' own usage history
  (UserAssist), so the list is useful from the first run. "All Programs"
  opens the merged Start Menu folders of the user and all users, plus
  packaged apps from `shell:AppsFolder`, folder by folder in place.
- **Search:** typing anywhere filters every program; matching ignores case,
  accents and Turkish dotted/dotless i (`core/search`).
- **Right:** the account picture, the user's folders and system places named
  by the shell in the user's language, and the power button.
- **Windows key:** a low-level keyboard hook on its own thread turns a lone
  Windows key press (and Ctrl+Esc) into "open our menu", by replaying the
  release after an unassigned key so the system menu stays closed. Every
  Win+key shortcut keeps working (`core/win_key`).
- Launches go through `ShellExecuteEx` with usage logging, so Windows keeps
  counting them and the frequent list stays accurate.

## Known limitations

- Primary monitor only.
- The system taskbar stays visible; Throwback docks next to it.
