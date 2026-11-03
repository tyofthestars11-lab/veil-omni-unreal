#include "Systems/PHIONE.h"
#include "VEIL.h"

#include "Character/VEILCharacter.h"
#include "Systems/VEILFastTravel.h"
#include "Systems/VEILLoopSubsystem.h"
#include "Systems/VEILRegionSubsystem.h"
#include "World/VEILGoldenSpawner.h"

#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "VoiceModule.h"

namespace
{
	// Named OnePhi, not Phi: the engine's Chaos headers declare locals called Phi,
	// and this project compiles that shadowing warning as an error (C4459).
	constexpr float OnePhi = 1.618033988749895f;

	AVEILCharacter* OneRunner(UWorld* World)
	{
		if (!World) { return nullptr; }
		APlayerController* PC = World->GetFirstPlayerController();
		return PC ? Cast<AVEILCharacter>(PC->GetPawn()) : nullptr;
	}

	FString StageName(EVEILStage S)
	{
		switch (S)
		{
		case EVEILStage::FindSignal:  return TEXT("find the signal");
		case EVEILStage::Fragments:   return TEXT("ten fragments");
		case EVEILStage::Artifact:    return TEXT("forge the artifact");
		case EVEILStage::FinalWave:   return TEXT("hold the final wave");
		case EVEILStage::Beacon:      return TEXT("light the beacon");
		default:                      return TEXT("the signal is restored");
		}
	}

	/** Where the Keeper stands: the statue is the beacon, so ONE flies to the
	 *  statue's foot and hovers, rather than to a coordinate with nothing at it. */
	FVector KeeperStand(UWorld* World)
	{
		// Trace from ABOVE THE STREET, not from the sky: the first attempt started at
		// Z=40000 and hit the roof over the plaza, so the target was 11 km up and the
		// carry climbed at a statue it could never reach. A hit that is not near the
		// ground is treated as a roof and ignored.
		FVector Ground(900.f, 0.f, 60.f);
		if (World)
		{
			FHitResult Hit;
			const FVector From(900.f, 0.f, 3000.f);
			const FVector To(900.f, 0.f, -6000.f);
			FCollisionQueryParams P(SCENE_QUERY_STAT(ONEKeeper), false);
			if (World->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility, P)
				&& FMath::Abs(Hit.ImpactPoint.Z) < 2500.f)
			{
				Ground = Hit.ImpactPoint;
			}
		}
		return Ground;
	}
}

// ---------------------------------------------------------------------------
// LIFECYCLE
// ---------------------------------------------------------------------------
void UPHIONE::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// DOMAINS -> TASKS. The table is data: the HUD, the ledger and the console all
	// read this one list, so a skill cannot exist in the voice but not in the world.
	auto Add = [this](FName Id, FName Domain, float Value, const FString& Hand, TArray<FString> Says)
	{
		FONESkill S;
		S.Id = Id;
		S.Domain = Domain;
		S.Value = Value;
		S.Hand = Hand;
		S.Says = MoveTemp(Says);
		SkillTable.Add(S);
	};

	Add(TEXT("fly.keeper"), TEXT("game"), OnePhi * OnePhi, TEXT("fly"),
		{ TEXT("fly me to the keeper"), TEXT("take me to the keeper"), TEXT("go to the keeper") });
	Add(TEXT("fly.land"), TEXT("game"), OnePhi, TEXT("fly"),
		{ TEXT("land"), TEXT("set me down"), TEXT("put me down") });
	Add(TEXT("say.where"), TEXT("voice"), 1.f, TEXT("eyes"),
		{ TEXT("where am i"), TEXT("where are we"), TEXT("position") });
	Add(TEXT("world.spiral"), TEXT("game"), OnePhi * OnePhi * OnePhi, TEXT("spiral"),
		{ TEXT("raise the spiral"), TEXT("spiral"), TEXT("golden spiral") });
	Add(TEXT("loop.beacon"), TEXT("game"), OnePhi * OnePhi * OnePhi * OnePhi * OnePhi, TEXT("beacon"),
		{ TEXT("light the beacon"), TEXT("beacon") });
	Add(TEXT("play.god"), TEXT("game"), OnePhi * OnePhi * OnePhi, TEXT("hands"),
		{ TEXT("godmode on"), TEXT("godmode off"), TEXT("god") });
	Add(TEXT("fly.on"), TEXT("game"), OnePhi, TEXT("fly"),
		{ TEXT("fly"), TEXT("take off") });

	State = EONEState::Ready;
	UE_LOG(LogVEIL, Warning, TEXT("PHI ONE: awake, ready - click the ONE bar to speak or type. %d skills across game / ledger / pc / voice. Memory: Saved/ONE/ledger.txt"),
		SkillTable.Num());
}

