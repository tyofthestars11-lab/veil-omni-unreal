"""Every Reach()/PhiPow() in the game, valued, so no constant can pass by assumption."""

import glob
import math
import re

PHI = 1.618033988749895
SPACING = 10.0
SRC = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def reach(x):
    return SPACING * PHI ** x


# ---- repair the collapsed enemy reach: 300 stands at rung 7 (290.3) ----
p = SRC + "VeilNpc.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")
if "Reach(1.283)" in s:
    s = s.replace("PHI_Constants::Reach(1.283)", "PHI_Constants::Reach(7.0)")
    open(p, "w", encoding="utf-8", newline="").write(s)
    print("repaired: enemy post reach", round(reach(1.283), 1), "->", round(reach(7.0), 1))

p = SRC + "VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")
if "Reach(1.8)" in s:
    s = s.replace("PHI_Constants::Reach(1.8)", "PHI_Constants::Reach(6.0)")
    open(p, "w", encoding="utf-8", newline="").write(s)
    print("repaired: ai proof posting", round(reach(1.8), 1), "->", round(reach(6.0), 1))

# ---- value every derived constant in the live tree ----
rows = []
pat = re.compile(r"(Reach|PhiPow)\(\s*(-?\d+(?:\.\d+)?)\s*\)")
for f in sorted(glob.glob(SRC + "*.cpp") + glob.glob(SRC + "*.h")):
    text = open(f, encoding="utf-8", errors="replace").read()
    for n, line in enumerate(text.split("\n"), 1):
        for kind, num in pat.findall(line):
            x = float(num)
            v = reach(x) if kind == "Reach" else PHI ** x
            rows.append((v, kind, x, f.split("\\")[-1].split("/")[-1], n))

print("\n=== derived constants that are effectively zero or enormous ===")
odd = [r for r in rows if r[0] < 1.0 or r[0] > 1.0e6]
for v, kind, x, f, n in sorted(odd):
    print(f"  {f}:{n}  {kind}({x}) = {v:.4g}")
if not odd:
    print("  none")

print(f"\n=== {len(rows)} derived constants total ===")
for v, kind, x, f, n in sorted(rows):
    print(f"  {v:12.4f}  {kind}({x})  {f}:{n}")
