#include "Systems/VEILInputHelp.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "InputCoreTypes.h"

namespace
{
	bool bLastWasGamepad = false;

	const TArray<FVEILInputRow>& BuildRows()
	{
		static const TArray<FVEILInputRow> Table = {
			{ TEXT("MOVE"),            TEXT("W A S D"),      TEXT("LEFT STICK") },
			{ TEXT("LOOK"),            TEXT("MOUSE"),        TEXT("RIGHT STICK") },
			{ TEXT("SPRINT"),          TEXT("SHIFT"),        TEXT("L3") },
			{ TEXT("JUMP"),            TEXT("SPACE"),        TEXT("A") },
			{ TEXT("DODGE"),           TEXT("LEFT ALT"),     TEXT("B") },
			{ TEXT("CAMERA 1ST/3RD"),  TEXT("V"),            TEXT("R3") },
			{ TEXT("ZOOM LENS"),       TEXT("MOUSE WHEEL"),  TEXT("LB / RB") },
			{ TEXT("AIM"),             TEXT("RIGHT MOUSE"),  TEXT("LT") },
			{ TEXT("FIRE LANCE"),      TEXT("LEFT MOUSE"),   TEXT("RT") },
			{ TEXT("SWING"),           TEXT("F"),            TEXT("Y") },
			{ TEXT("INTERACT / SPEAK"),TEXT("E"),            TEXT("X") },
			{ TEXT("NEXT LINE"),       TEXT("SPACE"),        TEXT("A") },
			{ TEXT("CHART / LIVING WORLD"), TEXT("TAB"),     TEXT("VIEW") },
			{ TEXT("SWITCH ARCHETYPE"),TEXT("1 - 0"),        TEXT("-") },
			{ TEXT("SWITCH LEAD"),     TEXT("C"),            TEXT("D-PAD RIGHT") },
			{ TEXT("RESPAWN"),         TEXT("R"),            TEXT("D-PAD UP") },
			{ TEXT("MUTE NARRATOR"),   TEXT("M"),            TEXT("D-PAD DOWN") },
			{ TEXT("FIELD MANUAL"),    TEXT("H / F1"),        TEXT("D-PAD LEFT") },
			{ TEXT("PAUSE"),           TEXT("P / ESC"),      TEXT("MENU") },
			{ TEXT("HIDE TIPS"),       TEXT("T"),            TEXT("") },
		};
		return Table;
	}
}

namespace VEILInput
{
	void NoteKeyboard()
	{
		bLastWasGamepad = false;
	}

	void NoteGamepad()
	{
		bLastWasGamepad = true;
	}

	void PollDevice(const APlayerController* PC)
	{
		if (!PC || !PC->PlayerInput)
		{
			return;
		}
		// Keyboard keys win if they are actually held: a resting controller on a
		// desk can sit on a noisy axis, but never reads a WASD key.
		static const FKey LetterKeys[] = {
			EKeys::W, EKeys::A, EKeys::S, EKeys::D, EKeys::SpaceBar, EKeys::LeftShift
		};
		for (const FKey& K : LetterKeys)
		{
			if (PC->PlayerInput->GetKeyValue(K) > 0.3f)
			{
				bLastWasGamepad = false;
				return;
			}
		}
		static const FKey PadAxes[] = {
			EKeys::Gamepad_LeftX, EKeys::Gamepad_LeftY,
			EKeys::Gamepad_RightX, EKeys::Gamepad_RightY,
			EKeys::Gamepad_LeftTriggerAxis, EKeys::Gamepad_RightTriggerAxis
		};
		for (const FKey& K : PadAxes)
		{
			if (FMath::Abs(PC->PlayerInput->GetKeyValue(K)) > 0.18f)
			{
				bLastWasGamepad = true;
				return;
			}
		}
	}

	bool IsGamepad()
	{
		return bLastWasGamepad;
	}

	const TArray<FVEILInputRow>& Rows()
	{
		return BuildRows();
	}

	FString GlyphFor(const FVEILInputRow& Row)
	{
		if (bLastWasGamepad)
		{
			return Row.Pad.IsEmpty() ? FString(TEXT("KEYBOARD ONLY")) : Row.Pad;
		}
		return Row.Key;
	}
}
