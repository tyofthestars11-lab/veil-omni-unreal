// Fast travel by lit beacons - no loading walls, no menu that breaks the world.
//
// A beacon lights the first time the runner walks up to it (the same physical
// act as touching it), and once two are lit, one key carries him between them.
// One open world: the travel is a move inside the same map, never a load.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VEILFastTravel.generated.h"

USTRUCT(BlueprintType)
struct FVEILWaypoint
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Travel") FString Name;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Travel") FVector Location = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Travel") bool bLit = false;
};

UCLASS()
class VEIL_API UVEILFastTravel : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { return GetStatID(); }

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Travel")
	TArray<FVEILWaypoint> Waypoints;

	/** How many beacons the runner has reached: only these can be travelled to. */
	UFUNCTION(BlueprintPure, Category = "VEIL|Travel")
	int32 LitCount() const;

	/** Index of the next lit beacon that is not the one he is standing at. */
	UFUNCTION(BlueprintPure, Category = "VEIL|Travel")
	int32 NextLitIndex(const FVector& From) const;

	/** Move the runner to the next lit beacon. False when none is lit yet. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Travel")
	bool TravelToNext(APawn* Pawn);

	/** Move the runner to a CHOSEN lit beacon. The chart asks for the index;
	 *  the world moves under his feet, no load, no wall. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Travel")
	bool TravelTo(int32 Index, APawn* Pawn);

	/** Index of the lit beacon he is standing at, or INDEX_NONE. */
	UFUNCTION(BlueprintPure, Category = "VEIL|Travel")
	int32 StandingAtIndex(const FVector& From) const;

	/** One line for the HUD: "2 of 6 beacons lit". */
	UFUNCTION(BlueprintPure, Category = "VEIL|Travel")
	FString Status() const;

	/** Registers a beacon the first time it is seen. Idempotent per actor. */
	void Register(AActor* Beacon, const FString& Name);

private:
	UPROPERTY() TArray<TWeakObjectPtr<AActor>> BeaconActors;
	float ScanTimer = 0.f;
	float TravelCooldown = 0.f;
	bool bFirstBeaconLighted = false;
	void Scan();
};
