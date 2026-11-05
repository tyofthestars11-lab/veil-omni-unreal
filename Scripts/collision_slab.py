"""The kit meshes carry no collision body; the plaza needs ground you can stand on."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

anchor = "\t{\n\t\t// Proof, not assumption: what is actually under the plaza centre."
if anchor not in s:
    raise SystemExit("MISS trace anchor")

slab = '''	// Hidden collision under the paving. PHI_APPLIED allows a slab that is collision
	// and not a visible shape; without it the market is scenery you fall through.
	if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		if (AStaticMeshActor* Floor = GetWorld()->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(),
			FVector(0.0, 0.0, GPlazaZ - PHI_Constants::Reach(0.5)), FRotator::ZeroRotator))
		{
			UStaticMeshComponent* FC = Floor->GetStaticMeshComponent();
			FC->SetMobility(EComponentMobility::Static);
			FC->SetStaticMesh(CubeMesh);
			FC->bUseDefaultCollision = true;
			FC->SetCollisionProfileName(TEXT("BlockAll"));
			FC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Floor->SetActorScale3D(FVector(27.0, 27.0, 2.0));
			Floor->SetActorHiddenInGame(true);
			UE_LOG(LogVEILWorld, Display,
				TEXT("VEIL HUB FLOOR: hidden collision slab under the paving, top %.1f"), GPlazaZ);
		}
	}
	
'''
s = s.replace(anchor, slab + anchor, 1)
open(p, "w", encoding="utf-8", newline="").write(s)
print("hub: hidden collision slab added under the plaza")
