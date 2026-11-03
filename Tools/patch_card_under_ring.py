"""The field manual card sits under the ring, not inside it.

The card was anchored at y = 30 + 244, which was clear of the old 180 px minimap. The
minimap is now sized by the law (ClipY / phi = 445 px at 720p), so the card landed
inside the ring - visible in the frame as SPRINT printed over the map. It now starts one
phi step under the ring's lower edge.
"""

import io

P = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
OLD = "const float CY = 30.f + 244.f;"
NEW = ("const float CY = 30.f + Canvas->ClipY / UPHIMITO_PHI_SAFE + 18.f;"
       "   // under the ring")
if src.count(OLD) != 1:
    print("ABORTED - count=%d" % src.count(OLD))
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="").write(src.replace(OLD, NEW, 1))
print("WROTE %s - manual card under the phi ring" % P)
