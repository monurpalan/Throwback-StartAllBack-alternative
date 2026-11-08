# Start button orbs

Orbs that StartAllBack ships, offered in Taskbar and Start Menu Properties
for the Windows 7 style menus (Vista, 7 and Plain8):

| File | Source | Drawn as |
|---|---|---|
| `clover.svg` | `research/startallback/assets/clover.svg` | a logo in the middle of the start button |
| `win8-logo.svg` | `research/startallback/assets/w8logo.svg` | a logo in the middle of the start button |
| `e1even.png` | `research/startallback/assets/orbs/e1evenorb-pr.png` | three frames (normal, hot, pressed) scaled to the Windows 7 orb's 54 x 54 px |

The files are unchanged copies. Like StartAllBack, Throwback draws the SVG
logos with Direct2D's own SVG renderer, at 24 px on a 96 DPI screen, and
lights up the button behind them when it is hot or pressed.

The images belong to their authors. Without this folder the orb choice falls
back to the style's own orb.
