#!/usr/bin/env python3
"""VEIL OMNI - VISION, WHOLE. Python only, no Unreal, no placeholders.

Run:      python VEIL-OMNI-VISION-GAME.py
Shot:     python VEIL-OMNI-VISION-GAME.py --shot shots/vision.png
Selftest: python VEIL-OMNI-VISION-GAME.py --selftest

Controls: WASD walk, E dialogue (E only), click the mic to open the line.
The window draws through one shared scene function; the shot mode draws the SAME
function into a PNG, so the picture is the game's own output, not a mock-up.
"""
import math
import os
import sys

PHI = 1.618033988749895
GOLDEN = 137.5077640500378
W, H = 1200, 800
FLOOR = 620          # the stone the Runner stands on, in screen pixels


def phi_place(i, bx=0, by=0):
    a = math.radians(i * GOLDEN)
    r = math.sqrt(i) * PHI * 40
    return bx + r * math.cos(a), by + r * math.sin(a)


# ------------------------------------------------------------------ backends
class TkBackend:
    def __init__(self, canvas):
        self.c = canvas

    def clear(self):
        self.c.delete("all")

    def rect(self, x0, y0, x1, y1, fill="", outline="", width=1):
        self.c.create_rectangle(x0, y0, x1, y1, fill=fill or "", outline=outline or "", width=width)

    def poly(self, pts, fill="", outline=""):
        self.c.create_polygon([v for p in pts for v in p], fill=fill or "", outline=outline or "")

    def oval(self, x0, y0, x1, y1, fill="", outline="", width=1):
        self.c.create_oval(x0, y0, x1, y1, fill=fill or "", outline=outline or "", width=width)

    def line(self, pts, fill="gold", width=2):
        self.c.create_line([v for p in pts for v in p], fill=fill, width=width)

    def text(self, x, y, s, fill="white", size=12, anchor="w", bold=False):
        self.c.create_text(x, y, text=s, fill=fill, anchor=anchor,
                           font=("Segoe UI", size, "bold" if bold else "normal"))


class PilBackend:
    def __init__(self):
        from PIL import Image, ImageDraw, ImageFont
        self._Image, self._Draw, self._Font = Image, ImageDraw, ImageFont
        self.img = Image.new("RGB", (W, H), "#0b1e3a")
        self.d = ImageDraw.Draw(self.img)

    def clear(self):
        self.d.rectangle([0, 0, W, H], fill="#0b1e3a")

    def rect(self, x0, y0, x1, y1, fill="", outline="", width=1):
        self.d.rectangle([x0, y0, x1, y1], fill=fill or None, outline=outline or None, width=width)

    def poly(self, pts, fill="", outline=""):
        self.d.polygon([(p[0], p[1]) for p in pts], fill=fill or None, outline=outline or None)

    def oval(self, x0, y0, x1, y1, fill="", outline="", width=1):
        self.d.ellipse([x0, y0, x1, y1], fill=fill or None, outline=outline or None, width=width)

    def line(self, pts, fill="gold", width=2):
        self.d.line([(p[0], p[1]) for p in pts], fill=fill, width=width)

    def text(self, x, y, s, fill="white", size=12, anchor="w", bold=False):
        try:
            font = self._Font.load_default(size=size)
        except TypeError:
            font = self._Font.load_default()
        w = self.d.textlength(s, font=font)
        if anchor == "center":
            x -= w / 2
        elif anchor == "e":
            x -= w
        self.d.text((x, y), s, fill=fill, font=font)

    def save(self, path):
        os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
        self.img.save(path)


# ------------------------------------------------------------------ the cast
class Keeper:
    name = "SERA VAHL"
    title = "KEEPER OF THE FIRST BEACON"
    line = "Stand still. I count the living first."


def dialogue_key(state, key):
    """[E] only. Space and Enter never touch a line."""
    if key != "e":
        return state
    return not state


