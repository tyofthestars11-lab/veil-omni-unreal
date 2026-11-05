"""The deck must sit on the land it is measured against, and spires are stone."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old_start = "\tGPlazaZ = GHubGroundZ + PHI_Constants::Reach(0.986);\n\tGSnapEnabled = false;"
if old_start in s:
    end_anchor = "FRotator::ZeroRotator, MatStone, true);\n"
    find_from = s.find(old_start)
    end = s.find(end_anchor, find_from)
    if end < 0:
        raise SystemExit("MISS terrace FitPart")
    end += len(end_anchor)
    new = '''	// The terrace is seated on the real surface under the Hub, and the deck height
	// is read back from where it actually landed, so nothing floats on a guess.
	GSnapEnabled = true;
	if (AActor* Terrace = FitPart(MeshFloor, FVector(0.0, 0.0, GHubGroundZ),
		PHI_Constants::Reach(5.771), PHI_Constants::Reach(0.986), FRotator::ZeroRotator, MatStone, true))
	{
		FVector Origin, Extent;
		Terrace->GetActorBounds(true, Origin, Extent);
		GPlazaZ = Origin.Z + Extent.Z;
		UE_LOG(LogVEILWorld, Display, TEXT("VEIL HUB TERRACE: seated top at %.1f over ground %.1f"),
			GPlazaZ, GHubGroundZ);
	}
	else
	{
		GPlazaZ = GHubGroundZ + PHI_Constants::Reach(0.986);
	}
'''
    s = s[:find_from] + new + s[end:]
    print("hub: terrace seated by trace, deck read from bounds")

# the spires were wearing the signal slab material at slab scale
pairs = [
    ("PHI_Constants::Reach(0.9), PHI_Constants::Reach(3.2),",
     "PHI_Constants::Reach(0.6), PHI_Constants::Reach(2.6),"),
    ("FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), MatSignal, true))",
     "FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), MatStone, true))"),
]
for old, new in pairs:
    if old in s:
        s = s.replace(old, new, 1)
        print("hub: spire", old[:40], "->", new[:40])
    else:
        print("hub: MISS", old[:40])

open(p, "w", encoding="utf-8", newline="").write(s)
print("terrace and spires done")
