// PHI CHEATS - the GODMODE page.
//
// GTA's cheat list and a trainer menu, folded into the game's own UI: one key,
// no download, no pay, nothing that survives a restart. Every row is a real
// switch wired into the systems that already exist (the character's damage
// gate, the movement component, the quest ledger, the machines' tick, the sky's
// own clock), so a toggle does the thing rather than printing that it did.
//
// Cheats are session state on purpose. Nothing here is written to a save, and
// nothing here is read back at boot, so a clean run is clean.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PHICheats.generated.h"

class AVEILCharacter;

UCLASS()
class VEIL_API UPHICheats : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** The switch rows, in menu order. Actions sit below them.
	 *  The first seventeen keep their original order - proof runs address rows
	 *  by index - and the rest extend the same page. */
	enum class ERow : uint8
	{
		GodMode = 0,
		InfiniteHealth,
		Fly,
		Noclip,
		DamageX2,
		DamageX5,
		DamageX10,
		OneHitKill,
		FreezeEnemies,
		InfiniteResources,
		SuperJump,
		TeleportKeeper,
		TeleportSeer,
		TeleportCache,
		MarkSpot,
		ReturnToMark,
		AllOff,
		// ---- appended: the rest of the GTA / trainer set ----
		KillAllNearby,
		MaxReputation,
		ZeroReputation,
		TeleportCartographer,
		TimeSkip,
		WeatherShift,
		SpawnAlly,
		SpawnEnemy,
		LevelUp,
		RestoreAll,
		SaveNow,
		Count
	};

	static constexpr int32 RowCount = (int32)ERow::Count;
	/** Rows per column on the page. 28 rows -> two columns of 14. */
	static constexpr int32 RowsPerColumn = 14;

	/** Flip or run the row. One entry point, so menu, console and proof all agree. */
	void Activate(int32 Row);
	bool IsOn(int32 Row) const;
	FString LabelOf(int32 Row) const;
	FString ValueOf(int32 Row) const;

	void AllOff();
	bool AnyOn() const;

	/** Freeze check the machines call from their own tick. */
	static bool AreEnemiesFrozen(const UWorld* World);

	FString StatusLine() const;

	/** Console + proof handles, one per action that needs no menu. */
	UFUNCTION(Exec) void CheatsOn(int32 Row) { Activate(Row); }
	UFUNCTION(Exec) void CheatsAllOff() { AllOff(); }

private:
	AVEILCharacter* Runner() const;
	void ApplyDamageTier(AVEILCharacter* R);
	void TeleportTo(const FString& Why, const FVector& Where);
	void SetReputation(float Amount, const TCHAR* Why);
	void KillNearby();
	void AdvanceWorldHour();
	void CycleWeather();
	void SummonAlly();
	void SummonEnemy();
	void GrantLevel();
	void RestoreAll();
	void SaveNow();

	bool bGodMode = false;
	bool bInfiniteHealth = false;
	bool bFly = false;
	bool bNoclip = false;
	int32 DamageTier = 1;            // 1, 2, 5, 10
	bool bOneHitKill = false;
	bool bFreezeEnemies = false;
	bool bInfiniteResources = false;
	bool bSuperJump = false;

	/** The runner's own numbers, kept so a switch can put them back exactly. */
	float BaseWeaponDamage = -1.f;
	float BaseSwingDamage = -1.f;
	float BaseWalkSpeed = -1.f;
	float BaseJumpZ = -1.f;

	FVector MarkLocation = FVector::ZeroVector;
	bool bHasMark = false;
	int32 Marks = 0;

	/** Which way the last hand on reputation went: 0 none, +1 max, -1 hunt. */
	int32 ReputationHand = 0;

	/** Counters the page shows so a press is never a no-op you cannot see. */
	int32 Kills = 0;
	int32 Summons = 0;
	int32 HoursSkipped = 0;
	int32 WeatherShifts = 0;
	int32 LevelsGiven = 0;
	int32 Saves = 0;
};
