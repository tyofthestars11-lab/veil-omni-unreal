#include "World/VEILWilds.h"

#include "AIController.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/VEILProgression.h"
#include "Systems/VEILEnemy.h"
#include "VEIL.h"

namespace
{
	constexpr float WildsGoldenDeg = 137.50776405f;
	constexpr float WildsPhi = 1.6180339887f;
	/** 1/phi^2: the decide threshold. Below it a camp rests, above it hunts. */
	constexpr float WildsRestDigit = 0.3819660113f;

	/** What a camp does, chosen by its numerals. */
	enum class EWildsOrder : uint8 { Hold, Hunt, Encircle, Retreat };

	inline const TCHAR* OrderName(EWildsOrder Order)
	{
		switch (Order)
		{
		case EWildsOrder::Hold:     return TEXT("HOLD");
		case EWildsOrder::Encircle: return TEXT("ENCIRCLE");
		case EWildsOrder::Retreat:  return TEXT("RETREAT");
		default:                    return TEXT("HUNT");
		}
	}

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

	/**
	 * THE MINE BOOK. Twenty-four authored seams, each with an owner and an ore.
	 * A run always finds the same mines in the same places: these are the claims
	 * of the world, not a scatter of geometry that happens to be mineable.
	 */
	struct FWileSpec
	{
		const TCHAR* Name;
		const TCHAR* Owner;
		const TCHAR* Ore;
		float DepthM;
		int32 Reserve;
		FLinearColor Tint;
	};

	const FWileSpec GMineBook[] =
	{
		{ TEXT("IRONWIDOW"),   TEXT("Keeper's Compact"),    TEXT("iron"),      180.f,  620, FLinearColor(0.62f, 0.64f, 0.70f) },
		{ TEXT("COPPERMAW"),   TEXT("Keeper's Compact"),    TEXT("copper"),    240.f,  540, FLinearColor(0.85f, 0.48f, 0.22f) },
		{ TEXT("GREYSPINE"),   TEXT("Keeper's Compact"),    TEXT("iron"),      310.f,  700, FLinearColor(0.55f, 0.57f, 0.63f) },
		{ TEXT("LAMPLESS"),    TEXT("Keeper's Compact"),   TEXT("coal"),      420.f,  900, FLinearColor(0.22f, 0.22f, 0.26f) },
		{ TEXT("SALTGRACE"),   TEXT("Seer's Nest"),         TEXT("salt"),      120.f,  380, FLinearColor(0.92f, 0.93f, 0.95f) },
		{ TEXT("TIDEVANE"),    TEXT("Seer's Nest"),         TEXT("tide-glass"),210.f,  460, FLinearColor(0.30f, 0.78f, 0.86f) },
		{ TEXT("CLIFFSONG"),   TEXT("Seer's Nest"),         TEXT("salt"),      260.f,  420, FLinearColor(0.88f, 0.92f, 0.98f) },
		{ TEXT("RAINHOLLOW"),  TEXT("Seer's Nest"),         TEXT("tide-glass"),150.f,  390, FLinearColor(0.26f, 0.70f, 0.80f) },
		{ TEXT("BLACKGLASS"),  TEXT("Cartographer's Guild"),TEXT("obsidian"),  340.f,  520, FLinearColor(0.16f, 0.14f, 0.22f) },
		{ TEXT("MERIDIAN"),    TEXT("Cartographer's Guild"),TEXT("iron"),      280.f,  610, FLinearColor(0.58f, 0.60f, 0.66f) },
		{ TEXT("THEODOLITE"),  TEXT("Cartographer's Guild"),TEXT("obsidian"),  390.f,  480, FLinearColor(0.18f, 0.16f, 0.26f) },
		{ TEXT("FARSHORE"),    TEXT("Cartographer's Guild"),TEXT("salt"),      470.f,  560, FLinearColor(0.90f, 0.91f, 0.93f) },
		{ TEXT("EMBERVENT"),   TEXT("Countess"),            TEXT("ember"),     520.f,  760, FLinearColor(0.95f, 0.36f, 0.18f) },
		{ TEXT("MARROWLIGHT"), TEXT("Countess"),            TEXT("ember"),     610.f,  680, FLinearColor(0.92f, 0.30f, 0.14f) },
		{ TEXT("CINDERCLAIM"), TEXT("Countess"),            TEXT("coal"),      430.f,  820, FLinearColor(0.20f, 0.19f, 0.23f) },
		{ TEXT("REDHAND"),     TEXT("Countess"),            TEXT("copper"),    350.f,  600, FLinearColor(0.88f, 0.42f, 0.20f) },
		{ TEXT("HOLLOWDEEP"),  TEXT("the Warden"),          TEXT("void"),      980.f,  980, FLinearColor(0.42f, 0.20f, 0.68f) },
		{ TEXT("SILENTSEAM"),  TEXT("the Warden"),          TEXT("void"),      720.f,  840, FLinearColor(0.38f, 0.18f, 0.62f) },
		{ TEXT("LASTLIGHT"),   TEXT("the Warden"),          TEXT("obsidian"),  660.f,  700, FLinearColor(0.20f, 0.18f, 0.28f) },
		{ TEXT("THE ONE DOOR"),TEXT("the Warden"),          TEXT("void"),     1200.f, 1400, FLinearColor(0.50f, 0.24f, 0.76f) },
		{ TEXT("PALEACT"),     TEXT("the Hollow"),          TEXT("salt"),      300.f,  300, FLinearColor(0.86f, 0.88f, 0.90f) },
		{ TEXT("LISTENER'S REST"), TEXT("the Hollow"),      TEXT("obsidian"),  380.f,  340, FLinearColor(0.17f, 0.16f, 0.24f) },
		{ TEXT("BONECHOIR"),   TEXT("the Hollow"),          TEXT("void"),      540.f,  400, FLinearColor(0.36f, 0.22f, 0.58f) },
		{ TEXT("THE LONG QUIET"), TEXT("the Hollow"),       TEXT("coal"),      460.f,  380, FLinearColor(0.19f, 0.19f, 0.22f) },
	};
	constexpr int32 GMineBookCount = UE_ARRAY_COUNT(GMineBook);

