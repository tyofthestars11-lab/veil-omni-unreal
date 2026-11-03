// OMNI RPG: PATH OF LIGHT - quest content implementation.
#include "World/OMNIQuestContent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Story/VEILCastBody.h"

#include "Character/VEILCharacter.h"
#include "Character/VEILCountessCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "VEIL.h"
#include "Systems/OMNIQuestSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Async/Async.h"

namespace
{
// The actual material swap, safe to call ONLY on the game thread.
void ApplyTintNow(UStaticMeshComponent* Comp, UMaterialInterface* Base,
	const FLinearColor& Color, float Emissive)
{
	if (!Comp || !Base) return;
	if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, Comp))
	{
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
		if (Emissive > 0.f)
		{
			MID->SetVectorParameterValue(TEXT("EmissiveColor"), Color * Emissive);
		}
		Comp->SetMaterial(0, MID);
	}
}

void Tint(UStaticMeshComponent* Comp, const TCHAR* BasePath, const FLinearColor& Color, float Emissive)
{
	if (!Comp) return;
	// PHI 835: always build from a REAL base, never from the live slot
	// (which may already hold a MID -> invalid nested parent).
	static TMap<FString, TObjectPtr<UMaterialInterface>> BaseCache;
	UMaterialInterface* Base = nullptr;
	if (BasePath)
	{
		if (TObjectPtr<UMaterialInterface>* F = BaseCache.Find(BasePath)) { Base = F->Get(); }
		else { Base = LoadObject<UMaterialInterface>(nullptr, BasePath); if (Base) { BaseCache.Add(BasePath, Base); } }
	}
	// PHI 835 root fix: never fall back to the live slot (it may hold a MID).
	// Fail closed so no MID is ever created with a MID parent.
	if (!Base) return;
	if (Cast<UMaterialInstanceDynamic>(Base)) return;

	// RENDERER WORK IS GAME-THREAD ONLY - AND CONSTRUCTORS ARE NOT.
	//
	// When an actor is loaded out of a package its constructor runs on the ASYNC
	// LOADING thread. Creating a dynamic material instance and writing a vector
	// parameter there reaches the renderer from a thread that does not own it, and
	// the engine asserts. That is the packaged-build crash, caught in the log with its
	// whole stack:
	//
	//   AOMNICache::AOMNICache() -> Tint() -> SetVectorParameterValue() ->
	//   FRendererModule::InvalidatePathTracedOutput() ->
	//   "Assertion failed: IsInGameThread() || IsInParallelGameThread()"
	//
	// The editor never showed it because editor loads happen on the game thread; the
	// packaged game streams and event-drives loads, so constructors really do run off
	// it. So: put the plain base material on now (a material assignment on a component
	// is construction-safe), and QUEUE the tint for the game thread, where it lands as
	// soon as loading yields. Same colours, correct thread.
	if (!IsInGameThread())
	{
		Comp->SetMaterial(0, Base);
		TWeakObjectPtr<UStaticMeshComponent> WeakComp(Comp);
		const FString PathCopy(BasePath);
		const FLinearColor ColorCopy = Color;
		const float EmissiveCopy = Emissive;
		AsyncTask(ENamedThreads::GameThread, [WeakComp, PathCopy, ColorCopy, EmissiveCopy]()
		{
			UStaticMeshComponent* C = WeakComp.Get();
			if (!C) { return; }
			// Reloaded by path on the game thread: a raw material pointer captured
			// across a load is not GC-safe, and the path finds the same asset.
			UMaterialInterface* Reloaded = LoadObject<UMaterialInterface>(nullptr, *PathCopy);
			if (!Reloaded || Cast<UMaterialInstanceDynamic>(Reloaded)) { return; }
			ApplyTintNow(C, Reloaded, ColorCopy, EmissiveCopy);
		});
		return;
	}

	ApplyTintNow(Comp, Base, Color, Emissive);
}

	USphereComponent* MakeRadius(AActor* Owner, USceneComponent* Root, float Radius)
	{
		USphereComponent* Sphere = NewObject<USphereComponent>(Owner);
		Sphere->SetupAttachment(Root);
		Sphere->InitSphereRadius(Radius);
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Sphere->SetCollisionObjectType(ECC_WorldDynamic);
		Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Sphere->SetGenerateOverlapEvents(true);
		Sphere->RegisterComponent();
		return Sphere;
	}
}

