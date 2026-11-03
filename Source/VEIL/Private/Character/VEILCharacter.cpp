// VEIL: THE LAST SIGNAL - player character implementation.
#include "Character/VEILCharacter.h"
#include "Systems/PHIONE.h"

#include "Engine/OverlapResult.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Systems/PHIMito.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Game/VEILGameMode.h"
#include "HAL/IConsoleManager.h"
#include "Systems/OMNIQuestSystem.h"
#include "World/VEILWilds.h"
#include "World/VEILConceptWorld.h"
#include "Systems/VEILBurstFX.h"
#include "Engine/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "HAL/IConsoleManager.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Systems/VEILDiscoverySystem.h"
#include "Systems/VEILDialogue.h"
#include "Systems/VEILInputHelp.h"
#include "Systems/VEILTime.h"
#include "Systems/VEILProgression.h"
#include "Systems/VEILEnemy.h"
#include "AI/VEILFactionAI.h"
#include "EngineUtils.h"
#include "VEIL.h"

AVEILCharacter::AVEILCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	// FLUID, NOT MARIONETTE. The default profile reaches full speed in one frame
	// and stops dead the next, which reads as a stiff puppet rather than a body
	// with weight. Every number here is phi-derived: a gentler ramp up, a
	// deliberate brake, real ground friction to carry momentum, and a turn rate
	// of 360/phi = 222.5 deg/s so he banks into a corner instead of snapping.
	Move->RotationRate = FRotator(0.f, 360.f / UPHIMito::Phi, 0.f);
	Move->MaxAcceleration = 512.f * UPHIMito::Phi;
	Move->BrakingDecelerationWalking = 640.f * UPHIMito::Phi;
	Move->GroundFriction = 8.f / UPHIMito::Phi;
	Move->AirControl = 1.f / UPHIMito::Phi;
	Move->JumpZVelocity = 620.f;
	Move->MaxWalkSpeed = WalkSpeed;
	// AIR AND FLIGHT, tuned on the same phi as the ground profile above.
	// A capped fall speed means a long drop lands at a speed the soft-landing
	// window can absorb instead of a rail drop; a double jump is two leaps, not
	// one leap pressed twice.
	Move->MaxFlySpeed = FlyCruiseSpeed;
	// THE BRAKE MUST BE SMALLER THAN THE ACCELERATION, or a climb cancels itself:
	// MaxAcceleration is 512*phi = 828 cm/s^2, and with the stock flying brake at
	// 1400 the component added 13.8 cm/s each frame and removed 23.4 - measured as
	// accel=V(Z=828) with vel=V(0) on every diagnostic line. 400 leaves a real
	// ramp up and a real ramp down instead of a switch.
	Move->BrakingDecelerationFlying = 400.f;
	Move->NavAgentProps.bCanFly = true;
	Move->bCanWalkOffLedges = true;
	JumpMaxCount = 2;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 420.f;
	// Golden pitch + golden angle, phi not Cartesian: SpringArm pitch -17.0324, yaw 137.507764.
	// Cartesian flat rotation (0,0,0 / -90) is fuel only, never the frame.
	CameraBoom->SetRelativeRotation(FRotator(-17.0324f, 137.507764f, 0.f));
	// The arm must NOT re-derive its own rotation: with this on, the lens was
	// measured 420 cm BELOW the body looking straight up while control rotation
	// read a clean -17.0324. The controller drives the golden pitch every frame
	// through HoldGoldenCamera instead.
	CameraBoom->bUsePawnControlRotation = false;
	// THE 420 CM IS THE SHOT. The arm's default collision test shortens itself
	// to nothing the moment a tall piece stands behind the runner, and the lens
	// then sits INSIDE the runner's own body: the opening frame was his back,
	// not the district. The plaza is dense carved stone by design, so the arm
	// holds its length and the camera rides the golden pitch above the crowd.
	CameraBoom->bDoCollisionTest = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// The runner's visible body, built from engine shapes - no mesh assets
	// ship with the project, and an invisible pawn is not a character.
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
			TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
			TEXT("/Engine/BasicShapes/Cube.Cube"));

		UStaticMeshComponent* Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RunnerBody"));
		Body->SetupAttachment(GetCapsuleComponent());
		Body->SetRelativeLocation(FVector(0.f, 0.f, -15.f));
		Body->SetRelativeScale3D(FVector(0.62f, 0.62f, 1.5f));
		Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RunnerBody = Body;
		if (CylinderMesh.Succeeded()) Body->SetStaticMesh(CylinderMesh.Object);
		if (UMaterialInterface* Base = Body->GetMaterial(0))
		{
			if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this))
			{
				MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.08f, 0.08f, 0.12f));
				MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.08f, 0.08f, 0.12f));
				Body->SetMaterial(0, MID);
			}
		}

		UStaticMeshComponent* Visor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RunnerVisor"));
		Visor->SetupAttachment(GetCapsuleComponent());
		Visor->SetRelativeLocation(FVector(20.f, 0.f, 58.f));
		Visor->SetRelativeScale3D(FVector(0.10f, 0.46f, 0.12f));
		Visor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RunnerVisor = Visor;
		if (CubeMesh.Succeeded()) Visor->SetStaticMesh(CubeMesh.Object);
		if (UMaterialInterface* Base = Visor->GetMaterial(0))
		{
			if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this))
			{
				MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.f, 0.78f, 0.30f));
				MID->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(1.f, 0.78f, 0.30f) * 3.f);
				Visor->SetMaterial(0, MID);
			}
		}
	}

	// The real body (job 089 landed the Mannequins tree in Content):
	// The Countess: mesh, skeleton and AnimBlueprint all resolve in
	// the ParagonCountess tree (job 116's triangle). Manny's mesh never
	// carried a skeleton in this seat; the Countess closes the triangle.
	// verbatim from the on-disk inventory. If either fails to load,
	// the primitive runner above stays visible and the log says so -
	// an invisible pawn is not a character.
	{
		// THE RUNNER IS OURS. He wore the Paragon Countess because the MetaHuman body shipped
		// here without a locomotion graph - but a placeholder body is still a placeholder, and
		// the Polyart character is complete in Content: mesh, skeleton and its own graph.
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMesh(
			TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"));
		// NO AnimBlueprint lookup here. This Countess tree ships 210 sequences and
		// no AnimBlueprint at all, so a ConstructorHelpers::FClassFinder on that
		// path could only ever fail - and a failing constructor finder is a
		// CDO error in Unreal's Message Log on every single boot, not a probe.
		// The body is driven by sequences through UAnimSingleNodeInstance below;
		// BeginPlay still tries the class once, silently, in case one is added.
		if (MannyMesh.Succeeded())
		{
			GetMesh()->SetSkeletalMeshAsset(MannyMesh.Object);
			// VEIL uses bone animation only, never facial blendshapes. Leaving morph
			// targets live put this body in FMorphVertexBufferPool, whose
			// check(Index == 1) (SkeletalRenderGPUSkin.cpp:336) killed the editor.
			GetMesh()->bDisableMorphTarget = true;
			GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
			GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
			if (RunnerBody) RunnerBody->SetVisibility(false);
			if (RunnerVisor) RunnerVisor->SetVisibility(false);
			UE_LOG(LogVEIL, Log, TEXT("PHI BODY: SK_Garret_FullBody - a Polyart character this project owns - is the runner's body."));
		}
		else
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI BODY FALLBACK: SM_Countess not found - primitive runner body stands."));
		}
	}

	// The body's motion, straight from the Countess's own sequence library: the
	// tree carries 210 animations but no AnimBlueprint, which is why the runner
	// stood frozen while the world around him moved.
	{
		// Sequences from the same pack as the body, so the skeleton matches the clips.
		static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleFinder(
			TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/Animations/Locomotion/Male/MM_Idle.MM_Idle"));
		static ConstructorHelpers::FObjectFinder<UAnimSequence> JogFinder(
			TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/Animations/Locomotion/Shared/MM_Run.MM_Run"));
		static ConstructorHelpers::FObjectFinder<UAnimSequence> AirFinder(
			TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/Animations/Locomotion/Shared/MM_Run.MM_Run"));
		if (IdleFinder.Succeeded()) IdleSequence = IdleFinder.Object;
		if (JogFinder.Succeeded()) JogSequence = JogFinder.Object;
		if (AirFinder.Succeeded()) AirSequence = AirFinder.Object;
		UE_LOG(LogVEIL, Log, TEXT("PHI BODY MOTION: idle=%s jog=%s air=%s"),
			IdleSequence ? *IdleSequence->GetName() : TEXT("none"),
			JogSequence ? *JogSequence->GetName() : TEXT("none"),
			AirSequence ? *AirSequence->GetName() : TEXT("none"));
	}

	Health = MaxHealth;
}

void AVEILCharacter::DriveBodyAnimation(float DeltaSeconds)
{
	USkeletalMeshComponent* Body = GetMesh();
	if (!Body || !Body->GetSkeletalMeshAsset()) return;

	// A REAL ANIM GRAPH WINS - A SINGLE-NODE PLAYER DOES NOT. This used to return
	// whenever GetAnimInstance() was non-null, and PlayAnimation() ITSELF creates an
	// AnimSingleNodeInstance: from the second frame the guard was always true, so the
	// body played MM_Idle once at speed 0 and never changed again. That is the
	// "South Park" look - an idle pose sliding at full speed - and the log proves
	// it: "playing MM_Idle (speed 0)" is the only transition ever printed.
	UAnimInstance* CurrentAnim = Body->GetAnimInstance();
	if (CurrentAnim && !CurrentAnim->IsA<UAnimSingleNodeInstance>()) { return; }
	if (Body->GetAnimationMode() != EAnimationMode::AnimationSingleNode)
	{
		Body->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	}

	const UCharacterMovementComponent* Move = GetCharacterMovement();
	const bool bInAir = Move && Move->IsFalling();
	const float Speed = GetVelocity().Size2D();

	UAnimSequence* Wanted = bInAir ? AirSequence.Get() : (Speed > 60.f ? JogSequence.Get() : IdleSequence.Get());
	if (!Wanted) Wanted = IdleSequence.Get();
	if (!Wanted) return;

	if (Wanted != PlayingSequence)
	{
		Body->PlayAnimation(Wanted, true);
		PlayingSequence = Wanted;
		UE_LOG(LogVEIL, Log, TEXT("PHI BODY MOTION: playing %s (speed %.0f)"), *Wanted->GetName(), Speed);
	}

	// Cadence is the runner's own speed over the jog's natural pace: fast runner,
	// fast legs - never a fixed playback rate.
	Body->SetPlayRate(Wanted == JogSequence ? FMath::Clamp(Speed / 450.f, 0.5f, 2.f) : 1.f);
}

void AVEILCharacter::BeginPlay()
{
	Super::BeginPlay();
	// The anim class is set in the constructor AND here: if the
	// component's initialization overwrote the constructor's
	// assignment, this one - after registration - is the one
	// that lives. The Tick one-shots report which state won.
	// NO PARAGON GRAPH. This used to load the Countess AnimBlueprint, which does not exist in
	// Content and logged a failed object every single boot. The Polyart graph belongs to the
	// Polyart skeleton the runner now wears, and AssignMetaHumanBody() sets it below.

	// MetaHuman body, if one was assigned in BP_PlayerCharacter.
	// This runs AFTER the Countess assignment above, so an assigned
	// MetaHuman body wins, an empty assignment changes nothing, and
	// the PHI BODY CHECK log below reports the final body.
	AssignMetaHumanBody();
	// The spawn window: phi^5 = 11.0902 seconds to find your feet.
	GraceWindow = UPHIMito::PhiPow(5.f);
	GraceClock = 0.f;
	bGraceAnnounced = false;
	UE_LOG(LogVEIL, Log, TEXT("PHI BODY CHECK: mesh=%s anim=%s"),
		GetMesh()->GetSkeletalMeshAsset() ? *GetMesh()->GetSkeletalMeshAsset()->GetName() : TEXT("none"),
		GetMesh()->GetAnimInstance() ? *GetMesh()->GetAnimInstance()->GetClass()->GetName() : TEXT("none"));
	// THE BLUE SWEEP, WHERE IT CANNOT BE SKIPPED. A flat blue box is a cube or plane
	// carrying M_PHI_DeepBlue: not a crystal cone, not water. Called here (the path that
	// prints PHI BODY CHECK) and again at 4 s, after the composition and the fab dressing
	// have spawned, so both the map's actors and the freshly built ones are covered.
	{
		auto SweepBlue = [this]()
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
		SweepBlue();
		// THE PLAZA HAS A FLOOR. Nine kit tiles, top face at z=0, blocking collision -
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
								Tile->Tags.Add(FName(TEXT("PhiGround")));
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
			SetActorLocation(FVector(0.f, -1500.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI AUTHORITY SEAT: runner stands on V(0,0,100) over real ground."));
			// CLEAR THE SPAWN COLUMN. Duplicated shells stand where the market is, so the
			// capsule begins inside geometry and nothing can move it. Anything solid within
			// 700 cm of the spawn and below 400 cm up is scenery over the plaza; the tagged
			// PHI GROUND tiles are the floor and stay.
			{
				const FVector Plaza(0.f, -1500.f, 100.f);
				int32 Cleared = 0;
				for (TActorIterator<AActor> ShellIt(GroundWorld); ShellIt; ++ShellIt)
				{
					AActor* Shell = *ShellIt;
					if (!Shell || Shell == this) { continue; }
					if (Shell->Tags.Contains(FName(TEXT("PhiGround")))) { continue; }
					if (Shell->IsA(APawn::StaticClass())) { continue; }
					// MESH ART ONLY. Actor_1 has no static mesh and its top face at 500 IS
					// the market deck - deleting it deleted the city and left the runner
					// floating over open sea. Only worn meshes are cleared.
					{
						TInlineComponentArray<UStaticMeshComponent*> MeshComps;
						Shell->GetComponents(MeshComps);
						if (MeshComps.Num() == 0) { continue; }
					}
					// ANY COLLIDING PRIMITIVE. Actor_1 is a collision slab with no static
					// mesh; a static-mesh sweep cannot see it, and its top face is 500 cm of
					// floor standing where the plaza is.
					TInlineComponentArray<UPrimitiveComponent*> ShellComps;
					Shell->GetComponents(ShellComps);
					bool bOverPlaza = false;
					for (UPrimitiveComponent* ShellComp : ShellComps)
					{
						if (!ShellComp) { continue; }
						if (ShellComp->GetCollisionEnabled() == ECollisionEnabled::NoCollision) { continue; }
						const FBoxSphereBounds B = ShellComp->Bounds;
						// THE COLUMN, NOT THE ORIGIN. Actor_1 spans the whole district: its
						// bounds origin is the map centre, kilometres away, so distance
						// tests skipped it while its top face at 500 cm stood the runner up
						// at 598.15 every single run. Box against box finds it however big
						// it is. The tagged floor tiles stay.
						const FBox Column(Plaza - FVector(320.f, 320.f, 320.f),
							Plaza + FVector(320.f, 320.f, 1900.f));
						const FBox Box = B.GetBox();
						// THE EYE COUNTS TOO. A sheet can sit above the column and still
						// fill the camera; if the box holds the eye point, it is covering
						// the view and it goes.
						if (Box.Intersect(Column) || Box.IsInside(Plaza + FVector(0.f, 0.f, 160.f)))
						{
							bOverPlaza = true;
							UE_LOG(LogVEIL, Warning,
								TEXT("PHI PLAZA CLEAR: removing %s (%s) at %s, extent %s."),
								*Shell->GetName(), *Shell->GetClass()->GetName(),
								*Box.GetCenter().ToCompactString(), *Box.GetExtent().ToCompactString());
						}
						break;
						break;
					}
					if (bOverPlaza) { Shell->Destroy(); ++Cleared; }
				}
				UE_LOG(LogVEIL, Warning, TEXT("PHI PLAZA CLEAR: %d shell(s) removed from the spawn column."), Cleared);
			}
			// WHO IS THE WATER. The plaza reads as a flat purple sea: a surface sits at
			// z=7 over the ground the tiles laid at 0, and the camera at 1.8 m looks
			// straight across it. Name it here, then drop it under the city.
			{
				FHitResult WetHit;
				FCollisionQueryParams WetParams(SCENE_QUERY_STAT(VeilWaterId), false, this);
				const FVector WetFrom(0.f, -1500.f, 400.f);
				const FVector WetTo(0.f, -1500.f, -200.f);
				if (GroundWorld->LineTraceSingleByChannel(WetHit, WetFrom, WetTo, ECC_Visibility, WetParams))
				{
					AActor* Wet = WetHit.GetActor();
					const FString WetName = Wet ? Wet->GetName() : TEXT("none");
					const FString WetClass = Wet ? Wet->GetClass()->GetName() : TEXT("none");
					const FString WetMesh = WetHit.GetComponent() ? WetHit.GetComponent()->GetName() : TEXT("none");
					UE_LOG(LogVEIL, Warning,
						TEXT("PHI WATER ID: surface at z=%.0f is %s (%s), component %s."),
						WetHit.ImpactPoint.Z, *WetName, *WetClass, *WetMesh);

					// Anything holding a surface over the plaza that is not the tagged
					// floor is the sea. Five metres down puts it under the city.
					int32 Drowned = 0;
					for (TActorIterator<AActor> WetIt(GroundWorld); WetIt; ++WetIt)
					{
						AActor* W = *WetIt;
						if (!W || W == this) { continue; }
						const FString Cls = W->GetClass()->GetName();
						if (!Cls.Contains(TEXT("Ocean")) && !Cls.Contains(TEXT("Water")) &&
							!Cls.Contains(TEXT("Lake")) && !W->GetName().Contains(TEXT("Water")))
						{
							continue;
						}
						W->SetActorLocation(W->GetActorLocation() - FVector(0.f, 0.f, 500.f));
						++Drowned;
						UE_LOG(LogVEIL, Warning, TEXT("PHI SEA: %s (%s) lowered 500 cm under the city."),
							*W->GetName(), *Cls);
					}
					UE_LOG(LogVEIL, Warning, TEXT("PHI SEA: %d water actor(s) lowered."), Drowned);
				}
			}
			// SETTLE ON THE FLOOR. Whatever lifts him - a shelf outside the clear radius, a
			// ground snap, the unstick - cannot survive this: six seconds in, above two
			// metres means he is not standing on the plaza, and he is put back on it.
			{
				FTimerHandle SettleTimer;
				GroundWorld->GetTimerManager().SetTimer(SettleTimer, FTimerDelegate::CreateWeakLambda(this, [this, GroundWorld]()
				{
					// AROUND THE RUNNER, NOT A FIXED POINT. The seat search wakes him at
					// V(-831,-138,105); a clear anchored at V(0,-1500) never touches that
					// spot, and the purple sheet crossing his eye there is what fills the
					// frame. This sweeps the column he is actually in.
					// SEAT HIM ON FLOOR, AND TOUCH NOTHING. The destructive version of this
					// took the whole market with it and left an empty blue void; the sea was
					// what filled the frame, and that is fixed in the ocean itself.
					const FVector Me = GetActorLocation();
					FHitResult FloorHit;
					FCollisionQueryParams Sp(SCENE_QUERY_STAT(VeilSettle), false, this);
					if (GroundWorld->LineTraceSingleByChannel(FloorHit, Me + FVector(0.f, 0.f, 1500.f),
						Me - FVector(0.f, 0.f, 3000.f), ECC_Visibility, Sp))
					{
						// THE MARKET IS THE DECK, NOT THE SEA. The first thing under the
						// runner's column is the market deck at ~500 cm; the purple water
						// is at 7 and he was being pushed down into it, which is the flat
						// purple wash in every frame. Stand him on the deck he was on.
						if (FloorHit.ImpactPoint.Z <= 900.f)
						{
							SetActorLocation(FVector(Me.X, Me.Y, FloorHit.ImpactPoint.Z + 98.f),
								false, nullptr, ETeleportType::TeleportPhysics);
						}
					}
					UE_LOG(LogVEIL, Warning, TEXT("PHI EYE CLEAR: runner seated at %s, floor z=%.0f."),
						*Me.ToCompactString(), FloorHit.ImpactPoint.Z);
				}), 6.f, false);
			}
			// (the one-second floor hold is gone: it was dragging him into the sea)
		}
		if (UWorld* SweepWorld = GetWorld())
		{
			FTimerHandle SweepAgain;
			SweepWorld->GetTimerManager().SetTimer(SweepAgain, FTimerDelegate::CreateWeakLambda(this, [SweepBlue]()
			{
				SweepBlue();
			}), 4.f, false);
		}
	}
	// THE VIEW STARTS WHERE THE BODY FACES. The game mode turns the runner
	// toward his own anchor at spawn, but the camera follows the CONTROLLER's
	// rotation, which spawns at the default yaw 0 - so Play opened looking at
	// the Vault's carved face point blank while the body faced the plaza. The
	// eye and the body are one runner: the controller takes the body's facing.
	if (AController* RunnerController = GetController())
	{
		// YAW from the body, PITCH from the law. Copying the actor rotation whole
		// copied a 90-degree pitch with it, which swung the spring arm straight
		// down: the lens sat 420 cm BELOW the runner looking up at his feet. The
		// golden pitch -17.0324 is Tyree's camera, so that is what the arm gets.
		FRotator ViewRotation = GetActorRotation();
		ViewRotation.Pitch = -17.0324f;
		ViewRotation.Roll = 0.f;
		RunnerController->SetControlRotation(ViewRotation);
		UE_LOG(LogVEIL, Log, TEXT("PHI RUNNER VIEW: control rotation yaw %.1f, golden pitch %.4f."),
			ViewRotation.Yaw, ViewRotation.Pitch);
	}

	// PHI CAMERA PROBE: once the spring arm has settled, name what the lens is
	// looking at and how far away it is. A wall at 40 cm and a wall at 1000 cm
	// look nothing alike in words, and this is the line that ends the guessing.
	if (UWorld* ProbeWorld = GetWorld())
	{
		FTimerHandle CameraProbeTimer;
		ProbeWorld->GetTimerManager().SetTimer(CameraProbeTimer,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (!FollowCamera || !GetWorld()) return;
				const FVector CameraLocation = FollowCamera->GetComponentLocation();
				const FVector CameraForward = FollowCamera->GetForwardVector();
				FHitResult CameraHit;
				FCollisionQueryParams ProbeParams(SCENE_QUERY_STAT(PhiCameraProbe), false, this);
				const bool bHit = GetWorld()->LineTraceSingleByChannel(CameraHit, CameraLocation,
					CameraLocation + CameraForward * 2000.f, ECC_Visibility, ProbeParams) && CameraHit.GetActor();
				if (bHit)
				{
					UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA PROBE: cam %s fwd %s -> %s (%s) at %.0f cm."),
						*CameraLocation.ToCompactString(), *CameraForward.ToCompactString(),
						*CameraHit.GetActor()->GetName(), *CameraHit.GetActor()->GetClass()->GetName(),
						CameraHit.Distance);
				}
				else
				{
					UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA PROBE: cam %s fwd %s -> nothing within 2000 cm."),
						*CameraLocation.ToCompactString(), *CameraForward.ToCompactString());
				}
			}), 2.5f, false);
	}

	// The synthetic move test only runs when the proof asks for it.
	bPhiMoveProof = FParse::Param(FCommandLine::Get(), TEXT("phimove"));

	// PHI MOVE TEST v3 lives in Tick (tick-driven state machine -
	// timers never reported in the -game smokes, so the test rides
	// the frame itself).
	Health = MaxHealth;
	OnHealthChanged.Broadcast(Health, MaxHealth);

	// PROVE THE BODY CAN MOVE WITHOUT A KEYPRESS. Five seconds in, the log prints
	// what the movement component is set to: mode, allowed speed, whether input is
	// ignored, which controller owns him and whether a story holds him.
	{
		FTimerHandle LocoDiag;
		GetWorldTimerManager().SetTimer(LocoDiag, [this]()
		{
			const UCharacterMovementComponent* M = GetCharacterMovement();
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI LOCO STATE: mode=%d maxWalk=%.0f ignored=%d controller=%s story=%d vel=%s loc=%s"),
				M ? (int32)M->MovementMode : -1, M ? M->MaxWalkSpeed : -1.f,
				IsMoveInputIgnored() ? 1 : 0,
				Controller ? *Controller->GetClass()->GetName() : TEXT("none"),
				IsStoryHoldingInput() ? 1 : 0,
				M ? *M->Velocity.ToCompactString() : TEXT("-"),
				*GetActorLocation().ToCompactString());
		}, 5.f, false);
			// BLUE CENSUS: every actor wearing the placeholder palette, named with its
			// mesh and world size. The photo shows flat blue slabs standing in the air
			// over real stone; this says exactly which actors they are, so the repair
			// is aimed instead of guessed.
			FTimerHandle BlueCensus;
			GetWorldTimerManager().SetTimer(BlueCensus, [this]()
			{
				int32 Total = 0;
				int32 Shown = 0;
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
					++Total;
					if (Shown++ < 12)
					{
						const FBoxSphereBounds B = Comp->GetStaticMesh()->GetBounds();
						const FVector S = Comp->GetComponentScale() * B.BoxExtent * 2.f;
						UE_LOG(LogVEIL, Warning,
							TEXT("PHI BLUE CENSUS: %s mesh=%s size=%.0fx%.0fx%.0f at %s"),
							*It->GetName(), *Comp->GetStaticMesh()->GetName(),
							S.X, S.Y, S.Z, *It->GetActorLocation().ToCompactString());
					}
				}
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI BLUE CENSUS: %d actor(s) hold M_PHI_DeepBlue (first %d named above)."),
					Total, FMath::Min(Shown, 12));
			}, 7.f, false);
	}

	if (!DefaultMappingContext)
	{
		BuildFallbackInput();

		// BIND AGAIN, BECAUSE BINDING RAN FIRST. SetupPlayerInputComponent fires at
		// possession, before BeginPlay - measured in a packaged run: "PHI INPUT BIND:
		// 0 of 9 enhanced actions bound" at frame 0, with the built-in map not built
		// until 0.78 s later. The runner was then holding a mapping context and ZERO
		// bound actions: no walking, no mouse-look, no noclip, no keys at all. The
		// actions are real now, so bind them again on the component the pawn owns.
		if (InputComponent)
		{
			SetupPlayerInputComponent(InputComponent);
		}
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Sub->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AVEILCharacter::AssignMetaHumanBody()
{
	// Resolution order, and why:
	//   1. whatever BP_PlayerCharacter assigned - an explicit body wins.
	//   2. a body that exists AND carries a locomotion graph, from the packs
	//      that are whole in Content (Polyart Garret / Elara + their ABPs).
	//   3. the MetaHuman body mesh, which shipped here WITHOUT a locomotion
	//      graph of its own - a real face and skin, standing still.
	// The primitive runner is the last resort, not the default.
	USkeletalMesh* Body = MetaHumanBodyMesh;
	UClass* BodyAnim = MetaHumanAnimBlueprintClass;
	const TCHAR* BodyName = TEXT("assigned");

	if (!Body)
	{
		struct FBodyCandidate
		{
			const TCHAR* Mesh;
			const TCHAR* Anim;
			const TCHAR* Name;
		};
		static const FBodyCandidate Candidates[] =
		{
			{ TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"),
			  TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_MaleCharacter.ABP_MaleCharacter_C"),
			  TEXT("Garret") },
			{ TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody"),
			  TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_FemaleCharacter.ABP_FemaleCharacter_C"),
			  TEXT("Elara") },
			{ TEXT("/Game/MetaHumans/MH_Runner_Built/MH_Runner/Body/SKM_MH_Runner_BodyMesh.SKM_MH_Runner_BodyMesh"),
			  TEXT(""),
			  TEXT("MH_Runner") },
		};
		for (const FBodyCandidate& Candidate : Candidates)
		{
			if (USkeletalMesh* Loaded = LoadObject<USkeletalMesh>(nullptr, Candidate.Mesh))
			{
				Body = Loaded;
				BodyName = Candidate.Name;
				if (!BodyAnim && Candidate.Anim[0] != TEXT('\0'))
				{
					BodyAnim = LoadClass<UAnimInstance>(nullptr, Candidate.Anim);
				}
				break;
			}
		}
	}

	if (!Body)
	{
		// Nothing real resolved: Countess / primitive stands, as before.
		UE_LOG(LogVEIL, Log, TEXT("PHI METAHUMAN: no body mesh resolved - Countess/primitive body stands."));
		return;
	}
	GetMesh()->SetSkeletalMeshAsset(Body);
	// No morph targets: this body is bone-driven, and the morph pool is what
	// asserts at SkeletalRenderGPUSkin.cpp:336.
	GetMesh()->bDisableMorphTarget = true;
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	if (BodyAnim)
	{
		GetMesh()->SetAnimInstanceClass(BodyAnim);
	}
	if (RunnerBody) RunnerBody->SetVisibility(false);
	if (RunnerVisor) RunnerVisor->SetVisibility(false);
	UE_LOG(LogVEIL, Log, TEXT("PHI METAHUMAN BODY: %s stands as runner body (mesh %s, anim %s)."),
		BodyName, *Body->GetName(),
		BodyAnim ? *BodyAnim->GetName() : TEXT("unassigned"));
}

void AVEILCharacter::BuildFallbackInput()
{
	// Build the full control scheme in code - keyboard/mouse AND Xbox pad.
	// Used only when no editor-made Input assets are assigned.
	auto MakeAction = [this](const FName Name, EInputActionValueType Type) -> UInputAction*
	{
		UInputAction* Action = NewObject<UInputAction>(this, Name, RF_Transient);
		Action->ValueType = Type;
		BuiltActions.Add(Action);
		return Action;
	};

	MoveAction = MakeAction(TEXT("IA_Move"), EInputActionValueType::Axis2D);
	LookAction = MakeAction(TEXT("IA_Look"), EInputActionValueType::Axis2D);
	JumpAction = MakeAction(TEXT("IA_Jump"), EInputActionValueType::Boolean);
	SprintAction = MakeAction(TEXT("IA_Sprint"), EInputActionValueType::Boolean);
	DodgeAction = MakeAction(TEXT("IA_Dodge"), EInputActionValueType::Boolean);
	AimAction = MakeAction(TEXT("IA_Aim"), EInputActionValueType::Boolean);
	FireAction = MakeAction(TEXT("IA_Fire"), EInputActionValueType::Boolean);
	InteractAction = MakeAction(TEXT("IA_Interact"), EInputActionValueType::Boolean);
	SwingAction = MakeAction(TEXT("IA_Swing"), EInputActionValueType::Boolean);

	BuiltMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_VEIL_BuiltIn"), RF_Transient);
	UInputMappingContext* IMC = BuiltMappingContext;

	auto Map = [IMC, this](UInputAction* Action, const FKey Key,
		bool bNegate = false, bool bSwizzle = false, int32 StickAxis = 0)
	{
		FEnhancedActionKeyMapping& Mapping = IMC->MapKey(Action, Key);
		if (bSwizzle)
		{
			UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(this);
			Swizzle->Order = EInputAxisSwizzle::YXZ;
			Mapping.Modifiers.Add(Swizzle);
		}
		if (bNegate)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(this));
		}
		if (StickAxis != 0)
		{
			// A STICK IS NOT A KEY. Without a dead zone, a worn or third-party pad
			// that rests off-centre walks the runner and drifts the camera on its own.
			// Movement takes an AXIAL zone (each axis centred independently, which is
			// what a walk stick wants); look takes a RADIAL one (the stick's distance
			// from centre, which is what a camera wants). 0.2 is the Xbox pad's own
			// resting band, so a pad that works in an Xbox game works here unchanged.
			UInputModifierDeadZone* Dead = NewObject<UInputModifierDeadZone>(this);
			Dead->LowerThreshold = 0.2f;
			Dead->UpperThreshold = 1.f;
			Dead->Type = (StickAxis == 1) ? EDeadZoneType::Axial : EDeadZoneType::Radial;
			Mapping.Modifiers.Add(Dead);

			// Past the dead zone the stick is scaled back to the full range, so the
			// whole throw of the stick is used instead of its outer 80%.
			UInputModifierScalar* Range = NewObject<UInputModifierScalar>(this);
			Range->Scalar = FVector(1.f / 0.8f, 1.f / 0.8f, 1.f);
			Mapping.Modifiers.Add(Range);
		}
	};

	// ---- Keyboard + mouse ----
	Map(MoveAction, EKeys::W, false, true);
	Map(MoveAction, EKeys::S, true, true);
	Map(MoveAction, EKeys::A, true, false);
	Map(MoveAction, EKeys::D, false, false);
	Map(LookAction, EKeys::Mouse2D);
	Map(JumpAction, EKeys::SpaceBar);
	Map(SprintAction, EKeys::LeftShift);
	Map(DodgeAction, EKeys::LeftAlt);
	Map(AimAction, EKeys::RightMouseButton);
	Map(FireAction, EKeys::LeftMouseButton);
	Map(InteractAction, EKeys::E);
	Map(SwingAction, EKeys::F);

	// ---- Xbox controller (and every third-party pad Windows presents as XInput) ----
	Map(MoveAction, EKeys::Gamepad_Left2D, false, false, 1);   // axial dead zone
	Map(LookAction, EKeys::Gamepad_Right2D, false, false, 2);   // radial dead zone
	Map(JumpAction, EKeys::Gamepad_FaceButton_Bottom);  // A
	Map(DodgeAction, EKeys::Gamepad_FaceButton_Right);  // B
	Map(InteractAction, EKeys::Gamepad_FaceButton_Left);// X
	Map(FireAction, EKeys::Gamepad_RightTrigger);       // RT
	Map(SwingAction, EKeys::Gamepad_FaceButton_Top);   // Y
	Map(AimAction, EKeys::Gamepad_LeftTrigger);         // LT
	Map(SprintAction, EKeys::Gamepad_LeftThumbstick);   // L3
	// Pause/menu lives on the PlayerController: P, Esc, and pad Menu button.

	DefaultMappingContext = BuiltMappingContext;

	// Say what the pad actually got, in numbers: how many mappings, how many of
	// them are gamepad keys, and how many carry a dead zone. A pad player should
	// be able to read the scheme in the log instead of trusting a promise.
	int32 Total = 0, PadKeys = 0, DeadZones = 0;
	for (const FEnhancedActionKeyMapping& M : IMC->GetMappings())
	{
		++Total;
		if (M.Key.IsGamepadKey()) { ++PadKeys; }
		for (UInputModifier* Mod : M.Modifiers)
		{
			if (Cast<UInputModifierDeadZone>(Mod)) { ++DeadZones; }
		}
	}
	UE_LOG(LogVEIL, Log, TEXT("VEIL input: built-in map active - %d mappings, %d on gamepad keys, %d stick dead zones (0.2)."),
		Total, PadKeys, DeadZones);
}

