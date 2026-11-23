"""Builds the Gridline icon set for File Explorer locations (.ico, 16-256 px).

Personal folders reuse the Gridline folder with a line symbol; drives, This PC,
Home, Gallery, Network and Recycle Bin use the same dark panel / cyan outline / orange marker style.
usage: python make_explorer_icons.py <out_dir>
requires: pillow
"""
import os
import sys

from PIL import Image, ImageDraw

from make_folder_icon import SIZES, SS, BACK, FRONT, EDGE, DOT, RED, folder, edge_width, grid_texture, marker

GLYPHS = {
    "desktop": ["#########", "#.......#", "#.......#", "#.......#", "#########", "....#....", "..#####.."],
    "documents": ["######.", "#....##", "#.....#", "#.###.#", "#.....#", "#.###.#", "#.....#", "#######"],
    "downloads": ["...#...", "...#...", "...#...", ".#.#.#.", "..###..", "...#...", "#.....#", "#######"],
    "music": ["..#####", "..#...#", "..#...#", "..#...#", ".##..##", "###.###", ".#...#."],
    "pictures": ["#########", "#.......#", "#.....#.#", "#..#....#", "#.###.#.#", "#########"],
    "videos": ["#########", "#.......#", "#..#....#", "#..##...#", "#..###..#", "#..##...#", "#..#....#", "#.......#",
               "#########"],
}


def canvas(size):
    S = size * SS
    img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    return img, ImageDraw.Draw(img), S / 32.0


def ew(size, u):
    return edge_width(size, u)


def dot_glyph(d, rows, cx, cy, pitch, color=DOT):
    """Draws a small bitmap as joined line strokes (neighbours and lone diagonals)."""
    h, w = len(rows), max(len(r) for r in rows)
    on = {(x, y) for y, row in enumerate(rows) for x, ch in enumerate(row) if ch == "#"}
    lw = max(1, round(pitch * 0.42))
    P = lambda x, y: (cx + (x - (w - 1) / 2) * pitch, cy + (y - (h - 1) / 2) * pitch)
    for (x, y) in sorted(on):
        linked = False
        for dx, dy in ((1, 0), (0, 1)):
            if (x + dx, y + dy) in on:
                d.line((P(x, y), P(x + dx, y + dy)), fill=color, width=lw)
                linked = True
        for dx in (1, -1):
            if (x + dx, y + 1) in on and (x + dx, y) not in on and (x, y + 1) not in on:
                d.line((P(x, y), P(x + dx, y + 1)), fill=color, width=lw)
                linked = True
        r = lw * 0.5
        px, py = P(x, y)
        d.rectangle((px - r, py - r, px + r, py + r), fill=color)


def texture(d, x0, y0, x1, y1, size, u):
    grid_texture(d, x0, y0, x1, y1, size, u)


def red(d, cx, cy, size, u, k=1.0):
    marker(d, cx, cy, size, u, k)


def special_folder(kind):
    def draw(size):
        base = folder(size).resize((size * SS, size * SS), Image.NEAREST)
        d = ImageDraw.Draw(base)
        u = size * SS / 32.0
        # cover the plain dot texture, then draw the symbol
        d.rounded_rectangle((3.4 * u, 12 * u, 28.6 * u, 25.6 * u), 1.0 * u, fill=FRONT)
        if size >= 24:
            dot_glyph(d, GLYPHS[kind], 16 * u, 18.8 * u, (1.45 if size >= 48 else 1.75) * u)
        return base.resize((size, size), Image.LANCZOS)
    return draw


def drive(system=False, usb=False):
    def draw(size):
        img, d, u = canvas(size)
        w = ew(size, u)
        if usb:
            d.rounded_rectangle((11 * u, 4 * u, 21 * u, 10 * u), 1 * u, fill=BACK, outline=EDGE, width=w)
            box = (6 * u, 9 * u, 26 * u, 28 * u)
        else:
            box = (2.5 * u, 10 * u, 29.5 * u, 23.5 * u)
        d.rounded_rectangle(box, 2.6 * u, fill=FRONT, outline=EDGE, width=w)
        x0, y0, x1, y1 = box
        if size >= 32:
            n = 6 if not usb else 3
            for i in range(n):
                x = x0 + 3.5 * u + i * 2.2 * u
                d.rectangle((x - 0.6 * u, y1 - 3.4 * u, x + 0.6 * u, y1 - 2.2 * u), fill=DOT)
        if system and size >= 24:
            for (sx, sy) in ((0, 0), (1, 0), (0, 1), (1, 1)):
                px, py = x0 + 3.2 * u + sx * 2.3 * u, y0 + 2.8 * u + sy * 2.3 * u
                d.rectangle((px, py, px + 1.8 * u, py + 1.8 * u), fill=DOT)
        red(d, x1 - 3.4 * u, y1 - 2.8 * u if not usb else y0 + 3 * u, size, u)
        return img.resize((size, size), Image.LANCZOS)
    return draw


def this_pc(size):
    img, d, u = canvas(size)
    w = ew(size, u)
    d.rounded_rectangle((2 * u, 4 * u, 30 * u, 23 * u), 2.4 * u, fill=FRONT, outline=EDGE, width=w)
    texture(d, 5 * u, 7 * u, 27 * u, 20 * u, size, u)
    d.rectangle((13 * u, 23 * u, 19 * u, 26.5 * u), fill=BACK, outline=EDGE, width=w)
    d.rounded_rectangle((8 * u, 26 * u, 24 * u, 29 * u), 1.2 * u, fill=BACK, outline=EDGE, width=w)
    red(d, 26 * u, 19.5 * u, size, u)
    return img.resize((size, size), Image.LANCZOS)


