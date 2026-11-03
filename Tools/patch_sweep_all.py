"""The sweep was blind by construction: it iterated AStaticMeshActor.

The district's slabs are static mesh components hanging on the composition's and the
fab dresser's own actor classes, so `TActorIterator<AStaticMeshActor>` saw none of
them and three sweeps printed nothing while the frames stayed blue. This walks every
actor and every UStaticMeshComponent on it.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

start_m = "\t\tauto SweepBlue = [this]()"
end_m = "\t\tSweepBlue();"
i, j = src.find(start_m.replace("\n", NL)), src.find(end_m.replace("\n", NL))
if i < 0 or j < 0 or j < i:
    print("ABORTED - span not found (%d, %d)" % (i, j))
    raise SystemExit(1)

NEW = """		auto SweepBlue = [this]()
		{
			UWorld* SweepWorld = GetWorld();
			if (!SweepWorld) { return; }
			UStaticMesh* KitFloor = LoadObject<UStaticMesh>(nullptr,
				TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor1_Big.SM_Floor1_Big"));
			UMaterialInterface* Stone = LoadObject<UMaterialInterface>(nullptr,
				TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
			int32 Touched = 0, Rebuilt = 0, KeptCrystal = 0;
			// EVERY ACTOR, NOT JUST AStaticMeshActor. The composition and the fab dresser
			// spawn their own actor classes and hang static mesh components on them, so the
			// earlier sweeps found nothing at all - which is why the frames stayed blue.
			for (TActorIterator<AActor> It(SweepWorld); It; ++It)
			{
				TInlineComponentArray<UStaticMeshComponent*> Comps;
				It->GetComponents(Comps);
				for (UStaticMeshComponent* Comp : Comps)
				{
					if (!Comp || !Comp->GetStaticMesh()) { continue; }
					bool bBlue = false;
					for (int32 Slot = 0; Slot < Comp->GetNumMaterials(); ++Slot)
					{
						const UMaterialInterface* Mat = Comp->GetMaterial(Slot);
						if (Mat && Mat->GetName().Contains(TEXT("DeepBlue"))) { bBlue = true; break; }
					}
					if (!bBlue) { continue; }
					const FString MeshName = Comp->GetStaticMesh()->GetName();
					if (MeshName.Contains(TEXT("Cone")) || MeshName.Contains(TEXT("Crystal")))
					{
						++KeptCrystal;
						continue;
					}
					const bool bFlatBox = MeshName.Contains(TEXT("Cube")) || MeshName.Contains(TEXT("Plane"));
					if (bFlatBox && KitFloor) { Comp->SetStaticMesh(KitFloor); ++Rebuilt; }
					if (Stone)
					{
						for (int32 Slot = 0; Slot < Comp->GetNumMaterials(); ++Slot) { Comp->SetMaterial(Slot, Stone); }
					}
					++Touched;
					if (Touched <= 12)
					{
						UE_LOG(LogTemp, Display, TEXT("PHI BLUE FIX: %s mesh=%s -> stone"),
							*It->GetName(), *MeshName);
					}
				}
			}
			UE_LOG(LogTemp, Display,
				TEXT("PHI BLUE FIX TOTAL: %d slab(s) re-dressed, %d rebuilt as kit art, %d crystal(s) kept blue."),
				Touched, Rebuilt, KeptCrystal);
		};
"""

out = src[:i] + NEW.replace("\n", NL) + src[j:]
io.open(PATH, "w", encoding="utf-8", newline="").write(out)
print("WROTE %s: %d -> %d chars (sweep walks every actor)" % (PATH, len(src), len(out)))