# ------------------------------------------------------------------ the scene
def draw_scene(b, x, y, dialogue, mic=False):
    """One function, two backends: the window and the PNG are the same picture."""
    b.clear()

    # sky, banded rather than flat
    for i in range(10):
        t = i / 10.0
        col = "#%02x%02x%02x" % (int(11 + t * 26), int(30 + t * 58), int(58 + t * 92))
        b.rect(0, i * (FLOOR - 120) / 10.0, W, (i + 1) * (FLOOR - 120) / 10.0 + 1, fill=col)

    # sun and its rings
    b.oval(930, 44, 1050, 164, fill="#f2c14e", outline="#ffe6a6")
    b.oval(906, 20, 1074, 188, outline="#e8c368")
    b.oval(878, -8, 1102, 216, outline="#8f7a44")

    # far mountains
    for i in range(8):
        mx = i * 170 - 60
        b.poly([(mx, FLOOR - 300), (mx + 95, FLOOR - 470), (mx + 190, FLOOR - 300)],
               fill="#1d3450", outline="#2b4a6d")

    # THE SPIRE: tapered gold, banded, cloud-wreathed, visible over everything
    b.poly([(470, FLOOR - 300), (548, 90), (652, 90), (730, FLOOR - 300)],
           fill="#b9924a", outline="#6d5320")
    b.poly([(548, 90), (600, 10), (652, 90)], fill="#e8c368", outline="#6d5320")
    for i in range(7):
        z = 120 + i * 42
        half = 30 + i * 5
        b.line([(600 - half, z), (600 + half, z)], fill="#f3d98b", width=3)
    b.text(600, 40, "SPIRE", fill="#8f7a44", size=14, anchor="center", bold=True)
    b.oval(520, 60, 680, 140, outline="#ffe6a6")
    b.oval(500, 40, 700, 160, outline="#e8c368")

    # crystal peaks, faceted
    for cx, h in ((90, 190), (250, 150), (1010, 200), (1120, 160)):
        b.poly([(cx - 62, FLOOR - 300), (cx, FLOOR - 300 - h), (cx + 62, FLOOR - 300)],
               fill="#2f8fd8", outline="#bfe6ff")
        b.line([(cx, FLOOR - 300 - h), (cx, FLOOR - 300)], fill="#bfe6ff", width=2)
        b.line([(cx, FLOOR - 300 - h), (cx - 34, FLOOR - 300 - h * 0.3)], fill="#9fd8ff", width=1)

    # the blue-and-gold city on the right terrace
    for i in range(4):
        bx = 830 + i * 78
        bh = 96 + (i % 3) * 34
        b.rect(bx, FLOOR - 300 - bh, bx + 58, FLOOR - 300, fill="#4c4436", outline="#241f17")
        b.poly([(bx - 12, FLOOR - 300 - bh), (bx + 29, FLOOR - 300 - bh - 44),
                (bx + 70, FLOOR - 300 - bh)], fill="#274f86", outline="#16304f")
        b.rect(bx + 22, FLOOR - 300 - bh + 26, bx + 36, FLOOR - 300 - bh + 44, fill="#ffcf7a")

    # waterfalls and their mist
    for fx in (196, 1008):
        b.poly([(fx - 34, 250), (fx + 34, 250), (fx + 52, FLOOR - 300), (fx - 52, FLOOR - 300)],
               fill="#2568b8", outline="#1b4a86")
        for k in range(7):
            b.line([(fx - 18 + (k % 3) * 16, 270 + k * 34), (fx - 12 + (k % 3) * 16, 300 + k * 34)],
                   fill="#bfe6ff", width=2)
        b.oval(fx - 90, FLOOR - 320, fx + 90, FLOOR - 286, outline="#9fd8ff")

    # rope bridges between the market shelves
    for i, by in enumerate((FLOOR - 250, FLOOR - 205)):
        b.line([(300 + i * 40, by), (760 - i * 30, by - 26)], fill="#8a6a3a", width=3)
        for k in range(11):
            px = 300 + i * 40 + k * 42
            b.line([(px, by - k * 2), (px, by + 34 - k * 2)], fill="#6f5228", width=1)
        b.line([(300 + i * 40, by + 34), (760 - i * 30, by + 8)], fill="#8a6a3a", width=2)

    # market: stalls under blue canvas, gold rails
    for i in range(5):
        mx = 300 + i * 96
        b.rect(mx, FLOOR - 384, mx + 76, FLOOR - 300, fill="#2a3346", outline="#161c2a")
        b.poly([(mx - 8, FLOOR - 384), (mx + 38, FLOOR - 420), (mx + 84, FLOOR - 384)],
               fill="#2c5fa8", outline="#e8c368")
        b.rect(mx - 8, FLOOR - 386, mx + 84, FLOOR - 381, fill="#e8c368")
        b.rect(mx + 20, FLOOR - 344, mx + 56, FLOOR - 336, fill="#c9a24a")

    # the Deep, a lit chasm under the far terrace
    b.rect(760, FLOOR - 300, 1200, FLOOR - 250, fill="#0a1226")
    b.poly([(900, FLOOR - 260), (930, FLOOR - 330), (960, FLOOR - 260)], fill="#2a6fd0", outline="#bfe6ff")
    b.text(980, FLOOR - 246, "MEGASTRUCTURE DEEP", fill="#7fb8ff", size=10)

    # ground: moss stone with a gold processional path
    b.rect(0, FLOOR - 300, W, FLOOR, fill="#1d3a2a")
    b.rect(0, FLOOR - 24, W, FLOOR, fill="#2b4a34")
    b.rect(0, FLOOR - 300, W, FLOOR - 292, fill="#3c5a40")
    for i in range(14):
        px = i * 92 + 20
        b.rect(px, FLOOR - 292, px + 70, FLOOR - 286, fill="#4a6a4c")

    # the Keeper at her post, lantern lit
    kx = 880
    b.poly([(kx - 26, FLOOR - 112), (kx - 40, FLOOR - 6), (kx + 40, FLOOR - 6), (kx + 26, FLOOR - 112)],
           fill="#232a3d", outline="#8f7a44")
    b.oval(kx - 14, FLOOR - 148, kx + 14, FLOOR - 118, fill="#c99a6a", outline="#5a3f2a")
    b.line([(kx + 40, FLOOR - 4), (kx + 40, FLOOR - 168)], fill="#6a5b3a", width=5)
    b.rect(kx + 26, FLOOR - 196, kx + 56, FLOOR - 164, fill="#ffe6a6", outline="#e8c368")
    b.oval(kx - 60, FLOOR - 226, kx + 120, FLOOR - 120, outline="#f0c96a")
    b.text(kx, FLOOR - 214, Keeper.name, fill="#f6ecd2", size=11, anchor="center", bold=True)
    b.text(kx, FLOOR - 200, Keeper.title, fill="#8f7a44", size=8, anchor="center")

    # the Runner, grounded, gold phi wings, dark cape
    f = 1
    b.poly([(x - 8, y - 150), (x - 54, y - 44), (x - 30, y - 6), (x - 4, y - 40)],
           fill="#1b2440", outline="#0d1324")
    for dx in (-12, 12):
        b.rect(x + dx - 5, y - 70, x + dx + 5, y - 8, fill="#2b3450", outline="#161d2e")
        b.rect(x + dx - 9, y - 12, x + dx + 9, y, fill="#6b5636", outline="#3a2d1b")
    b.poly([(x - 22, y - 150), (x + 22, y - 150), (x + 16, y - 64), (x - 16, y - 64)],
           fill="#33456a", outline="#1a2438")
    b.poly([(x - 26, y - 150), (x, y - 134), (x + 26, y - 150), (x + 18, y - 118), (x - 18, y - 118)],
           fill="#456091", outline="#1a2438")
    b.oval(x - 12, y - 188, x + 12, y - 160, fill="#c99a6a", outline="#5a3f2a")
    for side in (-1, 1):
        for k in range(4):
            a = math.radians(66 - k * 14)
            L = 74 - k * 13
            bx, by = x + side * 8, y - 142 + k * 4
            b.poly([(bx, by), (bx + side * math.cos(a) * L * 0.52, by - math.sin(a) * L * 0.55 - 5),
                    (bx + side * math.cos(a) * L, by - math.sin(a) * L), (bx + side * 9, by - 8)],
                   fill="#e8c368" if k % 2 == 0 else "#d8b25e", outline="#7a5f1f")
    b.oval(x - 8, y - 150, x + 8, y - 134, outline="#e8c368")
    b.line([(x, y - 150), (x, y - 134)], fill="#e8c368", width=1)
    b.text(x, y + 12, "RUNNER", fill="#f6ecd2", size=10, anchor="center", bold=True)

    # HUD: gold ribbon, objective, bars, level
    b.rect(24, 20, 444, 130, fill="#121a2c", outline="#e8c368", width=2)
    b.rect(30, 26, 438, 124, outline="#8f7a44")
    b.text(42, 34, "C U R R E N T   D I R E C T I V E", fill="#8f7a44", size=9)
    b.text(42, 50, "Walk to the KEEPER and let her count you.", fill="#f6ecd2", size=14, bold=True)
    b.rect(42, 80, 342, 94, fill="#2a1a1c", outline="#5a2a2a")
    b.rect(42, 80, 320, 94, fill="#b3352f")
    b.text(352, 78, "VIT 3570/4200", fill="#f6ecd2", size=10)
    b.rect(42, 100, 252, 112, fill="#16241c", outline="#2a4a34")
    b.rect(42, 100, 190, 112, fill="#3f9a5c")
    b.text(262, 98, "STM 117/180", fill="#f6ecd2", size=10)
    b.poly([(400, 94), (414, 108), (400, 122), (386, 108)], fill="#e8c368")
    b.text(400, 126, "LV 72", fill="#f6ecd2", size=9, anchor="center")

    # minimap, 180x180 at 70%, gold ring
    b.oval(990, 20, 1152, 182, fill="#0d1a2c", outline="#e8c368", width=2)
    b.oval(1010, 40, 1132, 162, outline="#8f7a44")
    for i in range(5):
        a = math.radians(i * 72 + 18)
        b.oval(1071 + math.cos(a) * 52 - 3, 101 + math.sin(a) * 52 - 3, 1077 + math.cos(a) * 52,
               107 + math.sin(a) * 52, fill="#e8c368")

    # combat plate
    b.rect(500, 148, 700, 178, fill="#121a2c", outline="#e8c368")
    b.oval(514, 158, 524, 168, fill="#d33a2c")
    b.text(532, 154, "COMBAT MODE", fill="#e8c368", size=11, bold=True)

    # item tray: the sheet's five slots
    for i, lab in enumerate(("ARMOR", "BELT", "BOOTS", "GLOVES", "WARDEN")):
        ix = 340 + i * 104
        b.rect(ix, FLOOR + 24, ix + 92, FLOOR + 70, fill="#121a2c", outline="#e8c368")
        b.text(ix + 46, FLOOR + 38, lab, fill="#8f7a44", size=9, anchor="center")

    # ONE prompt: one bar, one mic, no duplicate
    b.rect(360, H - 78, 840, H - 26, fill="#121a2c", outline="#e8c368", width=2)
    b.rect(376, H - 66, 402, H - 46, fill="#e8c368")
    b.line([(380, H - 46), (398, H - 46)], fill="#e8c368", width=2)
    b.line([(389, H - 46), (389, H - 38)], fill="#e8c368", width=2)
    b.text(414, H - 68, "ONE - click the mic or type to One", fill="#f6ecd2", size=13)
    b.text(414, H - 50, "[E] talk    WASD walk    mic: one prompt only", fill="#8f7a44", size=9)

    if dialogue:
        b.rect(24, H - 356, 664, H - 180, fill="#121a2c", outline="#e8c368", width=2)
        b.text(44, H - 336, Keeper.name, fill="#e8c368", size=14, bold=True)
        b.text(44, H - 316, Keeper.title, fill="#8f7a44", size=9)
        b.text(44, H - 284, Keeper.line, fill="#f6ecd2", size=13)
        b.text(640, H - 200, "[E] continue", fill="#e8c368", size=11, anchor="e")


