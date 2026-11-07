# Windows 7 images

Original images from Windows 7 (6.1.7601), embedded into `throwback.exe` so
the taskbar and start menu look exactly like the real thing:

| File | Source |
|---|---|
| `orb-*.png` | `explorer.exe`, bitmaps 6801–6804 (normal, hot and pressed, per DPI) |
| `picture-frame-*.png` | `explorer.exe`, bitmaps 7013–7016 |
| everything else | `aero.msstyles` (taskbar, task buttons, start menu parts, notification area) |

The theme stores its images with premultiplied alpha; these copies were
converted to straight alpha, nothing else was changed.

The images belong to Microsoft. Without this folder Throwback still builds
and draws its own approximation of each part.
