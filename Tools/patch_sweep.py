"""Sweep the district for blue placeholder boxes and rebuild them as real art.

The timer-based audit never printed, so the measurement (and the repair) is
synchronous now: the moment the content pass finishes, every StaticMeshActor whose
material is M_PHI_DeepBlue is examined. Engine Cubes are rebuilt on the Sicka ancient
kit's SM_Floor1_Big - the modular art this project already ships - and re-dressed in
cut stone with a gold trim slot. Blue NON-boxes (crystal cones) are counted and left
alone, because that blue is the vision, not a placeholder.
"""

import io

PATH = "Source/VEIL/Private/Game/VEILGameMode.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

start_marker = "\t\t\tvoid();"
end_marker = '\t\t\tUE_LOG(LogTemp, Display, TEXT("PHI CONTENT OWNER'
i = src.find(start_marker.replace("\n", NL))
j = src.find(end_marker.replace("\n", NL))
if i < 0 or j < 0 or j < i:
    print("ABORTED - block not found (start=%d end=%d)" % (i, j))
    raise SystemExit(1)

SWEEP = """			// NO BLUE BOX SURVIVES THE BOOT. The district's placeholder blocks are
			// engine Cubes wearing M_PHI_DeepBlue - in the saved map as well as in the
			// composition, which is why changing materials in the builders alone left
			// them in frame. Each one is rebuilt on the Sicka ancient kit's floor mesh
			// (real modular art this project ships) and re-dressed in cut stone with a
			// gold trim slot. Blue cones keep their blue: that is the vision's crystal,
			// not a placeholder.
			{
				UStaticMesh* KitFloor = LoadObject<UStaticMesh>(nullptr,
					TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor1_Big.SM_Floor1_Big"));
				UMaterialInterface* StoneMat = LoadObject<UMaterialInterface>(nullptr,
					TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
				UMaterialInterface* GoldMat = LoadObject<UMaterialInterface>(nullptr,
					TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
				int32 Rebuilt = 0, Resurfaced = 0, KeptBlue = 0;
				for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
				{
					UStaticMeshComponent* Comp = It->GetStaticMeshComponent();
					if (!Comp || !Comp->GetStaticMesh()) { continue; }
					bool bBlue = false;
					for (int32 Slot = 0; Slot < Comp->GetNumMaterials(); ++Slot)
					{
						const UMaterialInterface* Mat = Comp->GetMaterial(Slot);
						if (Mat && Mat->GetName().Contains(TEXT("DeepBlue"))) { bBlue = true; break; }
					}
					if (!bBlue) { continue; }
					if (!Comp->GetStaticMesh()->GetName().Contains(TEXT("Cube"))) { ++KeptBlue; continue; }
					if (KitFloor) { Comp->SetStaticMesh(KitFloor); ++Rebuilt; }
					if (StoneMat) { Comp->SetMaterial(0, StoneMat); }
					if (GoldMat && Comp->GetNumMaterials() > 1) { Comp->SetMaterial(1, GoldMat); }
					++Resurfaced;
				}
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI BLUE SWEEP: %d blue box(es) rebuilt as kit art, %d re-dressed stone+gold, %d blue non-box (vision) kept."),
					Rebuilt, Resurfaced, KeptBlue);
			}
"""

out = src[:i] + SWEEP.replace("\n", NL) + src[j:]
io.open(PATH, "w", encoding="utf-8", newline="").write(out)
print("WROTE %s: %d -> %d chars (synchronous sweep)" % (PATH, len(src), len(out)))
