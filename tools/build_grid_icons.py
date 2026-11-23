"""Builds "Gridline Icons", an original line icon font.

Each icon is drawn on a 9x9 grid; neighbouring cells are joined into straight
strokes, like circuit traces. Glyphs sit at the same Private Use codepoint
Windows uses for that system icon, sized to the Windows glyph's box.
Codepoints not defined here fall back to Segoe Fluent Icons.

usage: python build_grid_icons.py <SegoeIcons.ttf> <out.ttf> ["Family Name"]
requires: fonttools
"""
import sys

from fontTools.fontBuilder import FontBuilder
from fontTools.pens.cu2quPen import Cu2QuPen
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.ttLib import TTFont

N = 9  # dots per icon side; big dots stay readable at 16-20 px


def art(rows):
    return {(x, y) for y, r in enumerate(rows) for x, ch in enumerate(r) if ch == "#"}


WIFI = [
    ".#######.",
    "#.......#",
    "..#####..",
    ".#.....#.",
    "...###...",
    ".........",
    "....#....",
]


def wifi(level):
    keep = {3: range(7), 2: range(2, 7), 1: range(4, 7), 0: range(6, 7)}[level]
    return art([r if i in keep else "." * 9 for i, r in enumerate(WIFI)])


SPEAKER = [
    "...#...#.",
    "..##....#",
    "####.#..#",
    "####..#.#",
    "####..#.#",
    "####.#..#",
    "..##....#",
    "...#...#.",
]


def speaker(waves):
    rows = []
    for r in SPEAKER:
        body, inner, outer = r[:4], r[4:7], r[7:]
        rows.append(body + (inner if waves >= 1 else "...") + (outer if waves >= 2 else ".."))
    return art(rows)


MOON = [
    "..###....",
    ".##......",
    "##.......",
    "##.......",
    "##.......",
    "##.....#.",
    ".##...##.",
    "..#####..",
]
SUN = [
    "....#....",
    ".#.....#.",
    "...###...",
    "..#...#..",
    "#.#...#.#",
    "..#...#..",
    "...###...",
    ".#.....#.",
    "....#....",
]

ICONS = {}
ICONS[0xE701] = ICONS[0xEC3F] = ICONS[0xF657] = wifi(3)
ICONS[0xE874] = wifi(2)
ICONS[0xE873] = wifi(1)
ICONS[0xE872] = wifi(0)
ICONS[0xE702] = art([  # bluetooth
    "...#...",
    "...##..",
    ".#.#.#.",
    "..###..",
    "...#...",
    "..###..",
    ".#.#.#.",
    "...##..",
    "...#...",
])
ICONS[0xE709] = art([  # airplane
    "....#....",
    "...###...",
    "...###...",
    ".#######.",
    "#########",
    "...###...",
    "...###...",
    "..#####..",
    ".##...##.",
])
ICONS[0xE705] = art([  # vpn shield
    "...#...",
    ".##.##.",
    "#.....#",
    "#..#..#",
    "#.###.#",
    "#..#..#",
    ".#...#.",
    "..#.#..",
    "...#...",
])
ICONS[0xE708] = art(MOON)
ICONS[0xF08C] = art(MOON) | {(7, 0), (8, 2)}  # night light: moon + stars
ICONS[0xE706] = art(SUN)
ICONS[0xE793] = art(SUN) | {(4, 4)}
ICONS[0xE776] = art([  # accessibility
    "....#....",
    ".........",
    "#########",
    "....#....",
    "....#....",
    "...#.#...",
    "..#...#..",
    ".#.....#.",
])
ICONS[0xE704] = art([  # mobile hotspot
    ".#.....#.",
    "#..#.#..#",
    "#.#.#.#.#",
    "#..#.#..#",
    ".#..#..#.",
    "....#....",
    "....#....",
    "....#....",
])
ICONS[0xED1E] = ICONS[0xE7F0] = art([  # captions
    "#########",
    "#.......#",
    "#.......#",
    "#.###.#.#",
    "#.#.###.#",
    "#.......#",
    "#########",
])
ICONS[0xE72D] = art([  # share
    "......###",
    ".......##",
    "......#.#",
    ".....#...",
    "#...#....",
    "#..#....#",
    "#.......#",
    "#.......#",
    "#########",
])
ICONS[0xEC15] = art([  # cast
    "#########",
    "#.......#",
    "........#",
    "##......#",
    "..#.....#",
    "#..#....#",
    ".#.#....#",
    "#.#.#####",
])
ICONS[0xEBC6] = art([  # project
    "...######",
    "...#....#",
    "#####...#",
    "#...#...#",
    "#...#####",
    "#...#....",
    "#####....",
])
ICONS[0xF432] = art([  # energy saver: battery + leaf
    "########.",
    "#......##",
    "#...##.##",
    "#..##..##",
    "########.",
])
ICONS[0xE993] = speaker(1)
ICONS[0xE994] = speaker(2)
ICONS[0xE995] = ICONS[0xE767] = ICONS[0xE15D] = speaker(2)
ICONS[0xE713] = art([  # settings gear
    "....#....",
    ".#.###.#.",
    "..#...#..",
    ".#.....#.",
    "###.#.###",
    ".#.....#.",
    "..#...#..",
    ".#.###.#.",
    "....#....",
])
ICONS[0xF4A5] = art([  # studio effects: sparkles
    "...#.....",
    "...#.....",
    "..#.#....",
    "##...##..",
    "..#.#....",
    "...#...#.",
    "...#..#.#",
    ".......#.",
])
ICONS[0xE790] = art([  # color profile: palette
    "..#####..",
    ".#.....#.",
    "#..#.#..#",
    "#.......#",
    "#.#...##.",
    "#....#...",
    ".#...#...",
    "..####...",
])
ICONS[0xF4BD] = ICONS[0xE7ED] = art([  # do not disturb: bell
    "....#....",
    "...###...",
    "..#...#..",
    "..#...#..",
    "..#...#..",
    ".#.....#.",
    "#########",
    "...#.#...",
    "....#....",
])
ICONS[0xF4C3] = art([  # volume mixer
    "......#..",
    "#####.#.#",
    "......#..",
    ".........",
    "..#......",
    "#.#.#####",
    "..#......",
])


