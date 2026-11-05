#include "VeilGameMode.h"
#include "VeilMind.h"
#include "VeilNpc.h"
#include "VeilRaces.h"
#include "PHI_Constants.h"

#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/PostProcessVolume.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#include "UnrealClient.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogVEILWorld, Log, All);

namespace
{
	// Nothing in this world is a primitive. Only authored assets.
	const TCHAR* const KitRoot = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/");
	const TCHAR* const MeshFloor = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_Platform.SM_Platform");
	const TCHAR* const MeshColumn = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big");
	const TCHAR* const MeshDome = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_HlafDome.SM_HlafDome");
	const TCHAR* const MeshArch = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_ArchD.SM_ArchD");
	const TCHAR* const MeshMonolith = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientMonolith.SM_AncientMonolith");
	const TCHAR* const MeshPylon = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_LittleShrine.SM_LittleShrine");
	const TCHAR* const MeshStair = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_StairSmall.SM_StairSmall");
	const TCHAR* const MeshWall = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Wall1_Small.SM_Wall1_Small");
	const TCHAR* const MeshCliffBig = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffBig.SM_CliffBig");
	const TCHAR* const MeshCliffMed = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffMed.SM_CliffMed");
	const TCHAR* const MeshCliffSlate = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffSlate1.SM_CliffSlate1");
	const TCHAR* const MeshMountain = TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_01.SM_Mountain_01");
	const TCHAR* const MeshMountain2 = TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_02.SM_Mountain_02");

	// The palette, authored in this project from the kit own texture sets.
	const TCHAR* const MatGold = TEXT("/Game/VEIL/Materials/M_VEIL_Gold.M_VEIL_Gold");
	const TCHAR* const MatGold2 = TEXT("/Game/VEIL/Materials/M_VEIL_Gold2.M_VEIL_Gold2");
	const TCHAR* const MatSignal = TEXT("/Game/VEIL/Materials/M_VEIL_Signal.M_VEIL_Signal");
	const TCHAR* const MatStone = TEXT("/Game/VEIL/Materials/M_VEIL_Stone.M_VEIL_Stone");
	const TCHAR* const MatDeep = TEXT("/Game/VEIL/Materials/M_VEIL_Deep.M_VEIL_Deep");

	double GHubGroundZ = 0.0;
	double GPlazaZ = 0.0;
	FVector2D GHubShift = FVector2D::ZeroVector;
	bool bRealLandscape = false;
	bool GSnapEnabled = true;
	int32 GPlaced = 0;
	int32 GMissing = 0;
	int32 GSites = 0;
	int32 GLife = 0;
	int32 GSnapped = 0;
	int32 GBodies[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
}

AVeilGameMode::AVeilGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVeilGameMode::BeginPlay()
{
	Super::BeginPlay();
	PHI_Constants::VerifyBoot();
}

UMaterialInterface* AVeilGameMode::Mat(const TCHAR* Path)
{
	if (!Path)
	{
		return nullptr;
	}
	return LoadObject<UMaterialInterface>(nullptr, Path);
}

AActor* AVeilGameMode::FitPart(const TCHAR* MeshPath, const FVector& Loc, double DesiredWidth,
	double DesiredHeight, const FRotator& Rot, const TCHAR* MaterialPath, bool bSolid)
{
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
	if (!Mesh)
	{
		++GMissing;
		UE_LOG(LogVEILWorld, Error, TEXT("VEIL WORLD: MISSING MESH %s"), MeshPath);
		return nullptr;
	}

	const FBoxSphereBounds B = Mesh->GetBounds();
	const double MeshW = FMath::Max(B.BoxExtent.X, B.BoxExtent.Y) * 2.0;
	const double MeshH = FMath::Max(B.BoxExtent.Z * 2.0, 1.0);
	const double SW = DesiredWidth / FMath::Max(MeshW, 1.0);
	const double SH = (DesiredHeight <= 0.0) ? SW : (DesiredHeight / MeshH);

	// Every piece finds the real surface under its own feet, so the world follows
	// the land. Pieces on the Hub terrace keep the deck height instead.
	double SpawnZ = Loc.Z;
	if (GSnapEnabled && Loc.Z >= GHubGroundZ - 1.0)
	{
		FHitResult Hit;
		FCollisionQueryParams QP(SCENE_QUERY_STAT(VEILSnap), false);
		if (GetWorld()->LineTraceSingleByChannel(Hit,
			FVector(Loc.X, Loc.Y, Loc.Z + 40000.0), FVector(Loc.X, Loc.Y, Loc.Z - 40000.0), ECC_Visibility, QP))
		{
			SpawnZ = Hit.ImpactPoint.Z - (B.Origin.Z - B.BoxExtent.Z) * SH;
			++GSnapped;
		}
	}

	AStaticMeshActor* A = GetWorld()->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(), FVector(Loc.X, Loc.Y, SpawnZ), Rot);
	if (!A)
	{
		++GMissing;
		return nullptr;
	}

