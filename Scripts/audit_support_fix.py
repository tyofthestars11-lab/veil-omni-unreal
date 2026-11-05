"""Floating audit, done properly: measure the gap under the piece's own base.

The old test culled anything inside the Hub radius that sat more than 900 above the
plaza. That punished NPCs standing on real terrain peaks. A piece is floating only
when there is a gap between its lowest point and the geometry beneath it.
"""
import re

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
path = P + "VeilGameMode.cpp"

s = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n")

NEW = '''	// Nothing floats. A piece is floating only when there is a real gap between its
	// lowest point and the first geometry beneath it. Standing on a peak is legal.
	int32 Lifted = 0;
	const double GapTolerance = PHI_Constants::Reach(-0.95);
	for (AActor* A : Parts)
	{
		if (!A)
		{
			continue;
		}
		const FBox Bounds = A->GetComponentsBoundingBox(true);
		const FVector Bottom(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Min.Z + 2.0);
		FHitResult Hit;
		FCollisionQueryParams Q(SCENE_QUERY_STAT(VeilAudit), false, A);
		const bool bFound = GetWorld()->LineTraceSingleByChannel(
			Hit, Bottom, Bottom - FVector(0.0, 0.0, PHI_Constants::Reach(6)), ECC_Visibility, Q);
		if (bFound && (Bottom.Z - Hit.ImpactPoint.Z) <= GapTolerance)
		{
			continue;
		}
		UE_LOG(LogVEILWorld, Warning, TEXT("VEIL CULL: %s is hanging at (%.0f, %.0f, %.0f) gap=%.1f"),
			*A->GetName(), A->GetActorLocation().X, A->GetActorLocation().Y,
			A->GetActorLocation().Z, bFound ? (Bottom.Z - Hit.ImpactPoint.Z) : -1.0);
		A->Destroy();
		++Lifted;
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD AUDIT: %d floating piece(s) removed, support test on every part"), Lifted);
'''

pattern = re.compile(
    r"\t// Nothing floats\..*?UE_LOG\(LogVEILWorld, Display, TEXT\(\"VEIL WORLD AUDIT:.*?\);\n",
    re.S)

if "support test on every part" in s:
    print("already patched")
else:
    m = pattern.search(s)
    if not m:
        raise SystemExit("ANCHOR MISS: audit block not found, refusing to guess")
    s = s[:m.start()] + NEW + s[m.end():]
    open(path, "w", encoding="utf-8", newline="").write(s)
    print("audit replaced with support-gap test")