void UPHIONE::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// A key held and never let go (alt-tab, unplugged controller) must not hold the
	// microphone open forever. Twenty seconds is a ceiling on one breath, not a
	// conversation limit.
	if (bTalking && GetWorld() && GetWorld()->GetTimeSeconds() - TalkStart > 20.f)
	{
		EndTalk();
	}

	if (SpeakTime > 0.f)
	{
		SpeakTime -= DeltaTime;
		if (SpeakTime <= 0.f) { State = EONEState::Listening; }
	}

	// AUTOMATION: fires ON CHANGE, not on a clock. The moment the world's stage
	// moves, ONE says so - no polling loop, no job file, no waiting.
	if (UWorld* W = GetWorld())
	{
		if (const UVEILLoopSubsystem* Loop = W->GetSubsystem<UVEILLoopSubsystem>())
		{
			if (!bStageSeen)
			{
				SeenStage = Loop->Stage;
				bStageSeen = true;
			}
			else if (Loop->Stage != SeenStage)
			{
				SeenStage = Loop->Stage;
				Speak(FString::Printf(TEXT("The signal moved. Now: %s."), *StageName(SeenStage)), TEXT("automation"));
				WriteLedger(TEXT("[automation] stage changed"), Reply, HandUsed, RungOf((float)SeenStage + 1.f));
			}
		}
	}

	// HANDS: the flight ONE is driving. Velocity is interpolated, not stamped, so
	// the carry is a flight with acceleration - the same body the player flies.
	if (!bCarrying) { return; }
	UWorld* W = GetWorld();
	AVEILCharacter* C = OneRunner(W);
	if (!C)
	{
		bCarrying = false;
		return;
	}

	if (!C->IsFlyingNow()) { C->ToggleFlight(); }

	const FVector Loc = C->GetActorLocation();
	const FVector To = CarryTarget - Loc;
	const float Horiz = FMath::Sqrt(To.X * To.X + To.Y * To.Y);

	if (UCharacterMovementComponent* M = C->GetCharacterMovement())
	{
		FVector Desired = FVector(To.X, To.Y, 0.f).GetSafeNormal() * C->FlyCruiseSpeed;
		Desired.Z = FMath::Clamp(To.Z * 1.8f, -C->FlyClimbSpeed, C->FlyClimbSpeed);
		if (Horiz < 320.f)
		{
			Desired = FVector(0.f, 0.f, FMath::Clamp(To.Z, -120.f, 120.f));
		}
		M->Velocity = FMath::VInterpTo(M->Velocity, Desired, DeltaTime, 1.9f);

		static float LogClock = 0.f;
		LogClock += DeltaTime;
		if (LogClock > 0.75f)
		{
			LogClock = 0.f;
			UE_LOG(LogVEIL, Log, TEXT("PHI ONE FLY: %.0f cm to %s - loc=%s speed=%.0f"),
				Horiz, *CarryName, *Loc.ToCompactString(), M->Velocity.Size());
		}
	}

	// Arrival is the horizontal leg: altitude is a hover trim, not a gate. The
	// earlier test demanded both inside 200 cm of a target that was 10 km high, so
	// a runner standing 2.5 m from the statue never heard "here".
	if (Horiz < 320.f)
	{
		bCarrying = false;
		if (UCharacterMovementComponent* M = C->GetCharacterMovement())
		{
			M->Velocity = FVector::ZeroVector;
		}
		Speak(FString::Printf(TEXT("Here. %s."), *CarryName), TEXT("fly"));
		WriteLedger(TEXT("[skill] fly.keeper arrived"), Reply, HandUsed, RungOf(OnePhi * OnePhi));
		UE_LOG(LogVEIL, Warning, TEXT("PHI ONE: arrived at %s, loc=%s. Hovering."),
			*CarryName, *C->GetActorLocation().ToCompactString());
	}
}

