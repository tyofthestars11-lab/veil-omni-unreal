// VEILKitMesh.h - the bridge between "a cube" and the kit's stone.
//
// Measured: 45 call sites in this project still load /Engine/BasicShapes/Cube|Cylinder|Sphere|
// Cone, so a district built from authored stone still had engine primitives inside it - crowd
// walkers, agent bodies, statue bases, ocean skin, gear. A cube is a placeholder no matter what
// it is tinted with.
//
// One resolver, one mapping, both a runtime call and a compile-time path (ConstructorHelpers
// needs a literal). Every path here was seen loading in a real run: VEIL ASSET lines name
// SM_Floor1_Big, SM_AncientColumn_Big, SM_HlafDome, SM_AncientMonolith, SM_HolderRoof.

#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UStaticMeshComponent;

namespace VEILKit
{
	/** Engine shape name ("Cube", "Cylinder", "Sphere", "Cone", "Plane") -> the kit mesh that
	 *  stands in for it. Null only if every kit path failed, which the caller must log. */
	UStaticMesh* Shape(const TCHAR* EngineShape);

	/** Same mapping as a literal path, for ConstructorHelpers at CDO time. */
	const TCHAR* ShapePath(const TCHAR* EngineShape);

	/** True when this mesh is an engine primitive (i.e. a placeholder). */
	bool IsEngineShape(const UStaticMesh* Mesh);

	/** Re-dress a component: kit mesh in, world size preserved to the centimetre. */
	bool Redress(UStaticMeshComponent* Comp, int32& OutKind);

	/** Logs the mapping once, with what actually resolved. */
	void Report();
}
