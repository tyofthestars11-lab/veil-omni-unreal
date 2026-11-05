"""Bible M4: the Hub shot must show the market, and people stand on its paving."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

# 1. every body inside the plaza footprint stands on the authored deck
if "VEIL HUB CROWD" not in s:
    idx = s.find('TEXT("VEIL WORLD LIFE:')
    if idx < 0:
        raise SystemExit("MISS life log")
    stmt = s.rfind("\tUE_LOG(", 0, idx)
    if stmt < 0:
        raise SystemExit("MISS life statement")
    block = '''	// Bodies inside the plaza stand on the deck the market was authored on, not on
	// the slope below it. The deck is the floor, so the floor is where they stand.
	{
		const double Half = PHI_Constants::Reach(5.771) * 0.5;
		int32 OnDeck = 0;
		for (TActorIterator<AVeilNpc> It(GetWorld()); It; ++It)
		{
			const FVector Loc = It->GetActorLocation();
			if (FMath::Abs(Loc.X) <= Half && FMath::Abs(Loc.Y) <= Half
				&& FMath::Abs(Loc.Z - GPlazaZ) > 5.0)
			{
				It->SetActorLocation(FVector(Loc.X, Loc.Y, GPlazaZ));
				++OnDeck;
			}
		}
		UE_LOG(LogVEILWorld, Display,
			TEXT("VEIL HUB CROWD: %d bodies stood on the authored deck at %.1f"), OnDeck, GPlazaZ);
	}
	
'''
    s = s[:stmt] + block + s[stmt:]
    print("crowd: bodies inside the plaza stand on the authored deck")

# 2. the record camera rises above the market instead of standing inside it
pairs = [
    ("\tstatic const double BackPow[5] = { 0.44, 2.0, 2.0, 2.0, 2.0 };",
     "\tstatic const double BackPow[5] = { 0.0, 2.0, 2.0, 2.0, 2.0 };"),
    ("\tstatic const double UpPow[5] = { 3.04, 1.6, 1.6, 1.6, 1.6 };",
     "\tstatic const double UpPow[5] = { 5.0, 1.6, 1.6, 1.6, 1.6 };"),
]
for old, newv in pairs:
    if old not in s:
        raise SystemExit("MISS " + old[:44])
    s = s.replace(old, newv, 1)
print("shot: Hub camera above the market, aimed at its centre")

open(p, "w", encoding="utf-8", newline="").write(s)
print("done")