// ---------------------------------------------------------------------------
// VOICE
// ---------------------------------------------------------------------------
FString UPHIONE::StateLine() const
{
	const TCHAR* StateWord = State == EONEState::Ready ? TEXT("ready")
		: State == EONEState::Listening ? TEXT("listening")
		: State == EONEState::Thinking ? TEXT("working") : TEXT("speaking");
	if (Reply.IsEmpty()) { return FString::Printf(TEXT("ONE  %s"), StateWord); }
	return FString::Printf(TEXT("ONE  %s  -  %s"), StateWord, *Reply);
}

void UPHIONE::BeginTalk()
{
	UWorld* W = GetWorld();
	if (!W || bTalking) { return; }
	// Never talk over itself: if ONE is mid-sentence the key is ignored rather
	// than cutting the reply off half-spoken.
	if (State == EONEState::Speaking) { return; }

	// The microphone is opened HERE and only here. If the machine has no capture
	// device the state still changes and the hold is still measured - the honest
	// answer is "held, but I could not open a microphone", not silence.
	if (!Voice.IsValid() && FVoiceModule::IsAvailable())
	{
		Voice = FVoiceModule::Get().CreateVoiceCapture(FString(), 16000, 1);
	}
	bMicOpened = false;
	if (Voice.IsValid())
	{
		Voice->Start();
		bMicOpened = Voice->IsCapturing();
	}

	bTalking = true;
	TalkStart = W->GetTimeSeconds();
	State = EONEState::Listening;
	Reply = TEXT("Listening. Say it, or type it.");
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI ONE: TALK DOWN. microphone %s. Nothing is captured before this key goes down."),
		bMicOpened ? TEXT("open") : TEXT("could not be opened"));
}

void UPHIONE::EndTalk()
{
	if (!bTalking) { return; }
	UWorld* W = GetWorld();
	LastHeld = W ? FMath::Max(0.f, W->GetTimeSeconds() - TalkStart) : 0.f;
	bTalking = false;

	if (Voice.IsValid() && bMicOpened) { Voice->Stop(); }
	const bool bHadMic = bMicOpened;
	bMicOpened = false;

	State = EONEState::Ready;
	// HONEST: this project has no speech-to-text wired in, so the hold is measured
	// and reported instead of pretending a sentence was understood and acted on.
	Reply = bHadMic
		? FString::Printf(TEXT("Heard %.1f s. No words yet - nothing here turns sound into text."), LastHeld)
		: TEXT("No microphone on this machine.");
	WriteLedger(TEXT("[push-to-talk] key held"), Reply, TEXT("voice"), RungOf(FMath::Max(LastHeld, 0.01f)));
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI ONE: TALK UP. held %.2f s, microphone opened=%d. No transcriber is wired, so no skill fired."),
		LastHeld, bHadMic ? 1 : 0);
}

void UPHIONE::Speak(const FString& InReply, const FString& InHand)
{
	Reply = InReply;
	HandUsed = InHand;
	State = EONEState::Speaking;
	SpeakTime = FMath::Clamp(0.9f + InReply.Len() * 0.035f, 1.1f, 4.5f);
	UE_LOG(LogVEIL, Warning, TEXT("PHI ONE: \"%s\"  [hand: %s]"), *Reply, *HandUsed);
}

float UPHIONE::RungOf(float Value)
{
	if (Value <= 0.f) { return 0.f; }
	return FMath::Loge(Value) / FMath::Loge(OnePhi);
}

void UPHIONE::WriteLedger(const FString& Utterance, const FString& InReply, const FString& InHand, float Rung)
{
	const FString Line = FString::Printf(TEXT("%s  rung %.4f  %s  ->  %s  [%s]"),
		*FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")), Rung,
		*Utterance, *InReply, *InHand);
	LedgerLines.Add(Line);

	const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ONE"));
	IFileManager::Get().MakeDirectory(*Dir, true);
	FFileHelper::SaveStringToFile(Line + LINE_TERMINATOR,
		*FPaths::Combine(Dir, TEXT("ledger.txt")), FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(), FILEWRITE_Append);
}

bool UPHIONE::Say(const FString& Utterance)
{
	State = EONEState::Thinking;
	UE_LOG(LogVEIL, Warning, TEXT("PHI ONE hears: \"%s\""), *Utterance);

	FString OutReply;
	FString OutHand;
	const bool bFired = RunSkill(Utterance, OutReply, OutHand);

	FONESkill Best;
	Best.Value = 1.f;
	for (const FONESkill& S : SkillTable)
	{
		if (S.Hand == OutHand) { Best = S; break; }
	}
	WriteLedger(Utterance, OutReply, OutHand, RungOf(Best.Value));
	Speak(OutReply, OutHand);
	return bFired;
}

