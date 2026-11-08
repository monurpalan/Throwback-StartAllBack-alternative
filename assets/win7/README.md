# Windows 7 images

Original images from Windows 7 (6.1.7601), embedded into `throwback.exe` so
the taskbar and start menu look exactly like the real thing:

| File | Source |
|---|---|
| `orb-*.png` | `explorer.exe`, bitmaps 6801–6804 (normal, hot and pressed, per DPI) |
| `picture-frame-*.png` | `explorer.exe`, bitmaps 7013–7016 |
| everything else | `aero.msstyles` (taskbar, task buttons, start menu parts, notification area) |

The search box glyphs (`search-*`), the All Programs and Back arrows
(`more-arrow-*`, `back-arrow-*`), the power button's menu half and arrows
(`power-arrow.png`, `arrow-*`), the place row highlight (`place-item.png`),
the two separators and the halves of a row with a jump list (`split-*`) were
taken from the copy of the Windows 7 theme that StartAllBack bundles
(`research/startallback/assets/Windows 7.msstyles`, images 510–512, 518,
521, 530–537, 541, 543, 549 and 625–632); they are the same images as in
`aero.msstyles`.

The theme stores its images with premultiplied alpha; these copies were
converted to straight alpha, nothing else was changed.

The images belong to Microsoft. Without this folder Throwback still builds
and draws its own approximation of each part.
