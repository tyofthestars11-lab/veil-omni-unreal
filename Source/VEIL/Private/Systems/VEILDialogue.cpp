#include "Systems/VEILDialogue.h"

#include "AI/VEILFactionAI.h"
#include "Engine/World.h"
#include "Systems/PHILivingWorld.h"
#include "Systems/VEILLoopSubsystem.h"
#include "Story/VEILSpine.h"
#include "Story/VEILVoiceTake.h"
#include "Systems/VEILTime.h"
#include "VEIL.h"

namespace
{
	double Now(const UWorld* World)
	{
		return World ? World->GetTimeSeconds() : 0.0;
	}
}

const FVEILCharacterDef* UVEILDialogue::GetSpeaker() const
{
	return VEILCast::FindCharacter(SpeakerId);
}

bool UVEILDialogue::Begin(FName CharacterId)
{
	const FVEILCharacterDef* Who = VEILCast::FindCharacter(CharacterId);
	if (!Who || Who->Greetings.Num() == 0)
	{
		return false;
	}

	SpeakerId = CharacterId;
	// VISIT COUNT: the second conversation is not the first one. Counting the visit
	// is what lets the same person open differently without being random.
	int32& Talks = TalkCounts.FindOrAdd(CharacterId);
	++Talks;
	LineIndex = 0;
	LineStartTime = Now(GetWorld());
	bActive = true;

	// THE OPENING LINE IS THE ONE THAT REMEMBERS. If this person has a deed on
	// file, what the runner said last time is what they open with - the authored
	// greeting is for a stranger, and by now the runner is not one.
	Lines.Reset();
	if (const UPHILivingWorld* Living = GetWorld() ? GetWorld()->GetSubsystem<UPHILivingWorld>() : nullptr)
	{
		const FString Memory = Living->MemoryLineFor(CharacterId);
		if (!Memory.IsEmpty())
		{
			Lines.Add(FText::FromString(Memory));
		}
	}
	// THE ACT SPEAKS THROUGH WHOEVER IS STANDING IN FRONT OF YOU. The three
	// statues are the spine's mouthpieces: what they say is the act the world is
	// in, not a fixed greeting. Everyone else keeps their authored greeting, so
	// nobody in the district is ever mute.
	const TArray<FText>* ActScene = nullptr;
	if (UVEILLoopSubsystem* Loop = GetWorld() ? GetWorld()->GetSubsystem<UVEILLoopSubsystem>() : nullptr)
	{
		const TArray<FText>& Scene = VEILSpine::SceneFor(Loop->Stage, CharacterId);
		if (Scene.Num() > 0)
		{
			ActScene = &Scene;
			UE_LOG(LogVEIL, Log, TEXT("PHI STORY: %s speaks %s (%s)."),
				*Who->Name.ToString(), *VEILSpine::TitleFor(Loop->Stage).ToString(),
				*VEILSpine::EpigraphFor(Loop->Stage).ToString());
		}
	}
	if (ActScene)
	{
		for (const FText& Line : *ActScene) { Lines.Add(Line); }
	}
	else
	{
		// ROTATED, NOT LOOPED: the list starts one place further on every visit, so a
		// player who comes back hears a different opening before anything else moves.
		const int32 N = Who->Greetings.Num();
		const int32 Start = (N > 1) ? ((Talks - 1) % N) : 0;
		for (int32 i = 0; i < N; ++i)
		{
			Lines.Add(Who->Greetings[(Start + i) % N]);
		}
	}
	bChoosing = false;
	bAnswered = false;
	ChosenIndex = INDEX_NONE;
	Consequence.Reset();

	// THE DELIVERY FOR THIS LINE, from the speaker's profile and this line's own
	// words. Two visits to the same person never produce the same take twice.
	Voice = VEILVoice::TakeFor(CharacterId,
		Lines.Num() > 0 ? Lines[0].ToString() : FString(), Talks, 0);

	UE_LOG(LogVEIL, Log, TEXT("PHI DIALOGUE: %s (%s) opens [visit %d] - \"%s\""),
		*Who->Name.ToString(), *Who->Title.ToString(), Talks,
		Lines.Num() > 0 ? *Lines[0].ToString() : *FString());
	UE_LOG(LogVEIL, Log, TEXT("PHI VOICE: %s"), *VEILVoice::Describe(CharacterId, Voice));
	return true;
}

