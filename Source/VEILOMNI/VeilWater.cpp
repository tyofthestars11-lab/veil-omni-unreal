#include "VeilWater.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "UObject/SoftObjectPath.h"
#include "Engine/StaticMeshActor.h"

namespace
{
	UClass* WaterClass(const TCHAR* Path)
	{
		UClass* C = StaticLoadClass(AActor::StaticClass(), nullptr, Path);
		if (!C)
		{
			UE_LOG(LogTemp, Warning, TEXT("VEIL WATER: class missing at %s (Water plugin not enabled?)"), Path);
		}
		return C;
	}

	AActor* SpawnWater(UWorld* World, const TCHAR* ClassPath, const FVector& At)
	{
		if (!World)
		{
			return nullptr;
		}
		UClass* C = WaterClass(ClassPath);
		if (!C)
		{
			return nullptr;
		}
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<AActor>(C, At, FRotator::ZeroRotator, P);
	}

	/** Water bodies carry a spline; the rim or the course is written onto it. */
	void SetCourse(AActor* Body, const TArray<FVector>& Points)
	{
		if (!Body || Points.Num() == 0)
		{
			return;
		}
		if (USplineComponent* S = Body->FindComponentByClass<USplineComponent>())
		{
			S->ClearSplinePoints(false);
			for (const FVector& P : Points)
			{
				S->AddSplinePoint(P, ESplineCoordinateSpace::World, false);
			}
			S->UpdateSpline();
		}
	}
}

namespace VeilWater
{
	AActor* BuildZone(UWorld* World, const FVector& Centre, double Extent)
	{
		AActor* Zone = SpawnWater(World, TEXT("/Script/Water.WaterZone"), Centre);
		if (Zone)
		{
			Zone->SetActorScale3D(FVector(Extent / 1000.0));
			UE_LOG(LogTemp, Display, TEXT("VEIL WATER: zone at (%.0f, %.0f, %.0f) extent %.0f"),
				Centre.X, Centre.Y, Centre.Z, Extent);
		}
		return Zone;
	}

	AActor* BuildLake(UWorld* World, const TArray<FVector>& Rim)
	{
		if (Rim.Num() < 3)
		{
			return nullptr;
		}
		AActor* Lake = SpawnWater(World, TEXT("/Script/Water.WaterBodyLake"), Rim[0]);
		SetCourse(Lake, Rim);
		if (Lake)
		{
			UE_LOG(LogTemp, Display, TEXT("VEIL WATER: lake with %d rim points at (%.0f, %.0f, %.0f)"),
				Rim.Num(), Rim[0].X, Rim[0].Y, Rim[0].Z);
		}
		return Lake;
	}

	AActor* BuildRiver(UWorld* World, const TArray<FVector>& Points)
	{
		if (Points.Num() < 2)
		{
			return nullptr;
		}
		AActor* River = SpawnWater(World, TEXT("/Script/Water.WaterBodyRiver"), Points[0]);
		SetCourse(River, Points);
		if (River)
		{
			UE_LOG(LogTemp, Display, TEXT("VEIL WATER: river with %d course points from (%.0f, %.0f, %.0f)"),
				Points.Num(), Points[0].X, Points[0].Y, Points[0].Z);
		}
		return River;
	}

	void BuildBibleWater(UWorld* World)
	{
		if (!World)
		{
			return;
		}
		BuildZone(World, FVector(0.0, 0.0, 100.0), 14000.0);

		// The lake under the Nest, where the Seer sits at (-450, 780).
		const double Z = 182000.0;
		TArray<FVector> NestRim;
		for (int32 i = 0; i < 12; ++i)
		{
			const double A = (2.0 * PI * i) / 12.0;
			NestRim.Add(FVector(-450.0 + FMath::Cos(A) * 1400.0, 780.0 + FMath::Sin(A) * 1400.0, Z));
		}
		BuildLake(World, NestRim);

		// The river that leaves the Hub cliffs and drops toward the Deep.
		TArray<FVector> Course;
		Course.Add(FVector(-2000.0, 2400.0, Z + 300.0));
		Course.Add(FVector(-1200.0, 1800.0, Z + 120.0));
		Course.Add(FVector(-450.0, 780.0, Z));
		Course.Add(FVector(200.0, -900.0, Z - 60.0));
		Course.Add(FVector(400.0, -3000.0, Z - 90.0));
		BuildRiver(World, Course);

		// The blue city sits on a plateau; one lake feeds every light shaft.
		TArray<FVector> OmniRim;
		for (int32 i = 0; i < 10; ++i)
		{
			const double A = (2.0 * PI * i) / 10.0;
			OmniRim.Add(FVector(2800.0 + FMath::Cos(A) * 900.0, 500.0 + FMath::Sin(A) * 900.0, Z));
		}
		BuildLake(World, OmniRim);
	}
}
