"""Replace the audit block by line markers: robust, no formatting assumptions."""

path = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
lines = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n").split("\n")

start = next((i for i, l in enumerate(lines) if "// Nothing floats" in l), None)
if start is None:
    raise SystemExit("ANCHOR MISS: start marker not found")

end = next((i for i in range(start, len(lines)) if "VEIL WORLD AUDIT" in lines[i]), None)
if end is None:
    raise SystemExit("ANCHOR MISS: end marker not found")

if any("snapped down to support" in l for l in lines):
    print("already patched")
    raise SystemExit(0)

NEW = '''	// Nothing floats, and nothing is thrown away. The terrace deck is a built surface:
	// a piece seated at deck height is supported by the deck. Any other real gap is
	// closed by lowering the piece onto the geometry beneath it.
	int32 Lifted = 0;
	int32 Snapped = 0;
	const double GapTolerance = PHI_Constants::Reach(-1.8);
	for (AActor* A : Parts)
	{
		if (!A)
		{
			continue;
		}
		const FBox Bounds = A->GetComponentsBoundingBox(true);
		if (FMath::Abs(Bounds.Min.Z - GPlazaZ) <= GapTolerance)
		{
			continue;
		}
		const FVector Bottom(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Min.Z + 2.0);
		FHitResult Hit;
		FCollisionQueryParams Q(SCENE_QUERY_STAT(VeilAudit), false, A);
		const bool bFound = GetWorld()->LineTraceSingleByChannel(
			Hit, Bottom, Bottom - FVector(0.0, 0.0, PHI_Constants::Reach(6)), ECC_Visibility, Q);
		const double Gap = bFound ? (Bottom.Z - Hit.ImpactPoint.Z) : -1.0;
		if (bFound && Gap <= GapTolerance)
		{
			continue;
		}
		if (bFound)
		{
			A->AddActorWorldOffset(FVector(0.0, 0.0, -Gap));
			++Snapped;
			UE_LOG(LogVEILWorld, Warning, TEXT("VEIL SNAP DOWN: %s closed a %.1f gap onto its support"),
				*A->GetName(), Gap);
			continue;
		}
		UE_LOG(LogVEILWorld, Warning, TEXT("VEIL CULL: %s has no support at all at (%.0f, %.0f, %.0f)"),
			*A->GetName(), A->GetActorLocation().X, A->GetActorLocation().Y, A->GetActorLocation().Z);
		A->Destroy();
		++Lifted;
	}
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL WORLD AUDIT: %d unsupported piece(s) removed, %d snapped down to support, deck at %.1f"),
		Lifted, Snapped, GPlazaZ);'''.split("\n")

lines[start:end + 1] = NEW
open(path, "w", encoding="utf-8", newline="").write("\n".join(lines))
print("audit block replaced by line markers; lines {}..{}".format(start + 1, end + 1))