void UVEILDialogue::End()
{
	if (bActive)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI DIALOGUE: conversation closed after %d line(s)."), LineIndex + 1);
	}
	bActive = false;
	bChoosing = false;
	bAnswered = false;
	SpeakerId = NAME_None;
	LineIndex = 0;
	Lines.Reset();
}

bool UVEILDialogue::IsTyping() const
{
	if (!bActive) { return false; }
	const double Elapsed = Now(GetWorld()) - LineStartTime;
	return Elapsed * CharsPerSecond < GetFullText().Len();
}

FString UVEILDialogue::GetFullText() const
{
	// Once a reply has been given, the speaker's answer is the text: the reply
	// itself is what the runner said, and both halves belong on screen.
	if (bAnswered)
	{
		const TArray<FVEILReplyDef>& Replies = GetChoices();
		if (Replies.IsValidIndex(ChosenIndex)) { return Replies[ChosenIndex].Response.ToString(); }
	}

	if (!Lines.IsValidIndex(LineIndex)) { return FString(); }
	return Lines[LineIndex].ToString();
}

const TArray<FVEILReplyDef>& UVEILDialogue::GetChoices() const
{
	static const TArray<FVEILReplyDef> Empty;
	if (SpeakerId.IsNone()) { return Empty; }
	return VEILCast::RepliesFor(SpeakerId);
}

bool UVEILDialogue::Choose(int32 Index)
{
	if (!bActive || !bChoosing) { return false; }

	const TArray<FVEILReplyDef>& Replies = GetChoices();
	if (!Replies.IsValidIndex(Index)) { return false; }

	const FVEILReplyDef& Reply = Replies[Index];
	ChosenIndex = Index;
	bChoosing = false;
	bAnswered = true;
	LineStartTime = Now(GetWorld());
	// The answer carries its own take: an angry reply is answered hot, a quiet one
	// is answered quietly, because the mood is read from the words being spoken.
	Voice = VEILVoice::TakeFor(SpeakerId, Reply.Response.ToString(),
		TalkCounts.FindRef(SpeakerId), Index + 100);
	UE_LOG(LogVEIL, Log, TEXT("PHI VOICE: %s"), *VEILVoice::Describe(SpeakerId, Voice));

	UWorld* W = GetWorld();
	const FVEILCharacterDef* Who = GetSpeaker();
	const FVEILFactionDef* FactionDef = Who ? VEILCast::FindFaction(Who->FactionId) : nullptr;

	auto Rep = [W, FactionDef](float Amount) -> float
	{
		if (!W || !FactionDef) { return 0.f; }
		UVEILFactionManager* Manager = W->GetSubsystem<UVEILFactionManager>();
		if (!Manager) { return 0.f; }
		Manager->AdjustRep(FactionDef->Faction, Amount);
		return Manager->GetReputation(FactionDef->Faction);
	};

	switch (Reply.Effect)
	{
	case EVEILReplyEffect::FavourFaction:
	{
		const float Standing = Rep(Reply.Amount);
		Consequence = FString::Printf(TEXT("%s +%.0f  //  standing %.0f%s"),
			FactionDef ? *FactionDef->Name.ToString() : TEXT("FACTION"), Reply.Amount, Standing,
			Standing >= 20.f ? TEXT(" - THEY WILL NOT FIRE ON YOU") : TEXT(""));
		break;
	}
	case EVEILReplyEffect::DefyFaction:
	{
		const float Standing = Rep(-FMath::Abs(Reply.Amount));
		Consequence = FString::Printf(TEXT("%s -%.0f  //  standing %.0f%s"),
			FactionDef ? *FactionDef->Name.ToString() : TEXT("FACTION"), FMath::Abs(Reply.Amount), Standing,
			Standing <= -20.f ? TEXT(" - THEY ARE HUNTING YOU") : TEXT(""));
		break;
	}
	case EVEILReplyEffect::LightBeacon:
	{
		if (UVEILLoopSubsystem* Loop = W ? W->GetSubsystem<UVEILLoopSubsystem>() : nullptr)
		{
			const EVEILStage Before = Loop->Stage;
			if (Before == EVEILStage::FindSignal) { Loop->SignalFound(); }
			else { Loop->ActivateBeacon(); }
			Consequence = FString::Printf(TEXT("SIGNAL  //  %s -> %s"),
				Before == EVEILStage::FindSignal ? TEXT("UNFOUND") : TEXT("FOUND"),
				Loop->Stage == EVEILStage::FindSignal ? TEXT("UNFOUND")
				: Loop->Stage == EVEILStage::Won ? TEXT("BEACON LIT") : TEXT("SIGNAL RISING"));
		}
		else
		{
			Consequence = TEXT("SIGNAL  //  no beacon responded");
		}
		break;
	}
	default:
		Consequence = TEXT("NOTHING CHANGED  //  an answer, not a lever");
		break;
	}

	// The reply is a deed, filed against the person it was said to. Their next
	// greeting is built from this, so what the runner did changes what they hear.
	if (UPHILivingWorld* Living = W ? W->GetSubsystem<UPHILivingWorld>() : nullptr)
	{
		const bool bHelped = Reply.Effect == EVEILReplyEffect::FavourFaction
			|| Reply.Effect == EVEILReplyEffect::LightBeacon;
		Living->NoteDeed(SpeakerId, bHelped, Reply.Label.ToString());
	}

	UE_LOG(LogVEIL, Log, TEXT("PHI DIALOGUE: runner said \"%s\"; %s answered \"%s\". Effect: %s"),
		*Reply.Label.ToString(), Who ? *Who->Name.ToString() : TEXT("?"),
		*Reply.Response.ToString(), *Consequence);
	return true;
}

