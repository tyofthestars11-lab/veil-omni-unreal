"""One 26 m slab was invisible; build paving you can stand on and prove it by trace."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

start = s.find("\t// The terrace is seated on the real surface under the Hub")
if start < 0:
    raise SystemExit("MISS terrace block start")
endmark = "\t\tGPlazaZ = GHubGroundZ + PHI_Constants::Reach(0.986);\n\t}\n"
e = s.find(endmark, start)
if e < 0:
    raise SystemExit("MISS terrace block end")
e += len(endmark)

new = '''	// The terrace is paving: a grid of stone slabs seated on the real surface, so
	// the market stands on a floor that exists in the frame, slab by slab.
	GSnapEnabled = true;
	double DeckTop = 0.0;
	int32 Slabs = 0;
	const double Tile = 900.0;
	for (int32 ix = -1; ix <= 1; ++ix)
	{
		for (int32 iy = -1; iy <= 1; ++iy)
		{
			if (AActor* Slab = FitPart(MeshFloor, FVector(ix * Tile, iy * Tile, GHubGroundZ),
				Tile, PHI_Constants::Reach(0.986), FRotator::ZeroRotator, MatStone, true))
			{
				++Slabs;
				FVector Origin, Extent;
				Slab->GetActorBounds(true, Origin, Extent);
				DeckTop = FMath::Max(DeckTop, Origin.Z + Extent.Z);
			}
		}
	}
	GPlazaZ = (DeckTop > GHubGroundZ) ? DeckTop : (GHubGroundZ + PHI_Constants::Reach(0.986));
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL HUB TERRACE: %d paving slabs, deck top %.1f over ground %.1f"),
		Slabs, GPlazaZ, GHubGroundZ);
	{
		// Proof, not assumption: what is actually under the plaza centre.
		FHitResult DeckHit;
		if (GetWorld()->LineTraceSingleByChannel(DeckHit, FVector(0.0, 0.0, GPlazaZ + 500.0),
			FVector(0.0, 0.0, GHubGroundZ - 500.0), ECC_WorldStatic))
		{
			UE_LOG(LogVEILWorld, Display,
				TEXT("VEIL HUB FLOOR TRACE: hit %s at %.1f (deck %.1f)"),
				*DeckHit.GetActor()->GetName(), DeckHit.ImpactPoint.Z, GPlazaZ);
		}
		else
		{
			UE_LOG(LogVEILWorld, Display, TEXT("VEIL HUB FLOOR TRACE: NOTHING under (0,0)"));
		}
	}
'''
s = s[:start] + new + s[e:]

pairs = [
    ("\tstatic const double BackPow[5] = { 1.2, 2.0, 2.0, 2.0, 2.0 };",
     "\tstatic const double BackPow[5] = { 0.9, 2.0, 2.0, 2.0, 2.0 };"),
]
for old, newv in pairs:
    if old not in s:
        raise SystemExit("MISS camera")
    s = s.replace(old, newv, 1)

open(p, "w", encoding="utf-8", newline="").write(s)
print("hub: 9 paving slabs, floor trace, camera inside the rings")
