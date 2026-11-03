"""VEIL OMNI - gold ribbon HUD and menu, drawn from primitives on any backend.

Every panel here is built from rectangles, diamonds and lines, so the same code
draws the tkinter window and the PNG proof shots. No textures, no placeholders.
"""
import math

GOLD = "#e8c368"
GOLD_DEEP = "#8f7a44"
PALE = "#f6ecd2"
INK = "#070d18"
PANEL = "#0d1626"
PANEL_2 = "#101c30"
RED = "#c8382f"
BLUE = "#3f8fd8"
TEAL = "#35c9b8"
GREEN = "#3f9a5c"


class UIState:
    """What the HUD shows. One object, so the window and the shots agree."""

    def __init__(self):
        self.menu_open = False
        self.tab = 0
        self.tabs = ("SETTINGS", "QUESTS", "MAP", "INVENTORY")
        self.hp, self.hp_max = 320, 320
        self.shield, self.shield_max = 150, 150
        self.level, self.cls = 14, "WARDEN"
        self.credits = 12450
        self.quest = "SECURE THE RELIC"
        self.objective = "Retrieve the Warden's Sigil in the Lower District"
        self.progress = (1, 3)
        self.ammo = (28, 120)
        self.threat = "MODERATE"
        self.ability = "OVERCHARGE"
        self.ability_ready = True
        self.player = "ARIN SKYWARD"
        self.gold, self.load, self.power = 12450, (48, 60), 1240
        self.loadout = [
            ("ARMOR", "PLATED ARMOR MK-II", "98% DURABILITY", "PROTECTED"),
            ("BELT", "UTILITY BELT", "4/4 POUCHES", "FULL"),
            ("BOOTS", "STEALTH BOOTS", "MOVE SPEED +10%", "ENHANCED"),
            ("GLOVES", "GRIP GLOVES", "MELEE BONUS +5", "ENHANCED"),
        ]
        self.wheel = ["MEDKIT", "GRENADE", "SCAN PULSE", "EMP DISCHARGE",
                      "CLOAK", "SCOUT DRONE", "SIGNAL LANCE", "WARDEN"]
        self.sliders = [("MASTER VOLUME", 0.80), ("MUSIC VOLUME", 0.65),
                        ("SFX VOLUME", 0.72), ("BRIGHTNESS", 0.55), ("BLOOM EFFECT", 0.60)]
        self.inventory = [
            ("CRYSTAL SWORD", "DMG 124 - RARE"),
            ("GOLDEN PLATE", "ARMOR 86 - RARE"),
            ("CRYSTAL BOW", "DMG 98 - RARE"),
            ("GUARDIAN SHIELD", "BLOCK 45 - RARE"),
            ("CRYSTAL DAGGER", "DMG 52 - UNCOMMON"),
            ("GOLD GREAVES", "ARMOR 40 - RARE"),
            ("SHARD AMULET", "MAGIC +12 - RARE"),
        ]
        self.quests = [
            ("The Shattered Crystal", "Recover the Crystal of Veil from the Sunken Ruins", 2, 3),
            ("Bandit Camp Clearance", "Defeat 5 Bandit Captains in Whispering Woods", 3, 5),
            ("Escort the Caravan", "Safely escort caravan to Dawnreach Port", 7, 10),
        ]


# ------------------------------------------------------------------ pieces
def frame(b, x, y, w, h, fill=PANEL, thick=2):
    b.rect(x, y, x + w, y + h, fill=fill, outline=GOLD, width=thick)
    b.rect(x + 4, y + 4, x + w - 4, y + h - 4, outline=GOLD_DEEP)
    for cx, cy in ((x, y), (x + w, y), (x, y + h), (x + w, y + h)):
        b.poly([(cx - 6, cy), (cx, cy - 6), (cx + 6, cy), (cx, cy + 6)], fill=GOLD)


def ribbon(b, x, y, w, h, text, size=18):
    b.rect(x, y, x + w, y + h, fill=PANEL_2, outline=GOLD, width=2)
    b.rect(x + 6, y + 6, x + w - 6, y + h - 6, outline=GOLD_DEEP)
    for side in (-1, 1):
        cx = x if side < 0 else x + w
        b.poly([(cx, y + h * 0.3), (cx + side * 26, y + h * 0.15),
                (cx + side * 26, y + h * 0.85), (cx, y + h * 0.7)], fill=GOLD_DEEP, outline=GOLD)
    b.text(x + w / 2, y + h * 0.5 - size * 0.6, text, fill=GOLD, size=size, anchor="center", bold=True)


