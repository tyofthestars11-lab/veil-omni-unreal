"""Clear the plaza of the shells that hold the capsule still.

Measured every run: the runner spawns on the new floor and the unstick immediately says
"no open ground within 1500 cm - the capsule is boxed in", and the move proof reads
forward=0 with 121 zero frames. Something solid is standing where he stands. This removes
it: any actor whose static mesh bounds contain the spawn column within 700 cm is scenery
duplicated over the plaza, and it is destroyed. The nine PHI GROUND tiles are tagged and
kept.
"""

import io

P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []

# 1. tag the ground tiles so the clear pass never eats its own floor
TAG_ANCHOR = "\t\t\t\t\t\t\t\tif (FloorStone) { Comp->SetMaterial(0, FloorStone); }"
TAG_NEW = (TAG_ANCHOR + "\n"
           "\t\t\t\t\t\t\t\tTile->Tags.Add(FName(TEXT(\"PhiGround\")));")
if text.count(TAG_ANCHOR) != 1:
    fails.append("tag anchor count=%d" % text.count(TAG_ANCHOR))
else:
    text = text.replace(TAG_ANCHOR, TAG_NEW.replace("\n", NL), 1)

# 2. clear the spawn column
CLEAR_ANCHOR = "\t\t\tUE_LOG(LogVEIL, Warning,\n\t\t\t\tTEXT(\"PHI AUTHORITY SEAT: runner stands on V(0,0,100) over real ground.\"));"
CLEAR = CLEAR_ANCHOR + """
			// CLEAR THE SPAWN COLUMN. Duplicated shells stand where the market is, so the
			// capsule begins inside geometry and nothing can move it. Anything solid within
			// 700 cm of the spawn and below 400 cm up is scenery over the plaza; the tagged
			// PHI GROUND tiles are the floor and stay.
			{
				const FVector Plaza(0.f, -1500.f, 100.f);
				int32 Cleared = 0;
				for (TActorIterator<AActor> ShellIt(GroundWorld); ShellIt; ++ShellIt)
				{
					AActor* Shell = *ShellIt;
					if (!Shell || Shell == this) { continue; }
					if (Shell->Tags.Contains(FName(TEXT("PhiGround")))) { continue; }
					TInlineComponentArray<UStaticMeshComponent*> ShellComps;
					Shell->GetComponents(ShellComps);
					bool bOverPlaza = false;
					for (UStaticMeshComponent* ShellComp : ShellComps)
					{
						if (!ShellComp) { continue; }
						const FBoxSphereBounds B = ShellComp->Bounds;
						if (FVector::Dist2D(B.Origin, Plaza) > 700.f) { continue; }
						if (B.Origin.Z > 400.f || B.Origin.Z < -300.f) { continue; }
						bOverPlaza = true;
						break;
					}
					if (bOverPlaza) { Shell->Destroy(); ++Cleared; }
				}
				UE_LOG(LogVEIL, Warning, TEXT("PHI PLAZA CLEAR: %d shell(s) removed from the spawn column."), Cleared);
			}"""
o, n = CLEAR_ANCHOR.replace("\n", NL), CLEAR.replace("\n", NL)
if text.count(o) != 1:
    fails.append("clear anchor count=%d" % text.count(o))
else:
    text = text.replace(o, n, 1)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(P, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (plaza cleared)" % (P, len(src), len(text)))
