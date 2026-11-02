"""Reverse-engineer a game's architecture through phi.

Nothing here is typed. Every number is measured from the project or computed
from phi, then converted to a rung and round-tripped back:

    rung(x) = ln(x) / ln(phi)
    x'      = phi ^ rung(x)          # must return x within float error

Leverage of a system = the number of source files that name it. That is what
"reverse engineering the game" means here: not an opinion about importance, but
a count of how much of the game stands on it. High rung = build first.
"""

import math
import os
import re
import sys

PHI = (1 + 5 ** 0.5) / 2
LN_PHI = math.log(PHI)

def rung(x):
    if x <= 0:
        return 0.0
    return math.log(x) / LN_PHI

def phi_pow(r):
    return PHI ** r

def rt(x):
    """Round-trip a quantity through rung space. Returns (rung4, back, error)."""
    r = rung(x)
    r4 = round(r, 4)
    back = phi_pow(r4)
    return r4, back, abs(back - x)

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Source", "VEIL")
ROOT = os.path.normpath(ROOT)

def read(path):
    with open(path, "r", encoding="utf-8", errors="ignore") as f:
        return f.read()

def collect():
    headers, sources = [], []
    for base, _dirs, files in os.walk(ROOT):
        for name in files:
            p = os.path.join(base, name)
            if name.endswith(".h"):
                headers.append(p)
            elif name.endswith(".cpp"):
                sources.append(p)
    return headers, sources

def systems(headers):
    """Every class this project declares."""
    found = {}
    pat = re.compile(r"class\s+VEIL_API\s+(\w+)")
    for h in headers:
        for name in pat.findall(read(h)):
            found[name] = h
    return found

def dependents(name, files, owner):
    """Files that name this system, excluding its own declaration/definition."""
    word = re.compile(r"\b" + re.escape(name.lstrip("AUF")) + r"\b")
    hits = 0
    for f in files:
        if os.path.normpath(f) == os.path.normpath(owner):
            continue
        if os.path.basename(f).startswith(name.lstrip("AUF") + "."):
            continue
        if name in read(f) or word.search(read(f)):
            hits += 1
    return hits

def main():
    headers, sources = collect()
    table = []
    for name, owner in systems(headers).items():
        deps = dependents(name, headers + sources, owner)
        if deps <= 0:
            continue
        r4, back, err = rt(deps)
        table.append((r4, name, deps, back, err))
    table.sort(reverse=True)

    print("SYSTEMS MEASURED: %d of %d classes carry at least one dependent"
          % (len(table), len(systems(headers))))
    print()
    print("| rung | system | dependents | phi^rung | round-trip error |")
    print("|---:|---|---:|---:|---:|")
    for r4, name, deps, back, err in table[:24]:
        print("| %.4f | %s | %d | %.4f | %.2e |" % (r4, name, deps, back, err))
    print()

    # The law itself, and the constants the game actually runs on.
    print("LAW: phi^2 = %.6f, phi + 1 = %.6f, diff = %.2e"
          % (PHI * PHI, PHI + 1, abs(PHI * PHI - (PHI + 1))))
    print()
    print("| quantity | value | rung | phi^rung | error |")
    print("|---|---:|---:|---:|---:|")
    for label, value in [
        ("golden angle 360/phi^2", 360.0 / (PHI * PHI)),
        ("golden pitch (sealed)", 17.0324),
        ("respawn dead time (phi)", PHI),
        ("grace window (phi^3)", PHI ** 3),
        ("stand-down (phi^4)", PHI ** 4),
        ("max blow phi^-2 of max health", PHI ** -2),
        ("min blow phi^-5", PHI ** -5),
        ("nDNA base pairs", 3.2e9),
        ("mtDNA base pairs", 16569),
        ("mito genes", 37),
        ("protein-coding genes", 13),
        ("camp inner ring", 950.0),
        ("camp ring phi^1", 950.0 * PHI),
        ("camp ring phi^2", 950.0 * PHI * PHI),
        ("tetra seed 20cm", 20.0),
        ("tetra step 6 (20*phi^5)", 20.0 * PHI ** 5),
        ("sanctuary", 950.0 * PHI ** 0),
    ]:
        r4, back, err = rt(value)
        print("| %s | %.4f | %.4f | %.4f | %.2e |" % (label, value, r4, back, err))
    print()

    # Progression the game uses: XP = round(20 * phi^(level-1)).
    print("| level | XP = round(20 * phi^(level-1)) | rung of that XP |")
    print("|---:|---:|---:|")
    for level in range(1, 8):
        xp = round(20 * PHI ** (level - 1))
        print("| %d | %d | %.4f |" % (level, xp, rung(xp)))

if __name__ == "__main__":
    sys.exit(main())
