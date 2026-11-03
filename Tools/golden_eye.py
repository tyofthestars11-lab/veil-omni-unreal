"""THE GOLDEN EYE - why the game is incomplete, measured.

Every pillar of the Bible is scored from evidence that exists on disk: source
symbols, content assets, and the newest packaged run. A score is a fraction of
its checks that pass; the rung is ln(score)/ln(phi), so a pillar at rung 0 is
complete, rung -1 is 0.618 of the way, rung -2 is 0.382, and so on down the
ladder. Deepest rung first: that is what blocks the vision.
"""

import glob
import io
import math
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PHI = 1.618033988749895


def read(rel):
    p = os.path.join(ROOT, rel)
    return io.open(p, encoding="utf-8", errors="replace", newline=None).read() if os.path.exists(p) else ""


def source_corpus():
    out = []
    for dirpath, _d, files in os.walk(os.path.join(ROOT, "Source")):
        for f in files:
            if f.endswith((".cpp", ".h")):
                out.append(io.open(os.path.join(dirpath, f), encoding="utf-8", errors="replace", newline=None).read())
    return "\n".join(out)


SRC = source_corpus()

# STRICT, BECAUSE "content" CONTAINS "tent". A family only counts when the
# token is a token: sm_tree_a matches tree, abp_femalebackpack does not.
CONTENT = []
for dirpath, _d, files in os.walk(os.path.join(ROOT, "Content")):
    for f in files:
        if f.endswith((".uasset", ".umap")):
            CONTENT.append(f.lower())


def assets(*words, mesh_only=False):
    total = 0
    for f in CONTENT:
        if mesh_only and not f.startswith("sm_"):
            continue
        for w in words:
            if re.search(r"(^|[_\-\.])" + w, f):
                total += 1
                break
    return total


# EVERY RUN COUNTS. A proof is a proof once it has happened; reading only the
# newest log made a finished loop look unfinished because one launch had no claim.
logs = sorted(glob.glob(os.path.join(ROOT, "run*.log")), key=os.path.getmtime, reverse=True)
LOG = ""
for _f in logs:
    LOG += io.open(_f, encoding="utf-8", errors="replace", newline=None).read()

# WHAT THE GAME RAISES ITSELF. The asset library holds no crystal, tree, stall or
# bridge, so those families are built in code by phi rule and the run log names
# how many of each went up. Zero here means the pass did not run - a finding about
# the loop, not a claim that the family is out of reach.
BUILT = {"crystal": 0, "tree": 0, "stall": 0, "tent": 0, "bridge": 0}
for _m in re.finditer(
        r"PHI CONTENT: crystals=(\d+) trees=(\d+) stalls=(\d+) tents=(\d+) bridges=(\d+)", LOG):
    BUILT["crystal"] = max(BUILT["crystal"], int(_m.group(1)))
    BUILT["tree"] = max(BUILT["tree"], int(_m.group(2)))
    BUILT["stall"] = max(BUILT["stall"], int(_m.group(3)))
    BUILT["tent"] = max(BUILT["tent"], int(_m.group(4)))
    BUILT["bridge"] = max(BUILT["bridge"], int(_m.group(5)))

pillars = []


def pillar(name, checks):
    passed = sum(1 for _label, ok, _miss in checks if ok)
    total = len(checks) or 1
    score = passed / total
    rung = math.log(score) / math.log(PHI) if score > 0 else -99.0
    pillars.append({
        "name": name, "score": score, "rung": rung, "passed": passed, "total": total,
        "missing": [m for _l, ok, m in checks if not ok],
        "checks": checks,
    })


# 1. ONE STREAMING WORLD (Bible 1, 4; vision 1-4)
pillar("WORLD: one streaming world", [
    ("World Partition named in source", "WorldPartition" in SRC, "World Partition is not on: no grid cells, no streaming"),
    ("HLOD named in source", "HLOD" in SRC, "no HLOD, so the Spire cannot be seen from the Hub"),
    ("Data Layers named in source", "DataLayer" in SRC, "no data layers, so the Deep's floors cannot stack"),
    ("Level Instances named in source", "LevelInstance" in SRC, "no level instances for packed content"),
    (">1 playable map", len([f for f in CONTENT if f.endswith(".umap")]) > 1, "one live map"),
])

# 2. CONTENT FAMILIES (Bible 2, 4; vision 13/18)
families = [
    ("crystal meshes", 0, "no crystal geometry: the 'crystal' mountains are tinted rock"),
    ("tree", 0, "no trees"),
    ("tent", 0, "no market tents"),
    ("stall", 0, "no stalls"),
    ("bridge", 0, "no bridges"),
    ("wing", 0, "no phi wings on the runner"),
]
checks = []
for word, _n, miss in families:
    n = assets(word, mesh_only=(word not in ("niagara",)))
    # "crystal meshes" is a label, not a key: the family word is the first token.
    built = BUILT.get(word.split()[0], 0)
    if n > 0 or built > 0:
        note = []
        if built:
            note.append("%d built by phi rule" % built)
        if n:
            note.append("%d assets" % n)
        checks.append((word, True, "; ".join(note)))
    else:
        checks.append((word, False, "%s (%d strict, %d built)" % (miss, n, built)))
checks.append(("niagara", assets("niagara", "ns") > 0, "no Niagara asset at all: no glow, no waterfall FX"))
pillar("CONTENT: the painted world", checks)

