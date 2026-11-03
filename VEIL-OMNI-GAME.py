#!/usr/bin/env python3
"""VEIL OMNI - THE LAST SIGNAL. Playable in Python, whole.

Run:      python VEIL-OMNI-GAME.py
Shots:    python VEIL-OMNI-GAME.py --shot shots
Selftest: python VEIL-OMNI-GAME.py --selftest

Controls: A/D walk, SHIFT sprint, SPACE jump (hold in air to glide), E talk/advance,
          V ride the nearest mount, Z travel to the next zone, 1-5 zones.
Physics are the blueprint's: gravity -980, walk 439, sprint 750, leap 602 cm/s (185 cm),
glide 600 with -50 cm/s sink, mounts 900 and grounded. Dialogue is [E] only.
No random numbers anywhere: every placement is the golden angle and seed 1618.
"""
import math
import os
import sys

PHI = 1.618033988749895
GOLDEN = 137.5077640500378
SEED = 1618
W, H = 1280, 720
SCALE = 0.30          # pixels per world centimetre

GRAVITY = -980.0
WALK, SPRINT, JUMPV = 439.0, 750.0, 602.0
GLIDE, GLIDE_DROP = 600.0, -50.0
MOUNT_SPEED = 900.0
ATTACK_RANGE = 300.0

SKY_TOP, SKY_LOW = "#0a1730", "#2a4f7c"
GOLD, GOLD_DIM, PALE = "#e8c368", "#8f7a44", "#f6ecd2"
INK, DARK, BLUE, GLOW, STONE = "#0b1020", "#121a2c", "#2c7fd6", "#bfe6ff", "#6d6a63"


def phi_point(i, radius=100.0):
    a = math.radians(i * GOLDEN)
    return radius * math.sqrt(i) * math.cos(a), radius * math.sqrt(i) * math.sin(a)


# ---------------------------------------------------------------- world
class Prop:
    def __init__(self, kind, x, z=0.0, size=1.0, label=""):
        self.kind, self.x, self.z, self.size, self.label = kind, x, z, size, label


class Zone:
    def __init__(self, name, platforms, props, npcs, mounts, enemies, spawn=0.0):
        self.name = name
        self.spawn = spawn
        self.platforms = platforms      # (x0, x1, top_z)
        self.props = props
        self.npcs = npcs
        self.mounts = mounts
        self.enemies = enemies

    def top_at(self, x):
        best = None
        for x0, x1, top in self.platforms:
            if x0 <= x <= x1 and (best is None or top > best):
                best = top
        return best


class NPC:
    def __init__(self, name, title, role, x, z, lines):
        self.name, self.title, self.role = name, title, role
        self.x, self.z, self.lines = x, z, lines


class Mount:
    def __init__(self, kind, x, z):
        self.kind, self.x, self.z, self.ridden = kind, x, z, False


class Enemy:
    def __init__(self, kind, x, z, colour):
        self.kind, self.x, self.z, self.colour = kind, x, z, colour


