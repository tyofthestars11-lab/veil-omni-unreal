"""The Hub's crystal peaks were cliff slabs parked in the Omni district."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

start = s.find("\t// 5 blue crystal peaks of the Hub count")
if start < 0:
    raise SystemExit("MISS crystal block")

end = s.find("\t// The Vault sits below the terrace", start)
if end < 0:
    raise SystemExit("MISS crystal block end")

new = '''	// The five crystal peaks are spires standing on the Hub terrace itself, in
	// signal blue. A flat cliff slab is not a peak, and Omni is not the Hub.
	int32 Crystals = 0;
	for (int32 i = 1; i <= 5; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi);
		if (FitPart(MeshMonolith, FVector(P.X, P.Y, GPlazaZ),
			PHI_Constants::Reach(0.9), PHI_Constants::Reach(3.2),
			FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), MatSignal, true))
		{
			++Crystals;
		}
	}
	
'''
s = s[:start] + new + s[end:]
open(p, "w", encoding="utf-8", newline="").write(s)
print("hub: crystal peaks are standing spires on the terrace")
