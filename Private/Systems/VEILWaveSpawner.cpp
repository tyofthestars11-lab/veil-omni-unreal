#include "VEILWaveSpawner.h"
#include "PHI_Constants.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVEILWaveSpawner::AVEILWaveSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

UVEILLoopSubsystem* AVEILWaveSpawner::Loop() const
{
	return GetWorld() ? GetWorld()->GetSubsystem<UVEILLoopSubsystem>() : nullptr;
}

void AVEILWaveSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (UVEILLoopSubsystem* L = Loop())
	{
		L->OnStageChanged.AddDynamic(this, &AVEILWaveSpawner::HandleStage);
		if (L->Stage == EVEILStage::FinalWave) { HandleStage(EVEILStage::FinalWave); }
	}
}

float AVEILWaveSpawner::BranchAngleDeg(int32 Step)
{
	float A = FMath::Fmod(Step * static_cast<float>(PHI_Constants::GoldenAngleDeg), 360.f);
	return A < 0.f ? A + 360.f : A;
}

void AVEILWaveSpawner::HandleStage(EVEILStage NewStage)
{
	if (NewStage == EVEILStage::FinalWave && !bRunning)
	{
		bRunning = true;
		CurrentWave = 0;
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AVEILWaveSpawner::StartNextWave, PauseBetweenWaves, false);
	}
}

void AVEILWaveSpawner::StartNextWave()
{
	UVEILLoopSubsystem* L = Loop();
	if (!L || !EnemyClass) { return; }

	if (CurrentWave < UVEILLoopSubsystem::TotalWaves)
	{
		const int32 Drones = UVEILLoopSubsystem::DronesForWave(CurrentWave); // 3,5,8,13,21
		const int32 Soldiers = Drones / 2;
		++CurrentWave;
		for (int32 i = 0; i < Drones; ++i)   { SpawnOne(EVEILEnemyKind::Drone, SpawnCounter++); }
		for (int32 i = 0; i < Soldiers; ++i) { SpawnOne(EVEILEnemyKind::Soldier, SpawnCounter++); }
	}
	else if (!bBossPhase)
	{
		bBossPhase = true;
		CurrentWave = UVEILLoopSubsystem::TotalWaves + 1; // 6 = WARDEN PRIME
		SpawnOne(EVEILEnemyKind::WardenPrime, SpawnCounter++);
	}
}

void AVEILWaveSpawner::SpawnOne(EVEILEnemyKind Kind, int32 Index)
{
	const float Ang = FMath::DegreesToRadians(BranchAngleDeg(Index));
	const FVector Pos = GetActorLocation() + FVector(FMath::Cos(Ang), FMath::Sin(Ang), 0.f) * SpawnRadius
		+ FVector(0.f, 0.f, Kind == EVEILEnemyKind::Drone ? 400.f : 150.f);
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AVEILEnemy* E = GetWorld()->SpawnActor<AVEILEnemy>(EnemyClass, Pos, FRotator::ZeroRotator, P);
	if (!E) { return; }
	E->ConfigureForKind(Kind);
	E->OnEnemyDied.AddDynamic(this, &AVEILWaveSpawner::HandleEnemyDied);
	++Alive;
}

void AVEILWaveSpawner::HandleEnemyDied(AVEILEnemy* Enemy)
{
	Alive = FMath::Max(0, Alive - 1);
	UVEILLoopSubsystem* L = Loop();
	if (!L || Alive > 0) { return; }

	if (bBossPhase)
	{
		L->BossDefeated();       // -> Stage = Beacon
		bRunning = false;
	}
	else
	{
		L->WaveCleared();        // waves 1..5
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AVEILWaveSpawner::StartNextWave, PauseBetweenWaves, false);
	}
}
