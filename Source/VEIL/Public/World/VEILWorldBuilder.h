// VEIL: THE LAST SIGNAL - runtime world builder.
//
// The project ships zero level assets by design: this actor builds the
// playable district from engine primitives at runtime - ground, a phi-scaled
// tower grid around a central plaza, a beacon spire, and a golden dusk
// lighting rig (sun, sky, fog). The GameMode spawns it and calls BuildWorld()
// before anything else, so ANY map - including the editor's empty default -
// becomes VEIL the moment Play is pressed.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILWorldBuilder.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;

UCLASS()
class VEIL_API AVEILWorldBuilder : public AActor
{
	GENERATED_BODY()

public:
	AVEILWorldBuilder();

	/** Build the whole district once. Safe to call again - it builds once. */
	void BuildWorld();

	/** Falling water moves: the falls are strips with no particle system behind them,
	 *  so their Z is walked down each frame and wrapped at the base. */
	virtual void Tick(float DeltaSeconds) override;

	/**
	 * The gold, the crystal and the Spire, in the project's own palette.
	 *
	 * The kit's stone is authored tan and blue-grey and its tint writes land on
	 * parameters the kit materials do not carry, which is why the district kept its flat
	 * sand and blue blocks whatever tint was asked for. M_PHI_Gold, M_PHI_DeepBlue and
	 * M_PHI_WhiteEmissive were authored FOR this project and do carry the colour. So the
	 * new structures are kit geometry with the PHI palette on top: real assets, real
	 * material, and nothing that has to be created in the editor first.
	 */
	void DressPhiPalette();

	/**
	 * The five places, in silhouette.
	 *
	 * From the air a district has to read as its regions, not as one plaza with props: a
	 * market in the foreground, a cliff nest with falling water, gold towers on one flank,
	 * crystal mountains behind, and gold beacon light standing in the distance. Every
	 * piece is kit geometry wearing this project's own palette, placed on golden angles so
	 * nothing is scattered and every object has a job.
	 */
	void DressRegions();

	/**
	 * BAKE THE DISTRICT INTO THE LEVEL, then save the map.
	 *
	 * Until now the district existed only while Play was running: the builder spawned
	 * it every session, so the world was a property of the CODE and of whoever ran it,
	 * not of the project. Open the project anywhere else and there was nothing there.
	 * Baking runs the same build inside the editor and saves the resulting actors into
	 * VEILDistrict, so the city is data in the level - the same actors, in every
	 * session, for anyone who opens the file, with or without this code running.
	 *
	 * Editor only: SaveCurrentLevel does not exist in a cooked game.
	 */
	UFUNCTION(BlueprintCallable, Category = "VEIL|World")
	void BakeToLevel();

	/** phi.goto's body, callable from code: put the runner on a region's arrival
	 *  pad. The console command and every proof hook go through this one function,
	 *  so "it worked when typed" and "it worked in a proof" cannot disagree. */
	static bool GoToRegion(UWorld* World, int32 Index);

	/**
	 * Build only the streets layer - roads, lamps, shops, homes, camps, shrines
	 * and the Keeper's column - on a map that already carries its own stage.
	 * Used on VEILDistrict: the saved level is never torn down at runtime, so
	 * the phi layout is laid OVER it instead of replacing it.
	 */
	void BuildDistrictLayer();

	/** Plaza center, where the runner and the first machines stand. */
	FVector PlazaCenter = FVector(0.f, 0.f, 220.f);

	/** A room the runner can actually walk into, recorded so a proof can stand
	 *  inside it instead of describing it. Set by the first room built. */
	FVector FirstRoomInterior = FVector::ZeroVector;
	FVector FirstRoomDoor = FVector::ZeroVector;
	int32 RoomsBuilt = 0;

private:
	void BuildLighting();
	void BuildDistrict();
	void BuildPhiDistrict();
	void BuildPhiWhole();
	void BuildPhiForms();

	/** The silhouette the reference frame asks for: crystal range, cliff falls,
	 *  terraced houses, bridges, phi pillars, beacon shafts. */
	void BuildVista();

	UStaticMeshComponent* SpawnBlock(const FVector& Location, const FVector& Scale,
		UStaticMesh* Mesh, const FLinearColor& Tint, float EmissiveStrength);

	/** Same, with a yaw: a room's walls have to follow the street they face. */
	UStaticMeshComponent* SpawnBlockRot(const FVector& Location, const FVector& Scale,
		UStaticMesh* Mesh, const FLinearColor& Tint, float EmissiveStrength, const FRotator& Rotation);

	/**
	 * A BUILDING YOU CAN WALK INTO, not a solid block with a sign on it.
	 *
	 * Four walls, a door-sized gap facing the street, a floor you stand on, a lidded
	 * roof split by a skylight band (so the inside is daylit without one extra light
	 * per room), and a hearth in the middle. Nothing here is a new asset: it is the
	 * same authored wall and floor meshes the district already uses.
	 */
	void BuildRoom(const FVector& Centre, float HalfW, float HalfD, float Height,
		const FVector& DoorDir, UStaticMesh* Wall, UStaticMesh* Floor,
		const FLinearColor& Tint, bool bRecord);