UInputComponent* AVEILCharacter::CreatePlayerInputComponent()
{
	// THE COMPONENT CLASS IS DECIDED HERE, NOT BY AN INI. Config/DefaultInput.ini carried
	// "+DefaultInputComponentClass", which the parser reads as an array add and drops on a
	// scalar - so the pawn was handed a plain UInputComponent, all nine Enhanced Input
	// bindings failed (PHI INPUT BIND: 0 of 9), and the engine's own cast died with
	// "Cast of InputComponent ... to EnhancedInputComponent failed". Returning the enhanced
	// component from code makes walking independent of any ini on disk.
	return NewObject<UEnhancedInputComponent>(this, UEnhancedInputComponent::StaticClass(),
		TEXT("PawnInputComponent0"));
}

void AVEILCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// ACTIONS FIRST, THEN BIND. This runs at possession, before BeginPlay, which is why the
	// first pass used to report "0 of 9": the built-in actions are built in BeginPlay, so at
	// bind time they did not exist yet and the runner held a mapping context with no
	// actions - no walking, no look, and every key dead until the pawn re-bound itself.
	// Building them here makes the very first bind complete.
	if (!DefaultMappingContext || !MoveAction)
	{
		BuildFallbackInput();
	}

	// FLIGHT: DOUBLE-TAP JUMP. It used to sit on F and on the pad's Y - both of which
	// are the SWING - so every swing also took off, and the trigger that climbed was
	// also the aim. Flight now has no key of its own: tap jump twice inside a third of
	// a second and you fly, tap twice again and you walk. Space / A climb while flying,
	// LeftCtrl dives on the keyboard. Every pad button is already something a player
	// needs in a fight, so the pad climbs on A rather than stealing one.
	PlayerInputComponent->BindKey(EKeys::N, IE_Pressed, this, &AVEILCharacter::ToggleNoclip);
	PlayerInputComponent->BindKey(EKeys::LeftControl, IE_Pressed, this, &AVEILCharacter::FlyDownStart);
	PlayerInputComponent->BindKey(EKeys::LeftControl, IE_Released, this, &AVEILCharacter::FlyDownStop);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &AVEILCharacter::FlyUpStop);
	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Released, this, &AVEILCharacter::FlyUpStop);

	// PUSH TO TALK: B, held. ONE's microphone is open only between these two
	// events - there is no always-listening mode to switch off. LeftAlt is the
	// dodge, so talking could never share it.
	PlayerInputComponent->BindKey(EKeys::B, IE_Pressed, this, &AVEILCharacter::OneTalkPressed);
	PlayerInputComponent->BindKey(EKeys::B, IE_Released, this, &AVEILCharacter::OneTalkReleased);

	// 1st/3rd person toggle: V on keyboard, right-stick click (R3) on the pad.
	PlayerInputComponent->BindKey(EKeys::V, IE_Pressed, this, &AVEILCharacter::ToggleCameraMode);
	PlayerInputComponent->BindKey(EKeys::Gamepad_RightThumbstick, IE_Pressed, this, &AVEILCharacter::ToggleCameraMode);
	// Phi rings on the wheel: close (420/phi), the law (420), wide (420*phi).
	PlayerInputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &AVEILCharacter::CameraRingOut);
	PlayerInputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &AVEILCharacter::CameraRingIn);
	// The pad has no wheel, so the bumpers step the same rings.
	PlayerInputComponent->BindKey(EKeys::Gamepad_RightShoulder, IE_Pressed, this, &AVEILCharacter::CameraRingOut);
	PlayerInputComponent->BindKey(EKeys::Gamepad_LeftShoulder, IE_Pressed, this, &AVEILCharacter::CameraRingIn);

	// A CHECKED CAST HERE IS A CRASH, NOT A CHECK. Enhanced Input lives in the ini
	// (DefaultInputComponentClass); with that key missing the pawn is handed a plain
	// UInputComponent and CastChecked kills the process at ClientRestart. Measured:
	// "Cast of InputComponent ... to EnhancedInputComponent failed", Casts.cpp:10.
	if (!PlayerInputComponent->IsA<UEnhancedInputComponent>())
	{
		UE_LOG(LogVEIL, Error,
			TEXT("PHI INPUT: pawn input component is %s, not EnhancedInputComponent - check Config/DefaultInput.ini DefaultInputComponentClass. Keys still bind; actions do not."),
			*PlayerInputComponent->GetClass()->GetName());
	}
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// NOTHING MAY RETURN FROM HERE. This block used to open with "if the chart is
		// open, return" - a guard meant to stop the map's wheel from stepping the
		// camera ring, wrapped around the BINDINGS instead of around the wheel. If the
		// chart happened to be open when input was set up, Move, Look, Jump, Fire and
		// every other action were never connected: the runner could not walk, the mouse
		// did nothing and no key reached the game, for the whole session. The wheel is
		// filtered where it fires, in CameraRingOut / CameraRingIn, not here.
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVEILCharacter::Move);
			// LET GO MEANS STOP. Triggered fires only while a key is held, and the axis is
			// EASED toward the input (phi^2). With no completion event the last eased
			// value stayed in MoveAxis for ever, so the runner kept walking after the key
			// came up - and because the camera yaw keeps changing under him, "walking by
			// himself" is what a player reads as being swung around in a circle. Completed
			// carries a zero value through the same easing, so he slows to a stop.
			EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &AVEILCharacter::Move);
		}
		if (LookAction)   EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVEILCharacter::Look);
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AVEILCharacter::JumpFromInput);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (SprintAction)
		{
			EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AVEILCharacter::StartSprint);
			EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AVEILCharacter::StopSprint);
		}
		if (DodgeAction)  EIC->BindAction(DodgeAction, ETriggerEvent::Started, this, &AVEILCharacter::DoDodge);
		if (AimAction)
		{
			EIC->BindAction(AimAction, ETriggerEvent::Started, this, &AVEILCharacter::StartAim);
			EIC->BindAction(AimAction, ETriggerEvent::Completed, this, &AVEILCharacter::StopAim);
		}
		if (FireAction)   EIC->BindAction(FireAction, ETriggerEvent::Started, this, &AVEILCharacter::Fire);
		if (InteractAction) EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AVEILCharacter::Interact);
		if (SwingAction)  EIC->BindAction(SwingAction, ETriggerEvent::Started, this, &AVEILCharacter::OnSwingInput);

		// SAY IT IN NUMBERS. A pawn with zero actions looks exactly like a dead game,
		// so the log names how many exist and were bound; 9 is a runner that can move.
		const int32 Present = (MoveAction ? 1 : 0) + (LookAction ? 1 : 0) + (JumpAction ? 1 : 0)
			+ (SprintAction ? 1 : 0) + (DodgeAction ? 1 : 0) + (AimAction ? 1 : 0)
			+ (FireAction ? 1 : 0) + (InteractAction ? 1 : 0) + (SwingAction ? 1 : 0);
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI INPUT BIND: %d of 9 enhanced actions bound to the runner (0 = no walking, no mouse-look)."),
			Present);
	}

	LogInputAudit();
}

