"""Probe every roof in the plaza for its true clear height.

Counting raised roofs says the pass ran; it does not say a body fits under them. So each roof
in the plaza is probed from just under its own bottom edge straight down to the floor beneath
it, and the vertical gap is reported as that interior's clear height - min, how many are still
under 260 cm, and a verdict. A trace that hits nothing overhead is open sky, not a low roof.
"""

import io

P = "Source/VEIL/Private/Game/VEILGameMode.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"

OLD = '''			UE_LOG(LogVEIL, Warning,
				TEXT("PHI CEILING: %d sheet(s) culled, roof now %.0f cm over the runner at (%.0f, %.0f) - standing body 180 cm, verdict %s."),
				Sheets, Clear, Feet.X, Feet.Y,
				Clear >= 260.f ? TEXT("HEADROOM") : TEXT("STILL LOW"));'''

NEW = '''			UE_LOG(LogVEIL, Warning,
				TEXT("PHI CEILING: %d sheet(s) culled, roof now %s over the runner at (%.0f, %.0f) - standing body 180 cm."),
				Sheets,
				Clear < 0.f ? TEXT("OPEN SKY") : *FString::Printf(TEXT("%.0f cm"), Clear),
				Feet.X, Feet.Y);

			// Every roof in the plaza is probed from under its own bottom edge down to the floor
			// beneath it: that gap IS the interior's clear height, measured rather than assumed.
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
				FHitResult Floor;
				const FVector Start(At.X, At.Y, Box.Min.Z - 5.f);
				if (!W->LineTraceSingleByChannel(Floor, Start, Start - FVector(0.f, 0.f, 3000.f), ECC_Visibility, Q)) { continue; }
				const float Gap = Box.Min.Z - Floor.ImpactPoint.Z;
				if (Gap <= 0.f || Gap > 3000.f) { continue; }
				MinClear = FMath::Min(MinClear, Gap);
				++Probed;
				if (Gap < 260.f) { ++Low; }
			}
			if (Probed > 0)
			{
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI INTERIOR: %d roof(s) probed, min clear %.0f cm, %d under 260 cm - verdict %s."),
					Probed, MinClear, Low, Low == 0 ? TEXT("EVERY INTERIOR CLEARS A BODY") : TEXT("STILL CRAMPED"));
			}
			else
			{
				UE_LOG(LogVEIL, Warning, TEXT("PHI INTERIOR: no roof above a floor was found in the plaza - nothing to probe."));
			}'''

if s.count(OLD) != 1:
    print("verdict block count=%d" % s.count(OLD))
    raise SystemExit(1)

s = s.replace(OLD, NEW.replace("\n", N), 1)
io.open(P, "w", encoding="utf-8", newline="").write(s)
print("INTERIOR PROBE WRITTEN")