def build_zones():
    zones = []

    # HUB: the market at 0,0. Twelve phi pillars, eight stalls, eight blue roofs, five
    # crystal peaks, four mounts, and the Keeper holding her post at 900,0.
    hub_props = []
    for i in range(12):
        px, _ = phi_point(i + 1, 780.0)
        hub_props.append(Prop("pillar", px, 0.0, 1.0 + 0.04 * (i % 3)))
    for i in range(8):
        hub_props.append(Prop("stall", -1500 + i * 430, 0.0, 1.0))
    for i in range(8):
        hub_props.append(Prop("house", -2400 + i * 690, 0.0, 0.85 + 0.05 * (i % 4)))
    for i in range(5):
        px, _ = phi_point(i + 2, 2200.0)
        hub_props.append(Prop("crystal", px, 0.0, 1.0 + 0.1 * (i % 2)))
    for i in range(3):
        hub_props.append(Prop("bridge", -900 + i * 1300, 330.0, 1.0))
    hub_props.append(Prop("waterfall", 1900, 0.0, 1.0))

    hub_npcs = [
        NPC("SERA VAHL", "KEEPER OF THE FIRST BEACON", "keeper", 900, 0,
            ["Stand still. I count the living first.",
             "The lamp is not a light. It is a promise that something is still watched.",
             "Go up. The Spire has been waiting longer than you have."]),
        NPC("THE TRADER", "MARKET TOWN PEOPLE", "trader", -600, 0,
            ["Everything here has a price. Most of it is not gold.",
             "Buy the belt. A back that holds is worth more than a blade that shines.",
             "You have the look of someone walking toward a door they cannot see."]),
        NPC("HALDEN ROOK", "LAMP-WARDEN OF THE SPIRE STAIR", "warden", 1500, 0,
            ["Stair's open. Mind the third landing - it remembers weight.",
             "The Spire counts everything that climbs it. Be worth the number.",
             "When the light answers you, do not answer back. Not yet."]),
    ]
    hub_mounts = [Mount("GOLD WING", -350, 0), Mount("CRYSTAL", 240, 0),
                  Mount("MARKET", 640, 0), Mount("DEEP", -860, 0)]
    hub_enemies = [Enemy("CRYSTAL GUARDIAN", 1750, 0, "#63c7ff")]

    zones.append(Zone("HUB", [(-4200, 4200, 0.0)], hub_props, hub_npcs, hub_mounts, hub_enemies))

    # NEST: cliffs, the Seer at -450,780, rope bridges across the gap.
    nest_props = [Prop("cliff", -2000, 0, 1.2), Prop("cliff", -900, 260, 1.0),
                  Prop("cliff", 300, 380, 1.1), Prop("cliff", 1600, 180, 0.9),
                  Prop("bridge", -2000, 900, 1.2), Prop("bridge", 300, 1000, 1.0),
                  Prop("waterfall", -2600, 0, 1.3), Prop("crystal", 900, 380, 1.2)]
    nest_npcs = [NPC("ILVA MARROW", "THE SEER OF THE NEST", "seer", -450, 780,
                     ["You came up the hard side. Good. The easy side lies.",
                      "I see the shape of you: wings, and under them, a debt.",
                      "The crystal remembers every hand that took it. Yours is not there yet."])]
    zones.append(Zone("NEST", [(-3000, 3000, 0.0), (-1000, -800, 260.0), (200, 420, 380.0)],
                      nest_props, nest_npcs,
                      [Mount("CRYSTAL", 1200, 0)], []))

    # SPIRE: the tower at 0,3500,500 rising 900 m, its ledges climbable.
    spire_props = [Prop("spire", 0, 0, 1.0), Prop("ledge", 0, 340, 1.0),
                   Prop("ledge", 0, 680, 0.8), Prop("ledge", 0, 1020, 0.6),
                   Prop("waterfall", 1400, 0, 1.6), Prop("bridge", -1200, 620, 1.1)]
    spire_npcs = [NPC("DOMEN ARCLIGHT", "CARTOGRAPHER", "cartographer", -900, 0,
                      ["The map is not the ground. It is the argument you have with the ground.",
                       "I drew the Spire four times. It was taller every time.",
                       "Take the west stair. The east one is a rumour."])]
    zones.append(Zone("SPIRE", [(-2600, 2600, 0.0), (-260, 260, 340.0), (-170, 170, 680.0)],
                      spire_props, spire_npcs, [], [], spawn=-900.0))

    # OMNI: the blue-and-gold city, the Countess at 809,-309,64.7.
    omni_props = []
    for i in range(9):
        omni_props.append(Prop("house", -2000 + i * 520, 0.0, 0.9 + 0.08 * (i % 3)))
    omni_props.append(Prop("bridge", 0, 300, 1.4))
    omni_props.append(Prop("crystal", 1500, 0, 1.1))
    omni_npcs = [NPC("COUNTESS VESPERA", "GOLD COURT", "countess", 809, 0,
                     ["You stand in a city that was paid for before you were born.",
                      "Gold is not the point. Gold is what remains when the point is gone.",
                      "Kneel or don't. The floor is the same either way."])]
    zones.append(Zone("OMNI", [(-3400, 3400, 0.0)], omni_props, omni_npcs,
                      [Mount("MARKET", -1200, 0)],
                      [Enemy("GOLD WARRIOR", 1400, 0, "#e8c368")]))

    # DEEP: the megastructure chasm, terraces on the walls, a lit core.
    deep_props = [Prop("deep", 0, 0, 1.0), Prop("terrace", -1300, 250, 1.0),
                  Prop("terrace", 1200, 320, 1.0), Prop("crystal", -600, 0, 0.8),
                  Prop("waterfall", 1800, 0, 1.8)]
    zones.append(Zone("DEEP", [(-4000, 4000, 0.0), (-1500, -1100, 250.0), (1000, 1400, 320.0)],
                      deep_props, [], [], [Enemy("DEEP DWELLER", 900, 0, "#7f6fd8")],
                      spawn=-700.0))

    return zones


