#include "Story/VEILRaceGear.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "VEIL.h"

namespace
{
	UStaticMesh* GearCube = nullptr;
	UStaticMesh* GearCylinder = nullptr;
	UStaticMesh* GearCone = nullptr;
	UMaterialInterface* GearGold = nullptr;
	UMaterialInterface* GearBlue = nullptr;
	UMaterialInterface* GearGlow = nullptr;
	UMaterialInterface* GearStone = nullptr;
	UMaterialInterface* GearDark = nullptr;

	void LoadGear()
	{
		if (GearCube) { return; }
		GearCube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		GearCylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		GearCone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
		GearGold = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic"));
		GearBlue = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue"));
		GearGlow = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"));
		GearStone = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
		GearDark = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/StarterContent/Materials/M_Metal_Dark.M_Metal_Dark"));
	}

	UStaticMeshComponent* Attach(AActor* Owner, USkeletalMeshComponent* Body, UStaticMesh* Mesh,
		const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat)
	{
		if (!Owner || !Body || !Mesh) { return nullptr; }
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(Owner);
		if (!C) { return nullptr; }
		C->SetupAttachment(Body);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeRotation(Rot);
		C->SetRelativeScale3D(Scale);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetCastShadow(false);
		if (Mat) { C->SetMaterial(0, Mat); }
		C->RegisterComponent();
		return C;
	}
}

const TCHAR* VEILRaceGear::RoleOf(FName CastId, EVEILFaction Faction)
{
	const FString Id = CastId.ToString();
	if (CastId.IsNone() && Faction == EVEILFaction::None) { return TEXT("WINGS"); }
	if (Id == TEXT("sera_vahl")) { return TEXT("LANTERN"); }
	if (Id == TEXT("ilva_marrow")) { return TEXT("CRYSTAL STAFF"); }
	if (Id == TEXT("domen_arclight")) { return TEXT("MAP"); }
	if (Id == TEXT("vespera_dal") || Id == TEXT("elias_brume") || Id == TEXT("corin_vey")
		|| Id == TEXT("adea_lune") || Id == TEXT("sahn")) { return TEXT("GOLD COURT"); }
	if (Id == TEXT("the_listener") || Id == TEXT("pale_host") || Id == TEXT("the_third_echo")
		|| Id == TEXT("ash_witness") || Id == TEXT("pale_choir") || Id == TEXT("the_answerer"))
	{
		return TEXT("CRYSTAL HOLLOW");
	}
	if (Faction == EVEILFaction::Remnant) { return TEXT("GOLD COURT"); }
	if (Faction == EVEILFaction::Void) { return TEXT("CRYSTAL HOLLOW"); }
	if (Faction == EVEILFaction::Guardian) { return TEXT("HAND LANTERN"); }
	if (Faction == EVEILFaction::Scavenger) { return TEXT("SATCHEL"); }
	return TEXT("NONE");
}

