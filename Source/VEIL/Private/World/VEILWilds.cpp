#include "World/VEILWilds.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/VEILEnemy.h"
#include "VEIL.h"

namespace
{
	constexpr float WildsGoldenDeg = 137.50776405f;
	constexpr float WildsPhi = 1.6180339887f;
	/** 1/phi^2: the decide threshold. Below it a camp rests, above it hunts. */
	constexpr float WildsRestDigit = 0.3819660113f;

	const TCHAR* WildsGoldPath = TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic");
	const TCHAR* WildsCrystalPath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");

	// SIX SPECIES, SIX BODIES. Same machine class; what changes is what it is.
	// The rung is the address: phi^rung is the quantity the body actually uses,
	// so a species is a place on the ladder, not a typed constant.
	struct FWildsSpecies
	{
		const TCHAR* Name;
		EVEILEnemyKind Kind;
		float Health;
		float Damage;
		float Speed;
		int32 Pack;
		bool bChampion;
	};

	const FWildsSpecies GSpecies[] =
	{
		{ TEXT("HOLLOW"),       EVEILEnemyKind::Drone,   38.f,   6.f, 460.f, 5, false },
		{ TEXT("ASHWALKER"),    EVEILEnemyKind::Soldier, 70.f,  10.f, 420.f, 3, false },
		{ TEXT("TIDEKIN"),      EVEILEnemyKind::Soldier, 96.f,   9.f, 340.f, 3, false },
		{ TEXT("EMBERBORN"),    EVEILEnemyKind::Soldier, 88.f,  16.f, 380.f, 2, false },
		{ TEXT("STONEBORN"),    EVEILEnemyKind::Drone,  180.f,  12.f, 250.f, 2, false },
		{ TEXT("WARDEN SPAWN"), EVEILEnemyKind::Soldier, 320.f, 22.f, 300.f, 1, true },
	};
	constexpr int32 GSpeciesCount = UE_ARRAY_COUNT(GSpecies);

	/** Rung of a quantity: ln(x) / ln(phi). Logged so the body can be read as a place. */
	float RungOf(float Value)
	{
		return Value > 0.f ? FMath::Loge(Value) / FMath::Loge(WildsPhi) : 0.f;
	}
}

// ---------------------------------------------------------------------------
// THE MINE
// ---------------------------------------------------------------------------
AVEILMineNode::AVEILMineNode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Ore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ore"));
	RootComponent = Ore;
	if (UStaticMesh* Cone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone")))
	{
		Ore->SetStaticMesh(Cone);
	}
	Ore->SetWorldScale3D(FVector(2.2f, 2.2f, 3.4f));
	Ore->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Lamp = CreateDefaultSubobject<UPointLightComponent>(TEXT("Lamp"));
	Lamp->SetupAttachment(RootComponent);
	Lamp->SetLightColor(FLinearColor(0.35f, 0.62f, 1.f));
	Lamp->SetIntensity(3200.f);
	Lamp->SetAttenuationRadius(1600.f);
	Lamp->SetCastShadows(false);
	Lamp->SetAffectGlobalIllumination(false);

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(RootComponent);
	Trigger->InitSphereRadius(340.f);
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);
}

void AVEILMineNode::Init(int32 InIndex, int32 InGold)
{
	Index = InIndex;
	Gold = InGold;
	if (Trigger && !Trigger->OnComponentBeginOverlap.IsAlreadyBound(this, &AVEILMineNode::OnEnter))
	{
		Trigger->OnComponentBeginOverlap.AddDynamic(this, &AVEILMineNode::OnEnter);
	}
	if (UMaterialInterface* Crystal = LoadObject<UMaterialInterface>(nullptr, WildsCrystalPath))
	{
		Ore->SetMaterial(0, Crystal);
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI MINE: node %d at %s worth %d gold (rung %.2f)."),
		Index, *GetActorLocation().ToCompactString(), Gold, RungOf((float)Gold));
}