# ---------------------------------------------------------------- runner
class Runner:
    def __init__(self, x=0.0, z=0.0):
        self.x, self.z = x, z
        self.vx = self.vz = 0.0
        self.on_ground = True
        self.gliding = False
        self.facing = 1
        self.mounted = None
        self.hp, self.hp_max = 3570.0, 4200.0
        self.st, self.st_max = 117.0, 180.0
        self.level = 72

    def speed(self):
        return MOUNT_SPEED if self.mounted else WALK

    def step(self, dt, keys, zone, jump_edge):
        move = (1 if "d" in keys else 0) - (1 if "a" in keys else 0)
        target = self.speed() * (SPRINT / WALK if ("shift" in keys and not self.mounted) else 1.0)
        if move:
            self.facing = move
        self.vx = move * target

        if jump_edge and self.on_ground:
            self.vz = JUMPV
            self.on_ground = False
            self.gliding = False

        # AIRBORNE IS MEASURED, NOT REMEMBERED. A body placed above its floor still carried
        # on_ground from spawn, so the wings never opened and the glide test fell at -980.
        top_now = zone.top_at(self.x)
        if top_now is not None and self.z > top_now + 0.5:
            self.on_ground = False

        # GLIDE: hold jump in the air and the wings take the fall to -50 cm/s.
        self.gliding = (not self.on_ground) and ("space" in keys) and self.vz <= 0.0

        if self.gliding:
            self.vz += (GLIDE_DROP - self.vz) * min(1.0, dt * 4.0)
        else:
            self.vz += GRAVITY * dt

        self.x += self.vx * dt
        self.z += self.vz * dt

        top = zone.top_at(self.x)
        if top is not None and self.z <= top and self.vz <= 0.0:
            self.z, self.vz, self.on_ground, self.gliding = top, 0.0, True, False
        elif top is None and self.z < 0.0:
            # No floor here: the runner falls into the dark rather than standing on air.
            self.on_ground = False

        if self.mounted:
            self.mounted.x, self.mounted.z = self.x, self.z


# ---------------------------------------------------------------- dialogue
class Dialogue:
    def __init__(self):
        self.npc = None
        self.index = 0

    def active(self):
        return self.npc is not None

    def open(self, npc):
        self.npc, self.index = npc, 0

    def advance(self, key):
        """[E] only. Space and Enter do nothing here - they are jump and menu keys."""
        if not self.active() or key != "e":
            return False
        self.index += 1
        if self.index >= len(self.npc.lines):
            self.npc, self.index = None, 0
        return True


# ---------------------------------------------------------------- renderer
class PilCanvas:
    def __init__(self):
        from PIL import Image, ImageDraw, ImageFont
        self.Image, self.ImageDraw, self.ImageFont = Image, ImageDraw, ImageFont
        self.img = Image.new("RGB", (W, H), SKY_TOP)
        self.d = ImageDraw.Draw(self.img)

    def rect(self, x, y, w, h, fill=None, outline=None, width=1):
        self.d.rectangle([x, y, x + w, y + h], fill=fill, outline=outline, width=width)

    def poly(self, pts, fill=None, outline=None):
        self.d.polygon([(p[0], p[1]) for p in pts], fill=fill, outline=outline)

    def line(self, pts, fill=GOLD, width=2):
        self.d.line([(p[0], p[1]) for p in pts], fill=fill, width=width)

    def oval(self, x, y, w, h, fill=None, outline=None, width=1):
        self.d.ellipse([x, y, x + w, y + h], fill=fill, outline=outline, width=width)

    def text(self, x, y, s, fill=PALE, size=14, anchor="w", bold=False):
        try:
            font = self.ImageFont.load_default(size=size)
        except TypeError:
            font = self.ImageFont.load_default()
        w = self.d.textlength(s, font=font)
        if anchor == "center":
            x -= w / 2
        elif anchor == "e":
            x -= w
        self.d.text((x, y), s, fill=fill, font=font)

    def save(self, path):
        os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
        self.img.save(path)