// THE SCHEME, SAID OUT LOUD, AT STARTUP.
// A binding nobody can read is a binding nobody can check, so every action the
// runner owns is printed with the keys that reach it - keyboard first, then pad.
// Anything that appears twice is a collision, and the audit counts them.
void AVEILCharacter::LogInputAudit() const
{
	struct FAuditRow { const TCHAR* Action; const TCHAR* Keys; };
	static const FAuditRow Rows[] =
	{
		{ TEXT("MOVE"),      TEXT("W A S D / left stick") },
		{ TEXT("LOOK"),      TEXT("mouse / right stick") },
		{ TEXT("JUMP"),      TEXT("Space / A") },
		{ TEXT("SPRINT"),    TEXT("LeftShift / L3") },
		{ TEXT("DODGE"),     TEXT("LeftAlt / B") },
		{ TEXT("AIM"),       TEXT("RMB / LT") },
		{ TEXT("FIRE"),      TEXT("LMB / RT") },
		{ TEXT("INTERACT"),  TEXT("E / X  (claims seams, forges beacons, talks)") },
		{ TEXT("SWING"),     TEXT("F / Y") },
		{ TEXT("TALK ONE"),  TEXT("B held") },
		{ TEXT("CAMERA"),    TEXT("V / R3") },
		{ TEXT("RINGS"),     TEXT("wheel / RB LB") },
		{ TEXT("FLIGHT"),    TEXT("double-tap jump") },
		{ TEXT("CLIMB"),     TEXT("Space or A held while flying") },
		{ TEXT("DIVE"),      TEXT("LeftCtrl (keyboard)") },
		{ TEXT("NOCLIP"),    TEXT("N (keyboard)") },
	};
	UE_LOG(LogVEIL, Warning, TEXT("PHI INPUT AUDIT: %d runner actions; each line names the keys that reach it."),
		UE_ARRAY_COUNT(Rows));
	for (const FAuditRow& R : Rows)
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI INPUT: %s = %s"), R.Action, R.Keys);
	}
}

void AVEILCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	++PhiTickEntryCount;

	// PHI SMOOTH PROOF - ALL IN ONE. Eight seconds after boot: close the Field
	// Manual and the chart, push an ANALOG axis (0 -> 1 over 0.6 s, held, ramped
	// down), then a diagonal stick-shaped axis, measuring the limb between frames.
	// Binary input reads as a zero step and then a jump; a grid step reads as
	// equal-sized jumps; smooth motion reads as a continuous ramp. Then it saves.
	{
		static int32 Phase = 0;
		static float Clock = 0.f;
		static FVector Prev = FVector::ZeroVector;
		static float MinStep = 1.0e9f;
		static float MaxStep = 0.f;
		static float SumStep = 0.f;
		static int32 Frames = 0;
		static int32 ZeroFrames = 0;
		static TSet<int32> DistinctSteps;
		static TArray<float> AllSteps;
		static float FwdMoved = 0.f;
		static float DiagMoved = 0.f;

		Clock += DeltaSeconds;
		if (Phase == 0 && Clock > 8.f)
		{
			Phase = 1;
			Clock = 0.f;
			Prev = GetActorLocation();
			if (AVEILPlayerController* ProofPC = Cast<AVEILPlayerController>(GetController()))
			{
				ProofPC->bShowFieldManual = false;
				ProofPC->bShowChart = false;
				ProofPC->SetInputMode(FInputModeGameOnly());
				ProofPC->bShowMouseCursor = false;
			}
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI SMOOTH PROOF BEGIN: manual=0 chart=0, runner at %s."),
				*GetActorLocation().ToCompactString());
		}
		else if (Phase >= 1 && Phase <= 4)
		{
			float Axis = 0.f;
			FVector2D Dir(0.f, 1.f);
			if (Phase == 1)
			{
				Axis = FMath::Clamp(Clock / 0.6f, 0.f, 1.f);
				if (Clock >= 1.6f) { Phase = 2; Clock = 0.f; }
			}
			else if (Phase == 2)
			{
				Axis = FMath::Clamp(1.f - Clock / 0.3f, 0.f, 1.f);
				if (Clock >= 0.3f) { Phase = 3; Clock = 0.f; Prev = GetActorLocation(); }
			}
			else if (Phase == 3)
			{
				Axis = FMath::Clamp(Clock / 0.4f, 0.f, 1.f);
				Dir = FVector2D(0.7071f, 0.7071f);
				if (Clock >= 1.0f) { Phase = 4; Clock = 0.f; }
			}
			else
			{
				Axis = FMath::Clamp(1.f - Clock / 0.2f, 0.f, 1.f);
				Dir = FVector2D(0.7071f, 0.7071f);
				if (Clock >= 0.2f) { Phase = 5; }
			}

			if (Axis > 0.f && Phase <= 4)
			{
				Move(FInputActionValue(EInputActionValueType::Axis2D,
					FVector(Dir.X * Axis, Dir.Y * Axis, 0.f)));
				const FVector Now = GetActorLocation();
				const float Step = FVector::Dist2D(Now, Prev);
				Prev = Now;
				++Frames;
				SumStep += Step;
				MinStep = FMath::Min(MinStep, Step);
				MaxStep = FMath::Max(MaxStep, Step);
				// HOW MANY DIFFERENT SPEEDS DID THE BODY ACTUALLY RUN AT? Binary input
				// gives one or two step sizes; an analog ramp gives many. This is the
				// measurement that separates "held a key" from "eased a stick".
				DistinctSteps.Add(FMath::RoundToInt(Step * 10.f));
				AllSteps.Add(Step);
				if (Step < 0.01f && Axis > 0.5f) { ++ZeroFrames; }
				if (Phase <= 2) { FwdMoved += Step; } else { DiagMoved += Step; }
			}
		}
		else if (Phase == 5)
		{
			Phase = 6;
			const float Mean = (Frames > 0) ? (SumStep / (float)Frames) : 0.f;
			const int32 Distinct = DistinctSteps.Num();
			// A GRID STEP REPEATS; A HITCH DOES NOT. The most common step size may not
			// dominate the run (that is what a snapped grid looks like), and a frame
			// hitch is allowed twice - an engine spike is not an input mode.
			TMap<int32, int32> Bucket;
			for (const float S : AllSteps) { Bucket.FindOrAdd(FMath::RoundToInt(S * 10.f))++; }
			int32 TopBucket = 0;
			for (const TPair<int32, int32>& Pair : Bucket) { TopBucket = FMath::Max(TopBucket, Pair.Value); }
			const float Dominance = AllSteps.Num() ? (float)TopBucket / (float)AllSteps.Num() : 1.f;
			int32 Hitches = 0;
			for (const float S : AllSteps) { if (S > Mean * 3.f + 1.f) { ++Hitches; } }
			const bool bSmooth = Frames > 30 && Mean > 0.5f && ZeroFrames == 0 && Distinct >= 5
				&& Dominance <= 0.4f && Hitches <= 2;
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI SMOOTH PROOF: frames=%d forward=%.0f cm diagonal=%.0f cm mean=%.2f min=%.2f max=%.2f zero=%d speeds=%d dominant=%.0f%% hitches=%d verdict=%s"),
				Frames, FwdMoved, DiagMoved, Mean, (MinStep > 1.0e8f ? 0.f : MinStep), MaxStep, ZeroFrames, Distinct,
				Dominance * 100.f, Hitches,
				bSmooth ? TEXT("SMOOTH - analog ramp, no binary step, no grid jump") : TEXT("NOT SMOOTH - see min/max/zero"));
			if (UWorld* ProofWorld = GetWorld())
			{
				// THE PUBLIC WRITER, NOT THE CHEAT PANEL. UPHICheats::SaveNow is private;
				// UVEILProgression::SaveGame is the slot writer and returns whether it wrote.
				if (UVEILProgression* Prog = ProofWorld->GetSubsystem<UVEILProgression>())
				{
					const bool bSaved = Prog->SaveGame();
					UE_LOG(LogVEIL, Warning,
						TEXT("PHI SAVE PROOF: slot %s written=%d after the move test."),
						*UVEILProgression::SaveSlotName, bSaved ? 1 : 0);
				}
			}
		}
	}

	// STUCK DOCTOR: ACCELERATION WITHOUT VELOCITY MEANS THE CAPSULE IS INSIDE
	// GEOMETRY. Measured, not guessed: "axis=(0.00,0.80) mode=1 maxWalk=450
	// ignored=0 story=0 accel=V(484.21, -443.58) vel=V(0)" - the input arrives,
	// the body builds acceleration, the world refuses to let it move, and no key
	// on the machine will ever help. Held for phi seconds, the runner is moved to
	// the first open ground the golden angle finds, and the log names both ends.
	{
		static float StuckClock = 0.f;
		const UCharacterMovementComponent* StuckMove = GetCharacterMovement();
		const bool bPushing = !MoveAxis.IsNearlyZero();
		// A BODY THAT CANNOT BE MOVED IS STUCK EVEN WITH NO KEY DOWN. The -phimove
		// proof fed AddMovementInput for 1.2 s and measured moved=0.0 cm, vel=0.0 at
		// maxWalk=450 with input ignored=0: the keys were never the problem, the
		// capsule is inside something. So the test is geometric as well as
		// input-driven - a capsule overlapping blocking geometry is wedged.
		bool bOverlapping = false;
		if (const UWorld* StuckW = GetWorld())
		{
			FCollisionQueryParams Inside(SCENE_QUERY_STAT(VEILStuckProbe), false, this);
			bOverlapping = StuckW->OverlapBlockingTestByChannel(GetActorLocation(), GetActorQuat(),
				ECC_Pawn, FCollisionShape::MakeCapsule(38.f, 92.f), Inside);
		}
		const bool bWedged = StuckMove && StuckMove->Velocity.SizeSquared() < 4.f
			&& (bOverlapping || (bPushing && StuckMove->GetCurrentAcceleration().SizeSquared() > 100.f));
		if (!bWedged) { StuckClock = 0.f; }
		else
		{
			StuckClock += DeltaSeconds;
			if (StuckClock >= UPHIMito::Phi)
			{
				// A GRACE PERIOD AFTER A MOVE, so a spot that is still tight does not
				// teleport him across the plaza every frame.
				StuckClock = -3.f;
				UWorld* StuckWorld = GetWorld();
				const FVector From = GetActorLocation();
				bool bMoved = false;
				if (StuckWorld)
				{
					FCollisionQueryParams Probe(SCENE_QUERY_STAT(VEILUnstick), false, this);
					// THE COMPOSITION IS SCENERY, NOT FLOOR. Its shelves are 640 cm stone
					// platforms around the plaza, and the trace kept standing the runner on one
					// at z=598 - above the market and inside a fall sheet, which is the dark
					// water that filled the frame. Ignoring them finds the plaza at z=0.
					for (TActorIterator<AVEILConceptWorld> SceneryIt(StuckWorld); SceneryIt; ++SceneryIt)
					{
						Probe.AddIgnoredActor(*SceneryIt);
					}
					for (int32 i = 1; i <= 34 && !bMoved; ++i)
					{
						const float Ang = FMath::DegreesToRadians(i * 137.50776405f);
						const float Rad = 260.f * FMath::Sqrt((float)i);
						const float PX = From.X + FMath::Cos(Ang) * Rad;
						const float PY = From.Y + FMath::Sin(Ang) * Rad;
						// THE LOWEST FLOOR, NOT THE FIRST ONE. A first-hit trace over the
						// plaza answers with whatever is highest there - a surface at z=500 that
						// is a roof, not ground - and the runner was then stood 5 m above the
						// market and blocked by it. Walk down through every surface and keep the
						// lowest: that is the plaza the district is built on.
						bool bFoundFloor = false;
						float FloorZ = 0.f;
						AActor* FloorActor = nullptr;
						FVector Down(PX, PY, From.Z + 400.f);
						for (int32 Depth = 0; Depth < 5; ++Depth)
						{
							FHitResult Hit;
							if (!StuckWorld->LineTraceSingleByChannel(Hit, Down,
								FVector(PX, PY, From.Z - 4000.f), ECC_Visibility, Probe)) { break; }
							FloorZ = (float)Hit.ImpactPoint.Z;
							FloorActor = Hit.GetActor();
							bFoundFloor = true;
							Down = FVector(PX, PY, FloorZ - 5.f);
						}
						// A SURFACE ABOVE TWO METRES IS A ROOF. PHI GROUND puts the plaza at
						// z=0, so a shelf at 500 is no longer the only thing that answers.
						// THE DECK IS AT 500. The city's market stands on a slab whose
						// top face is 500 cm, so a 200 cm ceiling rejected the only real
						// ground and called the plaza "boxed in". Everything up to 900 is
						// deck; the roof is above that.
						if (!bFoundFloor || FloorZ > 900.f) { continue; }
						const FVector Stand(PX, PY, FloorZ + 98.f);
						FCollisionQueryParams Free = Probe;
						Free.AddIgnoredActor(FloorActor);
						if (StuckWorld->OverlapBlockingTestByChannel(Stand, FQuat::Identity, ECC_Pawn,
							FCollisionShape::MakeCapsule(38.f, 92.f), Free)) { continue; }
						SetActorLocation(Stand, false, nullptr, ETeleportType::TeleportPhysics);
						bMoved = true;
						UE_LOG(LogVEIL, Warning,
							TEXT("PHI UNSTICK: capsule was inside geometry at %s - moved to open ground at %s (%.0f cm out)."),
							*From.ToCompactString(), *Stand.ToCompactString(), Rad);
					}
				}
				if (!bMoved)
				{
					UE_LOG(LogVEIL, Warning,
						TEXT("PHI UNSTICK: no open ground within 1500 cm of %s - the capsule is boxed in."),
						*From.ToCompactString());
					// BOXED IN MEANS THE PLAZA, ONCE. He was left inside the PlayerStart's
					// corner every run, which is the stand-still frame; one teleport to his
					// own coordinate puts him on the floor PHI GROUND laid.
					static bool bPlazaRescue = false;
					if (!bPlazaRescue)
					{
						bPlazaRescue = true;
						SetActorLocation(FVector(0.f, -1500.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
						UE_LOG(LogVEIL, Warning, TEXT("PHI SEAT LAW: boxed in, so the runner stands on V(0,0,100)."));
					}
				}
			}
		}
	}
	DriveBodyAnimation(DeltaSeconds);

	// Flight and the +1 fuel run every frame, flying or not: ATP has to refill
	// while the runner walks too, or the pool is a punishment instead of a loop.
	ApplyFlight(DeltaSeconds);
	UpdateATP(DeltaSeconds);

	// The grace clock always runs, from the first frame on.
	GraceClock += DeltaSeconds;

	// A press that arrives before the seams exist is held for a moment rather than
	// thrown away: this is what makes the E key feel like it worked.
	if (ClaimArmUntil > 0.f && GetWorld() && GetWorld()->GetTimeSeconds() < ClaimArmUntil)
	{
		ClaimNearestSeam(ClaimArmReach);
	}

	// INFINITE RESPAWN: one phi second on the floor (1.618), then he stands
	// again. No counter is consulted - falling is a set-back, not an ending.
	if (!IsAlive())
	{
		DeathClock += DeltaSeconds;
		if (DeathClock >= UPHIMito::Phi)
		{
			RespawnNow();
		}
	}

	// The body is shown while the arm holds its 420 cm. The old per-frame
	// show/hide here fired whenever the lens drifted within 140 cm of the body,
	// and each toggle rebuilt the skeletal render state under the renderer - the
	// frame that killed the process was the morph vertex pool asserting with no
	// previous buffer written. The arm no longer collapses (its collision test is
	// off), so the toggle has nothing left to guard. First person still hides the
	// body, and it does it once, in ToggleCameraMode.
	// PHI MOVE TEST v3 - tick-driven, no timers. At t=5s of tick time it
	// names the possession state, drives synthetic forward input for
	// ~1.2s, then reports the velocity and displacement produced.
	// Gated behind -phimove: a shipped game never walks the player by itself.
	if (bPhiMoveProof)
	{
		static int32 MovePhase = 0;
		static float MoveClock = 0.f;
		static FVector MoveSpawn = FVector::ZeroVector;
		MoveClock += DeltaSeconds;
		if (MovePhase == 0 && MoveClock >= 5.f)
		{
			MovePhase = 1;
			MoveSpawn = GetActorLocation();
			AController* C = GetController();
			UE_LOG(LogVEIL, Log, TEXT("PHI MOVE TEST BEGIN: controller=%s pawn=%s locallyControlled=%d paused=%d velBefore=%.1f loc=%s"),
				C ? *C->GetClass()->GetName() : TEXT("NONE"), *GetName(),
				IsLocallyControlled() ? 1 : 0, UGameplayStatics::IsGamePaused(GetWorld()) ? 1 : 0,
				GetVelocity().Size(), *GetActorLocation().ToString());
		}
		else if (MovePhase == 1)
		{
			AddMovementInput(GetActorForwardVector(), 1.0f);
			if (MoveClock >= 6.2f)
			{
				MovePhase = 2;
				UE_LOG(LogVEIL, Log, TEXT("PHI MOVE TEST: velAfter=%.1f locAfter=%s moved=%.1f"),
					GetVelocity().Size(), *GetActorLocation().ToString(),
					FVector::Dist2D(GetActorLocation(), MoveSpawn));
			}
		}
	}
	// PHI CAMERA PROOF (only with -phimove): walk the rig through its three phi
	// rings and both ends of the pitch envelope, and report the lens each time.
	// A rig that only ever sits at one angle is not a camera, it is a tripod.
	if (bPhiMoveProof)
	{
		static int32 CamPhase = 0;
		static float CamClock = 0.f;
		CamClock += DeltaSeconds;
		auto ReportLens = [this](const TCHAR* Label)
		{
			if (!FollowCamera || !CameraBoom) return;
			UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA PROOF: %s -> ring %d, arm %.1f cm, pitch %.4f, lens %s, fwd %s."),
				Label, CameraRing, CameraArmLength, CameraPitch,
				*FollowCamera->GetComponentLocation().ToCompactString(),
				*FollowCamera->GetForwardVector().ToCompactString());
		};
		if (CamPhase == 0 && CamClock >= 8.f)
		{
			CamPhase = 1;
			CameraRing = 2; CameraPitch = 90.f / (UPHIMito::Phi * UPHIMito::Phi);
			ReportLens(TEXT("wide, looking up to the envelope"));
		}
		else if (CamPhase == 1 && CamClock >= 9.5f)
		{
			CamPhase = 2;
			CameraRing = 0; CameraPitch = -(90.f / UPHIMito::Phi);
			ReportLens(TEXT("close, looking down to the envelope"));
		}
		else if (CamPhase == 2 && CamClock >= 11.f)
		{
			CamPhase = 3;
			CameraRing = 1; CameraPitch = -17.0324f;
			ReportLens(TEXT("back to the law"));
		}
		else if (CamPhase == 3 && CamClock >= 12.5f)
		{
			// RESPAWN PROOF: take lethal damage through the real damage chain and
			// let the death clock walk him back. Proves the return, not the text.
			CamPhase = 4;
			UGameplayStatics::ApplyDamage(this, MaxHealth * 2.f, GetController(), this,
				UDamageType::StaticClass());
		}
	}

	// PHI TICKPROOF: every ~5s of tick time the game states it is turning.
	{
		static float ProofClock = 0.f;
		static int64 ProofFrames = 0;
		ProofClock += DeltaSeconds;
		++ProofFrames;
		if (ProofClock >= 5.f)
		{
			ProofClock = 0.f;
			UE_LOG(LogVEIL, Log, TEXT("PHI TICKPROOF: frames=%lld worldTime=%.1f paused=%d"),
				ProofFrames, GetWorld()->GetTimeSeconds(), UGameplayStatics::IsGamePaused(GetWorld()) ? 1 : 0);
		}
	}
	// One-shot animation proof: the anim instance is created after
	// BeginPlay, so the first tick that sees it logs the live class.
	static bool bAnimProofLogged = false;
	if (!bAnimProofLogged && GetMesh()->GetAnimInstance())
	{
		bAnimProofLogged = true;
		UE_LOG(LogVEIL, Log, TEXT("PHI BODY ANIM LIVE: %s"), *GetMesh()->GetAnimInstance()->GetClass()->GetName());
	static float AnimDiagTimer = 0.f;
	if (!bAnimProofLogged)
	{
		AnimDiagTimer += DeltaSeconds;
		if (AnimDiagTimer > 5.f)
		{
			bAnimProofLogged = true;
			UE_LOG(LogVEIL, Warning, TEXT("PHI BODY ANIM DIAG: instance=none class=%s skeleton=%s"),
				GetMesh()->GetAnimClass() ? *GetMesh()->GetAnimClass()->GetName() : TEXT("none"),
				(GetMesh()->GetSkeletalMeshAsset() && GetMesh()->GetSkeletalMeshAsset()->GetSkeleton())
					? *GetMesh()->GetSkeletalMeshAsset()->GetSkeleton()->GetName() : TEXT("none"));
		}
	}
	}
}

