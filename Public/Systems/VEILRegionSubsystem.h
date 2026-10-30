#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VEILRegionSubsystem.generated.h"

UENUM(BlueprintType)
enum class EVEILRegion : uint8 { Hub, Nest, Spire, OmniDistrict, MegastructureDeep };

USTRUCT(BlueprintType)
struct FVEILRegionInfo
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FText Name;
	UPROPERTY(BlueprintReadOnly) FVector2D Center = FVector2D::ZeroVector; // world cm (X,Y)
	UPROPERTY(BlueprintReadOnly) FLinearColor Palette = FLinearColor::White;
	UPROPERTY(BlueprintReadOnly) FName MusicCue;                          // Explore_* or Combat_*
	UPROPERTY(BlueprintReadOnly) int32 FragmentSites = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVEILRegionChanged, EVEILRegion, NewRegion);

// Knows the 5 regions of the one open map and tells you when the Countess crosses into another.
// No volumes to place: the region is computed from her position (no loading between regions).
UCLASS()
class VEIL_API UVEILRegionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { return GetStatID(); }

	// Metres on the web map x 100 = Unreal cm. Change if your level is a different size.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL") float WorldScale = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL") EVEILRegion Current = EVEILRegion::Hub;
	UPROPERTY(BlueprintAssignable) FVEILRegionChanged OnRegionChanged;

	UFUNCTION(BlueprintPure, Category = "VEIL") FVEILRegionInfo GetInfo(EVEILRegion R) const;
	UFUNCTION(BlueprintPure, Category = "VEIL") EVEILRegion RegionAt(const FVector& WorldLocation) const;

	// Explore music in Hub / Nest / Omni District, combat music in Spire / Megastructure Deep.
	UFUNCTION(BlueprintPure, Category = "VEIL") static bool IsCombatRegion(EVEILRegion R);

private:
	float PollTimer = 0.f;
};
