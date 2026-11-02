#include "Systems/VEILFastTravel.h"
#include "Systems/VEILRegionSubsystem.h"
#include "Systems/VEILLoopActors.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "VEIL.h"

namespace
{
	// The runner has to walk up to a beacon before it counts as lit.
	constexpr float LightRadiusCm = 700.f;
	// Standing this close to a waypoint means "this is where I am" - not a destination.
	constexpr float AtWaypointCm = 800.f;
	constexpr float ArriveClearanceCm = 220.f;
}

void UVEILFastTravel::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Scan();

	// NOTE: nothing is lit here. Initialize runs before the level's actors exist,
	// so Waypoints is empty at this point; the First Beacon is lit in Tick once
	// the world has been scanned (see bFirstBeaconLighted).
}

int32 UVEILFastTravel::LitCount() const
{
	int32 N = 0;
	for (const FVEILWaypoint& W : Waypoints) { if (W.bLit) { ++N; } }
	return N;
}

FString UVEILFastTravel::Status() const
{
	return FString::Printf(TEXT("%d of %d beacons lit"), LitCount(), Waypoints.Num());
}

void UVEILFastTravel::Register(AActor* Beacon, const FString& Name)
{
	if (!Beacon) { return; }
	for (const TWeakObjectPtr<AActor>& Known : BeaconActors)
	{
		if (Known.Get() == Beacon) { return; }
	}
	BeaconActors.Add(Beacon);		FVEILWaypoint W;
		// Two beacons can share a district, so the coordinates ride along - a list
		// of destinations nobody can tell apart is not a list of destinations.
		W.Name = FString::Printf(TEXT("%s  [X%.0f Y%.0f]"), *Name,
			Beacon->GetActorLocation().X, Beacon->GetActorLocation().Y);
	W.Location = Beacon->GetActorLocation();
	W.bLit = false;
	Waypoints.Add(W);
	UE_LOG(LogVEIL, Log, TEXT("FAST TRAVEL: waypoint registered - %s at %s (%s)."),
		*W.Name, *W.Location.ToCompactString(), *Status());
}

void UVEILFastTravel::Scan()
{
	UWorld* World = GetWorld();
	if (!World) { return; }

	// Beacons are the world's own furniture, so they are found, not spawned.
	for (TActorIterator<AVEILBeacon> It(World); It; ++It)
	{
		FString Name = TEXT("Beacon");
		if (const UVEILRegionSubsystem* Regions = World->GetSubsystem<UVEILRegionSubsystem>())
		{
			Name = FString::Printf(TEXT("Beacon @ %s"),
				*Regions->GetInfo(Regions->RegionAt(It->GetActorLocation())).Name.ToString());
		}
		Register(*It, Name);
	}

	// Re-sync locations: a beacon moved in the editor must not teleport anyone
	// to where it used to be.
	int32 Index = 0;
	for (const TWeakObjectPtr<AActor>& Known : BeaconActors)
	{
		if (Known.IsValid() && Waypoints.IsValidIndex(Index))
		{
			Waypoints[Index].Location = Known->GetActorLocation();
		}
		++Index;
	}
}

void UVEILFastTravel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TravelCooldown > 0.f) { TravelCooldown -= DeltaTime; }

	ScanTimer -= DeltaTime;
	if (ScanTimer <= 0.f)
	{
		ScanTimer = 0.5f;
		Scan();
	}

	// THE FIRST BEACON IS NEVER OUT. Sera Vahl's lamp at the Hub has burned for
	// four hundred years, so the runner always has one place to come home to and
	// travel is open from the first frame. Lit here, once the world is scanned.
	if (!bFirstBeaconLighted && Waypoints.Num() > 0)
	{
		bFirstBeaconLighted = true;
		Waypoints[0].bLit = true;
		UE_LOG(LogVEIL, Log, TEXT("FAST TRAVEL: the First Beacon is lit - %s."), *Status());
	}

	const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn) { return; }

	const FVector Where = Pawn->GetActorLocation();
	for (FVEILWaypoint& W : Waypoints)
	{
		if (W.bLit) { continue; }
		if (FVector::Dist(Where, W.Location) < LightRadiusCm)
		{
			W.bLit = true;
			UE_LOG(LogVEIL, Log, TEXT("FAST TRAVEL: %s is lit - %s."), *W.Name, *Status());
			// Light the beacon itself when one of them is the waypoint actor.
			for (const TWeakObjectPtr<AActor>& Known : BeaconActors)
			{
				if (Known.IsValid() && FVector::Dist(Known->GetActorLocation(), W.Location) < 1.f)
				{
					if (AVEILBeacon* Beacon = Cast<AVEILBeacon>(Known.Get()))
					{
						Beacon->LightMarker();
					}
					break;
				}
			}
		}
	}
}

int32 UVEILFastTravel::NextLitIndex(const FVector& From) const
{
	if (LitCount() < 2) { return INDEX_NONE; }
	// Nearest-other first: stays useful when the world grows to hundreds of beacons.
	int32 Best = INDEX_NONE;
	float BestD = TNumericLimits<float>::Max();
	for (int32 i = 0; i < Waypoints.Num(); ++i)
	{
		if (!Waypoints[i].bLit) { continue; }
		const float D = FVector::Dist(From, Waypoints[i].Location);
		if (D < AtWaypointCm) { continue; } // that is where he already stands
		if (D < BestD) { BestD = D; Best = i; }
	}
	return Best;
}

bool UVEILFastTravel::TravelToNext(APawn* Pawn)
{
	if (!Pawn) { return false; }
	if (TravelCooldown > 0.f) { return false; }
	const int32 Index = NextLitIndex(Pawn->GetActorLocation());
	if (Index == INDEX_NONE)
	{
		UE_LOG(LogVEIL, Log, TEXT("FAST TRAVEL: %s - walk up to a second beacon to open travel."), *Status());
		return false;
	}
	return TravelTo(Index, Pawn);
}

int32 UVEILFastTravel::StandingAtIndex(const FVector& From) const
{
	for (int32 i = 0; i < Waypoints.Num(); ++i)
	{
		if (FVector::Dist(From, Waypoints[i].Location) < AtWaypointCm) { return i; }
	}
	return INDEX_NONE;
}

bool UVEILFastTravel::TravelTo(int32 Index, APawn* Pawn)
{
	if (!Pawn || !Waypoints.IsValidIndex(Index)) { return false; }
	if (TravelCooldown > 0.f) { return false; }
	const FVEILWaypoint& W = Waypoints[Index];
	if (!W.bLit)
	{
		// A beacon nobody has walked to is a place on the map, not a destination.
		UE_LOG(LogVEIL, Log, TEXT("FAST TRAVEL: %s is not lit yet - reach it on foot first."), *W.Name);
		return false;
	}

	const FVector Before = Pawn->GetActorLocation();
	const FVector Dest = W.Location + FVector(0.f, 0.f, ArriveClearanceCm);
	Pawn->SetActorLocation(Dest, false, nullptr, ETeleportType::TeleportPhysics);
	TravelCooldown = 1.5f;
	UE_LOG(LogVEIL, Log, TEXT("FAST TRAVEL: runner carried to %s - from %s to %s, %.0f cm, same world, no loading."),
		*W.Name, *Before.ToCompactString(), *Dest.ToCompactString(), FVector::Dist(Before, Dest));
	return true;
}
