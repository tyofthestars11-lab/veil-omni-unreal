#include "Story/VEILVoiceTake.h"

namespace
{
	/** Deterministic 0..1 from four integers - no RNG, so a replay is identical. */
	float Hash01(uint32 A, uint32 B, uint32 C, uint32 D)
	{
		uint32 H = 2166136261u;
		const uint32 Parts[4] = { A, B, C, D };
		for (uint32 P : Parts)
		{
			H ^= P;
			H *= 16777619u;
			H ^= (H >> 13);
		}
		return (float)(H & 0x00FFFFFFu) / (float)0x01000000u;
	}

	struct FVibeProfile
	{
		float PitchLo = 0.94f, PitchHi = 1.06f;
		float RateLo = 0.92f, RateHi = 1.10f;
		float PauseLo = 0.85f, PauseHi = 1.20f;
		float Room = 0.05f;
		float Shimmer = 0.f;
	};

	FVibeProfile VibeFor(FName Id, bool& bOutMachine)
	{
		const FString S = Id.ToString();
		bOutMachine = false;
		FVibeProfile P;

		// MACHINES: narrow, low, clean. "Cool, not creepy" is a numeric instruction:
		// a third of a semitone of pitch travel, 6% of rate, a trace of room.
		if (S.StartsWith(TEXT("unit_")))
		{
			bOutMachine = true;
			P.PitchLo = 0.975f; P.PitchHi = 1.015f;
			P.RateLo = 0.97f;   P.RateHi = 1.03f;
			P.PauseLo = 1.0f;   P.PauseHi = 1.10f;
			P.Room = 0.10f;
			P.Shimmer = 0.12f;
			return P;
		}
		// Statues: old, slow, heavy. They have been saying things for centuries.
		if (S == TEXT("sera_vahl") || S == TEXT("the_listener") || S == TEXT("pale_host"))
		{
			P.PitchLo = 0.88f; P.PitchHi = 0.98f;
			P.RateLo = 0.80f;  P.RateHi = 0.92f;
			P.PauseLo = 1.15f; P.PauseHi = 1.50f;
			P.Room = 0.18f;
			return P;
		}
		// Court: quiet people who never raise their voice.
		if (S == TEXT("vespera_dal") || S == TEXT("ilva_marrow") || S == TEXT("elias_brume"))
		{
			P.PitchLo = 0.93f; P.PitchHi = 1.02f;
			P.RateLo = 0.85f;  P.RateHi = 0.98f;
			P.PauseLo = 1.10f; P.PauseHi = 1.40f;
			return P;
		}
		// Cutters and quartermasters: fast, flat, working voices.
		if (S == TEXT("tova_renn") || S == TEXT("bram_kessler") || S == TEXT("odalys_crane"))
		{
			P.PitchLo = 0.95f; P.PitchHi = 1.04f;
			P.RateLo = 1.02f;  P.RateHi = 1.16f;
			P.PauseLo = 0.75f; P.PauseHi = 0.95f;
			return P;
		}
		return P;
	}

	FString MoodOf(const FString& Line)
	{
		const FString L = Line.ToLower();
		// A REMEMBERED KINDNESS AND A REMEMBERED REFUSAL do not read the same, and
		// the delivery says so: the memory line a person opens with is coloured by
		// which of the two it is, without any extra state passed in.
		if (L.Contains(TEXT("carrying that around")) || L.Contains(TEXT("thank"))
			|| L.Contains(TEXT("welcome")) || L.Contains(TEXT("glad"))) return TEXT("WARM");
		if (L.Contains(TEXT("wrote it down")) || L.Contains(TEXT("you told me"))
			|| L.Contains(TEXT("die")) || L.Contains(TEXT("kill"))
			|| L.Contains(TEXT("debt")) || L.Contains(TEXT("answer"))) return TEXT("SHARP");
		if (L.Contains(TEXT("afraid")) || L.Contains(TEXT("fear")) || L.Contains(TEXT("shiver"))) return TEXT("SHAKY");
		if (L.Contains(TEXT("laugh")) || L.Contains(TEXT("hah"))) return TEXT("BRIGHT");
		return TEXT("DRY");
	}
}

FVEILVoiceTake VEILVoice::ProfileFor(FName CharacterId)
{
	bool bMachine = false;
	const FVibeProfile P = VibeFor(CharacterId, bMachine);
	FVEILVoiceTake T;
	T.Pitch = (P.PitchLo + P.PitchHi) * 0.5f;
	T.Rate = (P.RateLo + P.RateHi) * 0.5f;
	T.Pause = (P.PauseLo + P.PauseHi) * 0.5f;
	T.Room = P.Room;
	T.Shimmer = P.Shimmer;
	T.bMachine = bMachine;
	T.Mood = TEXT("DRY");
	return T;
}

FVEILVoiceTake VEILVoice::TakeFor(FName CharacterId, const FString& Line,
	int32 TalkCount, int32 LineIndex)
{
	bool bMachine = false;
	const FVibeProfile P = VibeFor(CharacterId, bMachine);

	const uint32 SeedA = GetTypeHash(CharacterId);
	const uint32 SeedB = GetTypeHash(Line);
	const uint32 SeedC = (uint32)FMath::Max(0, TalkCount);
	const uint32 SeedD = (uint32)FMath::Max(0, LineIndex);

	FVEILVoiceTake T;
	// Each number gets its own hash channel, so pitch does not move in lockstep
	// with rate - that lockstep is exactly what makes sampled voices sound looped.
	T.Pitch   = FMath::Lerp(P.PitchLo, P.PitchHi, Hash01(SeedA, SeedB, SeedC, SeedD + 1u));
	T.Rate    = FMath::Lerp(P.RateLo,  P.RateHi,  Hash01(SeedA, SeedB, SeedC + 7u, SeedD));
	T.Pause   = FMath::Lerp(P.PauseLo, P.PauseHi, Hash01(SeedA, SeedB + 3u, SeedC, SeedD));
	T.Room    = P.Room;
	T.Shimmer = P.Shimmer * Hash01(SeedA, SeedB + 11u, SeedC, SeedD);
	T.bMachine = bMachine;

	// The line's own words colour the delivery, so mood is not a dice roll.
	T.Mood = MoodOf(Line);
	if (T.Mood == TEXT("WARM"))   { T.Rate *= 1.04f; T.Pitch *= 1.01f; }
	if (T.Mood == TEXT("SHARP"))  { T.Rate *= 1.06f; T.Pitch *= 0.985f; T.Pause *= 0.85f; }
	if (T.Mood == TEXT("SHAKY"))  { T.Rate *= 0.93f; T.Pause *= 1.2f; }
	if (T.Mood == TEXT("BRIGHT")) { T.Pitch *= 1.03f; T.Rate *= 1.06f; }

	return T;
}

FString VEILVoice::Describe(FName CharacterId, const FVEILVoiceTake& Take)
{
	return FString::Printf(
		TEXT("%s  pitch %.3f  rate %.3f  pause %.2f  room %.2f  shimmer %.2f  mood %s%s"),
		*CharacterId.ToString(), Take.Pitch, Take.Rate, Take.Pause, Take.Room, Take.Shimmer,
		*Take.Mood, Take.bMachine ? TEXT("  [machine]") : TEXT(""));
}