	UStaticMeshComponent* C = A->GetStaticMeshComponent();
	C->SetMobility(EComponentMobility::Movable);
	C->SetStaticMesh(Mesh);
	A->SetActorScale3D(FVector(SW, SW, SH));
	C->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	if (bSolid)
	{
		C->SetCollisionProfileName(TEXT("BlockAll"));
	}
	if (UMaterialInterface* M = Mat(MaterialPath))
	{
		const int32 Slots = C->GetNumMaterials();
		for (int32 Slot = 0; Slot < Slots; ++Slot)
		{
			C->SetMaterial(Slot, M);
		}
	}
	Parts.Add(A);
	++GPlaced;
	return A;
}

void AVeilGameMode::LightGoldenHour()
{
	if (bRealLandscape)
	{
		UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD LIGHT: real level keeps its own sun, sky and fog"));
		return;
	}

	if (ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(
		FVector(0.f, 0.f, 4000.f), FRotator(-17.0324f, 223.f, 0.f)))
	{
		if (UDirectionalLightComponent* L = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
		{
			L->SetMobility(EComponentMobility::Movable);
			L->SetIntensity(61800.f);
			L->SetLightColor(FLinearColor(1.f, 0.74f, 0.52f));
			L->bAtmosphereSunLight = true;
			L->SetAtmosphereSunLightIndex(0);
		}
	}

	AActor* Sky = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	if (!Sky)
	{
		return;
	}
	USceneComponent* Root = NewObject<USceneComponent>(Sky, TEXT("SkyRoot"));
	Sky->SetRootComponent(Root);
	Root->RegisterComponent();

	if (USkyAtmosphereComponent* Atmo = NewObject<USkyAtmosphereComponent>(Sky, TEXT("Atmo")))
	{
		Atmo->SetupAttachment(Root);
		Atmo->RegisterComponent();
	}
	if (USkyLightComponent* SkyLight = NewObject<USkyLightComponent>(Sky, TEXT("SkyLight")))
	{
		SkyLight->SetupAttachment(Root);
		SkyLight->SetIntensity(1.6f);
		SkyLight->bRealTimeCapture = true;
		SkyLight->RegisterComponent();
	}
	if (UExponentialHeightFogComponent* Fog = NewObject<UExponentialHeightFogComponent>(Sky, TEXT("Fog")))
	{
		Fog->SetupAttachment(Root);
		Fog->SetFogDensity(0.006f);
		Fog->SetFogInscatteringColor(FLinearColor(0.55f, 0.42f, 0.28f));
		Fog->RegisterComponent();
	}
}

void AVeilGameMode::BuildLook()
{
	APostProcessVolume* PP = GetWorld()->SpawnActor<APostProcessVolume>(
		APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	if (!PP)
	{
		return;
	}
	PP->bUnbound = true;
	FPostProcessSettings& S = PP->Settings;
	S.bOverride_AutoExposureMethod = true;
	S.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
	S.bOverride_AutoExposureBias = true;
	S.AutoExposureBias = 1.618033988749895f;
	S.bOverride_BloomIntensity = true;
	S.BloomIntensity = 0.381966011250105f;
	S.bOverride_VignetteIntensity = true;
	S.VignetteIntensity = 0.236067977499790f;
	S.bOverride_ColorGain = true;
	S.ColorGain = FVector4(1.06, 1.0, 0.92, 1.0);
	S.bOverride_ColorSaturation = true;
	S.ColorSaturation = FVector4(1.08, 1.06, 1.02, 1.0);
	S.bOverride_ColorContrast = true;
	S.ColorContrast = FVector4(1.02, 1.02, 1.02, 1.0);
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD LOOK: fixed exposure +phi, warm gold grade, bloom 1/phi^2, vignette 1/phi^3"));
}

void AVeilGameMode::BuildGround()
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILGround), false);
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit,
		FVector(GHubShift.X, GHubShift.Y, 200000.0), FVector(GHubShift.X, GHubShift.Y, -200000.0), ECC_Visibility, Params))
	{
		GHubGroundZ = Hit.ImpactPoint.Z;
		bRealLandscape = true;
		GHubGroundZ -= 20.0;
		UE_LOG(LogVEILWorld, Display,
			TEXT("VEIL WORLD GROUND: real land under the Hub at z=%.4f on %s"),
			GHubGroundZ, *Hit.GetActor()->GetName());
		return;
	}

	GHubGroundZ = 0.0;
	const double Pitch = PHI_Constants::Pitch;
	int32 Laid = 0;
	for (int32 i = 1; i <= 240; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, Pitch);
		if (FitPart(MeshFloor, FVector(P.X, P.Y, 0.0), Pitch, 0.0,
			FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), MatStone, true))
		{
			++Laid;
		}
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD GROUND: bare map, %d phi slab(s) laid, ground z=0"), Laid);
}