bool AVEILCharacter::IsStoryHoldingInput() const
{
	const UWorld* W = GetWorld();
	if (!W) { return false; }
	const UVEILDialogue* Dlg = W->GetSubsystem<UVEILDialogue>();
	// ONLY A WAITING ANSWER HOLDS THE BODY. This used to return true whenever ANY
	// conversation was active - and the district plays ambient overheard lines at
	// boot ("PHI LIVING: overheard - DAL ..."), so Move() zeroed its own axis and
	// returned before the input ever reached the movement component. The runner
	// stood still for a whole session with every key correctly bound: that is
	// exactly "he cannot move". Walking away from a line is allowed; walking away
	// from a question is not.
	return Dlg && Dlg->IsChoosing();
}

void AVEILCharacter::JumpFromInput()
{
	// A conversation owns the body: no leaping out of a line, and no jumping
	// when the same button is the one that advances the text.
	if (IsStoryHoldingInput()) { return; }
	// In the air, the jump button is the CLIMB - held, not tapped. That is the
	// whole separation: the camera keeps looking level while the body rises.
	if (bFlying) { SetClimbAxis(1.f); return; }

	// DOUBLE-TAP JUMP ENTERS FLIGHT. One press is a jump; a second press inside
	// phi^-1 fifth of a second (0.382 s) is a takeoff. Nothing else on the keyboard
	// or the pad has to give up a button for it.
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Now - LastJumpTapAt <= 0.382f)
	{
		LastJumpTapAt = -100.f;
		ToggleFlight();
		return;
	}
	LastJumpTapAt = Now;
	Jump();
}

// ---------------------------------------------------------------------------
// FLIGHT
// ---------------------------------------------------------------------------
void AVEILCharacter::ToggleFlight()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) { return; }

	if (bFlying)
	{
		bFlying = false;
		ClimbAxis = 0.f;
		Move->SetMovementMode(MOVE_Walking);
		if (bNoclip) { ToggleNoclip(); }
		UE_LOG(LogVEIL, Log, TEXT("VEIL FLIGHT: off - walking at %s, ATP %.1f"),
			*GetActorLocation().ToCompactString(), ATP);
		return;
	}

	bFlying = true;
	ClimbAxis = 0.f;
	Move->SetMovementMode(MOVE_Flying);
	Move->MaxFlySpeed = bSprinting ? FlyFastSpeed : FlyCruiseSpeed;
	Move->Velocity.Z = 0.f;   // takeoff is a climb from here, never a drop
	UE_LOG(LogVEIL, Log, TEXT("VEIL FLIGHT: on - cruise %.0f, climb %.0f, vertical axis independent of camera (pitch %.2f)"),
		FlyCruiseSpeed, FlyClimbSpeed, CameraPitch);
}

void AVEILCharacter::ToggleNoclip()
{
	bNoclip = !bNoclip;
	// Noclip is a SEPARATE switch from flight: flying still collides, and walls
	// only stop mattering when this is on.
	SetActorEnableCollision(!bNoclip);
	UE_LOG(LogVEIL, Log, TEXT("VEIL NOCLIP: %s (flying=%d)"),
		bNoclip ? TEXT("on - collision off") : TEXT("off - collision on"), bFlying ? 1 : 0);
}

void AVEILCharacter::SetClimbAxis(float InAxis)
{
	ClimbAxis = FMath::Clamp(InAxis, -1.f, 1.f);
}

void AVEILCharacter::FlyUpStart()   { SetClimbAxis(1.f); UE_LOG(LogVEIL, Log, TEXT("VEIL FLIGHT: climb +1 (flying=%d)"), bFlying ? 1 : 0); }
void AVEILCharacter::FlyUpStop()    { if (ClimbAxis > 0.f) { SetClimbAxis(0.f); } }
void AVEILCharacter::FlyDownStart() { SetClimbAxis(-1.f); UE_LOG(LogVEIL, Log, TEXT("VEIL FLIGHT: dive -1 (flying=%d)"), bFlying ? 1 : 0); }
void AVEILCharacter::FlyDownStop()  { if (ClimbAxis < 0.f) { SetClimbAxis(0.f); } }

