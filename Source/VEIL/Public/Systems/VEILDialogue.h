#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Story/VEILCast.h"
#include "Story/VEILVoiceTake.h"
#include "VEILDialogue.generated.h"

/**
 * The conversation runner. One active speaker, one line at a time, drawn as a
 * typewriter so a line arrives at a speaking pace instead of appearing whole.
 *
 * It is a world subsystem, not a widget, because the HUD in this project is a
 * canvas pass (UMG does not composite in this -game session - see
 * VEILHudCanvas.h). State lives here; pixels live in the HUD.
 */
UCLASS()
class VEIL_API UVEILDialogue : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Open a conversation with a cast member. False if the id is unknown. */
	bool Begin(FName CharacterId);
	void End();

	/** Typewriter first, then next line; past the last line it offers replies. */
	void Advance();

	// ---- replies with consequences ----
	//
	// A name with a line and no answer is a plaque. Once the greeting is spent the
	// speaker puts two or three replies on the table; picking one applies its
	// effect to the world (faction standing, a beacon) BEFORE the box closes, and
	// then they answer in their own voice.
	bool IsChoosing() const { return bChoosing; }
	bool HasAnswered() const { return bAnswered; }
	const TArray<FVEILReplyDef>& GetChoices() const;
	/** Applies reply Index: effect first, then the spoken answer. */
	bool Choose(int32 Index);
	/** One line for the log and the HUD: what the world did about it. */
	const FString& GetConsequence() const { return Consequence; }

	bool IsActive() const { return bActive; }
	bool IsTyping() const;
	int32 GetLineIndex() const { return LineIndex; }

	FName GetSpeakerId() const { return SpeakerId; }

	/** The delivery for the line on screen: pitch / rate / pause / mood. */
	const FVEILVoiceTake& GetVoice() const { return Voice; }

	/** How many times the runner has spoken to this person this session. */
	int32 TalkCount(FName CharacterId) const { return TalkCounts.FindRef(CharacterId); }
	const FVEILCharacterDef* GetSpeaker() const;
	FString GetSpeakerName() const;
	FString GetSpeakerTitle() const;
	FString GetSpeakerCreed() const;
	FString GetFullText() const;
	FString GetVisibleText() const;
	FLinearColor GetSpeakerColor() const;

private:
	/** The lines this conversation actually runs on: memory first, then authored. */
	TArray<FText> Lines;

	bool bActive = false;
	bool bChoosing = false;
	bool bAnswered = false;
	int32 ChosenIndex = INDEX_NONE;
	FString Consequence;
	FName SpeakerId;
	/** Per-speaker visit count. Second visit is a different opening line, because a
	 *  person who has met you does not greet you like a stranger. */
	TMap<FName, int32> TalkCounts;
	FVEILVoiceTake Voice;
	int32 LineIndex = 0;
	double LineStartTime = 0.0;
	/** Reading pace, not a reveal effect: ~42 characters a second. */
	float CharsPerSecond = 42.f;
};