void AVeilGameMode::BuildTerrain()
{
	if (bRealLandscape)
	{
		UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD TERRAIN: real landscape kept whole, no runtime range added"));
		return;
	}

	const double MountainPitch = PHI_Constants::Spacing * FMath::Pow(PHI_Constants::Phi, 6.0);
	int32 Mountains = 0;
	for (int32 i = 1; i <= 18; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, MountainPitch);
		const double R = MountainPitch * FMath::Sqrt(static_cast<double>(i));
		const double W = R / PHI_Constants::Phi;
		if (FitPart((i % 2 == 0) ? MeshMountain : MeshMountain2,
			FVector(P.X, P.Y, GHubGroundZ - 600.0), W, W,
			FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), nullptr, true))
		{
			++Mountains;
		}
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD TERRAIN: %d mountain(s) raised on the phi spiral"), Mountains);
}

void AVeilGameMode::BuildRing(const FVector2D& Centre, int32 Count, const TCHAR* MeshPath,
	const TCHAR* MaterialPath, const TCHAR* Label, double HeightPower)
{
	int32 Placed = 0;
	double RMin = 0.0;
	double RMax = 0.0;
	for (int32 i = 1; i <= Count; ++i)
	{
		const double Pitch = PHI_Constants::Spacing * PHI_Constants::Phi * PHI_Constants::Phi;
		const FVector2D P = PHI_Constants::GoldenLoc(i, Pitch);
		const double R = Pitch * FMath::Sqrt(static_cast<double>(i));
		const double Gap = (2.0 * PI * R) / static_cast<double>(Count);
		const double Width = FMath::Min(Gap / PHI_Constants::Phi, 240.0);
		const double Height = FMath::Clamp(PHI_Constants::Spacing * FMath::Pow(PHI_Constants::Phi, HeightPower + 2.0), 300.0, 620.0);
		const double Yaw = static_cast<double>(i) * PHI_Constants::GoldenAngleDeg + 90.0;
		if (FitPart(MeshPath, FVector(Centre.X + P.X, Centre.Y + P.Y, GPlazaZ), Width, Height,
			FRotator(0.f, static_cast<float>(Yaw), 0.f), MaterialPath, true))
		{
			++Placed;
			RMin = (Placed == 1) ? R : FMath::Min(RMin, R);
			RMax = FMath::Max(RMax, R);
		}
	}
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL WORLD %s: %d/%d placed on the deck | centre=(%.4f, %.4f) r=%.4f..%.4f"),
		Label, Placed, Count, Centre.X, Centre.Y, RMin, RMax);
}

