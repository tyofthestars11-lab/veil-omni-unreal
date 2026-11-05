#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VeilGameMode.generated.h"

class UStaticMesh;
class UMaterialInterface;

/**
 * VEIL OMNI: THE LAST SIGNAL - the world builder.
 *
 * The Bible coordinates are never moved:
 *   Hub (0,0)  Nest (-450,780)  Spire (0,3500,500)  Omni (2800,500)  Deep (0,-5000)
 *   Vault (0,0,-80.9)  Cache (0,-6200)
 *   Keeper Sera Vahl (900,0)  Seer (-450,780)  Countess (809,-309,64.7)
 *   Cartographer (-4150,-780)  Beacons (2800,500) (-2000,-900) (600,-2100)
 * Placement is PHI_Constants only. Nothing is placed in the air: every piece
 * either stands on the Hub terrace deck or traces the real ground under itself.
 */
UCLASS()
class VEILOMNI_API AVeilGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AVeilGameMode();
	virtual void StartPlay() override;
	virtual void BeginPlay() override;

private:
	void BuildWorld();
	void LightGoldenHour();
	void BuildLook();
	void BuildGround();
	void BuildTerrain();
	void BuildHub();
	void BuildFiftySites();
	void BuildBeacons();
	void BuildNest();
	void BuildOmni();
	void BuildDeep();
	void BuildSpire();
	void BuildLife();
	void PlaceRunner();

	void BuildRing(const FVector2D& Centre, int32 Count, const TCHAR* MeshPath,
		const TCHAR* MaterialPath, const TCHAR* Label, double HeightPower);

	AActor* FitPart(const TCHAR* MeshPath, const FVector& Loc, double DesiredWidth,
		double DesiredHeight, const FRotator& Rot, const TCHAR* MaterialPath, bool bSolid);

	UMaterialInterface* Mat(const TCHAR* Path);

	void SeatBody(const FVector& At, int32 RaceIndex, int32 SeatIndex, double WalkRadius, const TCHAR* Label);

	UPROPERTY() TArray<AActor*> Parts;
	FTimerHandle ShotTimer;
	FTimerHandle QuitTimer;
	int32 ShotIdx = 0;

	void TakeShot();
	void Quit();
};
