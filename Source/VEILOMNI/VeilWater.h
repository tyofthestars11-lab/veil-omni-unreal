#pragma once
#include "CoreMinimal.h"

class UWorld;
class AActor;

/**
 * Real water, from the engine's own Water plugin. Not blue planes, not jello pools:
 * these are AWaterBodyLake / AWaterBodyRiver actors with real splines and flow.
 *
 * The classes are loaded by path so this module never hard-links the Water module;
 * the plugin being enabled in the .uproject is what makes them exist at runtime.
 */
namespace VeilWater
{
	/** Real water zone: water bodies only simulate and render inside one. */
	AActor* BuildZone(UWorld* World, const FVector& Centre, double Extent);

	/** A real lake or pool polygon, given its rim. */
	AActor* BuildLake(UWorld* World, const TArray<FVector>& Rim);

	/** A real river with flow along its spline. */
	AActor* BuildRiver(UWorld* World, const TArray<FVector>& Points);

	/** Every body of water the Bible and the painting put in frame. */
	void BuildBibleWater(UWorld* World);
}
