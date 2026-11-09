"""Builds Throwback's logo: four rounded panes like the Windows logo, one per
era, each with that era's own logo on its own colors."""
import sys
from PIL import Image, ImageDraw, ImageFilter

repo = sys.argv[1]
out_dir = sys.argv[2]
S = 1024  # drawn large, scaled down for every icon size


def load(path, box=None):
    im = Image.open(f'{repo}/{path}').convert('RGBA')
    return im.crop(box) if box else im


def vertical_gradient(size, top, bottom):
    w, h = size
    g = Image.new('RGBA', size)
    d = ImageDraw.Draw(g)
    for y in range(h):
        t = y / max(1, h - 1)
        c = tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)) + (255,)
        d.line([(0, y), (w, y)], fill=c)
    return g


def pane(size, top, bottom, logo, logo_size, resample):
    w, h = size
    tile = vertical_gradient(size, top, bottom)
    # A glassy sheen on the upper half and a thin light rim, as on Aero.
    sheen = Image.new('RGBA', size, (0, 0, 0, 0))
    ImageDraw.Draw(sheen).rounded_rectangle([0, 0, w, h // 2], radius=w // 8,
                                            fill=(255, 255, 255, 38))
    tile = Image.alpha_composite(tile, sheen)
    lw = logo_size
    lh = int(logo.height * lw / logo.width)
    if lh > logo_size:
        lh = logo_size
        lw = int(logo.width * lh / logo.height)
    art = logo.resize((lw, lh), resample)
    shadow = Image.new('RGBA', size, (0, 0, 0, 0))
    sx, sy = (w - lw) // 2, (h - lh) // 2
    shadow.paste((0, 0, 0, 90), (sx + w // 60, sy + w // 40), art)
    tile = Image.alpha_composite(tile, shadow.filter(ImageFilter.GaussianBlur(w // 50)))
    tile.alpha_composite(art, (sx, sy))
    mask = Image.new('L', size, 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, w - 1, h - 1], radius=w // 6, fill=255)
    rounded = Image.new('RGBA', size, (0, 0, 0, 0))
    rounded.paste(tile, (0, 0), mask)
    rim = ImageDraw.Draw(rounded)
    rim.rounded_rectangle([1, 1, w - 2, h - 2], radius=w // 6, outline=(255, 255, 255, 110),
                          width=max(2, w // 90))
    return rounded


gap = S // 32
half = (S - gap) // 2
win98 = load('assets/classic/flag.png')
xp = load('assets/xp/xp-flag.png')
orb_strip = load('assets/win7/orb-192.png')
orb = orb_strip.crop((0, 0, orb_strip.width, orb_strip.width))
win11 = load('assets/modern/w11-logo-192.png')

panes = [
    pane((half, half), (0, 150, 150), (0, 96, 104), win98, int(half * 0.62), Image.NEAREST),
    pane((half, half), (76, 150, 230), (24, 84, 196), xp, int(half * 0.66), Image.LANCZOS),
    pane((half, half), (40, 70, 110), (8, 22, 44), orb, int(half * 0.86), Image.LANCZOS),
    pane((half, half), (150, 205, 250), (70, 150, 230), win11, int(half * 0.58), Image.LANCZOS),
]
logo = Image.new('RGBA', (S, S), (0, 0, 0, 0))
for i, p in enumerate(panes):
    logo.alpha_composite(p, ((i % 2) * (half + gap), (i // 2) * (half + gap)))

logo.resize((256, 256), Image.LANCZOS).save(f'{out_dir}/logo-256.png')
logo.resize((512, 512), Image.LANCZOS).save(f'{out_dir}/logo-512.png')
sizes = [(16, 16), (20, 20), (24, 24), (32, 32), (40, 40), (48, 48), (64, 64), (96, 96),
         (128, 128), (256, 256)]
logo.save(f'{out_dir}/throwback.ico', sizes=sizes)
print('ok')