def home(size):
    img, d, u = canvas(size)
    w = ew(size, u)
    d.polygon([(16 * u, 3 * u), (30 * u, 15 * u), (27 * u, 15 * u), (27 * u, 29 * u), (5 * u, 29 * u),
               (5 * u, 15 * u), (2 * u, 15 * u)], fill=FRONT, outline=EDGE, width=w)
    d.rounded_rectangle((13 * u, 19 * u, 19 * u, 29 * u), 1 * u, fill=BACK, outline=EDGE, width=w)
    if size >= 40:
        for i in range(3):
            d.rectangle((8 * u + i * 0 - 0.6 * u, (18 + i * 3) * u - 0.6 * u, 8 * u + 0.6 * u, (18 + i * 3) * u + 0.6 * u), fill=DOT)
    red(d, 16 * u, 11.5 * u, size, u)
    return img.resize((size, size), Image.LANCZOS)


def gallery(size):
    img, d, u = canvas(size)
    w = ew(size, u)
    d.rounded_rectangle((7 * u, 3 * u, 30 * u, 21 * u), 2.2 * u, fill=BACK, outline=EDGE, width=w)
    d.rounded_rectangle((2 * u, 9 * u, 25 * u, 28 * u), 2.2 * u, fill=FRONT, outline=EDGE, width=w)
    if size >= 24:
        dot_glyph(d, ["......#..", ".........", "..#......", ".###..#..", "#####.###"], 13.5 * u, 20 * u,
                  (1.55 if size >= 48 else 1.9) * u)
    red(d, 20.5 * u, 13.5 * u, size, u, 0.9)
    return img.resize((size, size), Image.LANCZOS)


def recycle(full):
    def draw(size):
        img, d, u = canvas(size)
        w = ew(size, u)
        d.polygon([(6 * u, 9 * u), (26 * u, 9 * u), (24 * u, 29 * u), (8 * u, 29 * u)], fill=FRONT, outline=EDGE, width=w)
        d.rounded_rectangle((4 * u, 6 * u, 28 * u, 9.5 * u), 1 * u, fill=BACK, outline=EDGE, width=w)
        d.rectangle((13 * u, 3.5 * u, 19 * u, 6 * u), fill=BACK, outline=EDGE, width=w)
        if size >= 32:
            for x in (11, 16, 21):
                for y in range(12, 27, 3):
                    d.rectangle((x * u - 0.6 * u, y * u - 0.6 * u, x * u + 0.6 * u, y * u + 0.6 * u), fill=DOT)
        if full:
            red(d, 16 * u, 18.5 * u, size, u, 1.6)
        return img.resize((size, size), Image.LANCZOS)
    return draw


def network(size):
    import math
    img, d, u = canvas(size)
    w = ew(size, u)
    d.ellipse((2.5 * u, 2.5 * u, 29.5 * u, 29.5 * u), fill=FRONT, outline=EDGE, width=w)
    if size >= 32:
        pts = set()
        step = 2.4
        x = 5.0
        while x <= 27.01:  # equator
            pts.add((round(x, 1), 16.0)); x += step
        for yy in (9.5, 22.5):  # latitude lines
            half = math.sqrt(max(0.0, 11.5 ** 2 - (yy - 16) ** 2))
            x = 16 - half + 1.0
            while x <= 16 + half - 1.0:
                pts.add((round(x, 1), yy)); x += step
        for k in range(0, 360, 20):  # meridian ellipse + centre line
            a = math.radians(k)
            pts.add((round(16 + 6.0 * math.cos(a), 1), round(16 + 11.5 * math.sin(a), 1)))
        y = 5.0
        while y <= 27.01:
            pts.add((16.0, round(y, 1))); y += step
        for (px, py) in pts:
            d.rectangle((px * u - 0.6 * u, py * u - 0.6 * u, px * u + 0.6 * u, py * u + 0.6 * u), fill=DOT)
    red(d, 24 * u, 8 * u, size, u)
    return img.resize((size, size), Image.LANCZOS)


ICONS = {f"folder-{k}": special_folder(k) for k in GLYPHS}
ICONS.update({"drive": drive(), "drive-system": drive(system=True), "drive-usb": drive(usb=True),
              "this-pc": this_pc, "home": home, "gallery": gallery, "recycle-empty": recycle(False),
              "recycle-full": recycle(True), "network": network})


def save(name, fn, out):
    imgs = [fn(s) for s in SIZES]
    imgs[-1].save(os.path.join(out, f"{name}.ico"), format="ICO", sizes=[(s, s) for s in SIZES],
                  append_images=imgs[:-1])
    return imgs


if __name__ == "__main__":
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    names = list(ICONS)
    sheet = Image.new("RGBA", (len(names) * 110 + 10, 2 * 140 + 40), (2, 6, 8, 255))
    sheet.paste(Image.new("RGBA", (sheet.width, 150), (243, 251, 252, 255)), (0, 160))
    for i, n in enumerate(names):
        imgs = save(n, ICONS[n], out)
        big = imgs[SIZES.index(96)]
        small = imgs[SIZES.index(24)]
        for y0 in (10, 170):
            sheet.alpha_composite(big, (10 + i * 110, y0))
            sheet.alpha_composite(small, (46 + i * 110, y0 + 104))
    sheet.convert("RGB").save(os.path.join(out, "preview.png"))
    print("built", len(names), "icons")
