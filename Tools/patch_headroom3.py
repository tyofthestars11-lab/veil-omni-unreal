"""Cull the sheet that lies across the runner's eyes and measure the real clearance.

The proof frame shows a large dark plane filling the view at head height, which is what reads
as a low ceiling from inside a shell. A mesh that CONTAINS the head point and is thin in Z
under 120 cm cannot be a floor or a wall - it is a sheet lying across the view - so it is
hidden and named in the log. Then a trace straight up from the pawn reports the distance to
whatever is actually overhead against the 180 cm standing body.
"""

import io

P = "Source/VEIL/Private/Game/VEILGameMode.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"

ANCHOR = "\t// -phitype: THE CLICK, WITHOUT A MOUSE."
HOOK = '''	// THE SHEET AT EYE HEIGHT. A mesh that contains the head point and is thinner than 120 cm in
	// Z is neither floor nor wall - it is a plane lying across the view, and from inside a shell
	// it reads as the ceiling resting on the player. Each one is named and hidden, then the
	// clearance over the head is traced and reported against the 180 cm body.
	{
		FTimerHandle SheetTimer;
		GetWorldTimerManager().SetTimer(SheetTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWorld* W = GetWorld();
			APawn* Me = UGameplayStatics::GetPlayerPawn(this, 0);
			if (!W || !Me) { return; }
			const FVector Feet = Me->GetActorLocation();
			const FVector Head = Feet + FVector(0.f, 0.f, 60.f);

			int32 Sheets = 0;
			for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
			{
				AStaticMeshActor* Piece = *It;
				if (!Piece) { continue; }
				UStaticMeshComponent* Body = Piece->GetStaticMeshComponent();
				if (!Body || !Body->GetStaticMesh()) { continue; }
				const FBox Box = Body->Bounds.GetBox();
				const FVector Size = Box.GetSize();
				if (Size.Z > 120.f) { continue; }
				if (!Box.IsInsideOrOn(Head)) { continue; }
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI SHEET CULL: hid %s (%s) %.0f x %.0f x %.0f cm at z=%.0f - it lay across the eyes."),
					*Piece->GetName(), *Body->GetStaticMesh()->GetName(), Size.X, Size.Y, Size.Z, Box.GetCenter().Z);
				Piece->SetActorHiddenInGame(true);
				++Sheets;
			}

			FCollisionQueryParams Q(SCENE_QUERY_STAT(PhiCeiling), false, Me);
			float Clear = -1.f;
			const float Lifts[4] = { 0.f, 300.f, 900.f, 1600.f };
			for (int32 i = 0; i < 4; ++i)
			{
				FHitResult Hit;
				const FVector Start = Feet + FVector(0.f, 0.f, Lifts[i]);
				if (W->LineTraceSingleByChannel(Hit, Start, Start + FVector(0.f, 0.f, 4000.f), ECC_Visibility, Q))
				{
					Clear = Hit.ImpactPoint.Z - Feet.Z;
					break;
				}
			}
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI CEILING: %d sheet(s) culled, roof now %.0f cm over the runner at (%.0f, %.0f) - standing body 180 cm, verdict %s."),
				Sheets, Clear, Feet.X, Feet.Y,
				Clear >= 260.f ? TEXT("HEADROOM") : TEXT("STILL LOW"));
		}), 3.5f, false);
	}

''' + ANCHOR

if s.count(ANCHOR) != 1:
    print("anchor count=%d" % s.count(ANCHOR))
    raise SystemExit(1)

s = s.replace(ANCHOR, HOOK.replace("\n", N), 1)
io.open(P, "w", encoding="utf-8", newline="").write(s)
print("SHEET CULL + CEILING MEASUREMENT WRITTEN")
