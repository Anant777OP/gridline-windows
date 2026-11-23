"""Builds the "Gridline Pointer" cursor set: outlined shapes, small inner grid cells, one orange accent.

Two variants: carbon (near-black body, cyan outline) and clear (pale body, dark cyan outline).
usage: python make_cursors_gridline.py <out_dir>
requires: pillow
"""
import math
import os
import struct
import sys

from PIL import Image, ImageDraw

SIZES = [32, 48, 64, 96, 128]
SS = 4
RED = (255, 138, 31, 255)  # the orange accent (name kept so the designs below read the same)
VARIANTS = {
    "carbon": {"body": (3, 12, 16, 255), "edge": (0, 229, 255, 255), "dots": (0, 229, 255, 255)},
    "clear": {"body": (243, 251, 252, 255), "edge": (0, 111, 128, 255), "dots": (0, 111, 128, 255)},
}


class C:
    def __init__(self, size, v):
        self.size, self.k, self.v = size, size * SS / 32, VARIANTS[v]
        self.img = Image.new("RGBA", (size * SS, size * SS), (0, 0, 0, 0))
        self.d = ImageDraw.Draw(self.img)

    def P(self, pts):
        return [(x * self.k, y * self.k) for x, y in pts]

    def shape(self, pts, w=1.25):
        self.d.polygon(self.P(pts), fill=self.v["body"], outline=self.v["edge"], width=max(1, round(w * self.k)))

    def circle(self, x, y, r, w=1.25, fill=None):
        k = self.k
        self.d.ellipse(((x - r) * k, (y - r) * k, (x + r) * k, (y + r) * k), fill=fill or self.v["body"],
                       outline=self.v["edge"], width=max(1, round(w * k)))

    def dot(self, x, y, r, col=None):
        """A square grid cell."""
        k = self.k
        self.d.rectangle(((x - r) * k, (y - r) * k, (x + r) * k, (y + r) * k), fill=col or self.v["dots"])

    def red(self, x, y, r=0.95):
        self.dot(x, y, r, RED)

    def stroke(self, pts, w=1.3):
        """A line drawn as edge colour with a body-coloured halo, so it reads on any background."""
        k = self.k
        self.d.line(self.P(pts), fill=self.v["body"], width=round((w + 1.6) * k), joint="curve")
        for x, y in (pts[0], pts[-1]):
            self.dot(x, y, (w + 1.6) / 2, self.v["body"])
        self.d.line(self.P(pts), fill=self.v["edge"], width=max(1, round(w * k)), joint="curve")
        for x, y in (pts[0], pts[-1]):
            self.dot(x, y, w / 2, self.v["edge"])

    def image(self):
        return self.img.resize((self.size, self.size), Image.LANCZOS)


ARROW = [(1.5, 1.5), (1.5, 19.2), (5.8, 15.2), (8.5, 21.3), (11.4, 20.0), (8.7, 14.0), (14.4, 14.0)]


def arrow(c, accent=True):
    c.shape(ARROW)
    for y in (6.2, 8.6, 11.0):
        c.dot(3.7, y, 0.55)
    if accent:
        c.red(6.3, 12.6, 0.9)


def chevron_arrow(c, x0, y0, x1, y1, head=3.6):
    c.stroke([(x0, y0), (x1, y1)])
    ang = math.atan2(y1 - y0, x1 - x0)
    for (hx, hy), a in (((x0, y0), ang), ((x1, y1), ang + math.pi)):
        c.stroke([(hx + head * math.cos(a + 0.75), hy + head * math.sin(a + 0.75)), (hx, hy),
                  (hx + head * math.cos(a - 0.75), hy + head * math.sin(a - 0.75))])