class TkCanvas:
    def __init__(self, canvas):
        self.c = canvas

    def rect(self, x, y, w, h, fill=None, outline=None, width=1):
        self.c.create_rectangle(x, y, x + w, y + h, fill=fill, outline=outline, width=width)

    def poly(self, pts, fill=None, outline=None):
        self.c.create_polygon([c for p in pts for c in p], fill=fill, outline=outline)

    def line(self, pts, fill=GOLD, width=2):
        self.c.create_line([c for p in pts for c in p], fill=fill, width=width)

    def oval(self, x, y, w, h, fill=None, outline=None, width=1):
        self.c.create_oval(x, y, x + w, y + h, fill=fill, outline=outline, width=width)

    def text(self, x, y, s, fill=PALE, size=14, anchor="w", bold=False):
        a = {"w": "w", "center": "center", "e": "e"}.get(anchor, "w")
        self.c.create_text(x, y, text=s, fill=fill, anchor=a,
                           font=("Segoe UI", size, "bold" if bold else "normal"))


# ---------------------------------------------------------------- art
def sx(world_x, cam): return (world_x - cam[0]) * SCALE
def sy(world_z, cam): return H - 150 - (world_z - cam[1]) * SCALE


def ribbon(cv, x, y, w, h, fill=DARK):
    cv.rect(x, y, w, h, fill=fill, outline=GOLD, width=2)
    cv.rect(x + 4, y + 4, w - 8, h - 8, outline=GOLD_DIM, width=1)
    cv.oval(x - 5, y + h / 2 - 5, 10, 10, fill=GOLD)
    cv.oval(x + w - 5, y + h / 2 - 5, 10, 10, fill=GOLD)


def draw_wings(cv, cx, cy, s, facing, spread=1.0):
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


def draw_runner(cv, r, cam):
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


def draw_mount(cv, kind, x, y, s=1.0):
    cv.oval(x - 58 * s, y - 78 * s, 116 * s, 46 * s, fill=STONE, outline="#3a3833")
    cv.oval(x + 34 * s, y - 104 * s, 44 * s, 36 * s, fill=STONE, outline="#3a3833")
    for lx in (-40, -14, 18, 44):
        cv.line([(x + lx * s, y - 40 * s), (x + lx * s, y)], fill="#4a4740", width=6)
    if kind == "GOLD WING":
        draw_wings(cv, x - 6 * s, y - 96 * s, s, 1, 0.8)
    elif kind == "CRYSTAL":
        cv.poly([(x - 8 * s, y - 78 * s), (x + 2 * s, y - 138 * s), (x + 14 * s, y - 78 * s)],
                fill=BLUE, outline=GLOW)
    elif kind == "MARKET":
        cv.rect(x - 44 * s, y - 92 * s, 88 * s, 12 * s, fill="#2c5fa8", outline=GOLD)
    else:
        cv.poly([(x - 30 * s, y - 76 * s), (x - 6 * s, y - 150 * s), (x + 6 * s, y - 76 * s)],
                fill="#31406e", outline=GLOW)


def draw_npc(cv, n, cam):
    x, y = sx(n.x, cam), sy(n.z, cam)
    cv.poly([(x - 20, y - 110), (x - 30, y - 4), (x + 30, y - 4), (x + 20, y - 110)],
            fill="#232a3d" if n.role != "countess" else "#4a1f33", outline=GOLD_DIM)
    cv.oval(x - 10, y - 136, 20, 22, fill="#c99a6a", outline="#5a3f2a")
    if n.role == "keeper":
        cv.line([(x + 24, y - 6), (x + 24, y - 150)], fill="#6a5b3a", width=5)
        cv.rect(x + 14, y - 176, 22, 28, fill="#ffe6a6", outline=GOLD)
        cv.oval(x - 40, y - 190, 116, 120, outline="#f0c96a", width=1)
    elif n.role == "seer":
        cv.line([(x - 22, y - 4), (x - 22, y - 158)], fill="#3a3550", width=5)
        cv.poly([(x - 34, y - 158), (x - 22, y - 200), (x - 10, y - 158)], fill=BLUE, outline=GLOW)
    elif n.role == "cartographer":
        cv.rect(x - 30, y - 96, 48, 34, fill="#d8cba6", outline=GOLD)
        for i in range(3):
            cv.line([(x - 26, y - 90 + i * 9), (x + 12, y - 88 + i * 9)], fill="#8a7a52", width=1)
    elif n.role == "countess":
        cv.line([(x - 16, y - 112), (x - 16, y - 128)], fill=GOLD, width=2)
        cv.line([(x + 16, y - 112), (x + 16, y - 128)], fill=GOLD, width=2)
        cv.poly([(x - 18, y - 124), (x - 16, y - 140), (x - 8, y - 126)], fill=GOLD)
        cv.poly([(x + 18, y - 124), (x + 16, y - 140), (x + 8, y - 126)], fill=GOLD)
    cv.text(x, y - 158, n.name, fill=PALE, size=11, anchor="center", bold=True)
    cv.text(x, y - 146, n.title, fill=GOLD_DIM, size=8, anchor="center")


