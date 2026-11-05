"""The reseat ran before the bodies existed; run it after the world is built."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

start = s.find("\t// Bodies inside the plaza stand on the deck the market was authored on")
if start < 0:
    raise SystemExit("MISS reseat block")
endmark = 'TEXT("VEIL HUB CROWD: %d bodies stood on the authored deck at %.1f"), OnDeck, GPlazaZ);\n\t\t}\n\t}\n'
e = s.find(endmark, start)
if e < 0:
    raise SystemExit("MISS reseat end")
e += len(endmark)
s = s[:start] + s[e:]
print("crowd: reseat removed from the build loop")

anchor = "\t\tSanctuaryPass();"
if anchor not in s:
    raise SystemExit("MISS SanctuaryPass")
block = '''		// Every body inside the plaza stands on the deck the market was authored on.
		// This runs after the world is built, so it sees all of them.
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
s = s.replace(anchor, block + anchor, 1)
print("crowd: reseat moved to after world build")

open(p, "w", encoding="utf-8", newline="").write(s)
print("done")