def ring(c, cx, cy, r, frame, frames, small=False):
    c.circle(cx, cy, r)
    n = 8
    for i in range(n):
        a = 2 * math.pi * i / n - math.pi / 2
        lead = (frame * n // frames) % n
        age = (lead - i) % n
        rr = (0.42 if small else 0.62) * (1.25 if age == 0 else 1.0)
        if age < 4:
            c.dot(cx + r * 0.58 * math.cos(a), cy + r * 0.58 * math.sin(a), rr)
    c.red(cx, cy, 0.75 if small else 1.15)


HAND = [(7.4, 3.4), (8.0, 2.0), (9.0, 1.6), (10.0, 2.0), (10.6, 3.4), (10.6, 9.4), (12.3, 9.1), (13.1, 9.8),
        (14.7, 9.6), (15.5, 10.5), (16.9, 10.5), (17.8, 11.6), (17.8, 16.8), (16.3, 20.4), (8.7, 20.4),
        (5.0, 15.9), (4.4, 13.8), (5.3, 12.9), (7.4, 14.2)]

DESIGNS = {
    "arrow": ((1.5, 1.5), lambda c: arrow(c)),
    "link": ((9.0, 1.8), lambda c: (c.shape(HAND), c.red(9.0, 4.2, 0.85),
                                    [c.dot(x, y, 0.5) for x in (10.8, 12.8, 14.8) for y in (14.6, 16.8)])),
    "text": ((16, 16), lambda c: (c.shape([(12.5, 6), (19.5, 6), (19.5, 7.6), (16.9, 7.6), (16.9, 24.4), (19.5, 24.4),
                                           (19.5, 26), (12.5, 26), (12.5, 24.4), (15.1, 24.4), (15.1, 7.6),
                                           (12.5, 7.6)], 1.0), c.red(16, 16, 0.85))),
    "help": ((1.5, 1.5), lambda c: (arrow(c, False), c.circle(21.5, 21.5, 5.2),
                                    c.stroke([(19.8, 20.2), (20.4, 18.9), (21.8, 18.6), (23.0, 19.4), (22.9, 20.8),
                                              (21.6, 21.8), (21.6, 23.0)], 0.9), c.red(21.6, 25.0, 0.7))),
    "no": ((16, 16), lambda c: (c.circle(16, 16, 8.5, 1.2, RED), c.circle(16, 16, 6.0, 1.0),
                                c.stroke([(11.8, 20.2), (20.2, 11.8)], 1.6), c.red(16, 16, 0.9))),
    "ns": ((16, 16), lambda c: (chevron_arrow(c, 16, 6, 16, 26), c.red(16, 16))),
    "we": ((16, 16), lambda c: (chevron_arrow(c, 6, 16, 26, 16), c.red(16, 16))),
    "nwse": ((16, 16), lambda c: (chevron_arrow(c, 8.5, 8.5, 23.5, 23.5), c.red(16, 16))),
    "nesw": ((16, 16), lambda c: (chevron_arrow(c, 23.5, 8.5, 8.5, 23.5), c.red(16, 16))),
    "move": ((16, 16), lambda c: (chevron_arrow(c, 16, 6, 16, 26), chevron_arrow(c, 6, 16, 26, 16), c.red(16, 16, 1.2))),
    "up": ((16, 5), lambda c: (c.shape([(16, 5), (21.5, 11.5), (18, 11.5), (18, 26), (14, 26), (14, 11.5), (10.5, 11.5)]),
                               [c.dot(16, y, 0.5) for y in (14, 16.4, 18.8)], c.red(16, 22.5, 0.85))),
    "cross": ((16, 16), lambda c: (c.stroke([(16, 6), (16, 12.5)], 1.1), c.stroke([(16, 19.5), (16, 26)], 1.1),
                                   c.stroke([(6, 16), (12.5, 16)], 1.1), c.stroke([(19.5, 16), (26, 16)], 1.1),
                                   c.stroke([(10, 12), (10, 10), (12, 10)], 0.9), c.stroke([(20, 10), (22, 10), (22, 12)], 0.9),
                                   c.stroke([(10, 20), (10, 22), (12, 22)], 0.9), c.stroke([(20, 22), (22, 22), (22, 20)], 0.9),
                                   c.red(16, 16, 1.0))),
    "pen": ((3, 3), lambda c: (c.shape([(3, 3), (8.5, 5.0), (25.5, 22.0), (22.0, 25.5), (5.0, 8.5)]),
                               c.dot(6.9, 7.2, 1.0, RED), [c.dot(12 + i * 2.6, 12 + i * 2.6, 0.5) for i in range(4)])),
    "pin": ((1.5, 1.5), lambda c: (arrow(c, False), c.shape([(21.5, 27.0), (17.0, 21.5), (16.6, 19.0), (18.0, 16.6),
                                                             (21.5, 15.4), (25.0, 16.6), (26.4, 19.0), (26.0, 21.5)], 1.1),
                                   c.red(21.5, 19.8, 1.2))),
    "person": ((1.5, 1.5), lambda c: (arrow(c, False), c.circle(21.5, 21.5, 5.4), c.dot(21.5, 20.0, 1.3),
                                      c.stroke([(19.0, 24.2), (21.5, 22.9), (24.0, 24.2)], 0.9), c.red(25.6, 17.2, 0.8))),
}


def render(fn, size, v):
    c = C(size, v)
    fn(c)
    return c.image()


def cur_bytes(images):
    head = struct.pack("<HHH", 0, 2, len(images))
    offset, entries, body = 6 + 16 * len(images), b"", b""
    for img, (hx, hy) in images:
        s = img.size[0]
        px = img.load()
        xor = bytearray()
        for y in range(s - 1, -1, -1):
            for x in range(s):
                r, g, b, a = px[x, y]
                xor += bytes((b, g, r, a))
        andmask = bytes(((s + 31) // 32) * 4 * s)
        data = struct.pack("<IiiHHIIiiII", 40, s, s * 2, 1, 32, 0, len(xor) + len(andmask), 0, 0, 0, 0) + bytes(xor) + andmask
        entries += struct.pack("<BBBBHHII", s % 256, s % 256, 0, 0, hx, hy, len(data), offset + len(body))
        body += data
    return head + entries + body


def make_cur(fn, hot, v, sizes=SIZES):
    return cur_bytes([(render(fn, s, v), (round(hot[0] * s / 32), round(hot[1] * s / 32))) for s in sizes])


def make_ani(frame_fns, hot, v, jiffies=3, size=64):
    frames = [make_cur(f, hot, v, [size]) for f in frame_fns]
    anih = struct.pack("<9I", 36, len(frames), len(frames), 0, 0, 0, 0, jiffies, 1)
    fram = b"fram" + b"".join(b"icon" + struct.pack("<I", len(f)) + f + (b"\0" if len(f) % 2 else b"") for f in frames)
    body = b"ACON" + b"anih" + struct.pack("<I", 36) + anih + b"LIST" + struct.pack("<I", len(fram)) + fram
    return b"RIFF" + struct.pack("<I", len(body)) + body


if __name__ == "__main__":
    root = sys.argv[1]
    F = 16
    busy = [lambda c, i=i: ring(c, 16, 16, 7.5, i, F) for i in range(F)]
    working = [lambda c, i=i: (arrow(c), ring(c, 21.0, 21.0, 4.6, i, F, True)) for i in range(F)]
    for v in VARIANTS:
        out = os.path.join(root, v)
        os.makedirs(out, exist_ok=True)
        for name, (hot, fn) in DESIGNS.items():
            open(os.path.join(out, f"{name}.cur"), "wb").write(make_cur(fn, hot, v))
        open(os.path.join(out, "busy.ani"), "wb").write(make_ani(busy, (16, 16), v))
        open(os.path.join(out, "working.ani"), "wb").write(make_ani(working, (1.5, 1.5), v))
    names = list(DESIGNS) + ["busy", "working"]
    cell = 80
    sheet = Image.new("RGBA", (len(names) * cell + 10, 2 * cell + 10), (228, 242, 245, 255))
    sheet.paste(Image.new("RGBA", (len(names) * cell + 10, cell + 5), (2, 6, 8, 255)), (0, 0))
    for i, nme in enumerate(names):
        fn = DESIGNS[nme][1] if nme in DESIGNS else (busy[5] if nme == "busy" else working[5])
        sheet.alpha_composite(render(fn, 64, "carbon"), (10 + i * cell, 10))
        sheet.alpha_composite(render(fn, 64, "clear"), (10 + i * cell, cell + 12))
    sheet.convert("RGB").save(os.path.join(root, "preview.png"))
    # real-size check: 32 px cursors on both backgrounds
    small = Image.new("RGBA", (len(names) * 40 + 10, 90), (228, 242, 245, 255))
    small.paste(Image.new("RGBA", (len(names) * 40 + 10, 45), (2, 6, 8, 255)), (0, 0))
    for i, nme in enumerate(names):
        fn = DESIGNS[nme][1] if nme in DESIGNS else (busy[5] if nme == "busy" else working[5])
        small.alpha_composite(render(fn, 32, "carbon"), (8 + i * 40, 6))
        small.alpha_composite(render(fn, 32, "clear"), (8 + i * 40, 52))
    small.convert("RGB").save(os.path.join(root, "preview-32px.png"))
    print("built", len(names), "cursors x", len(VARIANTS), "variants")