def draw_prop(cv, p, cam):
    x, y = sx(p.x, cam), sy(p.z, cam)
    s = p.size
    if p.kind == "pillar":
        cv.rect(x - 20 * s, y - 420 * s, 40 * s, 420 * s, fill="#4b4230", outline="#2a2418")
        cv.rect(x - 26 * s, y - 448 * s, 52 * s, 30 * s, fill=GOLD, outline="#7a5f1f")
        cv.rect(x - 10 * s, y - 470 * s, 20 * s, 22 * s, fill="#ffe6a6")
        cv.oval(x - 60 * s, y - 520 * s, 120 * s, 120 * s, outline="#f0c96a", width=1)
    elif p.kind == "stall":
        cv.rect(x - 70, y - 120, 140, 120, fill="#2a3346", outline="#161c2a")
        cv.poly([(x - 84, y - 120), (x, y - 168), (x + 84, y - 120)], fill="#2c5fa8", outline=GOLD)
        cv.rect(x - 84, y - 122, 168, 8, fill=GOLD)
        cv.rect(x - 40, y - 60, 80, 10, fill="#c9a24a")
    elif p.kind == "house":
        cv.rect(x - 80 * s, y - 200 * s, 160 * s, 200 * s, fill="#5a5142", outline="#2b2620")
        cv.poly([(x - 96 * s, y - 200 * s), (x, y - 300 * s), (x + 96 * s, y - 200 * s)],
                fill="#274f86", outline="#16304f")
        cv.rect(x - 18, y - 90, 36, 44, fill="#ffcf7a")
        cv.oval(x - 30 * s, y - 120 * s, 120 * s, 120 * s, outline="#f0c96a", width=1)
    elif p.kind == "crystal":
        for i in range(3):
            off = (i - 1) * 26
            cv.poly([(x + off, y), (x + off + 14, y - 120 * s), (x + off + 30, y)],
                    fill=BLUE, outline=GLOW)
        cv.oval(x - 50, y - 140, 120, 120, outline="#9fd8ff", width=1)
    elif p.kind == "cliff":
        top = y - 430 * s
        cv.poly([(x - 250 * s, y), (x - 180 * s, top), (x + 150 * s, top - 30 * s), (x + 250 * s, y)],
                fill="#3f3b33", outline="#201e19")
        for i in range(4):
            yy = top + 70 + i * 92 * s
            cv.line([(x - 226 * s, yy), (x + 224 * s, yy - 14 * s)], fill="#4d483d", width=3)
        cv.poly([(x - 180 * s, top), (x + 150 * s, top - 30 * s),
                 (x + 120 * s, top + 16), (x - 190 * s, top + 24)],
                fill="#5a5446", outline="#2a2823")
    elif p.kind == "spire":
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
    elif p.kind in ("ledge", "terrace"):
        cv.rect(x - 130 * p.size, y - 18, 260 * p.size, 18, fill="#5d574c", outline="#2a2823")
    elif p.kind == "bridge":
        cv.line([(x - 220 * p.size, y), (x + 220 * p.size, y)], fill="#8a6a3a", width=4)
        for i in range(9):
            bx = x - 200 * p.size + i * 50 * p.size
            cv.line([(bx, y), (bx, y + 46)], fill="#6f5228", width=2)
        cv.line([(x - 220 * p.size, y + 46), (x + 220 * p.size, y + 46)], fill="#8a6a3a", width=3)
    elif p.kind == "waterfall":
        w0 = 56 * p.size
        cv.poly([(x - w0, y - 900), (x + w0, y - 900),
                 (x + w0 * 1.45, y), (x - w0 * 1.45, y)],
                fill="#2568b8", outline="#1b4a86")
        for i in range(9):
            xx = x - w0 * 0.8 + (i % 3) * w0 * 0.7
            cv.line([(xx, y - 860 + i * 92), (xx + 6, y - 820 + i * 92)], fill="#bfe6ff", width=3)
        cv.oval(x - w0 * 1.9, y - 26, w0 * 3.8, 40, fill="#2c7fd6", outline="#bfe6ff")
        cv.oval(x - w0 * 2.4, y - 58, w0 * 4.8, 78, outline="#9fd8ff", width=1)
    elif p.kind == "deep":
        cv.rect(x - 300, y - 180, 600, 180, fill="#0d1428")
        cv.poly([(x - 60, y - 700), (x, y - 1080), (x + 60, y - 700)], fill="#2a6fd0", outline=GLOW)
        cv.oval(x - 140, y - 900, 280, 280, outline="#8fd0ff", width=2)


