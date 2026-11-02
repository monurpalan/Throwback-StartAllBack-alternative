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
| `BarManager` | One bar per monitor; owns the shared settings and the windows' progress state. |
| `TaskbarWindow` | Owns a bar's window and message handling; wires the rest together. |
| `TrayHost` | Hosts the notification area (see below). |
| `TaskbandHook` | Receives the progress and overlay icons programs set through ITaskbarList3. |
| `SystemTaskbar` | Hides Explorer's taskbars on request and restores them, even after a crash. |
| `SettingsStore` | Settings per profile under `HKCU\Software\Throwback\Profiles`. |
| `AppBar` | Registers the window as a shell AppBar so maximized windows leave room for it. |
| `WindowTracker` | Enumerates top-level windows, keeps a stable button order, caches icons. |
| `DrawBar` | Turns a `BarModel` into UI kit calls; contains no drawing code of its own. |
| `ui::Canvas` | The UI kit: glass, panes, buttons, text, icons and the orb on a Direct2D surface; recovers from device loss. |
| `ui::Theme` | The Windows 7 Aero colors, spacing and type sizes. |
| `ui::EnableGlass` | Makes a window a sheet of blurred glass through DWM. |
| `ui::Fonts` | Segoe UI text formats through DirectWrite. |
| `core/*` | Pure, Win32-free logic: layout math, taskbar eligibility rules, options. |

## Design system

The look is Windows 7 Aero, reproduced as closely as possible and drawn
entirely in code (no bitmaps from Windows):

- **Glass:** the bar, the start menu and previews are blurred glass
  (`SetWindowCompositionAttribute` blur behind, premultiplied-alpha Direct2D
  frames) with Aero's default sky tint, a sheen and a bright rim.
- **Surfaces:** the white program pane with its blue hover highlight, glossy
  taskbar buttons (orange while flashing), the glossy Shut down button, the
  search box and the start orb with its four-color flag.
- **Type:** Segoe UI 9 pt (12 px at 96 DPI) everywhere; white with a dark halo
  on glass, black on the pane, an italic prompt in the search box.
- **Light and dark:** Windows 7 had one Aero look, so the glass is the same in
  both; the Windows setting only themes native menus.
- All colors live in `src/ui/theme.h`. Screens only draw through `ui::Canvas`,
  whose components accept fixed variants (`MenuItem`, `Task`, `Command`...),
  never raw colors or sizes.

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

- **Pinned programs and jump lists:** pins sit above the frequent list; a
  program with recent documents (IApplicationDocumentLists, by app ID) shows
  them in place of the right column while it is pointed at.
- **Settings pages:** search also lists common pages of the Settings app.

## Notification area

Programs talk to "the taskbar" by finding the topmost `Shell_TrayWnd`.
Throwback creates one of its own above Explorer's, so their
`Shell_NotifyIcon` and `SHAppBarMessage` calls come to it first:

- Every message is passed on to Explorer, so Explorer stays complete and
  nothing is lost when Throwback exits or crashes.
- AppBar requests that reply with a rect carry shared memory meant for the
  `Shell_TrayWnd`'s process; Explorer gets a copy of its own and the reply is
  copied back.
- On start, programs are told a taskbar was created so they add their icons
  again. Explorer and Windows' own shell UIs are left out: they rebuild their
  windows on that message and briefly cover the screen.
- Our own lookups of Explorer's taskbar skip our window (`ExplorerTray`).

## Known limitations

- Secondary monitor bars are untested on real hardware (written against a
  single monitor).
- Explorer's own icons (volume, network, battery) are part of its taskbar on
  Windows 11, not notification icons, so they don't show up.