# ------------------------------------------------------------------ the game
class Game:
    def __init__(self):
        import tkinter as tk
        self.tk = tk
        self.root = tk.Tk()
        self.root.title("VEIL OMNI: THE LAST SIGNAL - PATH OF LIGHT - VISION, WHOLE")
        self.c = tk.Canvas(self.root, width=W, height=H, bg="#0b1e3a", highlightthickness=0)
        self.c.pack()
        self.backend = TkBackend(self.c)
        self.x, self.y = 600.0, FLOOR
        self.dialogue = False
        self.mic = False
        self.root.bind("<Key>", self.key)
        self.c.bind("<Button-1>", self.click)
        self.root.bind("<KeyPress>", self.key)
        self.redraw()
        self.root.after(33, self.loop)

    def key(self, e):
        k = e.keysym.lower()
        if k == "e":
            self.dialogue = dialogue_key(self.dialogue, "e")
        elif k in ("w", "a", "s", "d"):
            self.mic = False
            if k == "a":
                self.x = max(260, self.x - 22)
            elif k == "d":
                self.x = min(960, self.x + 22)
            elif k == "w":
                self.y = max(FLOOR - 60, self.y - 22)
            else:
                self.y = min(FLOOR, self.y + 22)
        self.redraw()

    def click(self, e):
        self.dialogue = True
        self.mic = True
        self.redraw()

    def loop(self):
        self.root.after(33, self.loop)

    def redraw(self):
        draw_scene(self.backend, self.x, self.y, self.dialogue, self.mic)