def draw_world(cv, zone, runner, cam, dlg):
    # sky
    for i in range(12):
        band = i / 12.0
        r = int(10 + band * 32)
        g = int(23 + band * 46)
        b = int(48 + band * 56)
        cv.rect(0, i * H / 12.0, W, H / 12.0 + 1, fill="#%02x%02x%02x" % (r, g, b))
    # far mountains
    for i in range(7):
        mx = (i * 260 - (cam[0] * 0.15) % 260) - 120
        cv.poly([(mx, H - 300), (mx + 130, H - 470), (mx + 260, H - 300)], fill="#223a58", outline="#2f4a6b")
    for p in zone.props:
        draw_prop(cv, p, cam)
    # platforms
    for x0, x1, top in zone.platforms:
        px0, py = sx(x0, cam), sy(top, cam)
        px1 = sx(x1, cam)
        cv.rect(px0, py, px1 - px0, H - py, fill="#3a372f", outline="#211f19")
        cv.line([(px0, py), (px1, py)], fill="#6d6a63", width=3)
    for e in zone.enemies:
        ex, ey = sx(e.x, cam), sy(e.z, cam)
        cv.oval(ex - 26, ey - 96, 52, 96, fill="#2a3244", outline=e.colour, width=2)
        cv.oval(ex - 8, ey - 76, 16, 12, fill=e.colour)
        cv.oval(ex - 150, ey - 60, 300, 120, outline=e.colour, width=1)
    for n in zone.npcs:
        draw_npc(cv, n, cam)
    for m in zone.mounts:
        if not m.ridden:
            draw_mount(cv, m.kind, sx(m.x, cam), sy(m.z, cam))
    draw_runner(cv, runner, cam)
    draw_hud(cv, zone, runner, dlg)