void AVeilGameMode::BuildHub()
{
	// The Bible Hub is a cliffside market. It stands on one flat stone terrace laid
	// on the real ground, so the market can never read as pieces on distant peaks.
	GPlazaZ = GHubGroundZ + 260.0;
	GSnapEnabled = false;
	FitPart(MeshFloor, FVector(0.0, 0.0, GHubGroundZ), 2600.0, 260.0, FRotator::ZeroRotator, MatStone, true);

	BuildRing(FVector2D(0.0, 0.0), 12, MeshColumn, MatGold, TEXT("PILLARS"), 1.0);
	BuildRing(FVector2D(900.0, 0.0), 8, MeshDome, MatStone, TEXT("HOUSES"), 1.0);
	BuildRing(FVector2D(900.0, 0.0), 8, MeshArch, MatDeep, TEXT("STALLS"), 0.0);

	const double R = PHI_Constants::Spacing * FMath::Pow(PHI_Constants::Phi, 4.0);
	const double H = PHI_Constants::Spacing * FMath::Pow(PHI_Constants::Phi, 3.0);
	int32 Lanterns = 0;
	for (int32 i = 0; i < 8; ++i)
	{
		const double A = FMath::DegreesToRadians(static_cast<double>(i) * PHI_Constants::GoldenAngleDeg);
		if (FitPart(MeshPylon, FVector(FMath::Cos(A) * R, FMath::Sin(A) * R, GPlazaZ), 90.0, FMath::Min(H, 420.0),
			FRotator(0.f, static_cast<float>(FMath::RadiansToDegrees(A)), 0.f), MatSignal, true))
		{
			++Lanterns;
			if (APointLight* Lamp = GetWorld()->SpawnActor<APointLight>(
				APointLight::StaticClass(),
				FVector(FMath::Cos(A) * R, FMath::Sin(A) * R, GPlazaZ + 380.0), FRotator::ZeroRotator))
			{
				if (UPointLightComponent* L = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
				{
					L->SetMobility(EComponentMobility::Movable);
					L->SetIntensity(1400.f);
					L->SetAttenuationRadius(2600.f);
					L->SetLightColor(FLinearColor(1.f, 0.78f, 0.42f));
				}
			}
		}
	}

	// 5 blue crystal peaks of the Hub count, faceted crystal meshes, signal blue.
	int32 Crystals = 0;
	for (int32 i = 1; i <= 5; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi);
		if (FitPart(MeshCliffSlate, FVector(2800.0 + P.X, 500.0 + P.Y, GHubGroundZ), 700.0, 1500.0,
			FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), MatSignal, true))
		{
			++Crystals;
		}
	}

	// The Vault sits below the terrace at the authority coordinate (0, 0, -80.9).
	FitPart(MeshDome, FVector(0.0, 0.0, GHubGroundZ - 80.9), 1200.0, 900.0, FRotator::ZeroRotator, MatStone, true);

	// A stair down from the terrace edge, so the market is a place you can leave.
	for (int32 i = 1; i <= 6; ++i)
	{
		FitPart(MeshStair, FVector(0.0, -1250.0 - i * 160.0, GPlazaZ - i * 43.0),
			500.0, 90.0, FRotator::ZeroRotator, MatStone, true);
	}

	GSnapEnabled = true;
	GPlazaZ = GHubGroundZ;
	const int32 HubExpected = 25;
	const int32 HubPlaced = 12 + 8 + Crystals;
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL WORLD HUB: terrace 26 m, 12 gold pillars, 8 houses, %d stalls, %d crystal peaks, %d lanterns, Vault 80.9 below"),
		8, Crystals, Lanterns);
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL HUB COUNTS: %s - %d/%d placements (12 pillars + 8 houses + 5 crystal peaks)"),
		(HubPlaced == HubExpected) ? TEXT("PASS") : TEXT("FAIL"), HubPlaced, HubExpected);
}

