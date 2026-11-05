#pragma once
#include "CoreMinimal.h"
#include "PHI_Constants.h"

/**
 * The peoples and the enemies of the Bible, in one table.
 *
 * Every proportion here is derived, never typed: height is a step on the phi
 * ladder from Spacing, pace and reach are the Bible walk number 439 taken down
 * the ladder by phi. Change phi and the bodies change with it.
 *
 * Bible races: human, gold people, crystal people, Deep people.
 * Kraken enemy variety: Crystal Guardian, Gold Warrior, Deep Dweller, Beacon Shadow.
 * Every enemy holds its own post: follow_player False, orbit False, reach from the ladder.
 */
struct FVeilRace
{
	const TCHAR* Name;
	const TCHAR* Body;
	const TCHAR* Skin;
	double Height;
	double Speed;
	double Radius;
	bool bEnemy;
	double Reach;
	const TCHAR* Role;
};

namespace VeilRaces
{
	// The Bible walk, 439 cm/s, is the root of every living pace in the world.
	inline double Pace(double Rung) { return 439.0 * PHI_Constants::PhiPow(Rung); }

	inline const FVeilRace& Get(int32 Index)
	{
		static const FVeilRace Table[8] =
		{
			{
				TEXT("Human"),
				TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody"),
				nullptr,
				PHI_Constants::Reach(0.20), Pace(-1.75), Pace(-0.60), false, 0.0,
				TEXT("trade the market, keep the watch")
			},
			{
				TEXT("Gold"),
				TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"),
				TEXT("/Game/VEIL/Materials/M_VEIL_Gold2.M_VEIL_Gold2"),
				PHI_Constants::Reach(0.05), Pace(-2.25), Pace(-0.70), false, 0.0,
				TEXT("walk the pillars in slow procession")
			},
			{
				TEXT("Crystal"),
				TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody"),
				TEXT("/Game/VEIL/Materials/M_VEIL_Signal.M_VEIL_Signal"),
				PHI_Constants::Reach(0.40), Pace(-1.90), Pace(1.00), false, 0.0,
				TEXT("drift wide and carry the signal")
			},
			{
				TEXT("Deep"),
				TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"),
				TEXT("/Game/VEIL/Materials/M_VEIL_Deep.M_VEIL_Deep"),
				PHI_Constants::Reach(-0.10), Pace(-1.35), Pace(0.25), false, 0.0,
				TEXT("patrol the bridges of the pit")
			},
			{
				TEXT("Crystal Guardian"),
				TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"),
				TEXT("/Game/VEIL/Materials/M_VEIL_Signal.M_VEIL_Signal"),
				PHI_Constants::Reach(0.45), 0.0, 0.0, true, Pace(-0.75),
				TEXT("guard the blue city")
			},
			{
				TEXT("Gold Warrior"),
				TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody"),
				TEXT("/Game/VEIL/Materials/M_VEIL_Gold.M_VEIL_Gold"),
				PHI_Constants::Reach(0.25), 0.0, 0.0, true, Pace(-0.75),
				TEXT("guard the market walls")
			},
			{
				TEXT("Deep Dweller"),
				TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"),
				TEXT("/Game/VEIL/Materials/M_VEIL_Deep.M_VEIL_Deep"),
				PHI_Constants::Reach(-0.12), 0.0, 0.0, true, Pace(-0.75),
				TEXT("keep the pit bridges")
			},
			{
				TEXT("Beacon Shadow"),
				TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody"),
				TEXT("/Game/VEIL/Materials/M_VEIL_Signal.M_VEIL_Signal"),
				PHI_Constants::Reach(0.30), 0.0, 0.0, true, Pace(-0.75),
				TEXT("stand at the beacons")
			}
		};
		return Table[FMath::Clamp(Index, 0, 7)];
	}

	inline int32 Count()
	{
		return 8;
	}
}