void VEILRaceGear::Apply(AActor* Owner, USkeletalMeshComponent* Body, FName CastId, EVEILFaction Faction)
{
	LoadGear();
	if (!Owner || !Body || !GearCube) { return; }

	const TCHAR* Role = RoleOf(CastId, Faction);
	UE_LOG(LogVEIL, Log, TEXT("PHI RACE GEAR: %s wears %s."), *Owner->GetName(), Role);
	const FString Id = CastId.ToString();

	// THE RUNNER: GOLD PHI WINGS AND A DARK CAPE. The first figure in the lineup is
	// recognised before any nameplate: two swept gold blades off the shoulders and
	// cloth behind. Hung on the body, so it turns and walks with him.
	if (CastId.IsNone() && Faction == EVEILFaction::None)
	{
		Attach(Owner, Body, GearCube, FVector(-10.f, 36.f, 96.f), FRotator(-16.f, 24.f, 30.f),
			FVector(0.03f, 0.50f, 1.30f), GearGold);
		Attach(Owner, Body, GearCube, FVector(-10.f, -36.f, 96.f), FRotator(-16.f, -24.f, -30.f),
			FVector(0.03f, 0.50f, 1.30f), GearGold);
		Attach(Owner, Body, GearCube, FVector(-6.f, 0.f, 62.f), FRotator(-8.f, 0.f, 0.f),
			FVector(0.04f, 0.62f, 1.05f), GearDark);
		UE_LOG(LogVEIL, Warning, TEXT("PHI RACE GEAR: %s wears the RUNNER's gold phi wings and cape."), *Owner->GetName());
		return;
	}

	if (Id == TEXT("sera_vahl"))
	{
		// THE LANTERN STAFF. A dark haft in the right hand, a lit housing at its head:
		// the Keeper is recognisable by the light she carries, not by a label.
		Attach(Owner, Body, GearCylinder, FVector(22.f, 34.f, 40.f), FRotator(6.f, 0.f, -6.f),
			FVector(0.05f, 0.05f, 1.85f), GearDark);
		Attach(Owner, Body, GearCube, FVector(22.f, 34.f, 128.f), FRotator::ZeroRotator,
			FVector(0.22f, 0.22f, 0.30f), GearGlow);
		Attach(Owner, Body, GearCube, FVector(22.f, 34.f, 146.f), FRotator::ZeroRotator,
			FVector(0.28f, 0.28f, 0.05f), GearGold);
	}
	else if (Id == TEXT("ilva_marrow"))
	{
		// THE CRYSTAL STAFF: dark haft, a blue shard bound at the top.
		Attach(Owner, Body, GearCylinder, FVector(20.f, -30.f, 42.f), FRotator(4.f, 0.f, 8.f),
			FVector(0.05f, 0.05f, 1.95f), GearDark);
		Attach(Owner, Body, GearCone, FVector(20.f, -30.f, 150.f), FRotator(0.f, 22.f, 0.f),
			FVector(0.16f, 0.16f, 0.55f), GearBlue);
	}
	else if (Id == TEXT("domen_arclight"))
	{
		// THE MAP: a pale sheet held forward, with a gold edge.
		Attach(Owner, Body, GearCube, FVector(28.f, 0.f, 62.f), FRotator(0.f, 0.f, 14.f),
			FVector(0.02f, 0.62f, 0.46f), GearStone);
		Attach(Owner, Body, GearCube, FVector(30.f, 0.f, 84.f), FRotator(0.f, 0.f, 14.f),
			FVector(0.02f, 0.66f, 0.04f), GearGold);
	}
	else if (Faction == EVEILFaction::Remnant || Id == TEXT("vespera_dal"))
	{
		// GOLD COURT: pauldrons and a collar trim, the noble silhouette.
		Attach(Owner, Body, GearCube, FVector(0.f, 30.f, 78.f), FRotator(0.f, 0.f, -12.f),
			FVector(0.34f, 0.20f, 0.16f), GearGold);
		Attach(Owner, Body, GearCube, FVector(0.f, -30.f, 78.f), FRotator(0.f, 0.f, 12.f),
			FVector(0.34f, 0.20f, 0.16f), GearGold);
		Attach(Owner, Body, GearCube, FVector(0.f, 0.f, 88.f), FRotator::ZeroRotator,
			FVector(0.20f, 0.34f, 0.06f), GearGold);
	}
	else if (Faction == EVEILFaction::Void)
	{
		// CRYSTAL HOLLOW: shards growing out of the shoulders, lit blue.
		Attach(Owner, Body, GearCone, FVector(0.f, 26.f, 82.f), FRotator(18.f, 0.f, -16.f),
			FVector(0.16f, 0.16f, 0.52f), GearBlue);
		Attach(Owner, Body, GearCone, FVector(0.f, -26.f, 82.f), FRotator(18.f, 0.f, 16.f),
			FVector(0.14f, 0.14f, 0.44f), GearBlue);
		Attach(Owner, Body, GearCone, FVector(0.f, 0.f, 70.f), FRotator(12.f, 20.f, 0.f),
			FVector(0.12f, 0.12f, 0.38f), GearGlow);
	}
	else if (Faction == EVEILFaction::Guardian)
	{
		// THE WATCH CARRY A LIGHT. A hand lantern at the hip, lit.
		Attach(Owner, Body, GearCube, FVector(14.f, 30.f, 34.f), FRotator::ZeroRotator,
			FVector(0.16f, 0.16f, 0.22f), GearGlow);
		Attach(Owner, Body, GearCube, FVector(14.f, 30.f, 48.f), FRotator::ZeroRotator,
			FVector(0.19f, 0.19f, 0.04f), GearGold);
	}
	else if (Faction == EVEILFaction::Scavenger)
	{
		// ASHFALL, the market people: a satchel and a strap.
		Attach(Owner, Body, GearCube, FVector(-6.f, -30.f, 20.f), FRotator(0.f, 0.f, -8.f),
			FVector(0.30f, 0.20f, 0.28f), GearStone);
		Attach(Owner, Body, GearCube, FVector(0.f, 0.f, 52.f), FRotator(0.f, 40.f, 0.f),
			FVector(0.32f, 0.05f, 0.02f), GearGold);
	}
}
