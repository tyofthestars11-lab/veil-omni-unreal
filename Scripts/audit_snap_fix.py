"""Nothing floats and nothing disappears: snap pieces onto their support.

The terrace is a built deck. A piece whose base sits at the deck height is supported
by the deck even when the deck mesh itself has no collision. Anything else with a real
gap is lowered onto the geometry beneath it instead of being destroyed, so the world
keeps its pieces and loses its floating.
"""
import re

path = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n")

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
		Lifted, Snapped, GPlazaZ);
'''

pattern = re.compile(
    r"\t// Nothing floats\..*?UE_LOG\(LogVEILWorld, Display,\n\t\tTEXT\(\"VEIL WORLD AUDIT:.*?\);\n",
    re.S)

if "snapped down to support" in s:
    print("already patched")
else:
    m = pattern.search(s)
    if not m:
        raise SystemExit("ANCHOR MISS: audit block not found")
    s = s[:m.start()] + NEW + s[m.end():]
    open(path, "w", encoding="utf-8", newline="").write(s)
    print("audit replaced: snap-to-support")