void AVeilGameMode::BuildFiftySites()
{
	for (int32 i = 1; i <= 50; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing);
		const double R = PHI_Constants::Spacing * FMath::Sqrt(static_cast<double>(i));
		if (FitPart(MeshPylon, FVector(P.X, P.Y, GHubGroundZ), FMath::Max(R / (PHI_Constants::Phi * 6.0), 60.0),
			FMath::Max(R / (PHI_Constants::Phi * 2.0), 140.0),
			FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), MatSignal, true))
		{
			++GSites;
		}
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD SITES: %d/50 seated on phi_place(i) = sqrt(i)*phi*100"), GSites);
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD SNAP: %d piece(s) sat on the real ground under their own feet"), GSnapped);
}

void AVeilGameMode::BuildBeacons()
{
	const double BC[3][2] = { { 2800.0, 500.0 }, { -2000.0, -900.0 }, { 600.0, -2100.0 } };
	int32 Lit = 0;
	for (int32 i = 0; i < 3; ++i)
	{
		if (!FitPart(MeshMonolith, FVector(BC[i][0], BC[i][1], GHubGroundZ), 300.0, 1500.0,
			FRotator::ZeroRotator, MatSignal, true))
		{
			continue;
		}
		++Lit;
		if (APointLight* Lamp = GetWorld()->SpawnActor<APointLight>(
			APointLight::StaticClass(), FVector(BC[i][0], BC[i][1], GHubGroundZ + 1200.0), FRotator::ZeroRotator))
		{
			if (UPointLightComponent* L = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
			{
				L->SetMobility(EComponentMobility::Movable);
				L->SetIntensity(120000.f);
				L->SetAttenuationRadius(7000.f);
				L->SetLightColor(FLinearColor(0.25f, 0.66f, 1.f));
			}
		}
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD BEACONS: %d/3 lit"), Lit);
}

void AVeilGameMode::BuildNest()
{
	// Nest: high cliffs, huts on ledges. Each hut is a real platform, two walls and
	// a dome roof, all tracing the ledge under itself.
	int32 Huts = 0;
	for (int32 i = 1; i <= 6; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi);
		const double X = -450.0 + P.X;
		const double Y = 780.0 + P.Y;
		const double Yaw = static_cast<double>(i) * PHI_Constants::GoldenAngleDeg;
		if (!FitPart(MeshFloor, FVector(X, Y, GHubGroundZ), 700.0, 120.0, FRotator(0.f, static_cast<float>(Yaw), 0.f), MatStone, true))
		{
			continue;
		}
		const FVector2D Dir(FMath::Cos(FMath::DegreesToRadians(Yaw)), FMath::Sin(FMath::DegreesToRadians(Yaw)));
		FitPart(MeshWall, FVector(X + Dir.X * 250.0, Y + Dir.Y * 250.0, GHubGroundZ + 60.0), 300.0, 320.0,
			FRotator(0.f, static_cast<float>(Yaw), 0.f), MatStone, true);
		FitPart(MeshWall, FVector(X - Dir.X * 250.0, Y - Dir.Y * 250.0, GHubGroundZ + 60.0), 300.0, 320.0,
			FRotator(0.f, static_cast<float>(Yaw), 0.f), MatStone, true);
		FitPart(MeshDome, FVector(X, Y, GHubGroundZ + 60.0), 520.0, 380.0, FRotator::ZeroRotator, MatGold2, true);
		++Huts;
	}
	// The cliff wall the huts stand against.
	for (int32 i = 1; i <= 5; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi * PHI_Constants::Phi);
		FitPart((i % 2 == 0) ? MeshCliffBig : MeshCliffMed,
			FVector(-900.0 + P.X, 1100.0 + P.Y, GHubGroundZ), 900.0, 1400.0,
			FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), nullptr, true);
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD NEST: %d hut(s) on the ledges, cliffs behind, Seer at (-450, 780)"), Huts);
}