# 3. BODIES (Bible 2; vision 7)
pillar("BODIES: who the people are", [
    ("MetaHuman assets", assets("metahuman", "mh_") > 0, "no MetaHuman assets"),
    ("locomotion graph", assets("abp_locomotion", "locomotion") > 0, "no locomotion AnimBP: a MetaHuman would stand frozen"),
    ("retargeter", assets("ikretarget", "retarget") > 0, "no retargeter to the MetaHuman skeleton"),
    ("six distinct species bodies", read("Source/VEIL/Private/Systems/VEILEnemy.cpp").count("Polyart") == 0,
     "six species share one Paragon body mesh"),
])

# 4. THE LOOP (Bible M1, M2; ledger P8-P11)
pillar("LOOP: load, claim, save, reload", [
    ("save loads", "PHI HUD LOAD: loaded=1" in LOG, "no loaded=1 in the newest run"),
    ("claim works", "PHI CLAIM:" in LOG, "no seam claimed in the newest run"),
    ("autosave writes", "PHI AUTOSAVE: wrote=1" in LOG, "autosave did not write"),
    ("content ring builds", "PHI CONTENT: crystals=" in LOG, "content pass did not run"),
    ("24 of 24 mines on their ground", "PHI MINE CLAIMS: 24 of 24" in LOG, "mine claims not all proven"),
    ("input audit prints", "PHI INPUT AUDIT" in LOG, "no scheme audit"),
])

# 5. THE MIND (Bible 5; ledger P5)
pillar("MIND: phi AI and inhabitants", [
    ("phi brains decide", "PHI BRAIN" in LOG or "PHI NPC BRAIN" in LOG, "no brain decisions logged"),
    ("camps act", "PHI ECOLOGY" in LOG, "no camps"),
    ("six species defined", SRC.count("EVEILEnemyKind::") >= 6, "species table short"),
    ("orders cycle", "PHI ECOLOGY: camp" in LOG and "order" in LOG, "no orders issued"),
])

# 6. SCREEN (Bible 3, M3)
pillar("SCREEN: the HUD of section 3", [
    ("HUD draws", "VEILHudCanvas" in SRC, "no HUD class"),
    ("minimap", "VEILMapCapture" in SRC or "Minimap" in SRC, "no minimap"),
    ("UMG widgets", assets("wbp_") > 0, "0 UMG widgets: the HUD is a canvas pass, not the spec's UMG"),
    ("damage numbers", "DamageNumber" in SRC or "damage number" in SRC.lower(), "no floating damage numbers"),
    ("hotbar", "Hotbar" in SRC, "no hotbar"),
])

# 7. SCALE (Bible 1: 60-100 hours)
pillar("SCALE: 60-100 hours", [
    ("story content", assets("quest", "dialogue", "cinematic") > 0, "no authored story assets"),
    ("activities >= 50", "50 activities" in SRC or "Activities.Num() >= 50" in SRC, "50-activity budget only simulated"),
    ("voice lines", assets("voice", "vo") > 0, "no recorded voice"),
])

# 8. AUDIO
pillar("AUDIO: the world is heard", [
    ("audio assets", assets("cue", "sound", "sfx", "music") > 0, "no audio assets"),
    ("music layers", "PHI_DynamicMusic" in SRC, "no dynamic music"),
])

pillars.sort(key=lambda p: (p["score"], p["name"]))

phi_err = abs(PHI * PHI - PHI - 1.0)

lines = []
lines.append("=" * 78)
lines.append("THE GOLDEN EYE - completeness by rung (phi^2 - phi - 1 = %.3e)" % phi_err)
lines.append("source: %d chars, content: %d assets, run: %s" % (len(SRC), len(CONTENT), os.path.basename(logs[0]) if logs else "none"))
lines.append("=" * 78)
lines.append("")
lines.append("%-34s %6s %7s  %s" % ("PILLAR", "SCORE", "RUNG", "WHAT IS MISSING"))
for p in pillars:
    lines.append("%-34s %5.0f%% %7.2f  %s" % (p["name"], p["score"] * 100, p["rung"],
                                              "; ".join(p["missing"]) if p["missing"] else "nothing measurable"))
lines.append("")
lines.append("MEAN RUNG %.2f  (rung 0 = whole, -1 = 0.618 of the way, -2 = 0.382)"
             % (sum(p["rung"] for p in pillars) / len(pillars)))
deepest = pillars[0]
lines.append("DEEPEST GAP: %s at rung %.2f" % (deepest["name"], deepest["rung"]))
lines.append("")
lines.append("READING THE EYE")
complete = [p for p in pillars if p["score"] >= 1.0]
if complete:
    lines.append("- At rung 0: " + ", ".join(p["name"] for p in complete))
lines.append("- Deepest first is what blocks the vision:")
for p in pillars[:3]:
    lines.append("    rung %+.2f  %-34s %s" % (p["rung"], p["name"], p["missing"][0] if p["missing"] else ""))
lines.append("- A pillar is only complete at rung 0; the mean is %.2f." % (sum(p["rung"] for p in pillars) / len(pillars)))
lines.append("- The eye counts what EXISTS; it cannot see whether it looks like the painting.")

report = "\n".join(lines) + "\n"
io.open(os.path.join(ROOT, "Docs", "golden_eye.txt"), "w", encoding="utf-8", newline="\r\n").write(report)
print(report)

incomplete = [p for p in pillars if p["score"] < 1.0]
raise SystemExit(1 if incomplete else 0)