def bar(b, x, y, w, h, frac, fill, back="#241a1c"):
    b.rect(x, y, x + w, y + h, fill=back, outline="#4a2a2a")
    frac = max(0.0, min(1.0, frac))
    if frac > 0:
        b.rect(x, y, x + w * frac, y + h, fill=fill)


def mic(b, cx, cy, s=1.0):
    b.rect(cx - 9 * s, cy - 20 * s, cx + 9 * s, cy + 4 * s, fill=GOLD)
    b.oval(cx - 14 * s, cy - 4 * s, cx + 14 * s, cy + 16 * s, outline=GOLD, width=2)
    b.line([(cx, cy + 16 * s), (cx, cy + 24 * s)], fill=GOLD, width=2)
    b.line([(cx - 10 * s, cy + 24 * s), (cx + 10 * s, cy + 24 * s)], fill=GOLD, width=2)


def waveform(b, x, y, w, h, seed=0):
    for i in range(18):
        t = i / 17.0
        a = abs(math.sin(t * 9.0 + seed)) * 0.7 + 0.15
        px = x + t * w
        b.line([(px, y + h / 2 - a * h / 2), (px, y + h / 2 + a * h / 2)], fill=GOLD_DEEP, width=1)


def diamond(b, cx, cy, r, fill=GOLD):
    b.poly([(cx, cy - r), (cx + r, cy), (cx, cy + r), (cx - r, cy)], fill=fill)


def item_glyph(b, kind, cx, cy, s=1.0):
    if kind == "ARMOR":
        b.poly([(cx - 22 * s, cy - 18 * s), (cx, cy - 30 * s), (cx + 22 * s, cy - 18 * s),
                (cx + 16 * s, cy + 22 * s), (cx - 16 * s, cy + 22 * s)], fill="#6d6a63", outline=GOLD)
        b.poly([(cx, cy - 20 * s), (cx, cy + 14 * s)], fill=GOLD)
    elif kind == "BELT":
        b.rect(cx - 26 * s, cy - 10 * s, cx + 26 * s, cy + 10 * s, fill="#5a4a32", outline=GOLD)
        b.rect(cx - 8 * s, cy - 12 * s, cx + 8 * s, cy + 12 * s, fill=GOLD, outline=GOLD_DEEP)
    elif kind == "BOOTS":
        for side in (-1, 1):
            b.rect(cx + side * 8 * s - 7 * s, cy - 24 * s, cx + side * 8 * s + 7 * s, cy + 8 * s,
                   fill="#4a4740", outline=GOLD)
            b.rect(cx + side * 8 * s - 11 * s, cy + 8 * s, cx + side * 8 * s + 11 * s, cy + 16 * s,
                   fill="#6b5636", outline=GOLD)
    elif kind == "GLOVES":
        b.rect(cx - 18 * s, cy - 8 * s, cx + 18 * s, cy + 18 * s, fill="#4a4740", outline=GOLD)
        for i in range(4):
            b.rect(cx - 16 * s + i * 9 * s, cy - 26 * s, cx - 10 * s + i * 9 * s, cy - 8 * s,
                   fill="#5a564c", outline=GOLD_DEEP)


