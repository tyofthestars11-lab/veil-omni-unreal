# -*- coding: utf-8 -*-
"""Move the CauserName resolver to the head of TakeDamage: the token-hold log
uses it before the damage log did."""
import io
import sys

P = "D:/VEIL-OMNI/Source/VEIL/Private/Character/VEILCharacter.cpp"
with io.open(P, "r", encoding="utf-8", newline="") as f:
    raw = f.read()
crlf = "\r\n" in raw
t = raw.replace("\r\n", "\n")

START = "\t// WHO HIT YOU IS A PERSON."
END = "\t}();\n"
si = t.find(START)
ei = t.find(END, si)
fails = []
if si < 0 or ei < 0:
    fails.append("helper markers %d/%d" % (si, ei))
else:
    block = t[si:ei + len(END)]
    t = t[:si] + t[ei + len(END):]
    sig = "float AVEILCharacter::TakeDamage("
    fi = t.find(sig)
    if fi < 0:
        fails.append("TakeDamage signature missing")
    else:
        brace = t.find("{\n", fi)
        if brace < 0:
            fails.append("TakeDamage body missing")
        else:
            at = brace + 2
            t = t[:at] + block + t[at:]

if fails:
    print("FAILS:", fails)
    sys.exit(1)

with io.open(P, "w", encoding="utf-8", newline="") as f:
    f.write(t.replace("\n", "\r\n") if crlf else t)

# order check: helper must come before every use
lines = t.split("\n")
decl = next(i for i, s in enumerate(lines) if "const FString CauserName" in s)
uses = [i for i, s in enumerate(lines) if "*CauserName" in s]
print("decl line =", decl + 1, "uses =", [u + 1 for u in uses])
print("ordered =", all(u > decl for u in uses))
print("PATCH: PASS")