void AVEILMineNode::OnEnter(UPrimitiveComponent* OverlappedComp, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (bSpent) { return; }
	bSpent = true;
	RespawnAt = GetWorld() ? GetWorld()->GetTimeSeconds() + 120.f : 0.f;

	if (UOMNIQuestSystem* Q = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
	{
		Q->Gold += Gold;
		Q->AwardXP(45, TEXT("an ore seam stripped"));
	}
	SetActorHiddenInGame(true);
	UE_LOG(LogVEIL, Warning, TEXT("PHI MINE: node %d stripped, +%d gold."), Index, Gold);
}

void AVEILMineNode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bSpent || !GetWorld()) { return; }
	if (GetWorld()->GetTimeSeconds() < RespawnAt) { return; }
	bSpent = false;
	SetActorHiddenInGame(false);
	UE_LOG(LogVEIL, Log, TEXT("PHI MINE: node %d has re-formed."), Index);
}

// ---------------------------------------------------------------------------
// THE POPULATION
// ---------------------------------------------------------------------------
AVEILWildsPopulation::AVEILWildsPopulation()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

float AVEILWildsPopulation::IntentOf(int32 Camp) const
{
	return Camps.IsValidIndex(Camp) ? Camps[Camp].Intent : 0.f;
}

void AVEILWildsPopulation::Populate(int32 Camps_, int32 Mines_)
{
	UWorld* W = GetWorld();
	if (!W) { return; }

	auto GroundAt = [W](float X, float Y, float& OutZ) -> bool
	{
		FHitResult Hit;
		if (!W->LineTraceSingleByChannel(Hit, FVector(X, Y, 400000.f), FVector(X, Y, -40000.f),
			ECC_Visibility, FCollisionQueryParams::DefaultQueryParam))
		{
			return false;
		}
		OutZ = Hit.ImpactPoint.Z;
		return true;
	};

	// CAMPS: one species per camp, on the golden spiral between two and five km out.
	for (int32 c = 0; c < Camps_; ++c)
	{
		const float Angle = FMath::DegreesToRadians(c * WildsGoldenDeg + 15.f);
		const float Radius = 220000.f + 26000.f * c;
		const float CX = FMath::Cos(Angle) * Radius;
		const float CY = FMath::Sin(Angle) * Radius;
		float CZ = 0.f;
		if (!GroundAt(CX, CY, CZ)) { continue; }

		const FWildsSpecies& Species = GSpecies[c % GSpeciesCount];

		FVEILCamp Camp;
		Camp.Phase = c * WildsGoldenDeg * 0.0174533f;   // one golden step per camp
		Camp.BaseDamage = Species.Damage;
		Camp.BaseSpeed = Species.Speed;
		Camp.Intent = FMath::Frac(Camp.Phase * WildsPhi);

		// The camp's gold post, so a player can find it from a distance.
		if (UStaticMesh* Post = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
		{
			AActor* Marker = W->SpawnActor<AActor>(AActor::StaticClass(), FVector(CX, CY, CZ), FRotator::ZeroRotator);
			if (Marker)
			{
				UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(Marker);
				Comp->SetStaticMesh(Post);
				Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				if (UMaterialInterface* Gold = LoadObject<UMaterialInterface>(nullptr, WildsGoldPath))
				{
					Comp->SetMaterial(0, Gold);
				}
				Comp->RegisterComponent();
				Comp->SetWorldScale3D(FVector(0.4f, 0.4f, 9.f));
				Comp->SetWorldLocation(FVector(CX, CY, CZ + 450.f));
			}
		}

		for (int32 n = 0; n < Species.Pack; ++n)
		{
			const float A = FMath::DegreesToRadians(n * WildsGoldenDeg);
			const float R = 400.f + 180.f * n;
			const FVector At(CX + FMath::Cos(A) * R, CY + FMath::Sin(A) * R, CZ + 120.f);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AVEILEnemy* Foe = W->SpawnActor<AVEILEnemy>(AVEILEnemy::StaticClass(), At,
				FRotator(0.f, FMath::RadiansToDegrees(A), 0.f), Params);
			if (!Foe) { continue; }

			Foe->ConfigureForKind(Species.Kind);
			// THE SPECIES IS A PLACE ON THE LADDER, not a typed number.
			Foe->MaxHealth = FMath::Pow(WildsPhi, RungOf(Species.Health));
			Foe->Health = Foe->MaxHealth;
			Foe->AttackDamage = FMath::Pow(WildsPhi, RungOf(Species.Damage));
			Foe->MoveSpeed = FMath::Pow(WildsPhi, RungOf(Species.Speed));
			Foe->bChampion = Species.bChampion;
			if (Species.bChampion) { Foe->ChampTier = 3; }

			Camp.Members.Add(Foe);
			++SpawnedEnemies;
		}

		UE_LOG(LogVEIL, Warning,
			TEXT("PHI ECOLOGY: camp %d - %s x%d, rungs hp %.2f dmg %.2f spd %.2f, decide digit %.3f."),
			c, Species.Name, Species.Pack, RungOf(Species.Health), RungOf(Species.Damage),
			RungOf(Species.Speed), Camp.Intent);

		Camps.Add(Camp);
	}

	// MINES: ore seams on the same spiral, closer in, between the camps.
	for (int32 m = 0; m < Mines_; ++m)
	{
		const float Angle = FMath::DegreesToRadians(m * WildsGoldenDeg + 200.f);
		const float Radius = 150000.f + 14000.f * m;
		const float X = FMath::Cos(Angle) * Radius;
		const float Y = FMath::Sin(Angle) * Radius;
		float Z = 0.f;
		if (!GroundAt(X, Y, Z)) { continue; }

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AVEILMineNode* Node = W->SpawnActor<AVEILMineNode>(AVEILMineNode::StaticClass(),
			FVector(X, Y, Z + 160.f), FRotator::ZeroRotator, Params);
		if (!Node) { continue; }
		Node->Init(m, 30 + 10 * (m % 4));
		++SpawnedMines;
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI WILDS: %d species bodies in %d camps, %d mines, all on phi clocks (1.5-5 km out)."),
		SpawnedEnemies, Camps.Num(), SpawnedMines);
}

void AVEILWildsPopulation::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Camps.Num() == 0) { return; }

	// The decide clock is one golden fraction of a second, not a frame: a camp that
	// re-decided every frame would be noise wearing a number.
	EcoClock += DeltaSeconds;
	DecideClock += DeltaSeconds;
	if (DecideClock < 0.618f) { return; }
	const float Step = DecideClock;
	DecideClock = 0.f;

	for (int32 c = 0; c < Camps.Num(); ++c)
	{
		FVEILCamp& Camp = Camps[c];
		Camp.Phase += Step;                                  // the camp's own clock
		Camp.Intent = FMath::Frac(Camp.Phase * WildsPhi);     // the decide digit

		// Two quantities on opposite phases: aggression rises as weight falls.
		const float Breath = 1.f + 0.09f * FMath::Sin(Camp.Phase * WildsPhi);
		const bool bRest = Camp.Intent < WildsRestDigit;

		for (int32 m = Camp.Members.Num() - 1; m >= 0; --m)
		{
			AVEILEnemy* Foe = Camp.Members[m].Get();
			if (!Foe) { Camp.Members.RemoveAt(m); continue; }

			Foe->AttackDamage = Camp.BaseDamage * Breath;
			Foe->MoveSpeed = Camp.BaseSpeed * (2.f - Breath);
			// The digit is a decision, not a die roll: below 1/phi^2 the camp holds.
			if (bRest) { Foe->StandDown(1.2f); }
		}

		if (bRest != Camp.bResting)
		{
			Camp.bResting = bRest;
			UE_LOG(LogVEIL, Log, TEXT("PHI ECOLOGY: camp %d %s (decide digit %.3f, %s 0.382)."),
				c, bRest ? TEXT("HOLDS") : TEXT("HUNTS"), Camp.Intent,
				bRest ? TEXT("below") : TEXT("above"));
		}
	}
}
