"""The district gets a floor, and the runner stands on it.

Measured three runs: the runner floats at z=598 because nothing collidable exists at the
plaza's own height - the only surfaces under him are composition shelves at 500 and the
water sheet. A vision of a market on the ground cannot be walked on a shelf.

This lays nine kit floor tiles (Sicka SM_Floor1_Big, 80 m each, 3x3) with the top face at
z=0 and blocking collision, then places the runner on his authority coordinate V(0,0,100)
- 98 cm of stand-off over that floor. It runs once, in BeginPlay, before the unstick ever
looks for ground, so the unstick finds real ground instead of refusing every roof.
"""

import io

P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = "\t\tif (UWorld* SweepWorld = GetWorld())"
GROUND = """		// THE PLAZA HAS A FLOOR. Nine kit tiles, top face at z=0, blocking collision -
		// the market the vision shows is a market you can stand on, and every "floating"
		// frame was the runner being held five metres above it by a shelf.
		if (UWorld* GroundWorld = GetWorld())
		{
			UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(nullptr,
				TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor1_Big.SM_Floor1_Big"));
			UMaterialInterface* FloorStone = LoadObject<UMaterialInterface>(nullptr,
				TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
			int32 Tiles = 0;
			if (FloorMesh)
			{
				const FVector Extent = FloorMesh->GetBounds().BoxExtent;
				const float Half = FMath::Max(1.f, Extent.GetMax());
				const float Scale = 4000.f / Half;                 // 80 m across per tile
				const float Step = 8000.f;
				for (int32 GX = -1; GX <= 1; ++GX)
				{
					for (int32 GY = -1; GY <= 1; ++GY)
					{
						const FVector At(GX * Step, GY * Step, -Extent.Z * Scale);
						FActorSpawnParameters Params;
						Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						if (AStaticMeshActor* Tile = GroundWorld->SpawnActor<AStaticMeshActor>(
							AStaticMeshActor::StaticClass(), At, FRotator::ZeroRotator, Params))
						{
							if (UStaticMeshComponent* Comp = Tile->GetStaticMeshComponent())
							{
								Comp->SetMobility(EComponentMobility::Static);
								Comp->SetStaticMesh(FloorMesh);
								Tile->SetActorScale3D(FVector(Scale));
								Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
								Comp->SetCollisionProfileName(TEXT("BlockAll"));
								if (FloorStone) { Comp->SetMaterial(0, FloorStone); }
								++Tiles;
							}
						}
					}
				}
			}
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI GROUND: %d kit tile(s) laid, top face at z=0, collision blocking."), Tiles);
			// ON THE FLOOR, AT HIS OWN COORDINATE. RUNNER is 0,0,100: 98 cm of stand-off
			// over the new ground, which is where the vision's market is.
			SetActorLocation(FVector(0.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI AUTHORITY SEAT: runner stands on V(0,0,100) over real ground."));
		}
		if (UWorld* SweepWorld = GetWorld())"""

o, n = ANCHOR.replace("\n", NL), GROUND.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - anchor count=%d" % src.count(o))
    raise SystemExit(1)

text = src.replace(o, n, 1)
if '#include "Engine/StaticMeshActor.h"' not in text:
    text = text.replace('#include "EngineUtils.h"',
                        '#include "EngineUtils.h"' + NL + '#include "Engine/StaticMeshActor.h"', 1)
io.open(P, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (real floor + authority seat)" % (P, len(src), len(text)))
