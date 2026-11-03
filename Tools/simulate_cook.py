#!/usr/bin/env python3
"""Simulate a cook of VEILDistrict from the builder's own laws, without Unreal.

Rebuilds every pass numerically from the same constants the C++ uses, then asks the
questions a cook would: how many pieces, what bounding volume, what intersects what,
and what assets would be referenced. Writes Cook/VEIL-COOK.json + .png.
"""
import json, math, pathlib, sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

PHI = 1.618033988749895
GOLDEN = 137.5077640500378
ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "Cook"
OUT.mkdir(exist_ok=True)

pieces = []   # (group, x, y, z, sx, sy, sz)

def add(group, x, y, z, sx, sy, sz):
    pieces.append((group, x, y, z, sx, sy, sz))

# The terraces, defined before anything that stands on them.
DECKS = [(4200, 320, 1800), (7200, 820, 1800), (10400, 1900, 1800), (13800, 3800, 1800)]
def deck_at(r):
    for R, H, W in DECKS:
        if abs(r - R) <= W / 2:
            return H
    return 0.0

# --- forms pass: Spire, measured column 13 x 13 x 100 cm at scale 1 (from PHI SPIRE log)
SPX, SPY = 0.0, 3500.0
z, w, h = deck_at(math.hypot(SPX, SPY)) + 60.0, 1400.0, 500.0
for k in range(10):
    add("spire", SPX, SPY, z + h / 2, w, w, h)
    z += h; w /= PHI; h *= PHI
spire_top = z

# --- terraced ground: four decks, cliff faces, falls
for R, H, WIDTH in DECKS:
    seg = max(16, math.ceil(2 * math.pi * R / 1800))
    for i in range(seg):
        a = i * (2 * math.pi / seg)
        x, y = math.cos(a) * R, math.sin(a) * R
        add("deck", x, y, H, 2 * math.pi * R / seg * 1.06, WIDTH, 0.3)
        add("terrace_face", x, y, H / 2, 2 * math.pi * R / seg * 1.06, 700, H)
        if i % 6 == 0:
            add("terrace_fall", x, y, H - 240, 220, 40, 540)

# --- crystal range: 42 shards on the east flank
for k in range(42):
    a = math.radians(k * GOLDEN)
    RR = math.sqrt(k + 1) * 240
    H = 300 * PHI ** ((k % 6) + 1)
    W = 260 + RR * 0.08
    cx, cy = 6200 + math.cos(a) * RR, -1200 + math.sin(a) * RR
    add("crystal", cx, cy, deck_at(math.hypot(cx, cy)) + H / 2, W, W, H)

# --- terrace cliff wall at 3600 and its falls
for k in range(28):
    a = k * (2 * math.pi / 28)
    H = 700 * PHI ** (k % 3)
    add("cliff", math.cos(a) * 3600, math.sin(a) * 3600, H / 2, 1400, 600, H)
    if k % 3 == 0:
        add("cliff_fall", math.cos(a) * 3600, math.sin(a) * 3600, H + 300, 300, 40, 700)

# --- river Nest -> Hub -> Omni, water at Z=6
LEGS = [((-2000, 2000), (0, 0)), ((0, 0), (2800, 500))]
for A, B in LEGS:
    for i in range(12):
        t = (i + 0.5) / 12
        x, y = A[0] + (B[0] - A[0]) * t, A[1] + (B[1] - A[1]) * t
        L = math.dist(A, B) / 12 * 1.15
        add("river", x, y, 6, L, 400, 6)
    for t in (0.382, 0.618):
        x, y = A[0] + (B[0] - A[0]) * t, A[1] + (B[1] - A[1]) * t
        add("bridge", x, y, 430, 700, 50, 10)

# --- Megastructure Deep
for k in range(5):
    R = 1400 * (1 / PHI) ** k
    add("deep", 0, -5000, -250 * k, 2 * R, 2 * R, 30)

# --- four regions dressed
REGIONS = [(900, 0), (-450, 780), (600, -2100), (0, -5000)]
for cx, cy in REGIONS:
    for i in range(24):
        a = math.radians((i + 1) * GOLDEN)
        RR = math.sqrt(i + 1) * 260
        add("region_prop", cx + math.cos(a) * RR, cy + math.sin(a) * RR, 160, 160, 160, 200)

# --- runner wings
add("wing", 0, 0, 160, 90, 6, 160)

