"""Generates the Gridline OS wallpapers (2880x1800 PNG): neon grids, circuit
traces and light lines with a real glow (lines are blurred and added back).

usage: python make_wallpapers.py <out_dir> [name ...]
requires: pillow
"""
import math
import os
import random
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFilter

W, H = 2880, 1800
BG = (2, 6, 8)
CYAN = (0, 229, 255)
ICE = (141, 244, 255)
ORANGE = (255, 138, 31)
LIGHT_BG = (243, 251, 252)
TEAL = (0, 120, 138)


def canvas(bg=BG):
    return Image.new("RGB", (W, H), bg)


def glow(base, draw_fn, radii=(18, 6), strength=(0.9, 1.0), crisp=True):
    """Draws lines on a black layer, blurs copies of it for the glow and adds everything to base."""
    layer = Image.new("RGB", (W, H), (0, 0, 0))
    draw_fn(ImageDraw.Draw(layer))
    out = base
    for r, k in zip(radii, strength):
        b = layer.filter(ImageFilter.GaussianBlur(r))
        if k != 1:
            b = Image.eval(b, lambda v, k=k: min(255, int(v * k)))
        out = ImageChops.add(out, b)
    if crisp:
        out = ImageChops.add(out, layer)
    return out


def dim(c, k):
    return tuple(int(v * k) for v in c)


def floor_y(j, hy):
    """Screen y of the j-th receding floor line (j = 0 is nearest)."""
    z = j + 0.6
    return H - (H - hy) * (1 / (z * 0.5 + 0.5) - 0.02)


def floor_grid(d, hy, color=CYAN, n=30, rows=22, vx=W / 2, k=1.0):
    for i in range(-n, n + 1):
        d.line([(vx + i * 4, hy), (vx + i * W / 9, H)], fill=dim(color, 0.55 * k), width=2)
    for j in range(rows):
        y = floor_y(j, hy)
        if hy < y < H:
            d.line([(0, y), (W, y)], fill=dim(color, (0.25 + 0.6 * (y - hy) / (H - hy)) * k), width=2)


def horizon(d, hy, color=CYAN):
    d.line([(0, hy), (W, hy)], fill=color, width=4)


def w01_horizon_grid():
    hy = H * 0.6
    img = glow(canvas(), lambda d: (floor_grid(d, hy), horizon(d, hy, ICE)), radii=(40, 10), strength=(1.4, 1.0))
    sky = ImageDraw.Draw(img)
    rnd = random.Random(1)
    for _ in range(220):
        x, y = rnd.randrange(W), rnd.randrange(int(hy - 40))
        sky.point((x, y), fill=dim(ICE, rnd.uniform(0.15, 0.6)))
    return img


def w02_horizon_sunset():
    hy = H * 0.62

    def f(d):
        floor_grid(d, hy)
        horizon(d, hy, ORANGE)
        cx, r = W / 2, 520
        for i in range(0, 60):
            y = hy - i * (r / 60)
            if i % 6 in (4, 5) and i < 40:
                continue
            half = math.sqrt(max(0, r * r - (hy - y) ** 2))
            d.line([(cx - half, y), (cx + half, y)], fill=dim(ORANGE, 0.55), width=3)
    return glow(canvas(), f, radii=(50, 12), strength=(1.3, 1.0))


