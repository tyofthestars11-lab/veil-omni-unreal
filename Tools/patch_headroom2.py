"""Measure the clearance over the runner's own head after the shells are tallened.

Counting raised roofs proves the pass ran; it does not prove the player can stand under them.
So a second timer traces straight up from the pawn and reports the roof distance in cm against
the 180 cm standing body. A lift ladder of start heights is used because a trace that begins
inside a slab reports nothing.
"""

import io

P = "Source/VEIL/Private/Game/VEILGameMode.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"

ANCHOR = "\t\t}), 2.5f, false);" + N + "\t}"
HOOK = '''		}), 2.5f, false);

		FTimerHandle CeilingTimer;
		GetWorldTimerManager().SetTimer(CeilingTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWorld* W = GetWorld();
			APawn* Me = UGameplayStatics::GetPlayerPawn(this, 0);
			if (!W || !Me) { return; }
			const FVector From = Me->GetActorLocation();
			FCollisionQueryParams Q(SCENE_QUERY_STAT(PhiCeiling), false, Me);
			float Clear = -1.f;
			const float Lifts[4] = { 0.f, 300.f, 900.f, 1600.f };
			for (int32 i = 0; i < 4; ++i)
			{
				FHitResult Hit;
				const FVector Start = From + FVector(0.f, 0.f, Lifts[i]);
				if (W->LineTraceSingleByChannel(Hit, Start, Start + FVector(0.f, 0.f, 4000.f), ECC_Visibility, Q))
				{
					Clear = Hit.ImpactPoint.Z - From.Z;
					break;
				}
			}
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI CEILING: roof sits %.0f cm over the runner at (%.0f, %.0f) - a standing body is 180 cm, so this is %s."),
				Clear, From.X, From.Y,
				Clear >= 260.f ? TEXT("HEADROOM") : TEXT("STILL LOW"));
		}), 4.0f, false);
	}'''

if s.count(ANCHOR) != 1:
    print("anchor count=%d" % s.count(ANCHOR))
    raise SystemExit(1)

s = s.replace(ANCHOR, HOOK.replace("\n", N), 1)
io.open(P, "w", encoding="utf-8", newline="").write(s)
print("CEILING MEASUREMENT WRITTEN")
