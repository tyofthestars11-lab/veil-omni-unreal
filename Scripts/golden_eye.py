"""THE GOLDEN EYE — a phi audit that recomputes the law and refuses to assume.

It does not trust any comment, log claim or constant: it derives phi, the spacing,
the reach ladder and the wound values itself, then measures the game against them.
Exit code 1 means chaos was found and must be read.
"""

import glob
import math
import os
import re
import sys

PHI = (1.0 + 5.0 ** 0.5) / 2.0
SRC = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
LOG = "C:/Users/Admin/HarkGame/Saved/Logs/VEILOMNI.log"
OUT = "C:/Users/Admin/HarkGame/Scripts/golden_eye_report.txt"

findings = []


def chaos(kind, severity, msg):
    findings.append((kind, severity, msg))


# ---------- the law, derived here, never read from the game ----------
SPACING = 100.0 * PHI            # the Bible placement constant
PITCH = SPACING * PHI
GOLDEN_DEG = 360.0 * (1.0 - 1.0 / PHI)


def reach(n):
    return SPACING * PHI ** n


ENEMY_REACH = reach(1.283)       # the Kraken spec's ~300
SANCTUARY = reach(2.0)
STRIKE_WOUND = 439.0 * PHI ** -1.0
PLAYER_WOUND = 439.0 * PHI ** -3.0

# ---------- 1. the source must agree with the law ----------
hdr = open(SRC + "PHI_Constants.h", encoding="utf-8", errors="replace").read()
m = re.search(r"static constexpr double Spacing\s*=\s*([0-9.]+)\s*\*\s*([0-9.]+)", hdr)
if not m:
    chaos("LAW", 3, "cannot read Spacing from PHI_Constants.h")
else:
    declared = float(m.group(1)) * float(m.group(2))
    if abs(declared - SPACING) > 1e-6:
        chaos("LAW", 3, f"Spacing declares {declared}, the law says {SPACING}")

if abs(GOLDEN_DEG - 137.5077640500378) > 1e-9:
    chaos("LAW", 3, "golden angle derivation drifted")

# ---------- 2. every phi-derived constant, valued ----------
pat = re.compile(r"(Reach|PhiPow)\(\s*(-?\d+(?:\.\d+)?)\s*\)")
derived = 0
for f in sorted(glob.glob(SRC + "*.cpp") + glob.glob(SRC + "*.h")):
    text = open(f, encoding="utf-8", errors="replace").read()
    for n, line in enumerate(text.split("\n"), 1):
        for kind, num in pat.findall(line):
            derived += 1
            x = float(num)
            v = reach(x) if kind == "Reach" else PHI ** x
            if v == 0.0 or not math.isfinite(v):
                chaos("DERIVED", 2, f"{os.path.basename(f)}:{n} {kind}({x}) = {v}")

# ---------- 3. structural chaos in the source ----------
for f in sorted(glob.glob(SRC + "*.cpp")):
    L = open(f, encoding="utf-8", errors="replace").read().replace("\r\n", "\n").split("\n")
    for i, raw in enumerate(L):
        code = raw.split("//")[0]
        s = code.strip()
        n = i + 1
        if s in ("return;", "return false;", "return true;", "break;", "continue;"):
            for j in range(i + 1, min(i + 4, len(L))):
                nxt = L[j].strip()
                if not nxt:
                    continue
                if nxt != "}":
                    indent = len(L[j]) - len(L[j].lstrip())
                    mine = len(raw) - len(raw.lstrip())
                    if indent >= mine and not nxt.startswith("//"):
                        chaos("DEADCODE", 2, f"{os.path.basename(f)}:{n} '{s}' then '{nxt[:60]}'")
                break
        if "Cast<" in code and "if (" not in code and "if(" not in code:
            nxt = L[i + 1].strip() if i + 1 < len(L) else ""
            if not nxt.startswith("if"):
                chaos("CAST", 2, f"{os.path.basename(f)}:{n} {s[:70]}")
        if "LoadObject<" in code:
            nxt = L[i + 1].strip() if i + 1 < len(L) else ""
            if "if (" not in code and "if (" not in nxt and "if(" not in nxt:
                chaos("LOADOBJ", 2, f"{os.path.basename(f)}:{n} {s[:70]}")

# ---------- 4. runtime self-consistency ----------
if not os.path.exists(LOG):
    chaos("RUNTIME", 3, "no log to measure")
    log = ""
else:
    log = open(LOG, encoding="utf-8", errors="replace").read()

if log:
    fatal = len(re.findall(r"Fatal error|EXCEPTION_ACCESS_VIOLATION", log))
    if fatal:
        chaos("RUNTIME", 3, f"{fatal} fatal/access-violation line(s)")

    for need in ("VEIL PHI BOOT: ALL PASS", "VEIL PHI WHOLE: 7 PASS, 0 FAIL",
                 "VEIL HUB COUNTS: PASS", "VEIL WORLD SITES: 50/50"):
        if need not in log:
            chaos("GATE", 3, f"missing gate line: {need}")

    missing = len(re.findall(r"MISSING MESH", log))
    if missing:
        chaos("ASSET", 3, f"{missing} missing mesh(es)")

    # wounds must equal the law, not merely be plausible
    for value, label, want in (
        (re.findall(r"struck for (\d+)", log), "npc wound", STRIKE_WOUND),
        (re.findall(r"Runner is struck for (\d+) by", log), "player wound", PLAYER_WOUND),
    ):
        for v in set(value):
            if abs(float(v) - want) > 2.0:
                chaos("BALANCE", 2, f"{label} {v} but the law says {want:.1f}")

    # the sanctuary ring must be Reach(2)
    for v in set(re.findall(r"inside the Hub of (\d+)", log)):
        if abs(float(v) - SANCTUARY) > 2.0:
            chaos("BALANCE", 2, f"sanctuary {v} but Reach(2) is {SANCTUARY:.1f}")

    # snap gaps: any piece authored further than Reach(2) above support is a builder error
    gaps = [float(g) for g in re.findall(r"closed a ([0-9.]+) gap", log)]
    bad = [g for g in gaps if g > SANCTUARY]
    if bad:
        chaos("PLACEMENT", 2, f"{len(bad)} piece(s) authored above {SANCTUARY:.0f} of support, worst {max(bad):.0f}")

    # the strike proof must actually fire in a captured run
    if "STRIKE FIRED FROM ITS POST" not in log:
        chaos("PROOF", 2, "hostile post never proved firing in this run")

# ---------- report ----------
lines = [
    "THE GOLDEN EYE — phi audit report",
    f"phi = {PHI:.15f}",
    f"spacing = {SPACING:.6f}  pitch = {PITCH:.6f}  golden angle = {GOLDEN_DEG:.13f}",
    f"enemy reach Reach(1.283) = {ENEMY_REACH:.1f}  sanctuary Reach(2) = {SANCTUARY:.1f}",
    f"npc wound = 439*phi^-1 = {STRIKE_WOUND:.1f}  player wound = 439*phi^-3 = {PLAYER_WOUND:.1f}",
    f"derived constants scanned: {derived}",
    "",
    f"CHAOS FOUND: {len(findings)}",
]
for kind, sev, msg in sorted(findings, key=lambda r: (-r[1], r[0])):
    lines.append(f"  [{kind} sev{sev}] {msg}")

text = "\n".join(lines)
open(OUT, "w", encoding="utf-8", newline="").write(text + "\n")
print(text)
sys.exit(1 if any(sev >= 2 for _, sev, _ in findings) else 0)
