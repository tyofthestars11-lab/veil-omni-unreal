"""Intersect the plaza column, not a distance from each bounds origin.

Actor_1 covers the whole district: its bounds ORIGIN is the map centre, kilometres from
the spawn, so every distance test passed it by while its top face at 500 cm kept standing
the runner up at 598.15. The test is now a box against a box - the spawn column - so
anything whose bounds actually span the plaza is found, however large it is.
"""

import io

P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = """						if (FVector::Dist2D(B.Origin, Plaza) > 900.f) { continue; }
						// ANY HEIGHT, INSIDE 900 cm. The last pass kept a roof whose bounds
						// origin sat at z=500, and physics stood him on it - 598 cm of
						// "floating" that no settle could win against, because the roof was
						// under him. Below 20 m inside the spawn circle, everything goes
						// except the tagged floor tiles.
						if (B.Origin.Z + B.BoxExtent.Z < -300.f) { continue; }
						if (B.Origin.Z - B.BoxExtent.Z > 2000.f) { continue; }
						bOverPlaza = true;"""
NEW = """						// THE COLUMN, NOT THE ORIGIN. Actor_1 spans the whole district: its
						// bounds origin is the map centre, kilometres away, so distance
						// tests skipped it while its top face at 500 cm stood the runner up
						// at 598.15 every single run. Box against box finds it however big
						// it is. The tagged floor tiles stay.
						const FBox Column(Plaza - FVector(320.f, 320.f, 320.f),
							Plaza + FVector(320.f, 320.f, 1900.f));
						if (!B.GetBox().Intersect(Column)) { continue; }
						bOverPlaza = true;"""
o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - column anchor count=%d" % src.count(o))
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - plaza clear intersects the spawn column" % P)
