"""Compact HUD: six panels anchored to the edges, centre kept clear, dialogue centred."""
import io
import sys

P = "D:/VEIL-OMNI/veil_ui.py"
s = io.open(P, "r", encoding="utf-8", newline="").read()

start = s.index("def draw_hud(")
end = s.index("# ------------------------------------------------------------------ menu")

NEW = '''def draw_hud(b, st, dlg=None, zone_name="HUB", runner=None):
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


'''

s = s[:start] + NEW + s[end:]
io.open(P, "w", encoding="utf-8", newline="").write(s)
print("compact HUD written")
