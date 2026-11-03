"""Reconstruct the sheet-cull / lift / probe timer block as one balanced unit.

The previous splice left the lambda one brace short. Rather than patch around a broken region,
the whole block is replaced from its comment through its closing `}), 3.5f, false);`.
"""

import io

P = "Source/VEIL/Private/Game/VEILGameMode.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"
lines = s.split(N)

start = next((k for k, ln in enumerate(lines) if "THE SHEET AT EYE HEIGHT" in ln), None)
end = next((k for k, ln in enumerate(lines) if "}), 3.5f, false);" in ln and k > start), None)
if start is None or end is None:
    print("block not found start=%s end=%s" % (start, end))
    raise SystemExit(1)
print("replacing lines %d..%d" % (start, end))

B = '''	// THE CUT AND THE MEASUREMENT. Three things happen once the district has settled:
	//  1. A mesh that CONTAINS the head point and is thinner than 120 cm in Z is neither floor
	//     nor wall - it is a plane lying across the view, which from inside a shell reads as the
	//     ceiling resting on the player. Each one is named and hidden.
	//  2. A roof whose measured floor gap is under 340 cm is lifted by exactly its shortfall
	//     plus margin, which is what actually gives the short shells headroom.
	//  3. Every roof is probed again from under its own bottom edge down to its floor, and that
	//     gap is reported as the interior's clear height, so the number is the result and not
	//     the intent.
	{
		FTimerHandle SheetTimer;
		GetWorldTimerManager().SetTimer(SheetTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWorld* W = GetWorld();
			APawn* Me = UGameplayStatics::GetPlayerPawn(this, 0);
			if (!W || !Me) { return; }
			const FVector Feet = Me->GetActorLocation();
			const FVector Head = Feet + FVector(0.f, 0.f, 60.f);
			FCollisionQueryParams Q(SCENE_QUERY_STAT(PhiCeiling), false, Me);

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
				TEXT("PHI CEILING: %d sheet(s) culled, roof now %s over the runner at (%.0f, %.0f) - standing body 180 cm."),
				Sheets,
				Clear < 0.f ? TEXT("OPEN SKY") : *FString::Printf(TEXT("%.0f cm"), Clear),
				Feet.X, Feet.Y);

			int32 Lifted = 0;
			for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
			{
				AStaticMeshActor* Piece = *It;
				if (!Piece) { continue; }
				UStaticMeshComponent* Body = Piece->GetStaticMeshComponent();
				if (!Body || !Body->GetStaticMesh()) { continue; }
				const FString MeshName = Body->GetStaticMesh()->GetName();
				if (!MeshName.Contains(TEXT("Roof")) && !MeshName.Contains(TEXT("Ceil"))) { continue; }
				const FBox Box = Body->Bounds.GetBox();
				const FVector At = Box.GetCenter();
				if (FVector::Dist2D(At, FVector::ZeroVector) > 6000.f) { continue; }
				FHitResult Under;
				const float Down = Box.Min.Z - 5.f;
				if (!W->LineTraceSingleByChannel(Under, FVector(At.X, At.Y, Down),
					FVector(At.X, At.Y, Down - 3000.f), ECC_Visibility, Q)) { continue; }
				const float Gap = Down - Under.ImpactPoint.Z;
				if (Gap <= 0.f || Gap >= 340.f) { continue; }
				Piece->AddActorWorldOffset(FVector(0.f, 0.f, 380.f - Gap));
				++Lifted;
			}

			int32 Probed = 0, Low = 0;
			float MinClear = 1.0e9f;
			for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
			{
				AStaticMeshActor* Piece = *It;
				if (!Piece) { continue; }
				UStaticMeshComponent* Body = Piece->GetStaticMeshComponent();
				if (!Body || !Body->GetStaticMesh()) { continue; }
				const FString MeshName = Body->GetStaticMesh()->GetName();
				if (!MeshName.Contains(TEXT("Roof")) && !MeshName.Contains(TEXT("Ceil"))) { continue; }
				const FBox Box = Body->Bounds.GetBox();
				const FVector At = Box.GetCenter();
				if (FVector::Dist2D(At, FVector::ZeroVector) > 6000.f) { continue; }
				FHitResult FloorHit;
				const float Down = Box.Min.Z - 5.f;
				if (!W->LineTraceSingleByChannel(FloorHit, FVector(At.X, At.Y, Down),
					FVector(At.X, At.Y, Down - 3000.f), ECC_Visibility, Q)) { continue; }
				const float Gap = Down - FloorHit.ImpactPoint.Z;
				if (Gap <= 0.f || Gap > 2000.f) { continue; }
				MinClear = FMath::Min(MinClear, Gap);
				++Probed;
				if (Gap < 300.f) { ++Low; }
			}
			if (Probed > 0)
			{
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI INTERIOR: lifted %d short roof(s); %d probed, min clear %.0f cm, %d under 300 cm - verdict %s."),
					Lifted, Probed, MinClear, Low,
					Low == 0 ? TEXT("EVERY INTERIOR CLEARS A BODY") : TEXT("STILL CRAMPED"));
			}
			else
			{
				UE_LOG(LogVEIL, Warning, TEXT("PHI INTERIOR: no roof above a floor was found in the plaza - nothing to probe."));
			}
		}), 3.5f, false);
	}'''.split("\n")

lines[start:end + 1] = B
io.open(P, "w", encoding="utf-8", newline="").write(N.join(lines))
print("SHEET BLOCK REBUILT")