FString UVEILDialogue::GetVisibleText() const
{
	const FString Full = GetFullText();
	if (!bActive) { return FString(); }

	const double Elapsed = FMath::Max(0.0, Now(GetWorld()) - LineStartTime);
	const int32 Shown = FMath::Clamp((int32)(Elapsed * CharsPerSecond), 0, Full.Len());
	return Full.Left(Shown);
}

void UVEILDialogue::Advance()
{
	if (!bActive) { return; }

	// First press completes the line, second press moves on - the standard
	// contract, so a fast reader is never made to wait and a slow one is
	// never skipped.
	const FString Full = GetFullText();
	const double Elapsed = Now(GetWorld()) - LineStartTime;
	// Half a character of slack: setting LineStartTime so the line is exactly
	// finished leaves a float residue, and without the slack a press at that
	// boundary completed the line again instead of moving to the next one.
	if (Elapsed * CharsPerSecond + 0.5 < (double)Full.Len())
	{
		LineStartTime = Now(GetWorld()) - (double)Full.Len() / CharsPerSecond;
		return;
	}

	// A reply has been given and read: the scene is over.
	if (bAnswered)
	{
		End();
		return;
	}

	const FVEILCharacterDef* Who = GetSpeaker();
	if (Lines.IsValidIndex(LineIndex + 1))
	{
		++LineIndex;
		LineStartTime = Now(GetWorld());
		Voice = VEILVoice::TakeFor(SpeakerId, Lines[LineIndex].ToString(),
			TalkCounts.FindRef(SpeakerId), LineIndex);
		UE_LOG(LogVEIL, Log, TEXT("PHI VOICE: %s"), *VEILVoice::Describe(SpeakerId, Voice));
		UE_LOG(LogVEIL, Log, TEXT("PHI DIALOGUE: %s line %d - \"%s\""),
			Who ? *Who->Name.ToString() : TEXT("?"), LineIndex + 1, *Lines[LineIndex].ToString());
		return;
	}

	// The greeting is spent. If this person has something to offer, offer it -
	// a conversation that cannot be answered is a monologue.
	if (GetChoices().Num() > 0)
	{
		bChoosing = true;
		LineStartTime = Now(GetWorld());
		UE_LOG(LogVEIL, Log, TEXT("PHI DIALOGUE: %s offers %d repl%s."),
			Who ? *Who->Name.ToString() : TEXT("?"), GetChoices().Num(),
			GetChoices().Num() == 1 ? TEXT("y") : TEXT("ies"));
		return;
	}

	End();
}

FString UVEILDialogue::GetSpeakerName() const
{
	const FVEILCharacterDef* Who = GetSpeaker();
	return Who ? Who->Name.ToString() : FString();
}

FString UVEILDialogue::GetSpeakerTitle() const
{
	const FVEILCharacterDef* Who = GetSpeaker();
	return Who ? Who->Title.ToString() : FString();
}

FString UVEILDialogue::GetSpeakerCreed() const
{
	const FVEILCharacterDef* Who = GetSpeaker();
	if (!Who) { return FString(); }
	if (const FVEILFactionDef* F = VEILCast::FindFaction(Who->FactionId))
	{
		return FString::Printf(TEXT("%s  //  %s"), *F->Name.ToString(), *F->Creed.ToString());
	}
	return FString();
}

FLinearColor UVEILDialogue::GetSpeakerColor() const
{
	return VEILCast::ColorOf(SpeakerId);
}
