"""Replace the ceiling log (lines found by PHI CEILING .. STILL LOW) with the measured version
plus a per-roof interior probe. CRLF safe."""

import io

P = "Source/VEIL/Private/Game/VEILGameMode.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"
lines = s.split(N)

i = next((k for k, ln in enumerate(lines) if "PHI CEILING:" in ln), None)
if i is None:
    print("no PHI CEILING line")
    raise SystemExit(1)
start = i - 1
while "UE_LOG(" not in lines[start]:
    start -= 1
end = next(k for k in range(i, len(lines)) if "STILL LOW" in lines[k])
print("replacing lines %d..%d" % (start, end))

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
				FHitResult FloorHit;
				const float Down = Box.Min.Z - 5.f;
				if (!W->LineTraceSingleByChannel(FloorHit, FVector(At.X, At.Y, Down),
					FVector(At.X, At.Y, Down - 3000.f), ECC_Visibility, Q)) { continue; }
				const float Gap = Down - FloorHit.ImpactPoint.Z;
				if (Gap <= 0.f || Gap > 2000.f) { continue; }
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
			}'''.split("\n")

lines[start:end + 1] = NEW
io.open(P, "w", encoding="utf-8", newline="").write(N.join(lines))
print("INTERIOR PROBE WRITTEN")
