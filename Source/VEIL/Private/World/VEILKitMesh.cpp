// VEILKitMesh.cpp - the mapping itself.

#include "World/VEILKitMesh.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "VEIL.h"

namespace
{
	const TCHAR* KitPathFor(const FString& Kind)
	{
		if (Kind.Contains(TEXT("Cube")) || Kind.Contains(TEXT("Box")))
		{
			return TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor1_Big.SM_Floor1_Big");
		}
		if (Kind.Contains(TEXT("Cylinder")))
		{
			return TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big");
		}
		if (Kind.Contains(TEXT("Sphere")))
		{
			return TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_HlafDome.SM_HlafDome");
		}
		if (Kind.Contains(TEXT("Cone")))
		{
			return TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientMonolith.SM_AncientMonolith");
		}
		return TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor2_Big.SM_Floor2_Big");
	}

	TMap<FString, TWeakObjectPtr<UStaticMesh>>& Cache()
	{
		static TMap<FString, TWeakObjectPtr<UStaticMesh>> Map;
		return Map;
	}
}

namespace VEILKit
{
	const TCHAR* ShapePath(const TCHAR* EngineShape)
	{
		return KitPathFor(FString(EngineShape ? EngineShape : TEXT("Cube")));
	}

	UStaticMesh* Shape(const TCHAR* EngineShape)
	{
		const FString Kind = EngineShape ? EngineShape : TEXT("Cube");
		if (TWeakObjectPtr<UStaticMesh>* Found = Cache().Find(Kind))
		{
			if (UStaticMesh* Cached = Found->Get()) { return Cached; }
		}

		const TCHAR* Path = KitPathFor(Kind);
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path);
		if (!Mesh)
		{
			// Honest fallback: the engine shape, and the log says so.
			Mesh = LoadObject<UStaticMesh>(nullptr,
				*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"), *Kind, *Kind));
			UE_LOG(LogVEIL, Warning, TEXT("PHI KIT: %s kit mesh MISSING (%s) - engine shape stands in."),
				*Kind, Path);
		}
		Cache().Add(Kind, Mesh);
		return Mesh;
	}

	bool IsEngineShape(const UStaticMesh* Mesh)
	{
		return Mesh && Mesh->GetPathName().StartsWith(TEXT("/Engine/BasicShapes/"));
	}

	bool Redress(UStaticMeshComponent* Comp, int32& OutKind)
	{
		if (!Comp) { return false; }
		UStaticMesh* Old = Comp->GetStaticMesh();
		if (!IsEngineShape(Old)) { return false; }

		const FString OldPath = Old->GetPathName();
		const FString Kind = OldPath.RightChop(OldPath.Find(TEXT("/Engine/BasicShapes/")) + 20);
		UStaticMesh* New = Shape(*Kind);
		if (!New || New == Old) { return false; }

		// THE SIZE IS THE PROMISE. The placeholder occupied a measured volume in this world; the
		// kit mesh that replaces it is fitted to that same volume, so nothing grows, shrinks or
		// starts floating because the art changed.
		const FVector WantSize = Comp->Bounds.BoxExtent * 2.f;
		const FVector UnitSize = New->GetBoundingBox().GetSize();
		if (UnitSize.X <= 1.f || UnitSize.Y <= 1.f || UnitSize.Z <= 1.f) { return false; }
		if (WantSize.X <= 1.f || WantSize.Y <= 1.f || WantSize.Z <= 1.f) { return false; }

		Comp->SetStaticMesh(New);
		Comp->SetWorldScale3D(FVector(WantSize.X / UnitSize.X, WantSize.Y / UnitSize.Y, WantSize.Z / UnitSize.Z));

		OutKind = Kind.Contains(TEXT("Cylinder")) ? 1
			: Kind.Contains(TEXT("Sphere")) ? 2
			: Kind.Contains(TEXT("Cone")) ? 3
			: 0;
		return true;
	}

	void Report()
	{
		const TCHAR* Kinds[] = { TEXT("Cube"), TEXT("Cylinder"), TEXT("Sphere"), TEXT("Cone"), TEXT("Plane") };
		for (const TCHAR* Kind : Kinds)
		{
			UStaticMesh* Mesh = Shape(Kind);
			UE_LOG(LogVEIL, Warning, TEXT("PHI KIT: %s -> %s%s"), Kind,
				Mesh ? *Mesh->GetName() : TEXT("MISSING"),
				(Mesh && IsEngineShape(Mesh)) ? TEXT(" (engine shape - kit path failed)") : TEXT(""));
		}
	}
}