# ------------------------------------------------------------------ HUD
def draw_hud(b, st, dlg=None, zone_name="HUB", runner=None):
    W, H = 1280, 720

    # 1. player status, top-left, 330x130
    frame(b, 14, 12, 330, 130)
    b.text(28, 20, "PLAYER STATUS", fill=GOLD_DEEP, size=8, bold=True)
    b.text(28, 34, "HP %d/%d" % (st.hp, st.hp_max), fill=PALE, size=12, bold=True)
    bar(b, 28, 52, 240, 9, st.hp / st.hp_max, RED)
    b.text(28, 66, "SHIELD %d/%d" % (st.shield, st.shield_max), fill=PALE, size=12, bold=True)
    bar(b, 28, 84, 240, 9, st.shield / st.shield_max, BLUE, back="#16202e")
    b.text(28, 98, "LEVEL %d  -  %s" % (st.level, st.cls), fill=GOLD, size=11, bold=True)
    b.text(28, 114, "TACTICAL %s   CREDITS %s" % (st.cls, format(st.credits, ",")),
           fill=GOLD_DEEP, size=8)

    # 2. title ribbon, top-centre
    ribbon(b, W / 2 - 160, 10, 320, 46, "VEIL OMNI", 19)

    # 3. compass minimap, top-right, 168 across
    cx, cy, r = W - 96, 100, 84
    b.oval(cx - r, cy - r, cx + r, cy + r, fill="#0a1424", outline=GOLD, width=3)
    b.oval(cx - r + 12, cy - r + 12, cx + r - 12, cy + r - 12, outline=GOLD_DEEP)
    for i in range(24):
        a = math.radians(i * 15)
        b.line([(cx + math.cos(a) * (r - 8), cy + math.sin(a) * (r - 8)),
                (cx + math.cos(a) * (r - 16), cy + math.sin(a) * (r - 16))],
               fill=GOLD_DEEP, width=1)
    for lab, a in (("N", -90), ("E", 0), ("S", 90), ("W", 180)):
        b.text(cx + math.cos(math.radians(a)) * (r - 30), cy + math.sin(math.radians(a)) * (r - 30) - 4,
               lab, fill=GOLD, size=9, anchor="center", bold=True)
    b.poly([(cx, cy - 34), (cx + 8, cy + 10), (cx, cy + 4), (cx - 8, cy + 10)], fill="#d8c07a")
    b.oval(cx - 4, cy - 4, cx + 4, cy + 4, fill=GOLD)
    b.text(cx, cy + r + 6, "MINIMAP - %s" % zone_name, fill=GOLD_DEEP, size=8, anchor="center")

    # 4. quest panel, right, under the minimap
    qx, qy, qw, qh = W - 344, 196, 328, 92
    frame(b, qx, qy, qw, qh)
    b.text(qx + 14, qy + 8, "QUEST: %s" % st.quest, fill=GOLD, size=10, bold=True)
    b.text(qx + 14, qy + 28, st.objective, fill=PALE, size=8)
    done, total = st.progress
    b.text(qx + 14, qy + 50, "PROGRESS %d/%d" % (done, total), fill=GOLD_DEEP, size=8, bold=True)
    bar(b, qx + 14, qy + 64, 220, 7, done / max(1, total), GOLD)
    b.text(qx + qw - 14, qy + 60, "%d%%" % int(100 * done / max(1, total)),
           fill=GOLD, size=10, anchor="e")

    # 5. armour loadout, left, 246x236
    ax, ay = 14, 154
    frame(b, ax, ay, 246, 236)
    b.text(ax + 12, ay + 8, "ARMOR LOADOUT", fill=GOLD, size=10, bold=True)
    for i, (slot, name, stat, state) in enumerate(st.loadout):
        col, row = i % 2, i // 2
        sx = ax + 12 + col * 114
        sy = ay + 26 + row * 104
        b.rect(sx, sy, sx + 104, sy + 66, fill="#0a1424", outline=GOLD_DEEP)
        item_glyph(b, slot, sx + 52, sy + 26, 0.72)
        b.text(sx + 52, sy + 64, slot, fill=GOLD, size=8, anchor="center", bold=True)
        b.text(sx + 52, sy + 76, name, fill=PALE, size=7, anchor="center")
        b.text(sx + 52, sy + 88, stat, fill=GOLD_DEEP, size=7, anchor="center")

    # 6. item wheel, bottom-centre, clear of the ONE bar
    wx, wy, wr = W / 2, 508, 96
    b.oval(wx - wr, wy - wr, wx + wr, wy + wr, fill="#0a1424", outline=GOLD, width=2)
    b.oval(wx - wr + 10, wy - wr + 10, wx + wr - 10, wy + wr - 10, outline=GOLD_DEEP)
    for i, name in enumerate(st.wheel):
        a = math.radians(-90 + i * 45 + 22.5)
        lx = wx + math.cos(a) * (wr - 30)
        ly = wy + math.sin(a) * (wr - 30)
        b.text(lx, ly - 3, name, fill=PALE, size=6, anchor="center")
        b.line([(wx + math.cos(a - math.radians(22.5)) * 30, wy + math.sin(a - math.radians(22.5)) * 30),
                (wx + math.cos(a - math.radians(22.5)) * (wr - 4), wy + math.sin(a - math.radians(22.5)) * (wr - 4))],
               fill=GOLD_DEEP, width=1)
    b.oval(wx - 26, wy - 26, wx + 26, wy + 26, fill="#14213a", outline=GOLD)
    b.rect(wx - 6, wy - 8, wx + 6, wy + 4, fill=GOLD)
    b.oval(wx - 4, wy - 13, wx + 4, wy - 4, outline=GOLD, width=1)
    b.text(wx, wy + 10, "[R]", fill=GOLD_DEEP, size=7, anchor="center")

    # 7. combat panel, right, above the ONE bar
    kx, ky, kw, kh = W - 344, 430, 328, 108
    frame(b, kx, ky, kw, kh)
    b.text(kx + 12, ky + 6, "COMBAT MODE", fill=GOLD, size=10, bold=True)
    b.text(kx + kw - 12, ky + 6, "ACTIVE", fill=TEAL, size=8, anchor="e")
    b.text(kx + 12, ky + 24, "THREAT: %s" % st.threat, fill="#e0a03a", size=8)
    b.text(kx + 12, ky + 40, "PULSE RIFLE", fill=PALE, size=9)
    b.text(kx + kw - 12, ky + 40, "%d/%d" % st.ammo, fill=GOLD, size=9, anchor="e")
    b.text(kx + 12, ky + 58, "ABILITY Q: %s" % st.ability, fill=PALE, size=8)
    b.text(kx + 12, ky + 74, "READY" if st.ability_ready else "CHARGING", fill=TEAL, size=8, bold=True)

    # 8. ONE prompt: one bar, one mic, no duplicate
    ow, oh = 460, 54
    ox, oy = (W - ow) / 2, H - 68
    frame(b, ox, oy, ow, oh)
    mic(b, ox + 28, oy + 24, 0.8)
    b.text(ox + 52, oy + 8, "ONE - SPEAK COMMAND", fill=GOLD, size=10, bold=True)
    waveform(b, ox + 52, oy + 26, 168, 14, seed=1.7)
    b.text(ox + ow - 12, oy + 34, "Hold [V] - Listening...", fill=GOLD_DEEP, size=7, anchor="e")

    # 9. dialogue, centred and above the wheel when a line is open
    if dlg is not None and dlg.active():
        pw, ph = 600, 132
        px, py = (W - pw) / 2, 236
        frame(b, px, py, pw, ph)
        b.text(px + 18, py + 12, dlg.npc.name, fill=GOLD, size=12, bold=True)
        b.text(px + 18, py + 30, dlg.npc.title, fill=GOLD_DEEP, size=8)
        b.text(px + 18, py + 52, dlg.npc.lines[dlg.index], fill=PALE, size=11)
        b.text(px + pw - 18, py + ph - 20, "[E] continue", fill=GOLD, size=9, anchor="e")


