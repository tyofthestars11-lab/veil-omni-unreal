"""The people render blue because their people has no skin; dress them in real cloth."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilNpc.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

anchor = "\t\tif (Race.Skin)"
if anchor not in s:
    raise SystemExit("MISS skin block")

if "VEIL NPC SKIN" in s:
    print("already dressed")
else:
    i = s.find(anchor)
    depth = 0
    started = False
    end = -1
    for j in range(i, len(s)):
        if s[j] == "{":
            depth += 1
            started = True
        elif s[j] == "}":
            depth -= 1
            if started and depth == 0:
                end = j + 1
                break
    if end < 0:
        raise SystemExit("MISS skin block end")
    add = '''

		// The people are dressed in the same real cloth the market is: no body
		// enters the world as a bare blue mannequin.
		{
			static int32 Dressed = 0;
			if (UMaterialInterface* Cloth = LoadObject<UMaterialInterface>(nullptr,
				TEXT("/Game/VEIL/CC0/Materials/M_CC0_Cloth.M_CC0_Cloth")))
			{
				const int32 Slots = Body->GetNumMaterials();
				for (int32 Slot = 0; Slot < Slots; ++Slot)
				{
					Body->SetMaterial(Slot, Cloth);
				}
				if (Dressed == 0)
				{
					UE_LOG(LogTemp, Display, TEXT("VEIL NPC SKIN: cloth applied to the people"));
				}
				++Dressed;
			}
		}'''
    s = s[:end] + add + s[end:]
    open(p, "w", encoding="utf-8", newline="").write(s)
    print("npc: cloth applied to every body")