void AVEILCharacter::ApplyFlight(float DeltaSeconds)
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) { return; }

	if (!bFlying)
	{
		// FALL SPEED CAP. The cap lives in the physics volume in this engine version,
		// and the volume here is the level's own, so the clamp is applied where the
		// runner's motion actually is: a long drop lands at a speed the soft-landing
		// window can absorb instead of accelerating forever.
		if (Move->IsFalling() && Move->Velocity.Z < -MaxFallSpeed)
		{
			Move->Velocity.Z = -MaxFallSpeed;
		}
		return;
	}

	// Speed bands, with the acceleration the movement component already owns -
	// flying faster is a ramp, not a teleport.
	Move->MaxFlySpeed = bSprinting ? FlyFastSpeed : FlyCruiseSpeed;

	// THE VERTICAL AXIS. Camera pitch is not read here at all.
	//
	// It goes through AddMovementInput, not a Velocity.Z stamp: in MOVE_Flying the
	// movement component runs its own CalcVelocity, and the flying brake
	// (BrakingDecelerationFlying) treats a stamped velocity as unwanted speed and
	// eats it - the first proof run climbed at 40 cm/s instead of 800 and left the
	// ground unchanged. Input is what that brake respects.
	if (FMath::Abs(ClimbAxis) > 0.05f)
	{
		AddMovementInput(FVector::UpVector, ClimbAxis);
		// One line a second while climbing: what the movement component actually
		// received, so a climb that does not rise is diagnosed instead of guessed.
		static float ClimbLogClock = 0.f;
		ClimbLogClock += DeltaSeconds;
		if (ClimbLogClock >= 1.f)
		{
			ClimbLogClock = 0.f;
			UE_LOG(LogVEIL, Log, TEXT("VEIL FLIGHT DIAG: axis=%.2f accel=%s vel=%s maxFly=%.0f mode=%d ignored=%d canFly=%d"),
				ClimbAxis, *Move->GetCurrentAcceleration().ToCompactString(),
				*Move->Velocity.ToCompactString(), Move->MaxFlySpeed, (int32)Move->MovementMode,
				IsMoveInputIgnored() ? 1 : 0, Move->CanEverFly() ? 1 : 0);
		}
	}
	else
	{
		// HOVER: no input, and the flying brake settles Z to zero - held altitude,
		// not a slow sink.
		Move->Velocity.Z = FMath::FInterpTo(Move->Velocity.Z, 0.f, DeltaSeconds, 8.f);
	}

	if (FMath::Abs(ClimbAxis) < 0.05f && FMath::Abs(Move->Velocity.Z) < 6.f)
	{
		Move->Velocity.Z = 0.f;
	}

	// TOUCH-DOWN: only while descending, and only onto real floor.
	if (ClimbAxis < -0.1f && GetWorld())
	{
		FHitResult Ground;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILFlightLanding), false, this);
		const FVector From = GetActorLocation();
		const FVector To = From - FVector(0.f, 0.f, 160.f);
		if (GetWorld()->LineTraceSingleByChannel(Ground, From, To, ECC_Visibility, Params))
		{
			SetActorLocation(FVector(From.X, From.Y, Ground.ImpactPoint.Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.f),
				false, nullptr, ETeleportType::TeleportPhysics);
			Move->Velocity.Z = 0.f;
			ToggleFlight();
		}
	}
}

void AVEILCharacter::UpdateATP(float DeltaSeconds)
{
	if (bGodMode)
	{
		ATP = MaxATP;
		return;
	}
	// NOTE: no early return at full ATP. The first version returned there and the
	// pool never drained at all - a fuel gauge that only works once you are
	// already empty is not a gauge.

	// Spend on effort, return on rest. GODMODE never reaches here full-stop.
	const bool bWorking = bFlying || (bSprinting && GetVelocity().SizeSquared2D() > 25.f);
	const float Rate = bWorking ? 6.f : 9.f;
	ATP = FMath::Clamp(ATP + (bWorking ? -Rate : Rate) * DeltaSeconds, 0.f, MaxATP);
}

void AVEILCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Soft landing in the air rules, but a real fall still costs in clean play.
	// GODMODE never pays - and the fall speed cap above is what keeps the price
	// survivable instead of lethal.
	const float FallSpeed = FMath::Abs(GetCharacterMovement()->Velocity.Z);
	if (!bGodMode && FallSpeed > 900.f)
	{
		const float Damage = FMath::Clamp((FallSpeed - 900.f) / 25.f, 0.f, 60.f);
		if (Damage > 0.f)
		{
			TakeDamage(Damage, FDamageEvent(), nullptr, this);
			UE_LOG(LogVEIL, Log, TEXT("VEIL LANDING: fall %.0f cm/s -> %.1f damage (clean play)"), FallSpeed, Damage);
		}
	}
	else if (bGodMode)
	{
		UE_LOG(LogVEIL, Log, TEXT("VEIL LANDING: fall %.0f cm/s -> no damage (GODMODE)"), FallSpeed);
	}
}

void AVEILCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller || !IsAlive()) return;
	if (IsStoryHoldingInput())
	{
		MoveAxis = FVector2D::ZeroVector;
		AddMovementInput(FVector::ZeroVector, 0.f);
		return;
	}
	// EASED STICK, NOT A SWITCH. Raw axis values jump 0 to 1 in a single frame,
	// so the body jerks. The input is smoothed on phi^2 before it ever reaches
	// the movement component, which is what makes a start and a stop read as
	// weight instead of as a teleport.
	const FVector2D Raw = Value.Get<FVector2D>();
	MoveAxis = FMath::Vector2DInterpTo(MoveAxis, Raw,
		GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f,
		UPHIMito::Phi * UPHIMito::Phi);
	const FVector2D Axis = MoveAxis;
	// SAY WHAT THE BODY RECEIVED: axis, mode, allowed speed, ignored, story.
	if (!Axis.IsNearlyZero() && GetWorld())
	{
		static float MoveDiagClock = 0.f;
		MoveDiagClock += GetWorld()->GetDeltaSeconds();
		if (MoveDiagClock >= 2.f)
		{
			MoveDiagClock = 0.f;
			const UCharacterMovementComponent* Diag = GetCharacterMovement();
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI MOVE DIAG: axis=(%.2f,%.2f) mode=%d maxWalk=%.0f ignored=%d story=%d accel=%s vel=%s"),
				Axis.X, Axis.Y, Diag ? (int32)Diag->MovementMode : -1,
				Diag ? Diag->MaxWalkSpeed : -1.f, IsMoveInputIgnored() ? 1 : 0,
				IsStoryHoldingInput() ? 1 : 0,
				Diag ? *Diag->GetCurrentAcceleration().ToCompactString() : TEXT("-"),
				Diag ? *Diag->Velocity.ToCompactString() : TEXT("-"));
		}
	}
	const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	// GOLDEN PITCH, AROUND RIGHTVECTOR. 17.0324 deg - cos 0.9561, sin 0.2929 - so a
	// forward push is 95.6% of the heading and 29.3% of the pitch. UpVector is yaw
	// and rotates W sideways, which is why this uses the right axis. Every path
	// (keys, stick, the proof) arrives here, so there is one direction, not two.
	static const FVector PitchAxis = FVector::RightVector;
	AddMovementInput(Forward.RotateAngleAxis(-17.0324f, PitchAxis), Axis.Y);
	AddMovementInput(Right.RotateAngleAxis(-17.0324f, PitchAxis), Axis.X);
}

void AVEILCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	// Vertical look belongs to the rig, not the control rotation: the arm no
	// longer follows control pitch, so the pitch is moved where the rig reads it.
	AddCameraPitch(Axis.Y);
}

void AVEILCharacter::ApplySpeed()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (bAiming)        Move->MaxWalkSpeed = AimWalkSpeed;
	else if (bSprinting) Move->MaxWalkSpeed = SprintSpeed;
	else                 Move->MaxWalkSpeed = WalkSpeed;

	// Aiming locks facing to the camera; exploring lets movement orient the body.
	Move->bOrientRotationToMovement = !bAiming;
	bUseControllerRotationYaw = bAiming;
}

void AVEILCharacter::StartSprint() { bSprinting = true; ApplySpeed(); }
void AVEILCharacter::StopSprint()  { bSprinting = false; ApplySpeed(); }

void AVEILCharacter::StartAim()
{
	bAiming = true;
	CombatState = EVEILCombatState::Aiming;
	ApplySpeed();
}

void AVEILCharacter::StopAim()
{
	bAiming = false;
	if (CombatState == EVEILCombatState::Aiming) CombatState = EVEILCombatState::Exploring;
	ApplySpeed();
}

void AVEILCharacter::DoDodge()
{
	const float Now = GetWorld()->TimeSeconds;
	if (!IsAlive() || IsStoryHoldingInput() || Now - LastDodgeTime < DodgeCooldown) return;
	LastDodgeTime = Now;
	CombatState = EVEILCombatState::Dodging;

	FVector Dir = GetLastMovementInputVector();
	if (Dir.IsNearlyZero()) Dir = -GetActorForwardVector();
	Dir.Z = 0.f; Dir.Normalize();
	LaunchCharacter(Dir * DodgeDistance + FVector(0.f, 0.f, 120.f), true, false);

	FTimerHandle Timer;
	GetWorldTimerManager().SetTimer(Timer, [this]()
	{
		if (CombatState == EVEILCombatState::Dodging)
		{
			CombatState = bAiming ? EVEILCombatState::Aiming : EVEILCombatState::Exploring;
		}
	}, 0.35f, false);
}

void AVEILCharacter::Fire()
{
	const float Now = GetWorld()->TimeSeconds;
	if (!IsAlive() || IsStoryHoldingInput() || Now - LastFireTime < FireCooldown) return;
	LastFireTime = Now;

	FVector Start;
	FRotator ViewRot;
	if (Controller) Controller->GetPlayerViewPoint(Start, ViewRot);
	else { Start = GetActorLocation(); ViewRot = GetActorRotation(); }

	const FVector End = Start + ViewRot.Vector() * WeaponRange;
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (AActor* Victim = Hit.GetActor())
		{
			UGameplayStatics::ApplyDamage(Victim, WeaponDamage, Controller, this, UDamageType::StaticClass());
		}
	}
}

void AVEILCharacter::OnSwingInput()
{
	if (IsStoryHoldingInput()) { return; }
	Swing();
	// A strike observes the temporal field: it collapses onto the second
	// future, so the same world can resolve differently after a fight.
	if (UVEILTime* Clock = GetWorld() ? GetWorld()->GetSubsystem<UVEILTime>() : nullptr)
	{
		Clock->Observe(TEXT("strike"));
	}
}

void AVEILCharacter::Strike()
{
	OnSwingInput();
}

void AVEILCharacter::Swing()
{
	const float Now = GetWorld()->TimeSeconds;
	if (!IsAlive() || Now - LastSwingTime < SwingCooldown) return;
	LastSwingTime = Now;
	CombatState = EVEILCombatState::Aiming;

	// Melee arc: every damageable actor within SwingRange and in
	// front of the runner takes SwingDamage. No projectile, no new
	// asset: the arc is a radius + forward-cone check, logged so a
	// PC Output Log read can prove a swing landed.
	const FVector Origin = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const FVector Forward = GetActorForwardVector();
	TArray<FOverlapResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	const bool bHit = GetWorld()->OverlapMultiByChannel(Hits, Origin,
		FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(SwingRange), Params);
	int32 Struck = 0;
	if (bHit)
	{
		for (const FOverlapResult& Hit : Hits)
		{
			AActor* Victim = Hit.GetActor();
			if (!Victim || Victim == this) continue;
			FVector ToVictim = Victim->GetActorLocation() - GetActorLocation();
			ToVictim.Z = 0.f;
			if (ToVictim.IsNearlyZero()) continue;
			ToVictim.Normalize();
			// Forward cone: cos(70deg) ~ 0.34. Behind the runner: no hit.
			if (FVector::DotProduct(Forward, ToVictim) < 0.34f) continue;
			UGameplayStatics::ApplyDamage(Victim, SwingDamage, Controller, this, UDamageType::StaticClass());
			AVEILBurstFX::Fire(GetWorld(), Victim->GetActorLocation() + FVector(0.f, 0.f, 60.f), 0);
			++Struck;
		}
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI SWING: struck=%d damage=%.1f range=%.1f"), Struck, SwingDamage, SwingRange);
}

void AVEILCharacter::LanceBolt()
{
	// Lance bolt IS the ranged fire: same hitscan path as Fire(),
	// named for the quest / HUD so "swing + lance bolt" reads as two
	// distinct weapons, not one renamed button.
	Fire();
}

void AVEILCharacter::Interact()
{
	if (!IsAlive() || IsStoryHoldingInput()) return;

	// A SEAM IS CLAIMED BY PRESSING THE KEY, not by walking near it.
	ClaimNearestSeam();

	// A BEACON IS FORGED BY STANDING AT IT AND PRESSING THE INTERACT KEY, which is
	// the same key that talks, loots and opens doors. Tried before the trace so a
	// stone that the trace happens to miss is still forgeable, and so the HUD's
	// BEACONS count moves because the player acted, not because a flag was set.
	if (UOMNIQuestSystem* Quest = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
	{
		Quest->ClaimBeaconNear(GetActorLocation());
	}

	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const FVector End = Start + GetActorForwardVector() * 320.f;
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (AVEILInteractable* Target = Cast<AVEILInteractable>(Hit.GetActor()))
		{
			Target->Interact(this);
		}
	}
}

void AVEILCharacter::ClaimNearestSeam(float ReachCm)
{
	UWorld* World = GetWorld();
	if (!World || !IsAlive()) { return; }

	// THE NEAREST SEAM IN THE WORLD, not the first actor that happens to have one.
	// The first version stopped at the first population actor with nothing in it and
	// reported "no named seam within 50 m" with a seam 2.8 m away - the lookup was
	// asking one building whether it had mines instead of asking the district.
	AVEILMineNode* Best = nullptr;
	float BestD = ReachCm;
	for (TActorIterator<AVEILWildsPopulation> It(World); It; ++It)
	{
		AVEILMineNode* Mine = It->NearestUnclaimedMine(GetActorLocation(), BestD);
		if (!Mine) { continue; }
		const float D = FVector::Dist2D(Mine->GetActorLocation(), GetActorLocation());
		if (D <= BestD) { BestD = D; Best = Mine; }
	}

	if (!Best)
	{
		// A held press is silent: it retries every frame until the world has its seams,
		// or until the hold runs out. Only a press that finds nothing to say says it.
		if (ClaimArmUntil < 0.f)
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI CLAIM: no named seam within %.0f m of %s - nothing to take."),
				ReachCm / 100.f, *GetActorLocation().ToCompactString());
		}
		return;
	}

	ClaimArmUntil = -1.f;
	UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
	if (Quest && Quest->ClaimMine(FName(*Best->MineName()), Best->MineOwner(), Best->ClaimValue(), 24))
	{
		Best->Claim(this);
	}
	else if (Quest)
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI CLAIM: %s is already the runner's - nothing paid."),
			*Best->MineName());
	}
}

