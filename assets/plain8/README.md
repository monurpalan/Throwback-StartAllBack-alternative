# Plain8 images

Plain8 is the flat, Windows 8 looking theme StartAllBack ships for its
Windows 7 start menu. These images come from its theme file
(`research/startallback/assets/Plain8.msstyles`, as extracted to
`theme-plain8-named/`):

| File | Theme part |
|---|---|
| `p8-menu-programs.png`, `p8-menu-search.png`, `p8-menu-places.png`, `p8-menu-jump-list.png`, `p8-menu-power.png` | `StartPanelComposited::StartPanelPriv` PLACESLIST, TOPMATCH, LOGOFFBUTTONS, USERPICTURE, SOFTWAREEXPLORER |
| `p8-program-item.png`, `p8-split-main.png`, `p8-split-arrow.png`, `p8-place-item.png` | `StartPanelPriv` parts 29, 28, 30 and 36 |
| `p8-shut-down.png`, `p8-power-arrow.png`, `p8-arrow-*.png` | parts 37 and 23 (with its per-DPI glyphs) |
| `p8-program-separator.png`, `p8-place-separator.png` | LOGOFF, PREVIEW |
| `p8-more-arrow-*.png`, `p8-back-arrow-*.png` | PROGLIST and part 21, per DPI |
| `p8-taskbar.png`, `p8-task-button.png`, `p8-show-desktop.png`, `p8-tray-*.png` | `Taskbar2Composited::TaskBar`, `Taskband2Composited::TaskBand2` part 5, `ShowDesktop::Button`, `TrayNotifyHoriz(Open)::Button`, `TrayNotify::Toolbar` |

The files carry a `p8-` prefix because the resource compiler finds files by
name across all asset folders, and Windows 7's have the same names.

The theme stores its images with premultiplied alpha; these copies were
converted to straight alpha, nothing else was changed. Stretch edges come
from the theme's SIZINGMARGINS (`src/ui/skin.cpp`, `kPlain8Parts`). Plain8
has no search box images, so the Windows 7 magnifier stays.

The images belong to their authors. Without this folder the Plain8 style
draws with the Windows 7 images.