# ------------------------------------------------------------------ menu
def draw_menu(b, st, zone_name="HUB"):
    W, H = 1280, 720
    b.rect(0, 0, W, H, fill="#050b16")
    for i in range(8):
        t = i / 8.0
        b.rect(0, i * H / 8.0, W, (i + 1) * H / 8.0 + 1,
               fill="#%02x%02x%02x" % (int(6 + t * 10), int(13 + t * 18), int(26 + t * 30)))

    ribbon(b, W / 2 - 250, 12, 500, 58, "VEIL OMNI", 24)

    # tabs
    for i, tab in enumerate(st.tabs):
        tw = 150
        tx = W / 2 - 2 * tw + i * tw
        active = (i == st.tab)
        b.rect(tx + 4, 82, tx + tw - 4, 114, fill=PANEL_2 if active else PANEL,
               outline=GOLD if active else GOLD_DEEP, width=2 if active else 1)
        b.text(tx + tw / 2, 90, tab, fill=GOLD if active else GOLD_DEEP, size=11,
               anchor="center", bold=active)

    # left: settings with sliders
    frame(b, 28, 132, 380, 306)
    b.text(48, 142, "SETTINGS", fill=GOLD, size=15, anchor="center", bold=True)
    for i, (label, val) in enumerate(st.sliders):
        y = 172 + i * 44
        b.text(48, y, label, fill=PALE, size=10)
        b.text(356, y, "%d%%" % int(val * 100), fill=GOLD, size=10, anchor="e")
        b.rect(48, y + 18, 340, y + 26, fill="#16202e", outline=GOLD_DEEP)
        b.rect(48, y + 18, 48 + 292 * val, y + 26, fill=GOLD)
        diamond(b, 48 + 292 * val, y + 22, 6, PALE)
    b.rect(48, 386, 140, 418, fill=PANEL_2, outline=GOLD)
    b.text(118, 392, "APPLY", fill=GOLD, size=10, anchor="center", bold=True)
    b.rect(200, 386, 360, 418, fill=PANEL_2, outline=GOLD_DEEP)
    b.text(280, 392, "RESET TO DEFAULTS", fill=GOLD_DEEP, size=9, anchor="center")

    # middle: quests
    frame(b, 428, 132, 380, 306)
    b.text(448, 142, "QUESTS", fill=GOLD, size=15, anchor="center", bold=True)
    for i, (name, desc, done, total) in enumerate(st.quests):
        y = 176 + i * 86
        b.rect(448, y, 788, y + 74, fill="#0a1424", outline=GOLD_DEEP)
        diamond(b, 462, y + 18, 7, GOLD)
        b.text(476, y + 8, name, fill=PALE, size=11, bold=True)
        b.text(476, y + 26, desc, fill=GOLD_DEEP, size=8)
        b.text(476, y + 44, "%d/%d" % (done, total), fill=GOLD, size=9)
        bar(b, 520, y + 46, 240, 8, done / max(1, total), GOLD)
    b.text(618, 412, "Active Quests: 3/6", fill=GOLD_DEEP, size=9, anchor="center")

    # right: map
    frame(b, 828, 132, 424, 306)
    b.text(848, 142, "MAP", fill=GOLD, size=15, anchor="center", bold=True)
    b.rect(848, 172, 1232, 386, fill="#0f2a24", outline=GOLD_DEEP)
    for i in range(6):
        b.poly([(860 + i * 62, 380), (900 + i * 62, 250 + (i % 3) * 24), (940 + i * 62, 380)],
               fill="#1d3f33", outline="#2f5a44")
    b.line([(880, 350), (980, 320), (1080, 340), (1180, 300)], fill=GOLD_DEEP, width=2)
    b.poly([(1040, 240), (1052, 262), (1028, 262)], fill=RED, outline=GOLD)
    b.text(1040, 266, "YOU", fill=PALE, size=8, anchor="center")
    for lab, lx, ly in (("Sunken Ruins", 900, 200), ("Whispering Woods", 1030, 190),
                        ("Dawnreach Port", 1010, 360)):
        b.text(lx, ly, lab, fill=PALE, size=8, anchor="center")
    b.text(1030, 396, "REGION: VALE OF DAWN - LEVEL 30-40 AREA", fill=GOLD, size=8, anchor="center")

    # inventory strip
    frame(b, 28, 452, 1224, 132)
    b.text(48, 460, "INVENTORY - GOLD VAULT", fill=GOLD, size=12, bold=True)
    for i, (name, stat) in enumerate(st.inventory):
        sx = 44 + i * 132
        b.rect(sx, 486, sx + 116, 566, fill="#0a1424", outline=GOLD_DEEP)
        diamond(b, sx + 58, 508, 12, GOLD_DEEP)
        b.text(sx + 58, 532, name, fill=PALE, size=8, anchor="center", bold=True)
        b.text(sx + 58, 548, stat, fill=GOLD_DEEP, size=7, anchor="center")
    sx = 44 + 7 * 132
    b.rect(sx, 486, sx + 116, 566, fill="#0a1424", outline=GOLD_DEEP)
    b.text(sx + 58, 520, "+", fill=GOLD_DEEP, size=22, anchor="center")

    # bottom bar
    b.rect(0, H - 84, W, H, fill="#070d18", outline=GOLD, width=2)
    diamond(b, 40, H - 42, 12, GOLD)
    b.text(60, H - 58, "LVL %d" % st.level, fill=GOLD, size=10, bold=True)
    b.text(60, H - 42, "PLAYER: %s" % st.player, fill=PALE, size=10)
    b.text(300, H - 50, "GOLD: %s" % format(st.gold, ","), fill=GOLD, size=12, bold=True)
    b.text(520, H - 50, "LOAD: %d/%d" % st.load, fill=PALE, size=11)
    b.text(660, H - 50, "POWER: %s" % format(st.power, ","), fill=PALE, size=11)
    for i, lab in enumerate(("EQUIP", "SORT", "CLOSE")):
        bx = 860 + i * 132
        b.rect(bx, H - 66, bx + 116, H - 22, fill=PANEL_2, outline=GOLD)
        b.text(bx + 58, H - 54, lab, fill=GOLD, size=10, anchor="center", bold=True)
