#include "VeilNpc.h"
#include "VeilRaces.h"
#include "PHI_Constants.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

namespace
{
	const TCHAR* const AnimWalk = TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/Animations/Locomotion/Shared/MM_Walk_InPlace.MM_Walk_InPlace");
	const TCHAR* const AnimIdleMale = TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/Animations/Locomotion/Male/MM_Idle.MM_Idle");
	const TCHAR* const AnimIdleFemale = TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/Animations/Locomotion/Female/MF_Idle.MF_Idle");
	const TCHAR* const BodyFemale = TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody");
	const TCHAR* const BodyMale = TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody");
}

AVeilNpc::AVeilNpc()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.f;

	Body = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetMobility(EComponentMobility::Movable);
}

const TCHAR* AVeilNpc::GetRaceName() const
{
	return VeilRaces::Get(RaceIdx).Name;
}

void AVeilNpc::Seat(const FVector& Home, int32 RaceIndex, int32 SeatIndex, double WalkRadius)
{
	const FVeilRace& Race = VeilRaces::Get(RaceIndex);

	HomeLoc = Home;
	SeatIdx = SeatIndex;
	RaceIdx = RaceIndex;
	bHunter = Race.bHunter;
	Radius = (WalkRadius > 0.0) ? WalkRadius : Race.Radius;
	Speed = Race.Speed;

	// Own phase from the golden ratio, so no two bodies share a timetable.
	const double G = FMath::Fmod(static_cast<double>(SeatIndex) * PHI_Constants::Phi, 1.0);
	Phase = static_cast<float>(G);
	Speed *= (0.85 + 0.3 * G);
	Wait = 0.5f + 5.5f * Phase;

	// Humans alternate between the two human bodies; every other people is its own.
	const TCHAR* BodyPath = Race.Body;
	if (RaceIndex == 0 && (SeatIndex % 2) != 0)
	{
		BodyPath = BodyMale;
	}
	if (RaceIndex == 0 && (SeatIndex % 2) == 0)
	{
		BodyPath = BodyFemale;
	}

	USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, BodyPath);
	if (Mesh && Body)
	{
		Body->SetSkeletalMesh(Mesh);
		const FBoxSphereBounds B = Mesh->GetBounds();
		const double RawH = FMath::Max(B.BoxExtent.Z * 2.0, 1.0);
		Body->SetWorldScale3D(FVector(Race.Height / RawH));

		if (Race.Skin)
		{
			if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, Race.Skin))
			{
				const int32 Slots = Body->GetNumMaterials();
				for (int32 Slot = 0; Slot < Slots; ++Slot)
				{
					Body->SetMaterial(Slot, M);
				}
			}
		}
	}

	WalkAnim = LoadObject<UAnimSequence>(nullptr, AnimWalk);
	IdleAnim = LoadObject<UAnimSequence>(nullptr, RaceIndex == 0 ? AnimIdleFemale : AnimIdleMale);

	const double Z = GroundZ(Home.X, Home.Y, Home.Z);
	SetActorLocation(FVector(Home.X, Home.Y, Z));
	if (IdleAnim && Body)
	{
		Body->PlayAnimation(IdleAnim, true);
	}
	Retarget();
}

double AVeilNpc::GroundZ(double X, double Y, double FromZ) const
{
	FHitResult Hit;
	FCollisionQueryParams QP(SCENE_QUERY_STAT(VEILNpcGround), false);
	if (GetWorld() && GetWorld()->LineTraceSingleByChannel(Hit,
		FVector(X, Y, FromZ + 6000.0), FVector(X, Y, FromZ - 6000.0), ECC_Visibility, QP))
	{
		return Hit.ImpactPoint.Z;
	}
	return FromZ;
}

void AVeilNpc::Retarget()
{
	++Step;
	const double Rate = 0.618033988749895 + 0.382 * static_cast<double>(Phase);
	const double A = FMath::DegreesToRadians(
		(static_cast<double>(SeatIdx) + static_cast<double>(Step) * Rate) * PHI_Constants::GoldenAngleDeg);
	const double R = Radius * (0.35 + 0.65 * static_cast<double>(Phase));
	const double X = HomeLoc.X + FMath::Cos(A) * R;
	const double Y = HomeLoc.Y + FMath::Sin(A) * R;
	Target = FVector(X, Y, GroundZ(X, Y, HomeLoc.Z));
}

void AVeilNpc::Perceive(APawn* Player, double Dist, int32 State)
{
	if (!Player)
	{
		return;
	}
	const FVector Runner = Player->GetActorLocation();

	if (bHunter)
	{
		// Kraken physics: enemies hold their own post. They never follow the Runner
		// and never orbit. They answer only what comes inside their reach of 300.
		Wait = 0.f;
		Speed = 0.0;
		const FVector To = Runner - GetActorLocation();
		SetActorRotation(FRotator(0.f, static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(To.Y, To.X))), 0.f));
		if (Dist < 300.0 && HitCooldown <= 0.f)
		{
			HitCooldown = 2.f;
			UE_LOG(LogTemp, Display, TEXT("VEIL HIT: %s (%s) strikes the Runner from its post"), *GetName(), GetRaceName());
		}
		return;
	}

	if (State == 2)
	{
		// A body of the world stops what it is doing and turns to the Runner.
		Wait = 1.5f;
		const FVector To = Runner - GetActorLocation();
		SetActorRotation(FRotator(0.f, static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(To.Y, To.X))), 0.f));
	}
}

void AVeilNpc::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Wait > 0.f)
	{
		Wait -= DeltaSeconds;
		if (bWalking)
		{
			bWalking = false;
			if (IdleAnim && Body)
			{
				Body->PlayAnimation(IdleAnim, true);
			}
		}
		return;
	}

	const FVector Here = GetActorLocation();
	const FVector To = Target - Here;
	const double Dist = To.Size2D();
	if (Dist < 45.0)
	{
		if (bHunter)
		{
			return;
		}
		Wait = 2.0f + 6.0f * Phase;
		Retarget();
		return;
	}

	const double Facing = FMath::RadiansToDegrees(FMath::Atan2(To.Y, To.X));
	SetActorRotation(FRotator(0.f, static_cast<float>(Facing), 0.f));

	const FVector Next = Here + To.GetSafeNormal2D() * FMath::Min(Dist, Speed * DeltaSeconds);
	SetActorLocation(FVector(Next.X, Next.Y, GroundZ(Next.X, Next.Y, Here.Z)));

	if (!bWalking)
	{
		bWalking = true;
		if (WalkAnim && Body)
		{
			Body->PlayAnimation(WalkAnim, true);
		}
	}
}