def traces(d, rnd, count, step, cyan, orange, pad_color=None):
    for _ in range(count):
        x = rnd.randrange(0, W // step) * step
        y = rnd.randrange(0, H // step) * step
        col = orange if rnd.random() < 0.07 else cyan
        pts = [(x, y)]
        for _ in range(rnd.randint(3, 9)):
            dx, dy = rnd.choice([(1, 0), (-1, 0), (0, 1), (0, -1), (1, 1), (-1, 1)])
            n = rnd.randint(2, 8)
            x, y = x + dx * n * step, y + dy * n * step
            pts.append((x, y))
        d.line(pts, fill=col, width=3, joint="curve")
        for px, py in (pts[0], pts[-1]):
            d.rectangle([px - 9, py - 9, px + 9, py + 9], outline=pad_color or col, width=3)


def w03_circuit_traces():
    rnd = random.Random(3)
    return glow(canvas(), lambda d: traces(d, rnd, 140, 60, dim(CYAN, 0.75), dim(ORANGE, 0.85)),
                radii=(16, 5), strength=(1.0, 0.9))


def w04_grid_tunnel():
    def f(d):
        cx, cy = W / 2, H / 2
        for ang in range(0, 360, 15):
            r = math.radians(ang)
            d.line([(cx, cy), (cx + math.cos(r) * 3000, cy + math.sin(r) * 3000)], fill=dim(CYAN, 0.22), width=2)
        for i in range(1, 26):
            s = 1.18 ** i * 30
            a = min(1, 0.15 + i / 26)
            d.rectangle([cx - s * 1.6, cy - s, cx + s * 1.6, cy + s], outline=dim(CYAN, a * 0.8), width=2)
        d.rectangle([cx - 48, cy - 30, cx + 48, cy + 30], outline=ORANGE, width=4)
    return glow(canvas(), f, radii=(24, 6), strength=(1.1, 1.0))


def w05_light_rings():
    def f(d):
        cx, cy = W * 0.62, H * 0.5
        for i, r in enumerate(range(160, 1500, 95)):
            segs = 6 + i * 2
            col = ORANGE if i == 4 else CYAN
            for s in range(segs):
                a0 = 360 / segs * s
                d.arc([cx - r, cy - r, cx + r, cy + r], a0 + 4, a0 + 360 / segs - 4,
                      fill=dim(col, max(0.25, 0.85 - i * 0.04)), width=4 if i % 3 == 0 else 2)
    return glow(canvas(), f, radii=(28, 7), strength=(1.2, 1.0))


def w06_wire_terrain():
    hy = H * 0.42
    cols, rows = 64, 34

    def height(i, j):
        x, z = i / cols, j / rows
        return (math.sin(x * 9 + 1.3) * 0.5 + math.sin(x * 23 + z * 4) * 0.25
                + math.sin(x * 4 - z * 7) * 0.4) * (1 - z) ** 0.6

    def proj(i, j):
        z = 1 - j / rows * 0.94
        x = (i / cols - 0.5) * W * 2.6 * (1 - z * 0.8) + W / 2
        y = hy + (H - hy) * (1 - z) ** 1.6 - height(i, j) * 380 * (1 - z * 0.6)
        return x, y

    def f(d):
        for j in range(rows + 1):
            d.line([proj(i, j) for i in range(cols + 1)], fill=dim(CYAN, 0.25 + 0.6 * j / rows), width=2)
        for i in range(cols + 1):
            d.line([proj(i, j) for j in range(rows + 1)], fill=dim(CYAN, 0.45), width=2)
    return glow(canvas(), f, radii=(18, 5), strength=(1.0, 0.9))


def w07_data_rain():
    rnd = random.Random(7)

    def f(d):
        for x in range(30, W, 46):
            y = rnd.randrange(-H, H)
            ln = rnd.randrange(200, 900)
            col = ORANGE if rnd.random() < 0.05 else CYAN
            steps = 24
            for s in range(steps):
                y0 = y + ln * s / steps
                d.line([(x, y0), (x, y0 + ln / steps - 6)], fill=dim(col, (s + 1) / steps), width=3)
            d.rectangle([x - 4, y + ln - 4, x + 4, y + ln + 4], fill=ICE)
    return glow(canvas(), f, radii=(14, 4), strength=(1.0, 0.8))


def w08_light_trails():
    hy = H * 0.5

    def trail(d, col, off, amp, ph):
        pts = []
        for t in range(0, 201):
            u = t / 200
            y = hy + (H - hy) * u ** 1.7
            x = W / 2 + off * u ** 1.2 * W + math.sin(u * 6 + ph) * amp * u
            pts.append((x, y))
        for wdt, k in ((16, 0.35), (6, 1.0)):
            d.line(pts, fill=dim(col, k), width=wdt, joint="curve")

    def f(d):
        floor_grid(d, hy, k=0.55)
        horizon(d, hy, dim(ICE, 0.8))
        trail(d, CYAN, -0.42, 160, 0.4)
        trail(d, ORANGE, 0.36, 220, 2.2)
    return glow(canvas(), f, radii=(34, 9), strength=(1.3, 1.0))


def w09_blueprint():
    def f(d):
        for x in range(0, W, 40):
            major = x % 400 == 0
            d.line([(x, 0), (x, H)], fill=dim(CYAN, 0.42 if major else 0.12), width=2 if major else 1)
        for y in range(0, H, 40):
            major = y % 400 == 0
            d.line([(0, y), (W, y)], fill=dim(CYAN, 0.42 if major else 0.12), width=2 if major else 1)
        cx, cy = 1600, 1000
        d.ellipse([cx - 300, cy - 300, cx + 300, cy + 300], outline=CYAN, width=3)
        d.line([(cx - 380, cy), (cx + 380, cy)], fill=CYAN, width=2)
        d.line([(cx, cy - 380), (cx, cy + 380)], fill=CYAN, width=2)
        d.rectangle([cx - 12, cy - 12, cx + 12, cy + 12], outline=ORANGE, width=3)
    return glow(canvas(), f, radii=(10, 3), strength=(0.8, 0.6))


def w10_hud_brackets():
    def f(d):
        m, L = 160, 220
        for (x, y, sx, sy) in ((m, m, 1, 1), (W - m, m, -1, 1), (W - m, H - m, -1, -1), (m, H - m, 1, -1)):
            d.line([(x, y + sy * L), (x, y), (x + sx * L, y)], fill=CYAN, width=5)
        cx, cy = W / 2, H / 2
        d.ellipse([cx - 90, cy - 90, cx + 90, cy + 90], outline=dim(CYAN, 0.7), width=3)
        for a in (0, 90, 180, 270):
            r = math.radians(a)
            d.line([(cx + math.cos(r) * 110, cy + math.sin(r) * 110),
                    (cx + math.cos(r) * 170, cy + math.sin(r) * 170)], fill=CYAN, width=3)
        d.rectangle([cx - 8, cy - 8, cx + 8, cy + 8], fill=ORANGE)
        for i in range(12):
            d.line([(m + 40 + i * 22, H - m - 60), (m + 40 + i * 22, H - m - 60 - (i % 4 + 1) * 18)],
                   fill=dim(CYAN, 0.6), width=8)
    return glow(canvas(), f, radii=(22, 6), strength=(1.1, 0.9))


def w11_scan_horizon():
    hy = H * 0.66

    def f(d):
        horizon(d, hy, ICE)
        d.line([(W * 0.18, hy - 160), (W * 0.18, hy)], fill=dim(CYAN, 0.6), width=3)
        d.line([(W * 0.82, hy - 260), (W * 0.82, hy)], fill=dim(ORANGE, 0.6), width=3)
    img = glow(canvas(), f, radii=(60, 14), strength=(1.6, 1.1))
    d = ImageDraw.Draw(img)
    for y in range(0, H, 6):
        d.line([(0, y), (W, y)], fill=BG, width=1)
    return img


def w12_grid_sphere():
    def f(d):
        cx, cy, r = W * 0.5, H * 0.52, 640
        tilt = math.radians(22)

        def p(lat, lon):
            x = r * math.cos(lat) * math.sin(lon)
            y = r * math.sin(lat)
            z = r * math.cos(lat) * math.cos(lon)
            y, z = y * math.cos(tilt) - z * math.sin(tilt), y * math.sin(tilt) + z * math.cos(tilt)
            return (cx + x, cy - y), z

        for lat in range(-75, 90, 15):
            pts = [p(math.radians(lat), math.radians(lon)) for lon in range(0, 361, 4)]
            for (a, za), (b, zb) in zip(pts, pts[1:]):
                d.line([a, b], fill=dim(CYAN, 0.85 if za + zb > 0 else 0.2), width=2)
        for lon in range(0, 360, 15):
            pts = [p(math.radians(lat), math.radians(lon)) for lat in range(-90, 91, 4)]
            col = ORANGE if lon == 120 else CYAN
            for (a, za), (b, zb) in zip(pts, pts[1:]):
                d.line([a, b], fill=dim(col, 0.85 if za + zb > 0 else 0.2), width=2)
    return glow(canvas(), f, radii=(26, 6), strength=(1.1, 1.0))


def light_glow(draw_fn):
    """Light wallpapers: dark-cyan lines on a pale base with a soft tinted halo."""
    layer = Image.new("L", (W, H), 0)
    draw_fn(ImageDraw.Draw(layer))
    halo = layer.filter(ImageFilter.GaussianBlur(10))
    img = Image.new("RGB", (W, H), LIGHT_BG)
    tint = Image.new("RGB", (W, H), (120, 210, 225))
    img = Image.composite(tint, img, Image.eval(halo, lambda v: int(v * 0.5)))
    ink = Image.new("RGB", (W, H), TEAL)
    return Image.composite(ink, img, layer)


def w13_light_grid():
    hy = H * 0.6

    def f(d):
        for i in range(-30, 31):
            d.line([(W / 2 + i * 4, hy), (W / 2 + i * W / 9, H)], fill=110, width=2)
        for j in range(22):
            y = floor_y(j, hy)
            if hy < y < H:
                d.line([(0, y), (W, y)], fill=int(60 + 150 * (y - hy) / (H - hy)), width=2)
        d.line([(0, hy), (W, hy)], fill=255, width=4)
    return light_glow(f)


def w14_light_circuit():
    rnd = random.Random(14)
    return light_glow(lambda d: traces(d, rnd, 110, 60, 200, 200, 255))


def w15_orange_signal():
    def f(d):
        pts = []
        for x in range(0, W + 1, 8):
            u = x / W
            env = math.exp(-((u - 0.55) ** 2) / 0.02)
            pts.append((x, H / 2 + math.sin(u * 60) * 260 * env + math.sin(u * 7) * 30))
        d.line(pts, fill=ORANGE, width=4, joint="curve")
        d.line([(0, H / 2 + 300), (W, H / 2 + 300)], fill=dim(CYAN, 0.5), width=2)
    base = canvas()
    g = ImageDraw.Draw(base)
    for x in range(0, W, 120):
        g.line([(x, 0), (x, H)], fill=(0, 22, 28), width=1)
    for y in range(0, H, 120):
        g.line([(0, y), (W, y)], fill=(0, 22, 28), width=1)
    return glow(base, f, radii=(30, 8), strength=(1.3, 1.0))


def w16_cell_field():
    rnd = random.Random(16)

    def f(d):
        s, gap = 34, 14
        for gx in range(60, W - 40, s + gap):
            for gy in range(60, H - 40, s + gap):
                v = rnd.random()
                if v < 0.015:
                    d.rectangle([gx, gy, gx + s, gy + s], fill=ORANGE)
                elif v < 0.12:
                    d.rectangle([gx, gy, gx + s, gy + s], fill=dim(CYAN, rnd.uniform(0.4, 1)))
                else:
                    d.rectangle([gx, gy, gx + s, gy + s], outline=(0, 36, 44), width=2)
    return glow(canvas(), f, radii=(16, 5), strength=(0.9, 0.7))


WALLPAPERS = [
    ("01-horizon-grid", w01_horizon_grid), ("02-horizon-sunset", w02_horizon_sunset),
    ("03-circuit-traces", w03_circuit_traces), ("04-grid-tunnel", w04_grid_tunnel),
    ("05-light-rings", w05_light_rings), ("06-wire-terrain", w06_wire_terrain),
    ("07-data-rain", w07_data_rain), ("08-light-trails", w08_light_trails),
    ("09-blueprint", w09_blueprint), ("10-hud-brackets", w10_hud_brackets),
    ("11-scan-horizon", w11_scan_horizon), ("12-grid-sphere", w12_grid_sphere),
    ("13-light-grid", w13_light_grid), ("14-light-circuit", w14_light_circuit),
    ("15-orange-signal", w15_orange_signal), ("16-cell-field", w16_cell_field),
]


def contact_sheet(out_dir, path):
    tw, th, cols = 360, 225, 4
    sheet = Image.new("RGB", (cols * tw + (cols + 1) * 12, 4 * th + 5 * 12), BG)
    for i, (name, _) in enumerate(WALLPAPERS):
        p = os.path.join(out_dir, name + ".png")
        if not os.path.exists(p):
            continue
        im = Image.open(p).resize((tw, th), Image.LANCZOS)
        sheet.paste(im, (12 + (i % cols) * (tw + 12), 12 + (i // cols) * (th + 12)))
    sheet.save(path)


if __name__ == "__main__":
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    only = set(sys.argv[2:])
    for name, fn in WALLPAPERS:
        if only and name not in only:
            continue
        fn().save(os.path.join(out, name + ".png"), optimize=True)
        print("wrote", name)
    sheet = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "docs", "images", "wallpapers.png"))
    contact_sheet(out, sheet)
    print("contact sheet", sheet)
