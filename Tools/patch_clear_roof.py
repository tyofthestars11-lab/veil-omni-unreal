"""Clear the roof too, not just the street.

The hold fires every second because something still lifts him to z=598, which is a
surface 500 cm over the tiles: the market's own roof, standing inside 900 cm of the plaza.
The clear now takes every untagged static mesh within 900 cm of the spawn column at any
height below 20 m, so the tiles at z=0 are the only floor in the spawn area and there is
nothing left to stand on above them.
"""

import io

P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = """						if (FVector::Dist2D(B.Origin, Plaza) > 700.f) { continue; }
						// CONTAINMENT, NOT ORIGIN. A tall wall has its bounds origin at
						// z=500, so an origin test let it stand exactly where he stands -
						// and physics then shoved the capsule up onto it: that is the
						// z=598 frame the settle could never fix. The box must span the
						// plaza column to be cleared.
						if (B.Origin.Z + B.BoxExtent.Z < -100.f) { continue; }
						if (FMath::Abs(B.Origin.Z) > 400.f + B.BoxExtent.Z) { continue; }
						bOverPlaza = true;"""
NEW = """						if (FVector::Dist2D(B.Origin, Plaza) > 900.f) { continue; }
						// ANY HEIGHT, INSIDE 900 cm. The last pass kept a roof whose bounds
						// origin sat at z=500, and physics stood him on it - 598 cm of
						// "floating" that no settle could win against, because the roof was
						// under him. Below 20 m inside the spawn circle, everything goes
						// except the tagged floor tiles.
						if (B.Origin.Z + B.BoxExtent.Z < -300.f) { continue; }
						if (B.Origin.Z - B.BoxExtent.Z > 2000.f) { continue; }
						bOverPlaza = true;"""
o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - clear filter count=%d" % src.count(o))
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - clear radius 900 cm, all heights below 20 m" % P)
