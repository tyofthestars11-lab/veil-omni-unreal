"""Hold the runner on the floor, every second, until something else stops lifting him.

The settle fires once and logs V(0,-1500,120); two seconds later the proof begins at
z=598 again, so another system is raising him after the settle. This repeats the clamp on
a one-second timer: above two metres means he is not on the plaza, and he is put back.
Also the boxed-in rescue now uses the same open ground as the seat instead of 0,0,100,
which is the column the plaza clear proved is still solid.
"""

import io

fails = []
P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src

# the rescue must aim at the same open ground as the seat
OLD = "SetActorLocation(FVector(0.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);"
NEW = "SetActorLocation(FVector(0.f, -1500.f, 120.f), false, nullptr, ETeleportType::TeleportPhysics);"
c = text.count(OLD)
if c != 1:
    fails.append("rescue vector count=%d" % c)
else:
    text = text.replace(OLD, NEW, 1)

# a repeating clamp, anchored on the settle timer block
ANCHOR = """				}), 6.f, false);
			}"""
CLAMP = ANCHOR + """
			// AND HOLD IT. One settle was not enough: a second later he was at z=598 again,
			// so something raises him right after. This repeats once a second - above two
			// metres is not the plaza, and the plaza is where the market is walked.
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
o, n = ANCHOR.replace("\n", NL), CLAMP.replace("\n", NL)
if text.count(o) != 1:
    fails.append("settle block count=%d" % text.count(o))
else:
    text = text.replace(o, n, 1)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(P, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (floor held every second)" % (P, len(src), len(text)))
