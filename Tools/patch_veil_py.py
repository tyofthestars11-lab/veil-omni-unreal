"""Composition pass on VEIL-OMNI-GAME.py: the runner must be visible, the land must read."""
import io
import sys

P = "D:/VEIL-OMNI/VEIL-OMNI-GAME.py"
s = io.open(P, "r", encoding="utf-8", newline="").read()
fails = []


def sub(old, new, label, count=1):
    global s
    if s.count(old) != count:
        fails.append("%s=%d" % (label, s.count(old)))
        return
    s = s.replace(old, new, count)


# --- zone spawn, so the runner never wakes inside a tower or a cliff ------------
sub('''class Zone:
    def __init__(self, name, platforms, props, npcs, mounts, enemies):
        self.name = name''',
    '''class Zone:
    def __init__(self, name, platforms, props, npcs, mounts, enemies, spawn=0.0):
        self.name = name
        self.spawn = spawn''', "zone spawn")

sub('''zones.append(Zone("SPIRE", [(-2600, 2600, 0.0), (-260, 260, 340.0), (-170, 170, 680.0)],
                      spire_props, spire_npcs, [], []))''',
    '''zones.append(Zone("SPIRE", [(-2600, 2600, 0.0), (-260, 260, 340.0), (-170, 170, 680.0)],
                      spire_props, spire_npcs, [], [], spawn=-900.0))''', "spire spawn")

sub('''zones.append(Zone("DEEP", [(-4000, 4000, 0.0), (-1500, -1100, 250.0), (1000, 1400, 320.0)],
                      deep_props, [], [], [Enemy("DEEP DWELLER", 900, 0, "#7f6fd8")]))''',
    '''zones.append(Zone("DEEP", [(-4000, 4000, 0.0), (-1500, -1100, 250.0), (1000, 1400, 320.0)],
                      deep_props, [], [], [Enemy("DEEP DWELLER", 900, 0, "#7f6fd8")],
                      spawn=-700.0))''', "deep spawn")

sub("            self.runner.x, self.runner.z = 0.0, 0.0",
    "            self.runner.x, self.runner.z = self.zone().spawn, 0.0", "zone travel", 2)

# --- framing: the runner stands clear of the dialogue plate -------------------
sub("        cam = (self.runner.x - 420, self.runner.z - 120)",
    "        cam = (self.runner.x - 760 / SCALE, self.runner.z - 150)", "game cam")
sub("    cam = (r.x - 420, r.z - 120)",
    "    cam = (r.x - 760 / SCALE, r.z - 150)", "shot cam")

# --- dialogue plate moves left, out of the runner's way -----------------------
sub('''        pw, ph = 820, 150
        px, py = (W - pw) / 2, H - 330''',
    '''        pw, ph = 640, 172
        px, py = 24, H - 366''', "dialogue plate")

# --- cast art -----------------------------------------------------------------
start = s.index("def draw_wings(")
end = s.index("def draw_runner(")
s = s[:start] + '''def draw_wings(cv, cx, cy, s, facing, spread=1.0):
    """Four feathers a side, gold blade over blade, with the phi mark at the root."""
    for side in (-1, 1):
        for k in range(4):
            a = math.radians(66 - k * 14)
            length = (72 - k * 13) * s * spread
            bx = cx + side * 7 * s
            by = cy + k * 4 * s
            tip = (bx + side * math.cos(a) * length, by - math.sin(a) * length)
            mid = (bx + side * math.cos(a) * length * 0.52, by - math.sin(a) * length * 0.55 - 5 * s)
            cv.poly([(bx, by), mid, tip, (bx + side * 9 * s, by - 8 * s)],
                    fill=GOLD if k % 2 == 0 else "#d8b25e", outline="#7a5f1f")
    cv.oval(cx - 6 * s, cy - 6 * s, 12 * s, 12 * s, outline=GOLD, width=2)
    cv.line([(cx, cy - 6 * s), (cx, cy + 6 * s)], fill=GOLD, width=1)
    cv.line([(cx - 4 * s, cy), (cx + 4 * s, cy)], fill=GOLD, width=1)


''' + s[end:]