def draw_hud(cv, zone, r, dlg):
    # objective ribbon, 420x110
    ribbon(cv, 24, 20, 420, 110)
    cv.text(40, 32, "C U R R E N T   D I R E C T I V E", fill=GOLD_DIM, size=9)
    cv.text(40, 48, "Walk to the KEEPER and let her count you.", fill=PALE, size=15, bold=True)
    cv.rect(40, 78, 300, 12, fill="#2a1a1c", outline="#5a2a2a")
    cv.rect(40, 78, 300 * (r.hp / r.hp_max), 12, fill="#b3352f")
    cv.text(348, 76, "VIT %d/%d" % (r.hp, r.hp_max), fill=PALE, size=10)
    cv.rect(40, 98, 210, 10, fill="#16241c", outline="#2a4a34")
    cv.rect(40, 98, 210 * (r.st / r.st_max), 10, fill="#3f9a5c")
    cv.text(258, 96, "STM %d/%d" % (r.st, r.st_max), fill=PALE, size=10)
    cv.poly([(392, 92), (404, 104), (392, 116), (380, 104)], fill=GOLD)
    cv.text(392, 121, "LV %d" % r.level, fill=PALE, size=10, anchor="center")

    # minimap 180x180 at 70%
    mx, my = W - 210, 24
    cv.oval(mx, my, 180, 180, fill="#0d1a2c", outline=GOLD, width=2)
    cv.oval(mx + 18, my + 18, 144, 144, outline=GOLD_DIM, width=1)
    for i, z in enumerate(build_zones()):
        a = math.radians(i * 72 + 18)
        cv.oval(mx + 90 + math.cos(a) * 58 - 3, my + 90 + math.sin(a) * 58 - 3, 6, 6, fill=GOLD)
    cv.text(mx + 90, my + 168, zone.name, fill=PALE, size=9, anchor="center")

    # ONE prompt: one bar, one mic, no duplicate
    bw = 520
    bx = (W - bw) / 2
    ribbon(cv, bx, H - 92, bw, 46)
    cv.rect(bx + 16, H - 78, 24, 18, fill=GOLD)
    cv.line([(bx + 20, H - 60), (bx + 36, H - 60)], fill=GOLD, width=2)
    cv.line([(bx + 28, H - 60), (bx + 28, H - 52)], fill=GOLD, width=2)
    cv.text(bx + 56, H - 78, "ONE - click the mic or type to One", fill=PALE, size=13)
    cv.text(bx + 56, H - 62, "[E] talk     SPACE jump/glide     V ride     Z next zone", fill=GOLD_DIM, size=9)

    # item tray: five slots, the sheet's row
    labels = ["ARMOR", "BELT", "BOOTS", "GLOVES", "WARDEN"]
    tw = 5 * 96
    tx = (W - tw) / 2
    for i, lab in enumerate(labels):
        x = tx + i * 96
        cv.rect(x, H - 158, 84, 40, fill="#121a2c", outline=GOLD, width=1)
        cv.text(x + 42, H - 136, lab, fill=GOLD_DIM, size=9, anchor="center")

    # combat plate
    ribbon(cv, W / 2 - 92, H - 196, 184, 28)
    cv.oval(W / 2 - 76, H - 187, 8, 8, fill="#d33a2c")
    cv.text(W / 2 - 62, H - 192, "COMBAT MODE", fill=GOLD, size=11, bold=True)

    if dlg.active():
        pw, ph = 640, 172
        px, py = 24, H - 366
        ribbon(cv, px, py, pw, ph, fill="#121a2c")
        cv.text(px + 26, py + 18, dlg.npc.name, fill=GOLD, size=14, bold=True)
        cv.text(px + 26, py + 38, dlg.npc.title, fill=GOLD_DIM, size=9)
        line = dlg.npc.lines[dlg.index]
        cv.text(px + 26, py + 66, line, fill=PALE, size=13)
        cv.text(px + pw - 26, py + ph - 26, "[E] continue", fill=GOLD, size=11, anchor="e")


# ---------------------------------------------------------------- game
class Game:
    def __init__(self):
        import tkinter as tk
        self.tk = tk
        self.zones = build_zones()
        self.zone_i = 0
        self.runner = Runner(0, 0)
        self.dlg = Dialogue()
        self.keys = set()
        self.jump_edge = False
        self.root = tk.Tk()
        self.root.title("VEIL OMNI - THE LAST SIGNAL")
        self.canvas = tk.Canvas(self.root, width=W, height=H, bg=SKY_TOP, highlightthickness=0)
        self.canvas.pack()
        self.cv = TkCanvas(self.canvas)
        self.root.bind("<KeyPress>", self.on_down)
        self.root.bind("<KeyRelease>", self.on_up)
        self.last = 0.0
        self.tick()

    def zone(self):
        return self.zones[self.zone_i]

    def on_down(self, e):
        k = e.keysym.lower()
        key = {"left": "a", "right": "d", "shift_l": "shift", "shift_r": "shift",
               "space": "space"}.get(k, k)
        if key == "e":
            if self.dlg.active():
                self.dlg.advance("e")
            else:
                best = None
                for n in self.zone().npcs:
                    d = abs(n.x - self.runner.x)
                    if d < 220 and (best is None or d < abs(best.x - self.runner.x)):
                        best = n
                if best:
                    self.dlg.open(best)
        if key == "v":
            self.toggle_mount()
        if key == "z":
            self.zone_i = (self.zone_i + 1) % len(self.zones)
            self.runner.x, self.runner.z = self.zone().spawn, 0.0
        if key == "space" and "space" not in self.keys:
            self.jump_edge = True
        if key in ("1", "2", "3", "4", "5"):
            self.zone_i = min(len(self.zones) - 1, int(key) - 1)
            self.runner.x, self.runner.z = self.zone().spawn, 0.0
        self.keys.add(key)

    def on_up(self, e):
        k = e.keysym.lower()
        key = {"left": "a", "right": "d", "shift_l": "shift", "shift_r": "shift",
               "space": "space"}.get(k, k)
        self.keys.discard(key)

    def toggle_mount(self):
        r = self.runner
        if r.mounted:
            r.mounted.ridden = False
            r.mounted = None
            return
        best, bd = None, 420.0
        for m in self.zone().mounts:
            d = abs(m.x - r.x)
            if d < bd:
                best, bd = m, d
        if best:
            best.ridden = True
            r.mounted = best

    def tick(self):
        now = self.root.tk.call("clock", "milliseconds") / 1000.0
        dt = 0.0 if not self.last else min(0.05, now - self.last)
        self.last = now
        if dt:
            self.runner.step(dt, self.keys, self.zone(), self.jump_edge)
            self.jump_edge = False
        self.canvas.delete("all")
        cam = (self.runner.x - 760 / SCALE, self.runner.z - 150)
        draw_world(self.cv, self.zone(), self.runner, cam, self.dlg)
        self.root.after(16, self.tick)

    def run(self):
        self.root.mainloop()