	TObjectPtr<UStaticMesh> CubeMesh;
	TObjectPtr<UStaticMesh> CylinderMesh;
	TObjectPtr<UStaticMesh> SphereMesh;

	/** Authored architecture, loaded from the packs in Content. These are what the
	 *  district is actually built from; the engine shapes above are the fallback. */
	TObjectPtr<UStaticMesh> WallMesh;
	TObjectPtr<UStaticMesh> FloorMesh;
	TObjectPtr<UStaticMesh> ColumnMesh;
	TObjectPtr<UStaticMesh> CliffMesh;

	// VARIETY FROM THE PACK, NOT ONE MESH REPEATED. The district used a single
	// wall and a single cliff, so every roof and every rock in the world was the
	// same silhouette - which reads as placeholder even when the art is real.
	// These are the authored variants the packs actually ship.
	TArray<TObjectPtr<UStaticMesh>> WallVariants;    // 6 modular walls
	TArray<TObjectPtr<UStaticMesh>> FloorVariants;   // 5 modular floors
	TArray<TObjectPtr<UStaticMesh>> CliffVariants;   // 6 authored cliffs
	TArray<TObjectPtr<UStaticMesh>> DebrisVariants;  // 8 authored debris pieces
	TArray<TObjectPtr<UStaticMesh>> MountainVariants;// horizon ridges (Iceland pack)
	TObjectPtr<UStaticMesh> ArchMesh;               // a doorway that is an arch
	TObjectPtr<UStaticMesh> MonolithMesh;           // the standing stone at a beacon
	TObjectPtr<UStaticMesh> DomeMesh;               // a real roof
	TObjectPtr<UStaticMesh> ShrineMesh;             // a small shrine
	TObjectPtr<UStaticMesh> RoofMesh;               // an authored roof, not a flipped floor
	TObjectPtr<UStaticMesh> PyramidHeadMesh;        // the Vault's crown
	TObjectPtr<UStaticMesh> StairMesh;              // steps you can walk up

	/**
	 * THE NEST'S PROP BUDGET AND ITS LAYOUT, held per build.
	 *
	 * The starting area is capped at 150 props, spaced at least 300 cm apart and kept
	 * 600 cm clear of every path. It is enforced where a prop is placed - the only place
	 * that can refuse one - and reset when a build starts, so a re-bake counts from zero
	 * instead of inheriting the last pass's numbers.
	 */
	TArray<FVector> NestPropSpots;

	/** The far silhouette: authored ridges on the horizon, so the world does not
	 *  end in empty sky a hundred metres past the last street. */
	void BuildHorizon();

	/** Stone at the three beacon sites - monolith, debris, shrine - placed on the
	 *  authority addresses the statues actually stand on. */
	void DressBeaconSites();

	/** 1 unit of placement scale means "100 cm", whatever the source mesh is.
	 *  Authored meshes are not unit cubes, so every instance is normalised before
	 *  placement - the phi layout then reads the same with real art or without. */
	static float MeshNorm(const UStaticMesh* Mesh);

	/** Registered falling water and the height each strip wraps back to. */
	TMap<TObjectPtr<UStaticMeshComponent>, float> FlowBase;
	float FlowSpeed = 340.f;

	bool bBuilt = false;
	/** Set by BuildDistrictLayer: skip ground, plaza and the player start. */
	bool bSkipBase = false;

	// ------------------------------------------------------------------
	// THE REST OF THE MAP.
	//
	// One plaza with region labels is a single spot. These five build the
	// world the labels name - Hub, Nest, Spire, Omni District, Megastructure
	// Deep - each at the centre UVEILRegionSubsystem already uses, so the
	// region readout and the ground under the runner cannot disagree.
	//
	// All of it is INSTANCED: one component per (shape, colour) instead of an
	// actor per block. A district made of actors is what put this project at
	// 81k objects and a 6 GB card over budget; instances are one draw each.
	// ------------------------------------------------------------------
	void BuildRegions();

private:
	UInstancedStaticMeshComponent* MakeSet(UStaticMesh* Mesh, const FLinearColor& Tint, float Emissive);

	/** One invisible collision box per column instance. The kit's column mesh ships no
	 *  collision geometry, so an instanced set of them looked solid and traced clear. */
	void AddColumnBody(const FVector& Location, const FVector& Scale, UStaticMesh* Mesh);

	/** Enterable buildings from the modular kit, ringed around the plaza on the golden angle. */
	void BuildMarketRing();

	/** Water surfaces, the mountain skyline, and the three tall gold phi pillars. */
	void BuildWaterAndSkyline();
	void AddBlock(UInstancedStaticMeshComponent* Set, const FVector& Location, const FVector& Scale);
	void AddBlockYaw(UInstancedStaticMeshComponent* Set, const FVector& Location,
		const FVector& Scale, float YawDegrees);

	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> InstanceSets;
};