bool UPHIONE::RunSkill(const FString& In, FString& OutReply, FString& OutHand)
{
	const FString U = In.ToLower();

	// Ordered by what a word means, not by a regex engine: a sentence that names
	// the Keeper is a flight, even if it also says "where".
	if (U.Contains(TEXT("keeper")) && (U.Contains(TEXT("fly")) || U.Contains(TEXT("take")) || U.Contains(TEXT("go")) || U.Contains(TEXT("to"))))
	{
		FlyToKeeper();
		OutReply = TEXT("On my way. The Keeper is expecting a number.");
		OutHand = TEXT("fly");
		return true;
	}
	if (U.Contains(TEXT("land")) || U.Contains(TEXT("set me down")) || U.Contains(TEXT("put me down")))
	{
		Land();
		OutReply = TEXT("Down. The ground was where you left it.");
		OutHand = TEXT("fly");
		return true;
	}
	if (U.Contains(TEXT("where")))
	{
		OutReply = ReportWhere();
		OutHand = TEXT("eyes");
		return true;
	}
	if (U.Contains(TEXT("spiral")))
	{
		SpawnSpiral();
		OutReply = TEXT("Spiral raised. Golden angle held.");
		OutHand = TEXT("spiral");
		return true;
	}
	if (U.Contains(TEXT("beacon")) && !U.Contains(TEXT("keeper")))
	{
		ActivateBeacon();
		OutReply = TEXT("Beacon lit. That is one the dark does not get.");
		OutHand = TEXT("beacon");
		return true;
	}
	if (U.Contains(TEXT("god")))
	{
		const bool bOff = U.Contains(TEXT("off")) || U.Contains(TEXT("no"));
		SetGodMode(!bOff);
		OutReply = bOff ? TEXT("Godmode off. Clean run.") : TEXT("Godmode on.");
		OutHand = TEXT("hands");
		return true;
	}
	if (U.Contains(TEXT("fly")) || U.Contains(TEXT("take off")))
	{
		if (AVEILCharacter* C = OneRunner(GetWorld()))
		{
			if (!C->IsFlyingNow()) { C->ToggleFlight(); }
			OutReply = TEXT("Airborne. Mind the rings.");
			OutHand = TEXT("fly");
			return true;
		}
	}

	// Honest, not magic: an intent no skill owns says so instead of pretending.
	OutReply = TEXT("No hand of mine owns that. Yet.");
	OutHand = TEXT("none");
	return false;
}

// ---------------------------------------------------------------------------
// HANDS
// ---------------------------------------------------------------------------
void UPHIONE::FlyToKeeper()
{
	UWorld* W = GetWorld();
	AVEILCharacter* C = OneRunner(W);
	if (!C) { return; }

	if (!C->IsFlyingNow()) { C->ToggleFlight(); }
	const FVector Stand = KeeperStand(W);
	CarryTarget = Stand + FVector(0.f, 0.f, 340.f);
	CarryName = TEXT("The Keeper");
	bCarrying = true;
	UE_LOG(LogVEIL, Warning, TEXT("PHI ONE HANDS: flying to the Keeper at %s from %s."),
		*CarryTarget.ToCompactString(), *C->GetActorLocation().ToCompactString());
}

void UPHIONE::Land()
{
	UWorld* W = GetWorld();
	AVEILCharacter* C = OneRunner(W);
	if (!C) { return; }
	bCarrying = false;

	if (C->IsFlyingNow()) { C->ToggleFlight(); }

	// Set down ON the ground, not a metre above it: trace from the body down.
	FHitResult Hit;
	const FVector Loc = C->GetActorLocation();
	FCollisionQueryParams P(SCENE_QUERY_STAT(ONELand), false, C);
	if (W->LineTraceSingleByChannel(Hit, Loc, Loc - FVector(0.f, 0.f, 5000.f), ECC_Visibility, P))
	{
		C->SetActorLocation(FVector(Loc.X, Loc.Y, Hit.ImpactPoint.Z + 96.f));
	}
	UE_LOG(LogVEIL, Warning, TEXT("PHI ONE HANDS: landed at %s."), *C->GetActorLocation().ToCompactString());
}