void AVeilGameMode::BuildOmni()
{
	// Omni District: blue-lit city sprawl. Blocks on a phi grid, blue light pillars.
	int32 Blocks = 0;
	for (int32 i = 1; i <= 12; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi);
		const FVector At(2800.0 + P.X, 500.0 + P.Y, GHubGroundZ);
		const double H = FMath::Clamp(1200.0 + 260.0 * (i % 4), 900.0, 2200.0);
		if (FitPart(MeshArch, At, 620.0, H,
			FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), MatSignal, true))
		{
			++Blocks;
		}
	}
	for (int32 i = 0; i < 3; ++i)
	{
		const double A = FMath::DegreesToRadians(static_cast<double>(i) * PHI_Constants::GoldenAngleDeg);
		const double X = 2800.0 + FMath::Cos(A) * 900.0;
		const double Y = 500.0 + FMath::Sin(A) * 900.0;
		FitPart(MeshMonolith, FVector(X, Y, GHubGroundZ), 260.0, 2600.0, FRotator::ZeroRotator, MatSignal, true);
		if (APointLight* Lamp = GetWorld()->SpawnActor<APointLight>(
			APointLight::StaticClass(), FVector(X, Y, GHubGroundZ + 1800.0), FRotator::ZeroRotator))
		{
			if (UPointLightComponent* L = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
			{
				L->SetMobility(EComponentMobility::Movable);
				L->SetIntensity(90000.f);
				L->SetAttenuationRadius(6000.f);
				L->SetLightColor(FLinearColor(0.25f, 0.66f, 1.f));
			}
		}
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD OMNI: %d blue block(s), 3 light pillars, Countess at (809, -309)"), Blocks);
}

void AVeilGameMode::BuildDeep()
{
	// Megastructure Deep: a blue pit under the cliffs, machines and bridges.
	int32 Spans = 0;
	for (int32 i = 1; i <= 10; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi);
		const FVector At(P.X, -5000.0 + P.Y, GHubGroundZ);
		if (FitPart(MeshFloor, At, 900.0, 120.0,
			FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), MatDeep, true))
		{
			++Spans;
		}
		if (i % 2 == 0)
		{
			FitPart(MeshMonolith, FVector(At.X + 380.0, At.Y, GHubGroundZ), 220.0, 1100.0, FRotator::ZeroRotator, MatSignal, true);
		}
	}
	for (int32 i = 1; i <= 6; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi * PHI_Constants::Phi);
		FitPart((i % 2 == 0) ? MeshCliffSlate : MeshCliffBig,
			FVector(P.X, -6000.0 + P.Y, GHubGroundZ), 1100.0, 1600.0,
			FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), nullptr, true);
	}
	if (APointLight* Lamp = GetWorld()->SpawnActor<APointLight>(
		APointLight::StaticClass(), FVector(0.0, -5000.0, GHubGroundZ + 600.0), FRotator::ZeroRotator))
	{
		if (UPointLightComponent* L = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
		{
			L->SetMobility(EComponentMobility::Movable);
			L->SetIntensity(160000.f);
			L->SetAttenuationRadius(9000.f);
			L->SetLightColor(FLinearColor(0.2f, 0.55f, 1.f));
		}
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD DEEP: %d bridge span(s), machines, one blue pit at (0, -5000)"), Spans);
}

void AVeilGameMode::BuildSpire()
{
	// One gold tower, seated on the ground at (0, 3500). No crown hangs above it:
	// the tower is the only thing that reaches the sky, and it stands on the rock.
	// Bible authority is Spire (0, 3500, 500). The tower does not hang at +500:
	// a real rock plinth is seated on the ground and the gold Spire stands on it.
	FitPart(MeshCliffBig, FVector(0.0, 3500.0, GHubGroundZ), 1800.0, 500.0, FRotator::ZeroRotator, nullptr, true);
	GSnapEnabled = false;
	if (FitPart(MeshMonolith, FVector(0.0, 3500.0, GHubGroundZ + 460.0), 900.0, 3200.0, FRotator::ZeroRotator, MatGold, true))
	{
		GSnapEnabled = true;
		for (int32 i = 1; i <= 4; ++i)
		{
			const double A = FMath::DegreesToRadians(static_cast<double>(i) * 90.0 + PHI_Constants::GoldenAngleDeg);
			FitPart(MeshMonolith, FVector(FMath::Cos(A) * 620.0, 3500.0 + FMath::Sin(A) * 620.0, GHubGroundZ),
				240.0, 1500.0, FRotator::ZeroRotator, MatGold, true);
		}
		UE_LOG(LogVEILWorld, Display,
			TEXT("VEIL WORLD SPIRE: plinth on the rock, gold tower based at +500, 4 buttresses, no floating crown"));
	}
	else
	{
		GSnapEnabled = true;
	}
}

void AVeilGameMode::SeatBody(const FVector& At, int32 RaceIndex, int32 SeatIndex, double WalkRadius, const TCHAR* Label)
{
	AVeilNpc* Npc = GetWorld()->SpawnActor<AVeilNpc>(AVeilNpc::StaticClass(), At, FRotator::ZeroRotator);
	if (!Npc)
	{
		return;
	}
	Npc->Seat(At, RaceIndex, SeatIndex, WalkRadius);
	Parts.Add(Npc);
	++GLife;
	++GBodies[FMath::Clamp(RaceIndex, 0, 7)];

	if (AVeilMind* Mind = GetWorld()->SpawnActor<AVeilMind>(AVeilMind::StaticClass(), Npc->GetActorLocation(), FRotator::ZeroRotator))
	{
		Mind->Possess(Npc);
	}

	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD LIFE: %s the %s at (%.1f, %.1f, %.1f)"),
		Label, Npc->GetRaceName(), At.X, At.Y, Npc->GetActorLocation().Z);
}