void AVEILCharacter::RequestClaim(float ReachCm)
{
	ClaimArmReach = ReachCm;
	ClaimArmUntil = (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f) + 12.f;
	ClaimNearestSeam(ClaimArmReach);
}

// A KEYBOARD TEST THAT IS THE SAME CODE AS THE KEY. veil.claim runs the exact
// function the interact key runs, so what a run proves here a player can do.
static FAutoConsoleCommandWithWorldAndArgs GClaimSeam(
	TEXT("veil.claim"),
	TEXT("veil.claim [reach_cm] - claim the nearest named seam within reach (default 700), same code as pressing E."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic([](const TArray<FString>& Args, UWorld* World)
	{
		if (!World) { return; }
		const float Reach = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 700.f;
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (AVEILCharacter* Runner = Cast<AVEILCharacter>(PC->GetPawn())) { Runner->RequestClaim(Reach); }
		}
	}));

void AVEILCharacter::ToggleGodMode()
{
	bGodMode = !bGodMode;
	UE_LOG(LogVEIL, Warning, TEXT("PHI GODMODE: %s"), bGodMode ? TEXT("ON") : TEXT("OFF"));
}

float AVEILCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	// GATE LOG: something refused every blow in a 130 s run while the camp logged
	// 3 attacks, so the refusal names its own gate here.
	// LogTemp/Display on purpose: LogVEIL at Log verbosity never appeared in a -game
	// log (only its Warnings did), which is why an earlier probe looked like
	// "no damage" when damage was landing all along.
	UE_LOG(LogTemp, Display, TEXT("PHI DMG GATE: raw=%.1f god=%d alive=%d health=%.1f grace=%.2f/%.2f tokens=%d"),
		DamageAmount, bGodMode ? 1 : 0, IsAlive() ? 1 : 0, Health, GraceClock, GraceWindow, RecentHitTimes.Num());

	if (bGodMode) return 0.f;
	if (!IsAlive()) return 0.f;

	// REWARDING, NOT PUNISHING: the opening and every return are protected by
	// a phi window. Damage is refused, not reduced, so the runner gets to learn
	// the district before the camp is allowed to teach him anything.
	if (GraceClock < GraceWindow)
	{
		if (!bGraceAnnounced)
		{
			bGraceAnnounced = true;
			UE_LOG(LogVEIL, Log, TEXT("PHI GRACE: the camp strikes and the Runner is held - %.2f of %.2f protected seconds left."),
				GraceWindow - GraceClock, GraceWindow);
		}
		return 0.f;
	}
	// ATTACK TOKENS (God of War's aggression pool, enforced where every attacker
	// must pass): the camp may surround the runner, but only AttackersAllowed of
	// them can land a blow inside a phi^-1 window. The rest are held, not
	// deleted - exactly the "others circle and wait their turn" rule.
	{
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		RecentHitTimes.RemoveAll([Now, this](const float T) { return (Now - T) > TokenWindow; });
		if (RecentHitTimes.Num() >= AttackersAllowed)
		{
			if (!bTokenHoldAnnounced)
			{
				bTokenHoldAnnounced = true;
				UE_LOG(LogVEIL, Log, TEXT("PHI TOKEN: %d attacker(s) hold the token - blow from %s held back."),
					RecentHitTimes.Num(),
					DamageCauser ? *DamageCauser->GetName() : TEXT("(no causer)"));
			}
			return 0.f;
		}
		bTokenHoldAnnounced = false;
		RecentHitTimes.Add(Now);
	}

	// NO ONE-SHOT: a single blow can never take more than phi^-2 = 38.2% of max
	// health. The old lower bound did the opposite of protecting the player - it
	// RAISED every weak blow to MaxHealth * phi^-5 = 9.02, so a 5-damage agent hit
	// for 9. The damage log caught it: "9.0 applied (raw 5.0)". Ceiling only.
	const float ClampedDamage = FMath::Min(DamageAmount, MaxHealth * UPHIMito::PhiPow(-2.f));
	const float Applied = Super::TakeDamage(ClampedDamage, DamageEvent, EventInstigator, DamageCauser);
	Health = FMath::Clamp(Health - Applied, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(Health, MaxHealth);
	// EVERY BLOW NAMES ITSELF. The runner was falling at t~20s with no damage log
	// anywhere, so it was impossible to say what killed him. Now it is data.
	UE_LOG(LogTemp, Display, TEXT("PHI DMG: %.1f applied (raw %.1f, clamped %.1f) from %s at t=%.2f -> health %.1f/%.1f"),
		Applied, DamageAmount, ClampedDamage,
		DamageCauser ? *DamageCauser->GetName() : TEXT("(no causer)"),
		GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f, Health, MaxHealth);
	if (Health <= 0.f)
	{
		CombatState = EVEILCombatState::Dead;
		OnDied.Broadcast();
		DisableInput(Cast<APlayerController>(Controller));
		// INFINITE RESPAWN: the clock starts here and Tick walks him back.
		// Death never ends the run and is never rationed.
		DeathClock = 0.f;
		// "the runner fell" named the wrong thing for weeks: he was being beaten
		// down by a named machine, so the line says who, not "fell".
		UE_LOG(LogTemp, Display, TEXT("PHI DEATH: the runner went down to %s at t=%.2f. Returning in phi seconds - respawn %d is next, no cap."),
			DamageCauser ? *DamageCauser->GetName() : TEXT("(no causer)"),
			GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f, RespawnCount + 1);
	}
	return Applied;
}

void AVEILCharacter::RespawnNow()
{
	// He comes back at his anchored SEAT: Tyree's hand-set 0, 0 - and on the
	// FLOOR that is actually under it. The literal 0,0,100 was wrong by real
	// geometry, not by taste: the plaza floor there stands at Z~56 and the
	// capsule is 96 half-height, so the runner's centre belongs at ~154. Typed
	// at 100, every return planted his feet ~52 cm INSIDE the floor and the
	// capsule fell/wedged in place - the "falling in place" loop. The seat's
	// X/Y stay Tyree's; the height is traced and stood on, never typed.
	++RespawnCount;
	Health = MaxHealth;
	CombatState = EVEILCombatState::Exploring;
	DeathClock = 0.f;
	// A fresh phi^3 = 4.2361 second window: coming back never means walking
	// straight into the same blow. No penalty, no loss of gold or XP.
	GraceClock = 0.f;
	GraceWindow = UPHIMito::PhiPow(3.f);
	bGraceAnnounced = false;

	FVector Seat(0.f, 0.f, 100.f);
	if (const UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILRespawnSeat), false, this);
		// THE VAULT IS A LANDMARK, NOT A FLOOR. Its own bounds reach z=747 over the
		// plaza centre, so a downward trace met its roof and seated every return on
		// top of it at 598 instead of on the plaza floor it stands on.
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			if (*It && It->GetClass()->GetName().Contains(TEXT("Vault"))) { Params.AddIgnoredActor(*It); }
		}
		FHitResult Ground;
		// The trace starts just ABOVE the seat, not a roof-height above it. From
		// Z=1300 the first thing a downward trace meets in this district is a
		// rooftop, and every return seated the runner at Z=1398 - 24 m over the
		// plaza, unable to walk to the Keeper. The band is the seat and the floor
		// under it: 500 down to -3000 finds the plaza, never the roof above it.
		const FVector From(Seat.X, Seat.Y, Seat.Z + 400.f);
		const FVector To(Seat.X, Seat.Y, Seat.Z - 3000.f);
		if (GetWorld() && GetWorld()->LineTraceSingleByChannel(Ground, From, To, ECC_Visibility, Params))
		{
			Seat.Z = (float)Ground.ImpactPoint.Z + HalfHeight + 2.f;
		}
	}

	SetActorLocation(Seat, false, nullptr, ETeleportType::TeleportPhysics);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// No leftover downward velocity carried into the new life - a return
		// that arrives already falling reads as the same fall on loop.
		Move->StopMovementImmediately();
	}
	// A RETURN IS A FRESH ENGAGEMENT, not a feeding loop on the seat. Machines
	// that were on him stood inside their own 500 cm "heard" range, so they
	// never lost him and he was beaten down again in the same spot every time.
	// Every machine that can see the seat lets go for phi^4 = 6.854s - longer
	// than the phi^3 grace, so the window survives the return itself.
	if (GetWorld())
	{
		for (TActorIterator<AVEILEnemy> It(GetWorld()); It; ++It)
		{
			AVEILEnemy* Machine = *It;
			if (!Machine) { continue; }
			if (FVector::Dist2D(Machine->GetActorLocation(), Seat) <= Machine->LiveSightRange)
			{
				Machine->StandDown(UPHIMito::PhiPow(4.f));
			}
		}
		// The camp machines are their own class, and they are the ones that hold
		// the plaza the runner returns to. They get the same window.
		for (TActorIterator<AVEILFactionAgent> It(GetWorld()); It; ++It)
		{
			AVEILFactionAgent* Agent = *It;
			if (!Agent) { continue; }
			if (FVector::Dist2D(Agent->GetActorLocation(), Seat) <= Agent->SightRange)
			{
				Agent->StandDown(UPHIMito::PhiPow(4.f));
			}
		}
	}

	if (AController* C = GetController())
	{
		EnableInput(Cast<APlayerController>(C));
	}
	OnHealthChanged.Broadcast(Health, MaxHealth);
	UE_LOG(LogVEIL, Log, TEXT("PHI RESPAWN %d: standing again at %s with %.0f health. Infinite - no cap."),
		RespawnCount, *Seat.ToCompactString(), Health);
}