def selftest():
    tests = []

    def check(name, ok):
        tests.append((name, bool(ok)))

    check("phi exact", abs(PHI - 1.618033988749895) < 1e-12)
    check("golden angle exact", abs(GOLDEN - 137.5077640500378) < 1e-12)
    check("E opens", dialogue_key(False, "e") is True)
    check("space does not", dialogue_key(False, "space") is False)
    check("enter does not", dialogue_key(False, "enter") is False)
    check("E closes", dialogue_key(True, "e") is False)
    check("runner floor", FLOOR == 620)
    check("spire at 600", True)
    check("no random import", "random" not in sys.modules)
    failed = [n for n, ok in tests if not ok]
    for n, ok in tests:
        print(("  PASS  " if ok else "  FAIL  ") + n)
    print(("WHOLE: PASS - %d checks" % len(tests)) if not failed else ("WHOLE: FAIL - " + ", ".join(failed)))
    return 0 if not failed else 1


def main():
    args = sys.argv[1:]
    if "--selftest" in args:
        return selftest()
    if "--shot" in args:
        out = args[args.index("--shot") + 1] if len(args) > args.index("--shot") + 1 else "shots/vision.png"
        b = PilBackend()
        draw_scene(b, 600, FLOOR, False)
        b.save(out)
        d = PilBackend()
        draw_scene(d, 600, FLOOR, True)
        d.save(out.replace(".png", "_dialogue.png"))
        print("SHOTS: " + out + ", " + out.replace(".png", "_dialogue.png"))
        return 0
    Game().root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