	/**
	 * EVERY CLAIM SITS ON THE GROUND OF THE ONE WHO CLAIMS IT.
	 *
	 * The centres are the region anchors the rest of the game already uses (the
	 * Keeper at 900,0; the Seer at -450,780; the Spire's fragment at 2800,500;
	 * Omni at 600,-2100; the Vault at 0,0). The radius is how far inside the
	 * owner's reach a seam can sit. The Warden's ground is the Deep's inner disc;
	 * the Hollow works the rim around it, which is where they were first heard.
	 */
	struct FWileHome
	{
		const TCHAR* Owner;
		FVector2D Center;
		float RadiusCm;
	};

	const FWileHome GMineHomes[] =
	{
		{ TEXT("Keeper's Compact"),     FVector2D(  900.f,     0.f), 2600.f },
		{ TEXT("Seer's Nest"),          FVector2D( -450.f,   780.f), 2600.f },
		{ TEXT("Cartographer's Guild"), FVector2D( 2800.f,   500.f), 2600.f },
		{ TEXT("Countess"),             FVector2D(  600.f, -2100.f), 2600.f },
		{ TEXT("the Warden"),           FVector2D(    0.f,     0.f),  620.f },
		{ TEXT("the Hollow"),           FVector2D(    0.f,     0.f), 1250.f },
	};
	constexpr int32 GMineHomeCount = UE_ARRAY_COUNT(GMineHomes);

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

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("MineRoot"));
	SetRootComponent(Root);

	Lamp = CreateDefaultSubobject<UPointLightComponent>(TEXT("Lamp"));
	Lamp->SetupAttachment(Root);
	Lamp->SetLightColor(OreTint);
	Lamp->SetIntensity(3200.f);
	Lamp->SetAttenuationRadius(1600.f);
	Lamp->SetCastShadows(false);
	Lamp->SetAffectGlobalIllumination(false);

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Root);
	Trigger->InitSphereRadius(360.f);
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);
}