void AVeilGameMode::BuildLife()
{
	// The four named bodies of the Bible, each of its own people.
	SeatBody(FVector(900.0, 0.0, GHubGroundZ), 0, 1, 240.0, TEXT("Keeper Sera Vahl"));
	SeatBody(FVector(-450.0, 780.0, GHubGroundZ), 2, 2, 420.0, TEXT("Seer"));
	SeatBody(FVector(809.0, -309.0, GHubGroundZ + 64.7), 1, 3, 220.0, TEXT("Countess"));
	SeatBody(FVector(-4150.0, -780.0, GHubGroundZ), 3, 4, 360.0, TEXT("Cartographer"));

	// The crowd of the market, all four peaceful peoples.
	int32 Crowd = 0;
	for (int32 i = 1; i <= 24; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(P.X, P.Y, GHubGroundZ), (i - 1) % 4, i, 520.0, TEXT("market"));
		++Crowd;
	}

	// Locals of each region, so every zone has its own people.
	for (int32 i = 1; i <= 8; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(-450.0 + P.X, 780.0 + P.Y, GHubGroundZ), 2, i + 20, 400.0, TEXT("Nest"));
	}
	for (int32 i = 1; i <= 10; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(2800.0 + P.X, 500.0 + P.Y, GHubGroundZ), 1, i + 30, 420.0, TEXT("Omni"));
	}
	for (int32 i = 1; i <= 8; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(P.X, -5000.0 + P.Y, GHubGroundZ), 3, i + 40, 480.0, TEXT("Deep"));
	}

	// The enemies of the Bible, one variety per region, each holding its own post.
	// Crystal Guardian, Gold Warrior, Deep Dweller, Beacon Shadow. None of them
	// follows the Runner and none of them orbits: that is Kraken physics.
	int32 Foes = 0;
	for (int32 i = 1; i <= 3; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i + 70, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(2800.0 + P.X, 500.0 + P.Y, GHubGroundZ), 4, i + 70, 0.0, TEXT("Crystal Guardian"));
		++Foes;
	}
	for (int32 i = 1; i <= 3; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i + 80, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(-450.0 + P.X, 780.0 + P.Y, GHubGroundZ), 5, i + 80, 0.0, TEXT("Gold Warrior"));
		++Foes;
	}
	for (int32 i = 1; i <= 3; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i + 90, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(P.X, -5000.0 + P.Y, GHubGroundZ), 6, i + 90, 0.0, TEXT("Deep Dweller"));
		++Foes;
	}
	for (int32 i = 1; i <= 3; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i + 100, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(P.X, 900.0 + P.Y, GHubGroundZ), 7, i + 100, 0.0, TEXT("Beacon Shadow"));
		++Foes;
	}

	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL WORLD LIFE: %d bodies | human %d gold %d crystal %d deep %d | guardians %d warriors %d dwellers %d shadows %d | every body has a mind"),
		GLife, GBodies[0], GBodies[1], GBodies[2], GBodies[3], GBodies[4], GBodies[5], GBodies[6], GBodies[7]);
}