FString UPHIONE::ReportWhere()
{
	UWorld* W = GetWorld();
	AVEILCharacter* C = OneRunner(W);
	if (!C) { return TEXT("No body to stand in."); }

	FString RegionName = TEXT("the signal");
	if (const UVEILRegionSubsystem* R = W->GetSubsystem<UVEILRegionSubsystem>())
	{
		RegionName = R->GetInfo(R->Current).Name.ToString();
	}
	const FVector L = C->GetActorLocation();
	return FString::Printf(TEXT("%s. %.0f by %.0f."), *RegionName, L.X, L.Y);
}

void UPHIONE::SpawnSpiral()
{
	UWorld* W = GetWorld();
	AVEILCharacter* C = OneRunner(W);
	if (!W || !C) { return; }

	const FVector Ahead = C->GetActorLocation() + C->GetActorForwardVector() * 700.f;
	FVector At = Ahead;
	FHitResult Hit;
	FCollisionQueryParams P(SCENE_QUERY_STAT(ONESpiral), false, C);
	if (W->LineTraceSingleByChannel(Hit, Ahead + FVector(0.f, 0.f, 2000.f), Ahead - FVector(0.f, 0.f, 6000.f), ECC_Visibility, P))
	{
		At = Hit.ImpactPoint + FVector(0.f, 0.f, 5.f);
	}

	FActorSpawnParameters SP;
	SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AVEILGoldenSpawner* Spiral = W->SpawnActor<AVEILGoldenSpawner>(AVEILGoldenSpawner::StaticClass(), At, FRotator::ZeroRotator, SP))
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI ONE HANDS: spiral of %d cubes at %s."), Spiral->Count, *At.ToCompactString());
	}
}

void UPHIONE::ActivateBeacon()
{
	UWorld* W = GetWorld();
	if (!W) { return; }
	if (UVEILLoopSubsystem* Loop = W->GetSubsystem<UVEILLoopSubsystem>())
	{
		const bool bOk = Loop->ActivateBeacon();
		UE_LOG(LogVEIL, Warning, TEXT("PHI ONE HANDS: beacon %s (stage now %d)."),
			bOk ? TEXT("lit") : TEXT("refused - the loop is not there yet"), (int32)Loop->Stage);
	}
}

void UPHIONE::SetGodMode(bool bOn)
{
	if (AVEILCharacter* C = OneRunner(GetWorld()))
	{
		if (C->IsGodMode() != bOn) { C->ToggleGodMode(); }
	}
}

// ---------------------------------------------------------------------------
// PROOF HOOKS - the same path a spoken sentence takes.
// one.say <words>   |   one.state
// ---------------------------------------------------------------------------
static FAutoConsoleCommandWithWorldAndArgs GOneSay(
	TEXT("one.say"),
	TEXT("Speak to ONE: one.say fly me to the keeper"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UPHIONE* One = World ? World->GetSubsystem<UPHIONE>() : nullptr;
		if (!One) { UE_LOG(LogVEIL, Warning, TEXT("PHI ONE: no subsystem in this world.")); return; }
		const FString Spoken = FString::Join(Args, TEXT(" "));
		const bool bFired = One->Say(Spoken);
		UE_LOG(LogVEIL, Warning, TEXT("PHI ONE RESULT: fired=%d reply=\"%s\" hand=%s"),
			bFired ? 1 : 0, *One->GetReply(), *One->GetHand());
	}));

static FAutoConsoleCommandWithWorldAndArgs GOneTalk(
	TEXT("one.talk"),
	TEXT("Push to talk: one.talk start | one.talk stop"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UPHIONE* One = World ? World->GetSubsystem<UPHIONE>() : nullptr;
		if (!One) { return; }
		const FString What = Args.Num() > 0 ? Args[0].ToLower() : TEXT("start");
		if (What == TEXT("stop")) { One->EndTalk(); } else { One->BeginTalk(); }
		UE_LOG(LogVEIL, Warning, TEXT("PHI ONE STATE: %s  talking=%d"),
			*One->StateLine(), One->IsTalking() ? 1 : 0);
	}));

static FAutoConsoleCommandWithWorldAndArgs GOneState(
	TEXT("one.state"),
	TEXT("One line from ONE: state, last reply, hand, skills"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UPHIONE* One = World ? World->GetSubsystem<UPHIONE>() : nullptr;
		if (!One) { return; }
		UE_LOG(LogVEIL, Warning, TEXT("PHI ONE STATE: %s  skills=%d"), *One->StateLine(), One->Skills().Num());
	}));