void AVEILCharacter::Heal(float Amount)
{
	if (!IsAlive()) return;
	Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void AVEILCharacter::ApplyDrop(int32 Tier)
{
	const int32 T = FMath::Clamp(Tier, 0, 3);
	const float Before = Health;

	// Wounds close by a phi slice of his own maximum, hotter per tier.
	const float HealAmount = MaxHealth * UPHIMito::PhiPow(-1.f) * UPHIMito::PhiPow(-1.f)
		* UPHIMito::PhiPow(static_cast<float>(T)); // tier 0 ~15% ... tier 3 ~63%
	Heal(HealAmount);

	// Power rises by a phi slice of the base, and never past phi^4 of it.
	DamageBonus = FMath::Min(DamageBonus + UPHIMito::PhiPow(-2.f) * (1.f + static_cast<float>(T)),
		UPHIMito::PhiPow(4.f));
	WeaponDamage = BaseWeaponDamage * (1.f + DamageBonus);
	SwingDamage = BaseSwingDamage * (1.f + DamageBonus);

	UE_LOG(LogVEIL, Log, TEXT("PHI POWER: drop tier %d -> health %.1f -> %.1f (max %.1f), damage %.1f/%.1f (bonus %.3f of phi^4 cap)."),
		T, Before, Health, MaxHealth, SwingDamage, WeaponDamage, DamageBonus);
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void AVEILCharacter::HoldGoldenCamera(float DeltaSeconds)
{
	if (!CameraBoom || !FollowCamera) return;

	// THE THREE PHI RINGS of the arm: 420 is the law, and the other two are the
	// same law divided and multiplied by phi - never invented distances.
	static const float RingLengths[3] = {
		420.f / UPHIMito::Phi, 420.f, 420.f * UPHIMito::Phi };
	const float WantedLength = bFirstPerson ? 0.f : RingLengths[FMath::Clamp(CameraRing, 0, 2)];

	// THE EYE NEVER SINKS UNDER THE FLOOR. Looking up swings the arm down behind
	// the runner, and at ring 2 that put the lens 578 cm BELOW the plaza - an ugly
	// shot of the underside of the world. Only the REAL floor is traced here, so
	// the district's decoration can never collapse the arm the way it did before.
	// THE LENS MUST NEVER SIT UNDER HER. Shrinking the arm to keep the eye above
	// the floor was the wrong lever: below ~60 cm the camera ends up UNDER the body
	// looking up at it. The right lever is the PITCH - the eye rises only as far as
	// the space allows and then stops, exactly as a real third-person camera does.
	float Pitch = CameraPitch;
	float AllowedLength = WantedLength;
	if (!bFirstPerson && GetWorld())
	{
		FHitResult Ground;
		FCollisionQueryParams FloorParams(SCENE_QUERY_STAT(PhiCameraFloor), false, this);
		const FVector Pivot = GetActorLocation();
		if (GetWorld()->LineTraceSingleByChannel(Ground, Pivot + FVector(0.f, 0.f, 40.f),
			Pivot - FVector(0.f, 0.f, 2000.f), ECC_Visibility, FloorParams))
		{
			const float Headroom = 25.f;
			const float Room = Pivot.Z - (Ground.ImpactPoint.Z + Headroom);
			const float Arm = FMath::Max(WantedLength, 1.f);
			const float MaxSin = FMath::Clamp(Room / Arm, 0.f, 1.f);
			const float MaxUpPitch = FMath::RadiansToDegrees(FMath::Asin(MaxSin));
			Pitch = FMath::Min(Pitch, MaxUpPitch);
		}
		// A standoff floor: whatever the geometry says, the eye never comes closer
		// than phi^2 steps in - 420/phi^2 = 160.4 cm - so the shot is always the
		// runner's back, never the underside of the runner.
		AllowedLength = FMath::Max(WantedLength, 420.f / (UPHIMito::Phi * UPHIMito::Phi));
	}

	// THE LENS NEVER SITS INSIDE THE WORLD. bDoCollisionTest is off above because
	// the stock arm collapses to nothing the instant a tall piece stands behind
	// the runner - but off has its own failure, and Play showed it: a black frame
	// whose only lit edge was the inside of a wall. So the probe is ours, and it
	// is a probe, not a collapse. One trace down the arm from the pivot; the eye
	// stops a phi skin in FRONT of whatever it hits. A trace from the pivot can
	// never leave the lens on the far side of a blocker, so the shot is always in
	// open air, and the arm is only shortened as far as that wall demands.
	if (!bFirstPerson && GetWorld() && AllowedLength > 1.f)
	{
		const float ProbeYaw = Controller ? Controller->GetControlRotation().Yaw
			: GetActorRotation().Yaw;
		const FRotator ProbePose(Pitch, ProbeYaw, 0.f);
		const FVector Start = CameraBoom->GetComponentLocation()
			+ ProbePose.RotateVector(CameraBoom->SocketOffset);
		const FVector Dir = -ProbePose.Vector();
		FHitResult Wall;
		FCollisionQueryParams WallParams(SCENE_QUERY_STAT(PhiCameraWall), false, this);
		if (GetWorld()->LineTraceSingleByChannel(Wall, Start, Start + Dir * AllowedLength,
			ECC_Visibility, WallParams))
		{
			// 12 phi = 19.4 cm of air, the same skin the floor standoff uses. The
			// 40 cm floor keeps the lens out of the geometry even in a corner too
			// tight to hold a full third-person arm.
			const float Skin = 12.f * UPHIMito::Phi;
			// THE FLOOR OF THIS CLAMP WAS 40 CM, AND 40 CM IS INSIDE THE RUNNER. In
			// the plaza the probe hits dense carved stone a few centimetres behind the
			// body, so every frame was shot from inside his back at ankle height: the
			// opening picture was a wall of gold with no runner in it. The standoff the
			// code already believes in - 420/phi^2 = 160.4 cm - is the floor here too.
			// A tight corner now shows some stone at the edge of the frame, which is
			// what a real third-person camera does, instead of showing nothing.
			// THE STANDOFF IS 160.4 CM, AS SPECIFIED. I had raised it to 420/phi = 260 to
			// un-crowd a frame that was crowded for a different reason: the gold columns had
			// no collision, so the probe could not see the stone the lens was inside and the
			// arm never learned to shorten. Fix the world, not the standoff. 420/phi^2.
			const float Standoff = 420.f / (UPHIMito::Phi * UPHIMito::Phi);
			const float Clamped = FMath::Max(Wall.Distance - Skin, Standoff);
			if (Clamped < AllowedLength)
			{
				UE_LOG(LogVEIL, Verbose, TEXT("PHI CAMERA: wall at %.0f cm behind - arm held at %.0f cm, not inside the body."),
					Wall.Distance, Clamped);
			}
			AllowedLength = FMath::Min(AllowedLength, Clamped);
		}
	}

	// Eased on phi squared, so a ring change reads as the eye moving, not a cut.
	const float EaseSpeed = UPHIMito::Phi * UPHIMito::Phi;
	CameraArmLength = FMath::FInterpTo(CameraArmLength, AllowedLength, DeltaSeconds, EaseSpeed);
	// Out on phi time, IN at once. A wall appearing behind the runner is not a
	// camera move to be eased into: easing there is a second of black frame with
	// the lens inside the stone, which is exactly the frame the eye caught.
	CameraArmLength = FMath::Min(CameraArmLength, AllowedLength);
	CameraBoom->TargetArmLength = CameraArmLength;

	// Aim tightens the lens by the same law: 90/phi instead of 90.
	const float WantedFov = bAiming ? (90.f / UPHIMito::Phi) : 90.f;
	FollowCamera->SetFieldOfView(
		FMath::FInterpTo(FollowCamera->FieldOfView, WantedFov, DeltaSeconds, EaseSpeed));

	// THE CAMERA IS BUILT FOR THE BODY THAT IS IN IT. Countess is a tall hero
	// mesh, Garret is not, and a MetaHuman body is a third size again - a fixed
	// pivot framed exactly one of them. The eye line is read from the body's own
	// bounds and the shoulder offset is the capsule radius times phi, so the body
	// sits off-centre and never blocks the middle of the shot.
	if (USkeletalMeshComponent* Body = GetMesh())
	{
		const float BodyHeight = FMath::Max(Body->Bounds.BoxExtent.Z, 40.f);
		const float Shoulder = GetCapsuleComponent()
			? GetCapsuleComponent()->GetScaledCapsuleRadius() * UPHIMito::Phi : 55.f;
		CameraBoom->SocketOffset = bFirstPerson
			? FVector(0.f, 0.f, BodyHeight * 0.75f)
			: FVector(0.f, Shoulder, BodyHeight * 0.35f);
	}

	// Yaw follows the body's control rotation; pitch is the RIG'S OWN, held
	// inside the phi envelope with the golden pitch as its rest. This is why the
	// arm never re-derives its rotation: the rig is the one source of the angle.
	const float Yaw = Controller ? Controller->GetControlRotation().Yaw : GetActorRotation().Yaw;
	const FRotator Pose(Pitch, Yaw, 0.f);
	CameraBoom->SetWorldRotation(Pose);
	FollowCamera->SetWorldRotation(Pose);
}

void AVEILCharacter::AddCameraPitch(float DeltaDegrees)
{
	// THE ENVELOPE IS PHI, AND IT IS NOT SYMMETRIC. Down is 90/phi = 55.6231 -
	// a third-person eye wants to see the floor in front of the runner. Up is one
	// more phi step: 90/phi^2 = 34.3793 - because the district has a ceiling, and
	// swinging all the way up only ever showed the backside of the world.
	const float UpLimit = 90.f / (UPHIMito::Phi * UPHIMito::Phi);
	const float DownLimit = 90.f / UPHIMito::Phi;
	CameraPitch = FMath::Clamp(CameraPitch + DeltaDegrees, -DownLimit, UpLimit);
}

void AVEILCharacter::StepCameraRing(int32 Direction)
{
	const int32 Before = CameraRing;
	CameraRing = FMath::Clamp(CameraRing + Direction, 0, 2);
	if (CameraRing != Before)
	{
		static const float RingLengths[3] = {
			420.f / UPHIMito::Phi, 420.f, 420.f * UPHIMito::Phi };
		UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA RING: %d -> %d, arm %.1f cm."),
			Before, CameraRing, RingLengths[CameraRing]);
	}
}

// THE CHART OWNS THE WHEEL WHILE IT IS OPEN. Zooming the map also stepped the
// camera ring, so every map zoom silently changed the shot behind the panel. That
// filter belongs here, at the action that fires, NOT around the input bindings -
// wrapped around the bindings it skipped Move and Look entirely and left the
// runner unable to walk for a whole session.
void AVEILCharacter::CameraRingIn()
{
	if (const AVEILPlayerController* PC = Cast<AVEILPlayerController>(GetController()))
	{
		if (PC->bShowChart) { return; }
	}
	StepCameraRing(-1);
}

void AVEILCharacter::CameraRingOut()
{
	if (const AVEILPlayerController* PC = Cast<AVEILPlayerController>(GetController()))
	{
		if (PC->bShowChart) { return; }
	}
	StepCameraRing(1);
}

void AVEILCharacter::ToggleCameraMode()
{
	bFirstPerson = !bFirstPerson;
	if (CameraBoom)
	{
		// The same rig at two reaches: arm zero puts the golden eye in his head,
		// the ring puts it back behind him. Never a second camera system.
		static const float RingLengths[3] = {
			420.f / UPHIMito::Phi, 420.f, 420.f * UPHIMito::Phi };
		CameraArmLength = bFirstPerson ? 0.f : RingLengths[FMath::Clamp(CameraRing, 0, 2)];
		CameraBoom->TargetArmLength = CameraArmLength;
		CameraBoom->SetRelativeLocation(bFirstPerson ? FVector(30.f, 0.f, 64.f) : FVector::ZeroVector);
	}
	if (RunnerBody) RunnerBody->SetOwnerNoSee(bFirstPerson);
	if (RunnerVisor) RunnerVisor->SetOwnerNoSee(bFirstPerson);
	GetMesh()->SetOwnerNoSee(bFirstPerson);
	UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA: %s person, ring %d, arm %.1f cm, pitch %.4f (envelope +-%.4f)."),
		bFirstPerson ? TEXT("first") : TEXT("third"), CameraRing, CameraArmLength, CameraPitch,
		90.f / UPHIMito::Phi);
}

// ---------------------------------------------------------------------------
// PROOF HOOKS
//
// These call the SAME handlers the keys call, so what a proof run exercises is
// the code the player runs - not a parallel test path that can drift from it.
// phi.fly / phi.climb / phi.noclip / phi.state
// ---------------------------------------------------------------------------
namespace
{
	AVEILCharacter* VeilRunner(UWorld* World)
	{
		if (!World) { return nullptr; }
		APlayerController* PC = World->GetFirstPlayerController();
		return PC ? Cast<AVEILCharacter>(PC->GetPawn()) : nullptr;
	}
}

static FAutoConsoleCommandWithWorldAndArgs GPhiFly(
	TEXT("phi.fly"),
	TEXT("Flight on/off: phi.fly [0|1] (no argument toggles)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		AVEILCharacter* C = VeilRunner(World);
		if (!C) { UE_LOG(LogVEIL, Warning, TEXT("VEIL FLIGHT PROOF: no runner pawn")); return; }
		const bool bWant = Args.Num() > 0 ? FCString::Atoi(*Args[0]) != 0 : !C->IsFlyingNow();
		if (bWant != C->IsFlyingNow()) { C->ToggleFlight(); }
		UE_LOG(LogVEIL, Log, TEXT("VEIL FLIGHT PROOF: flying=%d climb=%.2f vel=%s loc=%s"),
			C->IsFlyingNow() ? 1 : 0, C->ClimbAxis, *C->GetVelocity().ToCompactString(),
			*C->GetActorLocation().ToCompactString());
	}));

static FAutoConsoleCommandWithWorldAndArgs GPhiClimb(
	TEXT("phi.climb"),
	TEXT("Flight vertical axis: phi.climb <-1|0|1> (camera is not read)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		AVEILCharacter* C = VeilRunner(World);
		if (!C) { return; }
		const float Axis = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 0.f;
		C->SetClimbAxis(Axis);
		UE_LOG(LogVEIL, Log, TEXT("VEIL CLIMB PROOF: axis=%.2f flying=%d velZ=%.1f loc=%s"),
			Axis, C->IsFlyingNow() ? 1 : 0, C->GetVelocity().Z, *C->GetActorLocation().ToCompactString());
	}));

static FAutoConsoleCommandWithWorldAndArgs GPhiNoclip(
	TEXT("phi.noclip"),
	TEXT("Noclip on/off, separate from flight: phi.noclip [0|1]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		AVEILCharacter* C = VeilRunner(World);
		if (!C) { return; }
		const bool bWant = Args.Num() > 0 ? FCString::Atoi(*Args[0]) != 0 : true;
		if (bWant != C->bNoclip) { C->ToggleNoclip(); }
		UE_LOG(LogVEIL, Log, TEXT("VEIL NOCLIP PROOF: noclip=%d flying=%d"), C->bNoclip ? 1 : 0, C->IsFlyingNow() ? 1 : 0);
	}));

static FAutoConsoleCommandWithWorldAndArgs GPhiState(
	TEXT("phi.state"),
	TEXT("Report the runner's motion state: ground/air, speed, ATP, flight"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		AVEILCharacter* C = VeilRunner(World);
		if (!C) { return; }
		const UCharacterMovementComponent* M = C->GetCharacterMovement();
		UE_LOG(LogVEIL, Log, TEXT("VEIL STATE: flying=%d noclip=%d climb=%.2f onGround=%d mode=%d speed=%.1f velZ=%.1f ATP=%.1f god=%d loc=%s"),
			C->IsFlyingNow() ? 1 : 0, C->bNoclip ? 1 : 0, C->ClimbAxis,
			M && M->IsMovingOnGround() ? 1 : 0, M ? (int32)M->MovementMode : -1,
			C->GetVelocity().Size(), C->GetVelocity().Z, C->GetATP(),
			C->IsGodMode() ? 1 : 0, *C->GetActorLocation().ToCompactString());
	}));


void AVEILCharacter::OneTalkPressed()
{
	if (UWorld* W = GetWorld())
	{
		if (UPHIONE* One = W->GetSubsystem<UPHIONE>()) { One->BeginTalk(); }
	}
}

void AVEILCharacter::OneTalkReleased()
{
	if (UWorld* W = GetWorld())
	{
		if (UPHIONE* One = W->GetSubsystem<UPHIONE>()) { One->EndTalk(); }
	}
}