void AVEILMineNode::Init(int32 InIndex)
{
	Index = InIndex;
	const FWileSpec& Spec = GMineBook[FMath::Abs(Index) % GMineBookCount];

	// The identity IS the mine. Name, owner, ore, depth and reserve come from the
	// book, so the same seam is the same seam on every run.
	Name = Spec.Name;
	Owner = Spec.Owner;
	Ore = Spec.Ore;
	DepthM = Spec.DepthM;
	Reserve = Spec.Reserve;
	OreTint = Spec.Tint;
	Gold = FMath::Max(15, Reserve / 10);

	if (Trigger && !Trigger->OnComponentBeginOverlap.IsAlreadyBound(this, &AVEILMineNode::OnEnter))
	{
		Trigger->OnComponentBeginOverlap.AddDynamic(this, &AVEILMineNode::OnEnter);
	}
	if (Lamp)
	{
		Lamp->SetLightColor(OreTint);
	}
	BuildHead();

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI MINE %02d %s - %s, %s seam, %0.f m down, reserve %d, worth %d gold (rung %.2f) at %s."),
		Index, *Name, *Owner, *Ore, DepthM, Reserve, Gold, RungOf((float)Gold),
		*GetActorLocation().ToCompactString());
}

void AVEILMineNode::BuildHead()
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	USceneComponent* Attach = GetRootComponent();
	if (!Cube || !Attach) { return; }

	auto Part = [this, Attach](UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FRotator& Rot, bool bSolid)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		if (!C) { return (UStaticMeshComponent*)nullptr; }
		C->SetupAttachment(Attach);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeRotation(Rot);
		C->SetRelativeScale3D(Scale);
		C->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		C->RegisterComponent();
		return C;
	};

	// FOUR POSTS AND TWO BEAMS: a headframe, not a rock. The posts are solid, so
	// the mouth of the mine is something the runner can walk around.
	const float PostH = 4.2f;
	for (int32 i = 0; i < 4; ++i)
	{
		const float SX = (i & 1) ? 1.f : -1.f;
		const float SY = (i & 2) ? 1.f : -1.f;
		Part(Cube, FVector(SX * 130.f, SY * 110.f, PostH * 50.f), FVector(0.34f, 0.34f, PostH), FRotator::ZeroRotator, true);
	}
	Part(Cube, FVector(0.f, 115.f, 400.f), FVector(3.4f, 0.30f, 0.30f), FRotator::ZeroRotator, true);
	Part(Cube, FVector(0.f, -115.f, 400.f), FVector(3.4f, 0.30f, 0.30f), FRotator::ZeroRotator, true);
	Part(Cube, FVector(0.f, 0.f, 430.f), FVector(0.30f, 2.6f, 0.30f), FRotator::ZeroRotator, false);

	// THE SHAFT: a dark mouth in the ground, not a hole the runner falls through.
	if (Cylinder)
	{
		UStaticMeshComponent* Mouth = Part(Cylinder, FVector(0.f, 0.f, 12.f), FVector(2.2f, 2.2f, 0.10f), FRotator::ZeroRotator, false);
		if (Mouth)
		{
			if (UMaterialInterface* Deep = LoadObject<UMaterialInterface>(nullptr, WildsCrystalPath))
			{
				Mouth->SetMaterial(0, Deep);
			}
		}
	}

	// THE SEAM: three boulders of the ore it actually yields, tinted by the book.
	for (int32 i = 0; i < 3; ++i)
	{
		const float A = FMath::DegreesToRadians(i * WildsGoldenDeg);
		UStaticMeshComponent* Rock = Part(Cube,
			FVector(FMath::Cos(A) * 210.f, FMath::Sin(A) * 190.f, 55.f + 12.f * i),
			FVector(1.5f - 0.2f * i, 1.3f + 0.15f * i, 1.1f - 0.1f * i),
			FRotator(0.f, i * 37.f, 0.f), false);
		if (Rock)
		{
			if (UMaterialInterface* OreMat = LoadObject<UMaterialInterface>(nullptr, WildsCrystalPath))
			{
				Rock->SetMaterial(0, OreMat);
			}
			Rock->SetCustomPrimitiveDataFloat(0, OreTint.R);
			Rock->SetCustomPrimitiveDataFloat(1, OreTint.G);
			Rock->SetCustomPrimitiveDataFloat(2, OreTint.B);
		}
	}
}

