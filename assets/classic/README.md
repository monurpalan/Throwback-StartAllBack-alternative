# Classic images

Original images from Windows 98 SE, Windows Me and Windows 2000 for the
classic start menu styles, embedded into `throwback.exe`:

| File | Source |
|---|---|
| `banner-98.png`, `banner-me.png`, `banner-2000.png` | `explorer.exe` bitmaps 157 (98, 2000) and 161 (Me): the start menu banners |
| `flag.png` | `explorer.exe` bitmap 143, the Start button's flag (gray made transparent) |
| `98-*.ico`, `2000-*.ico` | `shell32.dll` icons of Windows 98 SE and 2000, by their ID |

The images belong to Microsoft. Without this folder Throwback still builds;
the classic styles then draw without these images.
