"""Quantum verify: prove the game against the law, from artefacts only.

It trusts nothing I wrote in prose. It recomputes phi from scratch, re-reads the
captured runs, compares two runs for drift, and inspects the frames on disk.
Usage: python Scripts/quantum_verify.py <log_a> <log_b> <shots_dir> <game_dir>
"""
import sys, os, re, glob, hashlib

PHI = 1.618033988749895
SPACING = 100.0 * PHI
def Reach(x): return SPACING * (PHI ** x)

LAW = {
    "reach enemy": Reach(1.283),
    "reach sanctuary": Reach(2.0),
    "enemy return s": PHI ** 5.0,
    "npc wound": 439.0 / PHI,
    "player wound": 439.0 / (PHI ** 3),
    "strike cost": 89.0 / (PHI ** 2),
    "fov": 180.0 / (PHI + 1.0),
}

log_a, log_b, shots, game = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
rows = []
def check(name, ok, detail=""):
    rows.append((bool(ok), name, detail))

def load(p):
    with open(p, "r", encoding="utf-8", errors="replace") as f:
        return f.read()

A = load(log_a)
B = load(log_b)

# 1. the law itself must be self-consistent before it can judge anything
check("law: spacing = 100*phi", abs(SPACING - 161.80339887498948) < 1e-9, "%.6f" % SPACING)
check("law: enemy reach = 300.0", abs(LAW["reach enemy"] - 300.0) < 0.05, "%.3f" % LAW["reach enemy"])
check("law: sanctuary = 423.6", abs(LAW["reach sanctuary"] - 423.606865) < 0.01, "%.3f" % LAW["reach sanctuary"])
check("law: fov = 61.803", abs(LAW["fov"] - 61.80339887498948) < 0.01, "%.3f" % LAW["fov"])

# 2. every gate the game claims, present in BOTH runs
GATES = [
    ("boot", r"VEIL PHI BOOT: ALL PASS"),
    ("whole", r"VEIL PHI WHOLE: 7 PASS, 0 FAIL"),
    ("hub counts", r"VEIL HUB COUNTS: PASS"),
    ("sites", r"VEIL WORLD SITES: 50/50"),
    ("life", r"VEIL WORLD LIFE: 66 bodies"),
    ("seated", r"VEIL WORLD SEATED: .*missing=0"),
    ("runner body", r"VEIL RUNNER: skinned body"),
    ("people dressed", r"VEIL NPC SKIN: cloth applied"),
    ("ai strike", r"VEIL AI PROOF: .*VIT 439 -> 335"),
    ("ai return", r"VEIL AI PROOF: hunter returned"),
    ("gate enforced", r"VEIL PHI GATE: ENFORCED AND PASSED"),
]
for label, pat in GATES:
    ina, inb = re.search(pat, A) is not None, re.search(pat, B) is not None
    check("gate %s" % label, ina and inb, "windowed=%s headless=%s" % (ina, inb))

# 3. no chaos of the mechanical kind
for label, text in (("windowed", A), ("headless", B)):
    fatal = len(re.findall(r"Fatal error|EXCEPTION_ACCESS_VIOLATION", text))
    missing = len(re.findall(r"MISSING MESH", text))
    check("%s: 0 fatals" % label, fatal == 0, "%d found" % fatal)
    check("%s: 0 missing meshes" % label, missing == 0, "%d found" % missing)

# 4. combat numbers must equal the law, not near it
struck = set(re.findall(r"struck for ([0-9.]+)", A + "\n" + B))
allowed = [LAW["npc wound"], LAW["player wound"]]
bad = []
for s in struck:
    if not any(abs(float(s) - a) < 0.2 for a in allowed):
        bad.append(s)
check("combat damage obeys phi", not bad, "values=%s offenders=%s" % (sorted(struck), bad))

# 5. determinism: the world must rebuild identically twice
key = re.compile(r"(VEIL WORLD SEATED: .*|VEIL HUB COUNTS: .*|VEIL WORLD SITES: .*|VEIL AI PROOF: .*VIT.*)")
sa, sb = key.findall(A), key.findall(B)
check("deterministic rebuild", sa == sb and len(sa) > 0, "%d/%d lines identical" % (len(sa), len(sb)))

# 6. the binary on disk must be newer than the newest source
dll = os.path.join(game, "Binaries", "Win64", "UnrealEditor-VEILOMNI.dll")
srcs = glob.glob(os.path.join(game, "Source", "VEILOMNI", "*.cpp")) + \
       glob.glob(os.path.join(game, "Source", "VEILOMNI", "*.h"))
if os.path.exists(dll) and srcs:
    newest = max(os.path.getmtime(s) for s in srcs)
    check("binary newer than source", os.path.getmtime(dll) >= newest,
          "dll %.0f KB" % (os.path.getsize(dll) / 1024.0))
else:
    check("binary newer than source", False, "missing dll or sources")

# 7. real content and real plugins, not stubs
uassets = glob.glob(os.path.join(game, "Content", "VEIL", "**", "*.uasset"), recursive=True)
check("real assets imported", len(uassets) >= 25, "%d uassets under Content/VEIL" % len(uassets))
proj = load(os.path.join(game, "VEILOMNI.uproject"))
for plug in ("Water", "Niagara"):
    check("plugin %s" % plug, plug in proj, "declared in uproject")

# 8. no cook or package was performed, as instructed
cooked = glob.glob(os.path.join(game, "Saved", "Cooked", "*")) + glob.glob(os.path.join(game, "Saved", "StagedBuilds", "*"))
check("not cooked or packaged", not cooked, "%d artefacts" % len(cooked))

# 9. save slot survived boot
check("save slot kept", "boot keeps the existing slot" in A or "boot keeps the existing slot" in B,
      "respawn anchored, not zeroed")

# 10. the frames must be real images, not blank screens
try:
    from PIL import Image
    import statistics
    files = sorted(glob.glob(os.path.join(shots, "view_*.png")))
    check("frames captured", len(files) >= 4, "%d frames" % len(files))
    sigs = set()
    for f in files:
        im = Image.open(f).convert("L")
        px = list(im.getdata())
        sd = statistics.pstdev(px)
        sigs.add(hashlib.md5(open(f, "rb").read()).hexdigest()[:8])
        check("frame %s not blank" % os.path.basename(f), sd > 12.0,
              "mean=%.1f sd=%.1f" % (sum(px) / len(px), sd))
    check("frames are distinct", len(sigs) == len(files), "%d unique" % len(sigs))
except ImportError:
    files = sorted(glob.glob(os.path.join(shots, "view_*.png")))
    sizes = set(os.path.getsize(f) for f in files)
    check("frames captured (no PIL)", len(files) >= 4 and len(sizes) == len(files),
          "%d frames, %d distinct sizes" % (len(files), len(sizes)))

print("QUANTUM VERIFY - recomputed law")
for k in ("reach enemy", "reach sanctuary", "npc wound", "player wound", "enemy return s", "strike cost", "fov"):
    print("  %-18s %10.4f" % (k, LAW[k]))
print()
fails = [r for r in rows if not r[0]]
for ok, name, detail in rows:
    print("  [%s] %-30s %s" % ("PASS" if ok else "FAIL", name, detail))
print()
print("QUANTUM VERIFY: %d checks, %d passed, %d failed" % (len(rows), len(rows) - len(fails), len(fails)))
sys.exit(1 if fails else 0)