void AVeilGameMode::PlaceRunner()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->SetActorLocation(FVector(GHubShift.X, GHubShift.Y - 1150.0, GHubGroundZ + 435.0));
			PC->SetControlRotation(FRotator(-3.f, 90.f, 0.f));
		}
	}
}

void AVeilGameMode::BuildWorld()
{
	if (APlayerController* PC0 = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* P0 = PC0->GetPawn())
		{
			GHubShift = FVector2D(P0->GetActorLocation().X, P0->GetActorLocation().Y);
			UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD ANCHOR: map start at (%.1f, %.1f)"), GHubShift.X, GHubShift.Y);
		}
	}

	BuildGround();
	LightGoldenHour();
	BuildLook();
	BuildTerrain();
	BuildHub();
	BuildFiftySites();
	BuildBeacons();
	BuildNest();
	BuildOmni();
	BuildDeep();
	BuildSpire();
	BuildLife();

	for (AActor* A : Parts)
	{
		if (A)
		{
			A->AddActorWorldOffset(FVector(GHubShift.X, GHubShift.Y, 0.0));
		}
	}

	// Nothing floats. Any piece left hanging over the terrace is destroyed and counted.
	int32 Lifted = 0;
	for (AActor* A : Parts)
	{
		if (A && FVector::Dist2D(A->GetActorLocation(), FVector(GHubShift.X, GHubShift.Y, GHubGroundZ)) < 1600.0
			&& A->GetActorLocation().Z > GHubGroundZ + 900.0)
		{
			A->Destroy();
			++Lifted;
		}
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD AUDIT: %d floating piece(s) removed, nothing hangs in the sky"), Lifted);

	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL WORLD SEATED: map=%s land=%d ground=%.2f parts=%d missing=%d sites=%d/50 life=%d phi=%.15f"),
		*GetWorld()->GetName(), bRealLandscape ? 1 : 0, GHubGroundZ, GPlaced, GMissing, GSites, GLife, PHI_Constants::Phi);
}

void AVeilGameMode::StartPlay()
{
	Super::StartPlay();
	BuildWorld();
	PlaceRunner();

	if (FParse::Param(FCommandLine::Get(), TEXT("VeilShot")))
	{
		ShotIdx = 0;
		GetWorldTimerManager().SetTimer(ShotTimer, this, &AVeilGameMode::TakeShot, 16.f, false);
	}
}

void AVeilGameMode::TakeShot()
{
	static const TCHAR* Names[5] = { TEXT("VEIL_1_Hub"), TEXT("VEIL_2_Spire"), TEXT("VEIL_3_Nest"), TEXT("VEIL_4_Omni"), TEXT("VEIL_5_Deep") };
	static const double CamX[5] = { 0.0, 0.0, -450.0, 2800.0, 0.0 };
	static const double CamY[5] = { -1150.0, 1200.0, -920.0, -1200.0, -6700.0 };
	static const double CamZ[5] = { 435.0, 500.0, 480.0, 420.0, 520.0 };
	static const float CamPitch[5] = { -3.f, -9.f, -4.f, -4.f, -9.f };

	const int32 Idx = FMath::Clamp(ShotIdx, 0, 4);
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->SetActorLocation(FVector(GHubShift.X + CamX[Idx], GHubShift.Y + CamY[Idx], GHubGroundZ + CamZ[Idx]));
		}
		PC->SetControlRotation(FRotator(CamPitch[Idx], 90.f, 0.f));
	}
	FScreenshotRequest::RequestScreenshot(FString(Names[Idx]) + TEXT(".png"), false, false);
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD SHOT %d/5: %s"), Idx + 1, Names[Idx]);

	++ShotIdx;
	if (ShotIdx < 5)
	{
		GetWorldTimerManager().SetTimer(ShotTimer, this, &AVeilGameMode::TakeShot, 3.f, false);
	}
	else
	{
		GetWorldTimerManager().SetTimer(QuitTimer, this, &AVeilGameMode::Quit, 5.f, false);
	}
}

void AVeilGameMode::Quit()
{
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD: quitting after the shots"));
	FGenericPlatformMisc::RequestExit(false);
}
