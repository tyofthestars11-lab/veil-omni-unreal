"""One cyan marker survived the first pass - make it gold.

The brief: no blue blobs on the instrument. Every remaining FLinearColor(0.35, 0.95, 1.0)
in the HUD is a map marker, so all of them become the phi-shard gold the fragment marker
already uses.
"""

import io

P = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
OLD = "FLinearColor(0.35f, 0.95f, 1.f)"
NEW = "FLinearColor(1.f, 0.78f, 0.30f)"
n = src.count(OLD)
if n == 0:
    print("no cyan markers left - nothing to do")
    raise SystemExit(0)
io.open(P, "w", encoding="utf-8", newline="").write(src.replace(OLD, NEW))
print("WROTE %s - %d cyan marker(s) -> gold" % (P, n))