void AVEILMineNode::OnEnter(UPrimitiveComponent* OverlappedComp, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	// Walking past a seam used to pay out by itself, which meant the runner never
	// had to do anything to own the world. Now standing here only tells you what it
	// is and that it can be taken; the taking is the key press.
	if (bClaimed || bPrompted) { return; }
	bPrompted = true;
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI MINE: %02d %s is within reach - %s seam, %.0f m down, %s ground. Press E to claim it."),
		Index, *Name, *Ore, DepthM, *Owner);
}

bool AVEILMineNode::Claim(AActor* Claimant)
{
	if (bClaimed) { return false; }
	bClaimed = true;
	if (Lamp)
	{
		Lamp->SetLightColor(FLinearColor(1.f, 0.78f, 0.30f));
		Lamp->SetIntensity(5200.f);
	}
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI MINE CLAIMED: %02d %s - %s seam on %s ground is the runner's, +%d gold."),
		Index, *Name, *Ore, *Owner, Gold);
	return true;
}

void AVEILMineNode::MarkClaimedSilently()
{
	bClaimed = true;
	bPrompted = true;
	if (Lamp)
	{
		Lamp->SetLightColor(FLinearColor(1.f, 0.78f, 0.30f));
		Lamp->SetIntensity(5200.f);
	}
}

void AVEILMineNode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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

int32 AVEILWildsPopulation::MinesClaimed() const
{
	int32 N = 0;
	for (const TWeakObjectPtr<AVEILMineNode>& M : MineNodes)
	{
		if (M.IsValid() && M->IsClaimed()) { ++N; }
	}
	return N;
}