# ---------------------------------------------------------------- font
def strokes(cells):
    """Grid cells -> list of (a, b) segments (a == b for a lone cell)."""
    segs = []
    linked = set()
    for x, y in sorted(cells):
        for dx, dy in ((1, 0), (0, 1)):
            if (x + dx, y + dy) in cells:
                segs.append(((x, y), (x + dx, y + dy)))
                linked |= {(x, y), (x + dx, y + dy)}
        # diagonals only where no right-angle path joins the two cells
        for dx in (1, -1):
            q = (x + dx, y + 1)
            if q in cells and (x + dx, y) not in cells and (x, y + 1) not in cells:
                segs.append(((x, y), q))
                linked |= {(x, y), q}
    segs += [(p, p) for p in cells if p not in linked]
    return segs


def quad(pen, a, b, hw):
    """Stroke from a to b with half-width hw and square caps, drawn clockwise."""
    (ax, ay), (bx, by) = a, b
    dx, dy = bx - ax, by - ay
    ln = (dx * dx + dy * dy) ** 0.5
    if ln == 0:
        ux, uy = 1.0, 0.0
    else:
        ux, uy = dx / ln, dy / ln
    nx, ny = -uy * hw, ux * hw
    ax, ay, bx, by = ax - ux * hw, ay - uy * hw, bx + ux * hw, by + uy * hw
    pts = [(ax + nx, ay + ny), (bx + nx, by + ny), (bx - nx, by - ny), (ax - nx, ay - ny)]
    area = sum(pts[i][0] * pts[(i + 1) % 4][1] - pts[(i + 1) % 4][0] * pts[i][1] for i in range(4))
    if area > 0:  # TrueType outer contours run clockwise
        pts.reverse()
    pen.moveTo(tuple(round(v) for v in pts[0]))
    for p in pts[1:]:
        pen.lineTo(tuple(round(v) for v in p))
    pen.closePath()


def build(segoe_path, out_path):
    seg = TTFont(segoe_path)
    upm = seg["head"].unitsPerEm
    gs, hm, cmap = seg.getGlyphSet(), seg["hmtx"], seg.getBestCmap()

    names, glyphs, metrics, umap = [".notdef"], {}, {}, {}
    nd = TTGlyphPen(None)
    glyphs[".notdef"], metrics[".notdef"] = nd.glyph(), (upm, 0)

    for cp, cells in sorted(ICONS.items()):
        if cp not in cmap or not cells:
            continue
        sname = cmap[cp]
        from fontTools.pens.boundsPen import BoundsPen
        bp = BoundsPen(gs)
        gs[sname].draw(bp)
        if not bp.bounds:
            continue
        x0, y0, x1, y1 = bp.bounds
        size = max(x1 - x0, y1 - y0)
        cxm, cym = (x0 + x1) / 2, (y0 + y1) / 2
        cell = size / N
        hw = cell * 0.17  # stroke half-width
        # center the drawing's own bounding box inside the Windows glyph box
        xs = [p[0] for p in cells]; ys = [p[1] for p in cells]
        ox = (min(xs) + max(xs)) / 2; oy = (min(ys) + max(ys)) / 2
        pen = TTGlyphPen(None)
        to = lambda p: (cxm + (p[0] - ox) * cell, cym - (p[1] - oy) * cell)
        for a, b in strokes(cells):
            quad(pen, to(a), to(b), hw if a != b else hw * 1.6)
        name = f"uni{cp:04X}"
        names.append(name)
        glyphs[name] = pen.glyph()
        metrics[name] = (hm[sname][0], 0)
        umap[cp] = name

    fb = FontBuilder(upm, isTTF=True)
    fb.setupGlyphOrder(names)
    fb.setupCharacterMap(umap)
    fb.setupGlyf(glyphs)
    fb.setupHorizontalMetrics({n: (metrics[n][0], 0) for n in names})
    asc, desc = seg["hhea"].ascent, seg["hhea"].descent
    fb.setupHorizontalHeader(ascent=asc, descent=desc)
    fam = sys.argv[3] if len(sys.argv) > 3 else "Gridline Icons"
    ps = fam.replace(" ", "") + "-Regular"
    fb.setupNameTable({"familyName": fam, "styleName": "Regular",
                       "uniqueFontIdentifier": ps,
                       "fullName": fam, "psName": ps,
                       "version": "Version 1.000"})
    fb.setupOS2(sTypoAscender=asc, sTypoDescender=desc, usWinAscent=asc, usWinDescent=abs(desc),
                usWeightClass=400)
    fb.setupPost()
    fb.font["head"].fontRevision = 1.0
    # recalc glyph bounds
    for n in names:
        fb.font["glyf"][n].recalcBounds(fb.font["glyf"])
    fb.save(out_path)
    return len(umap)


if __name__ == "__main__":
    n = build(sys.argv[1], sys.argv[2])
    print(f"built {sys.argv[2]} with {n} icons")