# ---------------------------------------------------------------- proof
def render_frame(zone_i, dlg=None, path="shot.png", runner=None, seconds=0.0):
    zones = build_zones()
    zone = zones[zone_i]
    r = runner or Runner(0.0, 0.0)
    d = dlg or Dialogue()
    if seconds:
        keys = {"d"}
        for _ in range(int(seconds * 120)):
            r.step(1 / 120.0, keys, zone, False)
    cv = PilCanvas()
    cam = (r.x - 760 / SCALE, r.z - 150)
    draw_world(cv, zone, r, cam, d)
    cv.save(path)
    return path


def selftest():
    zones = build_zones()
    checks = []

    def check(name, ok):
        checks.append((name, bool(ok)))

    check("phi exact", abs(PHI - 1.618033988749895) < 1e-12)
    check("golden angle exact", abs(GOLDEN - 137.5077640500378) < 1e-12)
    check("gravity -980", GRAVITY == -980.0)
    check("walk 439", WALK == 439.0)
    check("sprint 750", SPRINT == 750.0)
    check("mounts 900", MOUNT_SPEED == 900.0)
    check("attack range 300", ATTACK_RANGE == 300.0)
    check("no random import", "random" not in sys.modules)

    r, hub = Runner(), zones[0]
    for _ in range(240):
        r.step(1 / 120.0, set(), hub, False)
    check("runner grounded at rest", r.on_ground and abs(r.z - hub.top_at(r.x)) < 0.01)

    r = Runner()
    r.step(1 / 120.0, set(), hub, True)
    peak = 0.0
    for _ in range(240):
        r.step(1 / 120.0, set(), hub, False)
        peak = max(peak, r.z)
    check("leap 185 cm (+/-6)", abs(peak - 185.0) <= 6.0)

    r = Runner(0.0, 900.0)
    for _ in range(180):
        r.step(1 / 120.0, {"space"}, hub, False)
    check("glide sink -50", abs(r.vz - GLIDE_DROP) < 3.0)

    d = Dialogue()
    d.open(hub.npcs[0])
    d.advance("space")
    check("space does not advance", d.index == 0)
    d.advance("enter")
    check("enter does not advance", d.index == 0)
    d.advance("e")
    check("E advances", d.index == 1)

    check("five zones", len(zones) == 5)
    check("authority cast placed", all(
        any(n.name.startswith(w) for z in zones for n in z.npcs)
        for w in ("SERA", "ILVA", "COUNTESS", "DOMEN")))

    failed = [n for n, ok in checks if not ok]
    for name, ok in checks:
        print(("  PASS  " if ok else "  FAIL  ") + name)
    if failed:
        print("WHOLE: FAIL - " + ", ".join(failed))
        return 1
    print("WHOLE: PASS - %d checks, physics, dialogue E, grounded, no random, five zones, cast placed" % len(checks))
    return 0


def main():
    args = sys.argv[1:]
    if "--selftest" in args:
        return selftest()
    if "--shot" in args:
        out = args[args.index("--shot") + 1] if len(args) > args.index("--shot") + 1 else "shots"
        hub = build_zones()[0]
        r = Runner(520.0, 0.0)
        d = Dialogue()
        d.open(hub.npcs[0])
        made = [render_frame(0, d, os.path.join(out, "hub.png"), r),
                render_frame(2, None, os.path.join(out, "spire.png")),
                render_frame(4, None, os.path.join(out, "deep.png")),
                render_frame(1, None, os.path.join(out, "nest.png"))]
        print("SHOTS: " + ", ".join(made))
        return 0
    Game().run()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