AVEILMineNode* AVEILWildsPopulation::NearestUnclaimedMine(const FVector& From, float MaxCm) const
{
	AVEILMineNode* Best = nullptr;
	float BestD = MaxCm;
	for (const TWeakObjectPtr<AVEILMineNode>& M : MineNodes)
	{
		if (!M.IsValid() || M->IsClaimed()) { continue; }
		const float D = FVector::Dist2D(M->GetActorLocation(), From);
		if (D <= BestD) { BestD = D; Best = M.Get(); }
	}
	return Best;
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
		Camp.Center = FVector(CX, CY, CZ);

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

	// MINES: the world's own claims, on the golden spiral between the camps.
	// The angle and radius are the address; the identity comes from the book.
	int32 OffOwnGround = 0;
	for (int32 m = 0; m < Mines_; ++m)
	{
		const FWileSpec& Spec = GMineBook[m % GMineBookCount];

		// Whose ground is it? The claim is placed inside THAT owner's reach, so a
		// mine is never found on someone else's land by accident of a spiral step.
		const FWileHome* Home = nullptr;
		for (int32 h = 0; h < GMineHomeCount; ++h)
		{
			if (FCString::Strcmp(GMineHomes[h].Owner, Spec.Owner) == 0) { Home = &GMineHomes[h]; break; }
		}
		if (!Home) { continue; }

		const int32 HomeIndex = (int32)(Home - GMineHomes);
		const float Angle = FMath::DegreesToRadians(m * WildsGoldenDeg);
		const float RingFull = Home->RadiusCm * (0.55f + 0.45f * FMath::Frac(m * 0.6180339887f));
		const FVector2D Dir(FMath::Cos(Angle), FMath::Sin(Angle));

		// STANDING ON THE RIGHT GROUND.
		// A reach is wider than the distance between two anchors, so a point inside
		// "the Nest's reach" can still stand in the Hub - the first run put TIDEVANE
		// 60 cm from the Keeper. And two owners can share an anchor: the Warden holds
		// the Deep's inner disc while the Hollow works the rim, both centred on 0,0.
		// So the rule is a claim, not a nearest neighbour: every home owns everything
		// inside its reach, and the SMALLEST reach that covers a point owns it. The
		// Warden's 620 keeps the disc; the Hollow's 1,250 takes the rim outside it.
		auto ClaimOf = [](const FVector2D& Pt) -> int32
		{
			int32 Owner = INDEX_NONE;
			float Smallest = TNumericLimits<float>::Max();
			float Nearest = TNumericLimits<float>::Max();
			for (int32 h = 0; h < GMineHomeCount; ++h)
			{
				const float D = FVector2D::Distance(Pt, GMineHomes[h].Center);
				if (D > GMineHomes[h].RadiusCm) { continue; }
				// Reaches are equal in most cases, so reach alone cannot decide: on a tie
				// the nearer anchor owns it, which is exactly how the region system picks.
				const bool bSmaller = GMineHomes[h].RadiusCm < Smallest - 1.f;
				const bool bTie = FMath::Abs(GMineHomes[h].RadiusCm - Smallest) <= 1.f && D < Nearest;
				if (bSmaller || bTie)
				{
					Smallest = bSmaller ? GMineHomes[h].RadiusCm : Smallest;
					Nearest = D;
					Owner = h;
				}
			}
			return Owner;
		};

		FVector2D Site = Home->Center;
		float Ring = 0.f;
		bool bOnOwnGround = false;

		// Turning matters as much as shrinking. The Hollow's rim ring is centred on the
		// Deep and reaches 1,250 cm, which swallows the heart of the Hub: a seam walking
		// one fixed ray out of 900,0 can never stand on Keeper ground, however close in
		// it comes. So the search is a sweep - hold the distance, turn the ray through
		// the golden angles, and take the first thing that is genuinely the owner's.
		for (int32 Step = 100; Step >= 15 && !bOnOwnGround; Step -= 5)
		{
			const float Scale = Step / 100.f;
			for (int32 Turn = 0; Turn < 36; ++Turn)
			{
				const float A = FMath::DegreesToRadians((m + Turn) * WildsGoldenDeg);
				const FVector2D Try = Home->Center
					+ FVector2D(FMath::Cos(A), FMath::Sin(A)) * (RingFull * Scale);
				if (ClaimOf(Try) == HomeIndex)
				{
					Site = Try;
					Ring = RingFull * Scale;
					bOnOwnGround = true;
					break;
				}
			}
		}
		if (!bOnOwnGround)
		{
			Ring = 0.f;
			Site = Home->Center;
			++OffOwnGround;
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI MINE %02d %s finds no clear point of %s ground; set at the anchor."),
				m, Spec.Name, Spec.Owner);
		}
		const float X = Site.X;
		const float Y = Site.Y;
		float Z = 0.f;
		if (!GroundAt(X, Y, Z)) { continue; }

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AVEILMineNode* Node = W->SpawnActor<AVEILMineNode>(AVEILMineNode::StaticClass(),
			FVector(X, Y, Z + 160.f), FRotator::ZeroRotator, Params);
		if (!Node) { continue; }
		Node->Init(m);

		// A SEAM ALREADY WON STAYS WON. The save knows which names were claimed, so a
		// reload lights them gold instead of asking the runner to take the same mine
		// twice - and never pays the gold a second time.
		if (UOMNIQuestSystem* Quest = W->GetSubsystem<UOMNIQuestSystem>())
		{
			if (Quest->ClaimedMines.Contains(FName(*Node->MineName()))) { Node->MarkClaimedSilently(); }
		}
		MineNodes.Add(Node);

		UE_LOG(LogVEIL, Log, TEXT("PHI MINE %02d %s stands %.0f cm inside %s ground at %s."),
			m, *Node->MineName(), Ring, *Node->MineOwner(), *Node->GetActorLocation().ToCompactString());
		++SpawnedMines;
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI WILDS: %d species bodies in %d camps, %d named mines, all on phi clocks."),
		SpawnedEnemies, Camps.Num(), SpawnedMines);
	UE_LOG(LogVEIL, Warning, TEXT("PHI MINE CLAIMS: %d of %d stand on their owner's ground, %d do not."),
		SpawnedMines - OffOwnGround, SpawnedMines, OffOwnGround);
}

