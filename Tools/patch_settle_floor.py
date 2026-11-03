"""Settle the runner on the plaza floor six seconds in.

The plaza clear removed 47 shells and the move proof now reads forward=459 cm with
zero=0 - but the frame still begins at z=598, because the seat is lifted onto a shelf that
stands outside the 700 cm clear radius before the proof starts. Six seconds in, anything
above two metres is brought back down to the ground the tiles laid: V(0,-1500,120).
"""

import io

P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = '\t\t\t\tUE_LOG(LogVEIL, Warning, TEXT("PHI PLAZA CLEAR: %d shell(s) removed from the spawn column."), Cleared);\n\t\t\t}'
ADD = ANCHOR + """
			// SETTLE ON THE FLOOR. Whatever lifts him - a shelf outside the clear radius, a
			// ground snap, the unstick - cannot survive this: six seconds in, above two
			// metres means he is not standing on the plaza, and he is put back on it.
			{
				FTimerHandle SettleTimer;
				GroundWorld->GetTimerManager().SetTimer(SettleTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					if (GetActorLocation().Z > 200.f)
					{
						SetActorLocation(FVector(0.f, -1500.f, 120.f), false, nullptr, ETeleportType::TeleportPhysics);
						UE_LOG(LogVEIL, Warning,
							TEXT("PHI SETTLE: runner brought down to the plaza floor at V(0,-1500,120)."));
					}
				}), 6.f, false);
			}"""

o, n = ANCHOR.replace("\n", NL), ADD.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - anchor count=%d" % src.count(o))
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - settle timer at 6 s" % P)