# ----------------------------------------------------------------------------------
def footprint_overlap(a, b):
    """True when two axis-aligned footprints overlap on the ground plane."""
    _, ax, ay, _, aw, ad, _ = a
    _, bx, by, _, bw, bd, _ = b
    return abs(ax - bx) < (aw + bw) / 2 and abs(ay - by) < (ad + bd) / 2

def radial_dist(a, b):
    return math.hypot(a[1] - b[1], a[2] - b[2])

# A structure inside another's footprint is only a fault when they share height.
def vertical_overlap(a, b):
    az, ah = a[3], a[6]
    bz, bh = b[3], b[6]
    # 1 cm of tolerance: a piece standing ON a deck shares its top sliver with the slab it
    # rests on. That is contact, not penetration, and counting it hid the real faults.
    a_lo, a_hi = az - ah / 2, az + ah / 2
    b_lo, b_hi = bz - bh / 2, bz + bh / 2
    return not (a_hi <= b_lo + 1.0 or b_hi <= a_lo + 1.0)

faults = []
by_group = {}
for p in pieces:
    by_group.setdefault(p[0], []).append(p)

key_pairs = [("spire", "deck"), ("spire", "crystal"), ("deep", "region_prop"),
             ("deep", "deck"), ("crystal", "deck"), ("wing", "deck")]
for ga, gb in key_pairs:
    n = 0
    for a in by_group.get(ga, []):
        for b in by_group.get(gb, []):
            if footprint_overlap(a, b) and vertical_overlap(a, b):
                n += 1
                if len(faults) < 12:
                    faults.append((ga, gb, round(a[1]), round(a[2]), round(a[3]), round(b[3])))
    print(f"COOK SIM: {ga} x {gb}: {n} intersecting pair(s)")

# Is the Spire standing on the ground or floating over it?
r_spire = math.hypot(SPX, SPY)
d_spire = deck_at(r_spire)
spire_base = min(p[3] - p[6] / 2 for p in by_group["spire"])
print(f"COOK SIM: spire base {spire_base:.0f} cm, deck at r={r_spire:.0f} is {d_spire:.0f} cm, top {spire_top:.0f} cm")

counts = {g: len(v) for g, v in by_group.items()}
total = sum(counts.values())
print(f"COOK SIM: {total} piece(s) across {len(counts)} group(s)")

# Asset references a cook would need to have mounted.
assets = [
    "/Game/Sicka_EnvPack2", "/Game/Polyart", "/Game/Iceland_Environment",
    "/Game/ParagonCountess", "/Game/ParagonMurdock", "/Game/MetaHumans",
    "/Game/PHI/Palette", "/Game/StarterContent", "/Game/Maps/VEILDistrict",
]
manifest = {
    "map": "VEILDistrict",
    "pieces": total,
    "groups": counts,
    "spire": {"x": SPX, "y": SPY, "base_cm": spire_base, "top_cm": spire_top, "deck_cm": d_spire},
    "faults": faults,
    "assets": assets,
}
(OUT / "VEIL-COOK.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")

# top view and elevation, so the cook can be looked at without the editor
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 8))
fig.patch.set_facecolor("#0b1e3a")
colours = {"spire": "#ffd700", "deck": "#6b5b45", "terrace_face": "#4a4038",
           "terrace_fall": "#4fc3f7", "crystal": "#00bfff", "cliff": "#5a5148",
           "cliff_fall": "#4fc3f7", "river": "#2196f3", "bridge": "#c19a6b",
           "deep": "#0a2a6b", "region_prop": "#8a5a2b", "wing": "#ffd700"}
for g, pts in by_group.items():
    ax1.scatter([p[1] for p in pts], [p[2] for p in pts], s=4, c=colours.get(g, "white"), label=g)
ax1.set_facecolor("#0b1e3a"); ax1.set_aspect("equal"); ax1.set_title("cook: plan", color="gold")
ax1.tick_params(colors="white"); ax1.legend(facecolor="#0b1e3a", labelcolor="white", fontsize=7)
for g, pts in by_group.items():
    ax2.scatter([math.hypot(p[1], p[2]) for p in pts], [p[3] for p in pts], s=4, c=colours.get(g, "white"))
ax2.set_facecolor("#0b1e3a"); ax2.set_title("cook: radius / height", color="gold")
ax2.tick_params(colors="white"); ax2.set_xlabel("radius cm", color="white"); ax2.set_ylabel("z cm", color="white")
plt.tight_layout()
plt.savefig(OUT / "VEIL-COOK.png", dpi=110, facecolor=fig.get_facecolor())

print(f"COOK SIM: wrote Cook/VEIL-COOK.json and Cook/VEIL-COOK.png")
sys.exit(0)