// ---------------------------------------------------------------------------
// Giver
// ---------------------------------------------------------------------------
AOMNIQuestGiver::AOMNIQuestGiver()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> StoneMat(TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GoldMat(TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GlowMat(TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// A REAL PERSON, NOT A STATUE. The giver was a cylinder with a column balanced on it; this
	// component carries an installed cast body, applied in BeginPlay once the name is known.
	USkeletalMeshComponent* Figure = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CastFigure"));
	Figure->SetupAttachment(Root);
	Figure->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Figure->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	UStaticMeshComponent* Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatueBody"));
	Body->SetupAttachment(Root);
	Body->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	Body->SetRelativeScale3D(FVector(0.9f, 0.9f, 3.f));
	if (CylinderMesh.Succeeded()) Body->SetStaticMesh(CylinderMesh.Object);
	if (StoneMat.Succeeded()) Body->SetMaterial(0, StoneMat.Object);
	Tint(Body, TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"), FLinearColor(0.35f, 0.33f, 0.38f), 0.f);

	UStaticMeshComponent* Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatueHead"));
	Head->SetupAttachment(Root);
	Head->SetRelativeLocation(FVector(0.f, 0.f, 345.f));
	Head->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.55f));
	if (SphereMesh.Succeeded()) Head->SetStaticMesh(SphereMesh.Object);
	if (StoneMat.Succeeded()) Head->SetMaterial(0, StoneMat.Object);
	Tint(Head, TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"), FLinearColor(0.35f, 0.33f, 0.38f), 0.f);

	UStaticMeshComponent* Visor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatueVisor"));
	Visor->SetupAttachment(Root);
	Visor->SetRelativeLocation(FVector(26.f, 0.f, 350.f));
	Visor->SetRelativeScale3D(FVector(0.10f, 0.5f, 0.10f));
	if (CubeMesh.Succeeded()) Visor->SetStaticMesh(CubeMesh.Object);
	if (GlowMat.Succeeded()) Visor->SetMaterial(0, GlowMat.Object);
	Tint(Visor, TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"), FLinearColor(1.f, 0.78f, 0.30f), 3.f);
}

void AOMNIQuestGiver::BeginPlay()
{
	Super::BeginPlay();
	TalkRadius = MakeRadius(this, RootComponent, 340.f);
	TalkRadius->OnComponentBeginOverlap.AddDynamic(this, &AOMNIQuestGiver::OnTalkRadius);
	TalkRadius->OnComponentEndOverlap.AddDynamic(this, &AOMNIQuestGiver::OnTalkEnd);
	UE_LOG(LogVEIL, Log, TEXT("OMNI QUEST CONTENT: the %s stands ready."), *GiverName.ToString());

	// THE STATUE STEPS ASIDE. Cylinder, column head and visor were the placeholder; the giver
	// now wears a body chosen by name, and the stone parts go dark so the person is what the
	// player meets rather than a pillar doing a person's job.
	if (USkeletalMeshComponent* CastFigure = FindComponentByClass<USkeletalMeshComponent>())
	{
		VEILCastBody::Apply(CastFigure, VEILCastBody::ForRole(GiverName.ToString()),
			GiverName.ToString(), 1.f);
	}
	{
		TArray<UStaticMeshComponent*> StatueParts;
		GetComponents<UStaticMeshComponent>(StatueParts);
		for (UStaticMeshComponent* Part : StatueParts)
		{
			if (!Part) { continue; }
			if (Part->GetName().Contains(TEXT("Statue")))
			{
				Part->SetVisibility(false);
				Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}

	// THE STATUE STEPS ASIDE. Cylinder, column head and visor were the placeholder; the giver
	// now wears a body chosen by name, and the stone parts go dark so the person is what the
	// player meets rather than a pillar with a person's job.
	if (USkeletalMeshComponent* CastFigure = FindComponentByClass<USkeletalMeshComponent>())
	{
		VEILCastBody::Apply(CastFigure, VEILCastBody::ForRole(GiverName.ToString()),
			GiverName.ToString(), 1.f);
	}
	{
		TArray<UStaticMeshComponent*> StatueParts;
		GetComponents<UStaticMeshComponent>(StatueParts);
		for (UStaticMeshComponent* Part : StatueParts)
		{
			if (!Part) { continue; }
			if (Part->GetName().Contains(TEXT("Statue")))
			{
				Part->SetVisibility(false);
				Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
	{ // PHI BIGBANG: every voice wears its own light
		FLinearColor Visor(1.f, 0.78f, 0.3f), Robe(0.35f, 0.33f, 0.38f);
		if (GiverName == FName(TEXT("Engineer"))) { Visor = FLinearColor(1.f, 0.45f, 0.15f); Robe = FLinearColor(0.4f, 0.25f, 0.15f); }
		else if (GiverName == FName(TEXT("Medic"))) { Visor = FLinearColor(0.2f, 1.f, 0.8f); Robe = FLinearColor(0.15f, 0.35f, 0.3f); }
		else if (GiverName == FName(TEXT("Scout"))) { Visor = FLinearColor(0.4f, 0.8f, 1.f); Robe = FLinearColor(0.2f, 0.3f, 0.45f); }
		else if (GiverName == FName(TEXT("Seer"))) { Visor = FLinearColor(0.7f, 0.4f, 1.f); Robe = FLinearColor(0.3f, 0.2f, 0.45f); }
		else if (GiverName == FName(TEXT("Cartographer"))) { Visor = FLinearColor(0.4f, 1.f, 0.4f); Robe = FLinearColor(0.2f, 0.4f, 0.2f); }
		TArray<UStaticMeshComponent*> Meshes;
		GetComponents<UStaticMeshComponent>(Meshes);
		for (UStaticMeshComponent* M : Meshes)
		{
			if (!M) continue;
			if (M->GetName() == TEXT("StatueVisor")) { Tint(M, TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"), Visor, 3.f); }
			else { Tint(M, TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"), Robe, 0.f); }
		}
	}
}

static bool PhiIsQuestWalker(AActor* OtherActor)
{
	// PHI PARTY: either playable walks the quest - Runner or Countess leads.
	return Cast<AVEILCharacter>(OtherActor) != nullptr || Cast<AVEILCountessCharacter>(OtherActor) != nullptr;
}

void AOMNIQuestGiver::OnTalkRadius(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!PhiIsQuestWalker(OtherActor)) return;
	if (bSpokenThisVisit) return; // one greeting per visit, never spam
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastTalkWorldTime < 20.f) return; // edge-jitter cooldown
	bSpokenThisVisit = true;
	LastTalkWorldTime = Now;
	if (!bMetRunner)
	{
		bMetRunner = true;
		UE_LOG(LogVEIL, Log, TEXT("OMNI TALK: the %s meets the walker and remembers."), *GiverName.ToString());
	}
	if (UOMNIQuestSystem* Quests = GetWorld()->GetSubsystem<UOMNIQuestSystem>())
	{
		Quests->TalkTo(GiverName);
	}
}

void AOMNIQuestGiver::OnTalkEnd(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32)
{
	if (!PhiIsQuestWalker(OtherActor)) return;
	bSpokenThisVisit = false; // the next approach is a new visit
	UE_LOG(LogVEIL, Log, TEXT("OMNI TALK: the Runner leaves the %s."), *GiverName.ToString());
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Lore note
// ---------------------------------------------------------------------------
AOMNILoreNote::AOMNILoreNote()
{
 PrimaryActorTick.bCanEverTick = false;
 USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
 SetRootComponent(Root);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
 TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big"));
 Leaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoteLeaf"));
 Leaf->SetupAttachment(Root);
 Leaf->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.5f));
 Leaf->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 if (SphereMesh.Succeeded()) Leaf->SetStaticMesh(SphereMesh.Object);
 Tint(Leaf, TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"), FLinearColor(1.f, 0.85f, 0.4f), 2.f);
}

void AOMNILoreNote::BeginPlay()
{
 Super::BeginPlay();
 ReadRadius = MakeRadius(this, RootComponent, 400.f);
 ReadRadius->OnComponentBeginOverlap.AddDynamic(this, &AOMNILoreNote::OnReadRadius);
}

void AOMNILoreNote::OnReadRadius(UPrimitiveComponent*, AActor* OtherActor,
 UPrimitiveComponent*, int32, bool, const FHitResult&)
{
 if (!PhiIsQuestWalker(OtherActor)) return;
 if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor(255, 200, 60), FString::Printf(TEXT("LEAF: %s - press E to read aloud"), *NoteTitle)); }
 UE_LOG(LogVEIL, Log, TEXT("OMNI LORE: leaf in reach: %s."), *NoteTitle);
}


// Fragment
// ---------------------------------------------------------------------------
AOMNIFragment::AOMNIFragment()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> StoneMat(TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GoldMat(TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GlowMat(TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big"));

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FragmentBody"));
	Body->SetupAttachment(Root);
	Body->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.45f));
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (SphereMesh.Succeeded()) Body->SetStaticMesh(SphereMesh.Object);
	if (GoldMat.Succeeded()) Body->SetMaterial(0, GoldMat.Object);
	Tint(Body, TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"), FLinearColor(1.f, 0.78f, 0.30f), 3.5f);
}

void AOMNIFragment::BeginPlay()
{
	Super::BeginPlay();
	BaseZ = GetActorLocation().Z;
	TouchRadius = MakeRadius(this, RootComponent, 500.f); // PHI LAW: epic loot never pixel-hunt
	TouchRadius->OnComponentBeginOverlap.AddDynamic(this, &AOMNIFragment::OnTouched);
}

void AOMNIFragment::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AddActorLocalRotation(FRotator(0.f, 60.f * DeltaSeconds, 0.f));
	FVector Loc = GetActorLocation();
	Loc.Z = BaseZ + FMath::Sin(GetWorld()->GetTimeSeconds() * 1.7f) * 18.f;
	SetActorLocation(Loc);
}

void AOMNIFragment::OnTouched(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!PhiIsQuestWalker(OtherActor)) return;
	if (UOMNIQuestSystem* Quests = GetWorld()->GetSubsystem<UOMNIQuestSystem>())
	{
		Quests->CollectFragment();
	}
	Destroy();
}

// ---------------------------------------------------------------------------
// Cache
// ---------------------------------------------------------------------------
AOMNICache::AOMNICache()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> StoneMat(TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GoldMat(TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GlowMat(TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	UStaticMeshComponent* Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CacheBody"));
	Body->SetupAttachment(Root);
	Body->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	Body->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
	if (CubeMesh.Succeeded()) Body->SetStaticMesh(CubeMesh.Object);
	if (GlowMat.Succeeded()) Body->SetMaterial(0, GlowMat.Object);
	Tint(Body, TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"), FLinearColor(0.15f, 0.9f, 0.35f), 2.5f);
}

void AOMNICache::BeginPlay()
{
	Super::BeginPlay();
	OpenRadius = MakeRadius(this, RootComponent, 300.f);
	OpenRadius->OnComponentBeginOverlap.AddDynamic(this, &AOMNICache::OnOpened);
	UE_LOG(LogVEIL, Log, TEXT("OMNI QUEST CONTENT: the survey cache sleeps by the south wall."));
}

void AOMNICache::OnOpened(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!PhiIsQuestWalker(OtherActor)) return;
	if (UOMNIQuestSystem* Quests = GetWorld()->GetSubsystem<UOMNIQuestSystem>())
	{
		Quests->ReachCache();
	}
}