start = s.index("def draw_runner(")
end = s.index("def draw_mount(")
s = s[:start] + '''def draw_runner(cv, r, cam):
    """A body with weight: boots on the stone, a cloak, and gold wings off the shoulders."""
    x, y = sx(r.x, cam), sy(r.z, cam)
    f = 1 if r.facing >= 0 else -1

    # cloak behind, then boots, legs, torso, head, wings in front of the cloak
    cv.poly([(x - 8 * f, y - 148), (x - 54 * f, y - 44), (x - 30 * f, y - 6), (x - 4 * f, y - 40)],
            fill="#1b2440", outline="#0d1324")
    for dx in (-11, 11):
        cv.rect(x + dx * f - 5, y - 72, 10, 64, fill="#2b3450", outline="#161d2e")
        cv.rect(x + dx * f - 9, y - 10, 20, 12, fill="#6b5636", outline="#3a2d1b")
    cv.poly([(x - 20 * f, y - 150), (x + 22 * f, y - 150), (x + 16 * f, y - 64), (x - 16 * f, y - 64)],
            fill="#33456a", outline="#1a2438")
    cv.poly([(x - 24 * f, y - 150), (x, y - 136), (x + 26 * f, y - 150),
             (x + 18 * f, y - 118), (x - 18 * f, y - 118)], fill="#456091", outline="#1a2438")
    cv.oval(x - 11, y - 188, 22, 26, fill="#c99a6a", outline="#5a3f2a")
    cv.poly([(x - 13 * f, y - 184), (x - 22 * f, y - 172), (x - 6 * f, y - 178)], fill="#2a2438")
    draw_wings(cv, x, y - 142, 1.0, f, 1.3 if r.gliding else 1.0)
    if r.mounted:
        draw_mount(cv, r.mounted.kind, x, y + 6, 1.05)


''' + s[end:]

# --- land: cliffs with strata, a tapered fall with mist, a tapered Spire -------
start = s.index('    elif p.kind == "cliff":')
end = s.index('    elif p.kind == "spire":')
s = s[:start] + '''    elif p.kind == "cliff":
        top = y - 430 * s
        cv.poly([(x - 250 * s, y), (x - 180 * s, top), (x + 150 * s, top - 30 * s), (x + 250 * s, y)],
                fill="#3f3b33", outline="#201e19")
        for i in range(4):
            yy = top + 70 + i * 92 * s
            cv.line([(x - 226 * s, yy), (x + 224 * s, yy - 14 * s)], fill="#4d483d", width=3)
        cv.poly([(x - 180 * s, top), (x + 150 * s, top - 30 * s),
                 (x + 120 * s, top + 16), (x - 190 * s, top + 24)],
                fill="#5a5446", outline="#2a2823")
''' + s[end:]

start = s.index('    elif p.kind == "spire":')
end = s.index('    elif p.kind in ("ledge", "terrace"):')
s = s[:start] + '''    elif p.kind == "spire":
        base_w, top_z = 150.0, 900.0
        cv.poly([(x - base_w, y), (x - 52, y - top_z), (x + 52, y - top_z), (x + base_w, y)],
                fill="#b9924a", outline="#6d5320")
        cv.line([(x - base_w * 0.55, y), (x - 20, y - top_z)], fill="#e8c368", width=3)
        for i in range(6):
            zz = y - 150 - i * 118
            cv.line([(x - base_w * (0.9 - i * 0.11), zz), (x + base_w * (0.9 - i * 0.11), zz)],
                    fill="#f3d98b", width=4)
        cv.poly([(x - 52, y - top_z), (x, y - top_z - 210), (x + 52, y - top_z)],
                fill=GOLD, outline="#6d5320")
        cv.oval(x - 78, y - top_z - 238, 156, 156, outline="#ffe6a6", width=2)
''' + s[end:]

start = s.index('    elif p.kind == "waterfall":')
end = s.index('    elif p.kind == "deep":')
s = s[:start] + '''    elif p.kind == "waterfall":
        w0 = 56 * p.size
        cv.poly([(x - w0, y - 900), (x + w0, y - 900),
                 (x + w0 * 1.45, y), (x - w0 * 1.45, y)],
                fill="#2568b8", outline="#1b4a86")
        for i in range(9):
            xx = x - w0 * 0.8 + (i % 3) * w0 * 0.7
            cv.line([(xx, y - 860 + i * 92), (xx + 6, y - 820 + i * 92)], fill="#bfe6ff", width=3)
        cv.oval(x - w0 * 1.9, y - 26, w0 * 3.8, 40, fill="#2c7fd6", outline="#bfe6ff")
        cv.oval(x - w0 * 2.4, y - 58, w0 * 4.8, 78, outline="#9fd8ff", width=1)
''' + s[end:]

if fails:
    print("FAILS:", fails)
    sys.exit(1)
io.open(P, "w", encoding="utf-8", newline="").write(s)
print("composition pass applied")
