"""Exposure bias was +phi; the Bible says -phi. That sign is the white-out."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old_bias = "\tS.AutoExposureBias = 1.618033988749895f;"
new_bias = ("\t// PHI_APPLIED: exposure stop is -phi. A positive bias blew every frame out.\n"
            "\tS.AutoExposureBias = -1.618033988749895f;")
if old_bias in s:
    s = s.replace(old_bias, new_bias, 1)
    print("exposure: bias corrected to -phi")
else:
    raise SystemExit("MISS exposure bias")

# proof that the reseat actually seated them, measured after the fact
anchor = '\t\t\t\tTEXT("VEIL HUB CROWD: %d bodies stood on the authored deck at %.1f"), OnDeck, GPlazaZ);\n'
if anchor not in s:
    raise SystemExit("MISS crowd log")
proof = anchor + '''			int32 OnFloor = 0;
			int32 OffDeck = 0;
			for (TActorIterator<AVeilNpc> It(GetWorld()); It; ++It)
			{
				const FVector Loc = It->GetActorLocation();
				if (FMath::Abs(Loc.X) <= Half && FMath::Abs(Loc.Y) <= Half)
				{
					if (FMath::Abs(Loc.Z - GPlazaZ) <= 5.0)
					{
						++OnFloor;
					}
					else
					{
						++OffDeck;
					}
				}
			}
			UE_LOG(LogVEILWorld, Display,
				TEXT("VEIL HUB CROWD CHECK: %d on the deck, %d still off it"), OnFloor, OffDeck);
'''
s = s.replace(anchor, proof, 1)
print("crowd: post-reseat measurement added")

open(p, "w", encoding="utf-8", newline="").write(s)
print("done")