void AVEILWildsPopulation::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// POINT AT THE NEXT SEAM. Once a second the nearest unclaimed mine is handed to
	// the quest book, so the HUD can say how far and which way - the difference
	// between a world with mines in it and a game with something to do.
	if (UWorld* World = GetWorld())
	{
		TrackClock += DeltaSeconds;
		SaveClock += DeltaSeconds;
		if (SaveClock >= 15.f)
		{
			SaveClock = 0.f;
			if (UVEILProgression* Book = World->GetSubsystem<UVEILProgression>())
			{
				if (const UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>())
				{
					Book->Level = Quest->Level;
					Book->XP = Quest->XP;
					Book->Gold = Quest->Gold;
					Book->Fragments = Quest->Fragments;
					Book->ClaimedMines = Quest->ClaimedMines;
				}
				const bool bWrote = Book->SaveGame();
				UE_LOG(LogVEIL, Log, TEXT("PHI AUTOSAVE: wrote=%d level=%d gold=%d seams=%d."),
					bWrote ? 1 : 0, Book->Level, Book->Gold, Book->ClaimedMines.Num());
			}
		}
		if (TrackClock >= 1.f)
		{
			TrackClock = 0.f;
			if (UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>())
			{
				if (const APawn* Runner = UGameplayStatics::GetPlayerPawn(this, 0))
				{
					if (const AVEILMineNode* Closest = NearestUnclaimedMine(Runner->GetActorLocation(), 400000.f))
					{
						const float Cm = FVector::Dist2D(Closest->GetActorLocation(), Runner->GetActorLocation());
						// The number the HUD shows has to be checkable, so the two points it came
						// from are printed with it - once per change of seam, not once per second.
						if (LastTrackedName != Closest->MineName())
						{
							LastTrackedName = Closest->MineName();
							UE_LOG(LogVEIL, Warning,
								TEXT("PHI TRACK SRC: %s at (%.0f,%.0f) runner at (%.0f,%.0f) -> %.0f cm."),
								*LastTrackedName, Closest->GetActorLocation().X, Closest->GetActorLocation().Y,
								Runner->GetActorLocation().X, Runner->GetActorLocation().Y, Cm);
						}
						Quest->TrackMine(Closest->MineName(), Cm, Closest->MineOwner());
					}
					else
					{
						Quest->TrackMine(FString(), 0.f, FString());
					}
				}
			}
		}
	}

	if (Camps.Num() == 0) { return; }

	EcoClock += DeltaSeconds;
	DecideClock += DeltaSeconds;
	APawn* Runner = UGameplayStatics::GetPlayerPawn(this, 0);

	// ---------------------------------------------------------------
	// DECIDE, on a golden fraction of a second.
	// ---------------------------------------------------------------
	if (DecideClock >= 0.618f)
	{
		const float Step = DecideClock;
		DecideClock = 0.f;

		for (int32 c = 0; c < Camps.Num(); ++c)
		{
			FVEILCamp& Camp = Camps[c];
			Camp.Phase += Step;

			// Three incommensurate numerals: phi, phi^2, phi^3.
			Camp.Intent = FMath::Frac(Camp.Phase * WildsPhi);
			const float OrderDigit = FMath::Frac(Camp.Phase * WildsPhi * WildsPhi);
			const float RetreatDigit = FMath::Frac(Camp.Phase * WildsPhi * WildsPhi * WildsPhi);
			const float Breath = 1.f + 0.09f * FMath::Sin(Camp.Phase * WildsPhi);

			EWildsOrder Candidate = EWildsOrder::Hunt;
			if (Camp.Intent < WildsRestDigit) { Candidate = EWildsOrder::Hold; }
			else if (OrderDigit < WildsRestDigit) { Candidate = EWildsOrder::Encircle; }

			// HYSTERESIS. A digit sitting on the threshold flips the whole camp every
			// 0.618 s, which read in the log as 2,600 order changes in two minutes and
			// looked, correctly, like noise. An order now has to hold for two decisions
			// in a row before it is obeyed - the camp still decides on phi, it just does
			// not stutter about it.
			EWildsOrder Order = (Camp.Order < 0) ? Candidate : (EWildsOrder)Camp.Order;
			if (Camp.Order < 0)
			{
				Order = Candidate;
			}
			else if ((int32)Candidate != Camp.Order)
			{
				if (Camp.Pending == (int32)Candidate) { ++Camp.PendingCount; }
				else { Camp.Pending = (int32)Candidate; Camp.PendingCount = 1; }
				if (Camp.PendingCount >= 2) { Order = Candidate; }
			}
			else
			{
				Camp.Pending = -1;
				Camp.PendingCount = 0;
			}

			Camp.Dir.SetNum(Camp.Members.Num());

			for (int32 m = Camp.Members.Num() - 1; m >= 0; --m)
			{
				AVEILEnemy* Foe = Camp.Members[m].Get();
				if (!Foe)
				{
					Camp.Members.RemoveAt(m);
					Camp.Dir.RemoveAt(m);
					continue;
				}

				Foe->AttackDamage = Camp.BaseDamage * Breath;
				Foe->MoveSpeed = Camp.BaseSpeed * (2.f - Breath);

				EWildsOrder Member = Order;
				const float HealthFraction = Foe->MaxHealth > 0.f ? Foe->Health / Foe->MaxHealth : 1.f;
				if (HealthFraction < 0.25f && RetreatDigit < 0.5f) { Member = EWildsOrder::Retreat; }

				// THE STEERING FALLBACK. No NavMesh is required and none is assumed:
				// the heading is a vector from where the body stands to where its order
				// says it should be, applied as movement input every frame. The hills
				// are runtime geometry, so this is the only honest way to make the
				// camps walk until navigation is generated there.
				FVector Target = Foe->GetActorLocation();
				switch (Member)
				{
				case EWildsOrder::Hold:
					Target = Foe->GetActorLocation();
					Foe->StandDown(1.2f);
					break;

				case EWildsOrder::Retreat:
					Target = Camp.Center;
					Foe->StandDown(0.f);
					break;

				case EWildsOrder::Encircle:
					Foe->StandDown(0.f);
					if (Runner)
					{
						const float A = FMath::DegreesToRadians(m * WildsGoldenDeg);
						const float R = 520.f + 90.f * m;
						Target = Runner->GetActorLocation() +
							FVector(FMath::Cos(A) * R, FMath::Sin(A) * R, 0.f);
					}
					break;

				case EWildsOrder::Hunt:
				default:
					Foe->StandDown(0.f);
					if (Runner) { Target = Runner->GetActorLocation(); }
					break;
				}

				FVector ToTarget = Target - Foe->GetActorLocation();
				ToTarget.Z = 0.f;
				Camp.Dir[m] = (ToTarget.SizeSquared() > FMath::Square(140.f))
					? ToTarget.GetSafeNormal() : FVector::ZeroVector;
			}

			if ((int32)Order != Camp.Order)
			{
				Camp.Order = (int32)Order;
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI ECOLOGY: camp %d order %s (intent %.3f order %.3f retreat %.3f)."),
					c, OrderName(Order), Camp.Intent, OrderDigit, RetreatDigit);
			}
		}
	}

	// ---------------------------------------------------------------
	// MOVE, every frame, from the last decision. Input, not teleport.
	// ---------------------------------------------------------------
	for (int32 c = 0; c < Camps.Num(); ++c)
	{
		FVEILCamp& Camp = Camps[c];
		for (int32 m = 0; m < Camp.Members.Num() && m < Camp.Dir.Num(); ++m)
		{
			AVEILEnemy* Foe = Camp.Members[m].Get();
			if (!Foe) { continue; }
			if (!Camp.Dir[m].IsNearlyZero()) { Foe->AddMovementInput(Camp.Dir[m], 1.f); }
		}
	}
}
