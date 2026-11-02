// VEIL input help: one table of every action the game binds, on both devices,
// plus the knowledge of which device the player last touched. The tutorial and
// the field manual draw from here, so a key that is advertised is a key that
// is actually bound - the HUD used to promise [TAB] and [R] with nothing
// behind them.
#pragma once

#include "CoreMinimal.h"

/** One row of the field manual: an action, its key, its pad button. */
struct FVEILInputRow
{
	FString Action;
	FString Key;
	/** Empty when the action has no pad binding. */
	FString Pad;

	FVEILInputRow() = default;
	FVEILInputRow(const TCHAR* InAction, const TCHAR* InKey, const TCHAR* InPad)
		: Action(InAction), Key(InKey), Pad(InPad ? InPad : TEXT("")) {}
};

namespace VEILInput
{
	/** The player pressed a key or moved the mouse. */
	void NoteKeyboard();
	/** The player pressed a pad button. */
	void NoteGamepad();

	/** Reads the sticks and triggers directly, so a stick move alone counts. */
	void PollDevice(const class APlayerController* PC);

	/** True when the last real input came from a controller. */
	bool IsGamepad();

	/** Every bound action, in the order the field manual prints it. */
	const TArray<FVEILInputRow>& Rows();

	/** "SPACE" on keyboard, "A" on a pad. */
	FString GlyphFor(const FVEILInputRow& Row);
}
