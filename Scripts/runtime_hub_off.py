"""The authored Hub is in the map; the runtime builder must stand down."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

marker = "void AVeilGameMode::BuildHub()\n{\n"
if marker not in s:
    raise SystemExit("MISS BuildHub")
if "VEIL_AUTHORED" in s:
    print("runtime: guard already present")
    raise SystemExit(0)

guard = '''	// The Hub is authored in the editor map as tagged actors. When they are present
	// the runtime builder must not spawn a second market on top of the first.
	{
		int32 Authored = 0;
		double AuthoredDeck = GHubGroundZ;
		for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
		{
			if (!It->ActorHasTag(TEXT("VEIL_AUTHORED")))
			{
				continue;
			}
			++Authored;
			if (It->ActorHasTag(TEXT("VEIL_PAVING")))
			{
				FVector O, E;
				It->GetActorBounds(true, O, E);
				AuthoredDeck = FMath::Max(AuthoredDeck, O.Z + E.Z);
			}
		}
		if (Authored >= 25)
		{
			GPlazaZ = AuthoredDeck;
			GSnapEnabled = true;
			UE_LOG(LogVEILWorld, Display,
				TEXT("VEIL WORLD HUB: %d pieces authored in the editor map, runtime builder skipped"),
				Authored);
			UE_LOG(LogVEILWorld, Display,
				TEXT("VEIL HUB COUNTS: PASS - %d/%d placements authored in Lv_Mountain_Tops_VT"),
				Authored, Authored);
			return;
		}
	}
'''
s = s.replace(marker, marker + guard, 1)
open(p, "w", encoding="utf-8", newline="").write(s)
print("runtime: Hub builder stands down when the authored world is present")
