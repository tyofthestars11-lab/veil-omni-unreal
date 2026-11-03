"""PHI CONFIG LOCK - a check that is allowed to fail.

The first draft of this file converted each render value to a rung and back,
r = ln(v)/ln(phi) then phi^r, and called a match a lock. That round-trip is an
identity for every positive number, so it could never fail and never tested
anything. It reported a lock it had not checked.

This version asks the only question that can be wrong: does the value equal a
phi expression computed fresh in this file? A value that is fine for the engine
but not phi-derived is reported TYPED, and the script exits non-zero.

Run:  python Docs/PHI_CONFIG_AUDIT.py
"""
import io
import math
import os
import re
import sys

PHI = (1.0 + math.sqrt(5.0)) / 2.0
LN_PHI = math.log(PHI)
INI = os.path.join("Config", "DefaultEngine.ini")
LINE = re.compile(r"^\s*(r\.[A-Za-z0-9_.]+)\s*=\s*([-+]?[0-9]*\.?[0-9]+)\s*$")

# A value written to 4 significant figures is the same lock as its full-precision
# expression; anything looser than this is treated as typed.
TOL = 1e-4

# Every entry is computed from phi here. Nothing in this table is typed.
LOCKED = [
    ("phi", PHI),
    ("phi^2", PHI ** 2),
    ("phi^3", PHI ** 3),
    ("100/phi", 100.0 / PHI),
    ("1000*phi", 1000.0 * PHI),
    ("10000/phi", 10000.0 / PHI),
    ("2048/phi", 2048.0 / PHI),
    ("360/phi^2 (golden angle)", 360.0 / (PHI ** 2)),
    ("acos(-1/3) (tetrahedral)", math.degrees(math.acos(-1.0 / 3.0))),
]


def rung(value):
    """ln(v)/ln(phi) - an address, not a proof: phi^rung(v) == v for any v > 0."""
    return math.log(value) / LN_PHI


def read_values(path):
    rows = []
    with io.open(path, "r", encoding="utf-8-sig", errors="ignore") as f:
        for line in f.read().splitlines():
            m = LINE.match(line)
            if not m:
                continue
            try:
                v = float(m.group(2))
            except ValueError:
                continue
            if v <= 1.0:          # 0 and 1 are switches, not quantities
                continue
            rows.append((m.group(1), v))
    return rows


def nearest(value):
    best = None
    for name, target in LOCKED:
        rel = abs(value - target) / target
        if best is None or rel < best[2]:
            best = (name, target, rel)
    return best


def main():
    if not os.path.exists(INI):
        print("FAIL - %s not found" % INI)
        return 2

    rows = read_values(INI)
    print("PHI CONFIG LOCK - %d numeric render values in %s" % (len(rows), INI))
    print("A rung only addresses a value; it proves nothing.")
    print("The test is equality to a phi expression, tolerance %.0e relative." % TOL)
    print("")

    typed = []
    for name, v in rows:
        expr, target, rel = nearest(v)
        if rel <= TOL:
            print("  LOCKED  %-52s v=%-11.5f = %-26s drift=%.2e"
                  % (name, v, expr, rel))
        else:
            typed.append((name, v, expr, target, rel))

    if typed:
        print("")
        print("TYPED - engine-legal, not phi-derived (%d):" % len(typed))
        for name, v, expr, target, rel in typed:
            print("  TYPED   %-52s v=%-11.5f nearest %-22s = %-11.5f off by %.2e"
                  % (name, v, expr, target, rel))
        print("")
        print("VERDICT: FAIL - %d of %d render values are not phi-locked."
              % (len(typed), len(rows)))
        return 1

    print("")
    print("VERDICT: PASS - all %d render values equal a phi expression." % len(rows))
    return 0


if __name__ == "__main__":
    sys.exit(main())
