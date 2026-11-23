"""Builds the Gridline folder icon (.ico) with hand-tuned sizes from 16 to 256 px.

Dark teal panels with bright cyan outlines (they read on dark and light Explorer),
a fine grid texture on the front panel and one orange marker.
usage: python make_folder_icon.py <out_dir>
requires: pillow
"""
import os
import sys

from PIL import Image, ImageDraw

SIZES = [16, 20, 24, 32, 40, 48, 64, 96, 128, 256]
SS = 4
BACK = (10, 42, 50, 245)
FRONT = (5, 20, 26, 245)
EDGE = (0, 229, 255, 255)
DOT = (0, 190, 215, 255)       # symbols and texture
GRID = (0, 110, 128, 255)      # fine grid lines
RED = (255, 138, 31, 255)      # the orange marker (name kept for the other scripts)


def edge_width(size, u):
    return max(1, round((0.8 if size >= 64 else 1.1 if size >= 32 else 1.3) * u))


def grid_texture(d, x0, y0, x1, y1, size, u, cols=8, rows=4):
    if size < 40:
        return
    w = max(1, round(0.25 * u))
    for i in range(1, cols):
        x = x0 + (x1 - x0) * i / cols
        d.line((x, y0, x, y1), fill=GRID, width=w)
    for j in range(1, rows):
        y = y0 + (y1 - y0) * j / rows
        d.line((x0, y, x1, y), fill=GRID, width=w)


def marker(d, cx, cy, size, u, k=1.0):
    rr = (1.1 if size >= 64 else 1.45 if size >= 32 else 2.1) * u * k
    d.rectangle((cx - rr, cy - rr, cx + rr, cy + rr), fill=RED)


def folder(size, open_=False):
    S = size * SS
    img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    u = S / 32.0
    ew = edge_width(size, u)
    r = 1.6 * u
    # back panel with tab
    d.rounded_rectangle((2 * u, 6 * u, 30 * u, 27 * u), r, fill=BACK, outline=EDGE, width=ew)
    d.rounded_rectangle((2 * u, 4 * u, 13 * u, 10 * u), r, fill=BACK, outline=EDGE, width=ew)
    d.rectangle((2 * u + ew, 8 * u, 13 * u - ew, 10.5 * u), fill=BACK)
    # front panel
    if open_:
        d.polygon([(4.5 * u, 11 * u), (31 * u, 11 * u), (28 * u, 27 * u), (2 * u, 27 * u)], fill=FRONT, outline=EDGE, width=ew)
        fx0, fy0, fx1, fy1 = 6 * u, 13 * u, 27 * u, 25 * u
    else:
        d.rounded_rectangle((2 * u, 10.5 * u, 30 * u, 27 * u), r, fill=FRONT, outline=EDGE, width=ew)
        fx0, fy0, fx1, fy1 = 4.5 * u, 13 * u, 27.5 * u, 25 * u
    grid_texture(d, fx0, fy0, fx1, fy1, size, u)
    marker(d, 25.5 * u, 7.6 * u, size, u)
    return img.resize((size, size), Image.LANCZOS)


def save_ico(path, open_=False):
    imgs = [folder(s, open_) for s in SIZES]
    big = imgs[-1]
    big.save(path, format="ICO", sizes=[(s, s) for s in SIZES], append_images=imgs[:-1])


if __name__ == "__main__":
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    save_ico(os.path.join(out, "gridline-folder.ico"))
    save_ico(os.path.join(out, "gridline-folder-open.ico"), True)
    print("built folder icons")
