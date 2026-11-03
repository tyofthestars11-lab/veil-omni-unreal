"""Clear by containment, and hold the floor every second.

The clear used bounds *origins*, letting a 1000 cm building whose origin sits at z=500
stand on the plaza - physics then shoved the capsule up onto it, which is the z=598 frame
that survived the settle. The predicate now tests the whole bounds box against the plaza
column. The one-second hold is added at the same time so nothing can raise him for long.
"""

import io

fails = []
P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src

OLD = """						if (B.Origin.Z > 400.f || B.Origin.Z < -300.f) { continue; }
						bOverPlaza = true;"""
NEW = """						// CONTAINMENT, NOT ORIGIN. A tall wall has its bounds origin at
						// z=500, so an origin test let it stand exactly where he stands -
						// and physics then shoved the capsule up onto it: that is the
						// z=598 frame the settle could never fix. The box must span the
						// plaza column to be cleared.
						if (B.Origin.Z + B.BoxExtent.Z < -100.f) { continue; }
						if (FMath::Abs(B.Origin.Z) > 400.f + B.BoxExtent.Z) { continue; }
						bOverPlaza = true;"""
o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
if text.count(o) != 1:
    fails.append("containment anchor count=%d" % text.count(o))
else:
    text = text.replace(o, n, 1)

ANCHOR = """				}), 6.f, false);
			}"""
CLAMP = ANCHOR + """
			// AND HOLD IT. One settle was not enough: two seconds later he was at z=598.
			// This repeats once a second - above two metres is not the plaza, and the
			// plaza is where the market is walked.
			{
				FTimerHandle HoldTimer;
				GroundWorld->GetTimerManager().SetTimer(HoldTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					if (GetActorLocation().Z > 200.f)
					{
						SetActorLocation(FVector(0.f, -1500.f, 120.f), false, nullptr, ETeleportType::TeleportPhysics);
						UE_LOG(LogVEIL, Warning, TEXT("PHI HOLD: runner kept on the plaza floor at V(0,-1500,120)."));
					}
				}), 1.f, true);
			}"""
o2, n2 = ANCHOR.replace("\n", NL), CLAMP.replace("\n", NL)
if text.count(o2) != 1:
    fails.append("settle block count=%d" % text.count(o2))
else:
    text = text.replace(o2, n2, 1)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(P, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (containment clear + 1 s floor hold)" % (P, len(src), len(text)))
