#pragma once

#include "CoreMinimal.h"

/**
 * VOICE, NOT A SAMPLE.
 *
 * Every speaker has a PROFILE - a range of pitch, rate, pause and shimmer - and
 * every line picks a take inside that range from a hash of (speaker, line text,
 * how many times you have spoken to them). Same person every time, never the
 * same take twice, and reproducible: the hash means a replay of the same line in
 * the same conversation state sounds the same.
 *
 * Machines are not exempt and they are not monotone. A construct's profile is
 * narrow, low and clean - cool, not creepy: small shimmer, small room, no echo
 * chamber - and its mood still moves the take like anyone else's.
 *
 * HONEST LIMIT: this project has no speech synthesiser wired in. This is the
 * performance data a voice backend would consume - pitch/rate/pause/emotion per
 * line - and it is logged for every line so the variation is measurable now. It
 * is not audio, and it is not described as audio.
 */
struct FVEILVoiceTake
{
	/** Multiplier around the speaker's own pitch. 1.0 = their centre. */
	float Pitch = 1.f;
	/** Multiplier on delivery speed. */
	float Rate = 1.f;
	/** Multiplier on the pause before the line lands. */
	float Pause = 1.f;
	/** Wetness of the room on the voice, 0..1. Machines get a little, nobody gets an echo. */
	float Room = 0.f;
	/** Metallic edge, 0..1. Machines only, and small. */
	float Shimmer = 0.f;
	/** One word the delivery is coloured by: WARM / SHARP / SHAKY / DRY / BRIGHT. */
	FString Mood;
	bool bMachine = false;
};

namespace VEILVoice
{
	/** The profile for a speaker, whether or not they are in the cast table. */
	FVEILVoiceTake ProfileFor(FName CharacterId);

	/** The take for one line, deterministic in (speaker, line, talk count, line index). */
	FVEILVoiceTake TakeFor(FName CharacterId, const FString& Line, int32 TalkCount, int32 LineIndex);

	/** One log line: who, and the four numbers that would drive a synthesiser. */
	FString Describe(FName CharacterId, const FVEILVoiceTake& Take);
}
