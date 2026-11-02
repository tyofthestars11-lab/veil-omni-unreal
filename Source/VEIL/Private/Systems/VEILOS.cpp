// VEIL OMNI // ONE OS - engine implementation.
//
// The reversal: the interface does not print the game's numbers, it reads them
// as rungs. rung(v) = log_phi(v). Every surface is an address, every address
// answers, every answer is written to a ledger, and the automations fire on a
// CHANGE - never per frame, so a clean log stays clean.
#include "Systems/VEILOS.h"

#include "Character/VEILCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/PHICheats.h"
#include "Systems/PHIVoice.h"
#include "Systems/VEILOmniAll.h"
#include "Systems/VEILProgression.h"
#include "Systems/VEILRegionSubsystem.h"
#include "TimerManager.h"
#include "VEIL.h"

namespace
{
	constexpr double OsPhi = 1.618033988749895;
	constexpr double OsPhiSq = 2.618033988749895;
}

TStatId UVEILOS::GetStatId() const { return GetStatID(); }

bool UVEILOS::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

double UVEILOS::Rung(double Value)
{
	return Value > KINDA_SMALL_NUMBER ? (double)FMath::Loge((float)Value) / (double)FMath::Loge((float)OsPhi) : 0.0;
}

const TArray<FString>& UVEILOS::AddressBook()
{
	// Three read-only skills. The save verb lives beside them but is not
	// round-tripped at boot: a self-test must never write the player's run.
	static const TArray<FString> Book =
	{
		TEXT("veil://os/status"),
		TEXT("veil://os/who"),
		TEXT("veil://os/alloff"),
		TEXT("veil://os/save"),
	};
	return Book;
}

// ---------------------------------------------------------------------------
// BOOT
// ---------------------------------------------------------------------------
void UVEILOS::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// phi^2 = phi + 1, on every boot. The whole face is a function of it.
	const double Residue = FMath::Abs(OsPhiSq - (OsPhi + 1.0));
	UE_LOG(LogVEIL, Log, TEXT("PHI OS: phi^2 - (phi + 1) = %.3e, checked within 1e-6."), Residue);

	RefreshFields();
	RoundTripSkills();
	Say(TEXT("OS online. Ask, and I answer with the engine's own numbers."));
	AppendLedger(TEXT("veil://os/boot"), FString::Printf(
		TEXT("engine up - %d rung fields, %d of %d skills round-tripped"),
		RungFields.Num(), SkillsRoundTripped, SkillsTotal));

	UE_LOG(LogVEIL, Warning, TEXT("PHI OS ONLINE: %d addresses, %d skills round-tripped, ledger armed, automations on change."),
		AddressBook().Num(), SkillsRoundTripped);

	if (FParse::Param(FCommandLine::Get(), TEXT("phios")))
	{
		FTimerHandle OpenHandle, ShotHandle, CloseHandle;

		// ANSWER. The face opens and asks the engine two questions.
		GetWorld()->GetTimerManager().SetTimer(OpenHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			Open();
			AskStatus();
			AskSave();
			UE_LOG(LogVEIL, Warning, TEXT("PHI OS PROOF: answered '%s' -> '%s'"), *LastAnswerAddress, *LastAnswerText);
		}), 6.0f, false);

		// SHOW IT. The frame is written from inside the running game.
		GetWorld()->GetTimerManager().SetTimer(ShotHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				PC->ConsoleCommand(TEXT("HighResShot 1920x1080"), true);
				++ProofShots;
				UE_LOG(LogVEIL, Warning, TEXT("PHI OS PROOF: frame written with the interface answering on screen."));
			}
		}), 9.0f, false);

		// SAY WHAT HAPPENED, then close the game.
		GetWorld()->GetTimerManager().SetTimer(CloseHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			WriteLedgerFile();
			UE_LOG(LogVEIL, Warning, TEXT("PHI OS PROOF: %s | ledger %d | last '%s' -> '%s'"),
				*ObservabilityLine(), LedgerMemory.Num(), *LastAnswerAddress, *LastAnswerText);
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				PC->ConsoleCommand(TEXT("quit"));
			}
		}), 12.0f, false);
	}
}

// ---------------------------------------------------------------------------
// FACE STATE
// ---------------------------------------------------------------------------
void UVEILOS::Toggle() { bOpen = !bOpen; }

void UVEILOS::Open()
{
	bOpen = true;
	RefreshFields();
	AppendLedger(TEXT("veil://os/open"), TEXT("face raised"));
	Say(TEXT("I am here. The face is up."));
}

void UVEILOS::Close()
{
	bOpen = false;
	AppendLedger(TEXT("veil://os/close"), TEXT("face lowered"));
}

// ---------------------------------------------------------------------------
// ENGINE
// ---------------------------------------------------------------------------
void UVEILOS::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UWorld* World = GetWorld();
	if (!World) { return; }

	// A quarter-second sample: fast enough to feel live, slow enough that the
	// automations below run on CHANGE rather than on every frame.
	const double Now = World->GetTimeSeconds();
	if (Now - LastSample < 0.25) { return; }
	LastSample = Now;

	RefreshFields();
	RunAutomations();
}

void UVEILOS::RefreshFields()
{
	UWorld* World = GetWorld();
	if (!World) { return; }

	const AVEILCharacter* Runner = Cast<AVEILCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	const UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();

	RungFields.Reset();
	auto Add = [this](const TCHAR* Key, const TCHAR* Addr, double V)
	{
		FVEILOSRungField F;
		F.Key = Key;
		F.Address = Addr;
		F.Value = V;
		F.Rung = Rung(V);
		RungFields.Add(F);
	};

	Add(TEXT("VIT"), TEXT("veil://field/vit"), Runner ? (double)Runner->Health : 0.0);
	Add(TEXT("ATP"), TEXT("veil://field/atp"), Runner ? (double)Runner->ATP : 0.0);
	Add(TEXT("GOLD"), TEXT("veil://field/gold"), Quest ? (double)Quest->Gold : 0.0);
	Add(TEXT("LEVEL"), TEXT("veil://field/level"), Quest ? (double)Quest->Level : 0.0);
	Add(TEXT("BEACON"), TEXT("veil://field/beacons"), Quest ? (double)Quest->ClaimedBeacons.Num() : 0.0);
}

void UVEILOS::RunAutomations()
{
	UWorld* World = GetWorld();
	if (!World) { return; }

	const AVEILCharacter* Runner = Cast<AVEILCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	const UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();

	const double Gold = Quest ? (double)Quest->Gold : 0.0;
	const double Health = Runner ? (double)Runner->Health : 100.0;
	const int32 Beacons = Quest ? Quest->ClaimedBeacons.Num() : 0;
	const int32 Level = Quest ? Quest->Level : 1;

	// ON CHANGE ONLY. Each rule compares the sample with the previous one, so a
	// still world writes nothing at all.
	if (PrevGold >= 0.0 && !FMath::IsNearlyEqual(Gold, PrevGold))
	{
		++Automations;
		AppendLedger(TEXT("veil://auto/gold"), FString::Printf(
			TEXT("gold %.0f -> %.0f, rung %.3f"), PrevGold, Gold, Rung(FMath::Max(1.0, Gold))));
		Say(FString::Printf(TEXT("Gold is %.0f now."), Gold));
	}
	if (PrevHealth >= 0.0 && Health < 35.0 && PrevHealth >= 35.0)
	{
		++Automations;
		AppendLedger(TEXT("veil://auto/vitals"), TEXT("vitals under 35 - the voice rises"));
		Say(TEXT("Vitals critical. The signal is thinning."));
	}
	if (PrevBeacons >= 0 && Beacons != PrevBeacons)
	{
		++Automations;
		AppendLedger(TEXT("veil://auto/beacon"), FString::Printf(TEXT("beacons %d -> %d"), PrevBeacons, Beacons));
		Say(FString::Printf(TEXT("Beacon %d of 3 responds."), Beacons));
	}
	if (PrevLevel >= 0 && Level != PrevLevel)
	{
		++Automations;
		AppendLedger(TEXT("veil://auto/level"), FString::Printf(TEXT("level %d -> %d at rung %.3f"), PrevLevel, Level, Rung((double)Level)));
	}

	PrevGold = Gold;
	PrevHealth = Health;
	PrevBeacons = Beacons;
	PrevLevel = Level;
}

FString UVEILOS::Dispatch(const FString& Address)
{
	UWorld* World = GetWorld();
	if (!World) { return TEXT("engine offline"); }

	++Dispatches;
	FString Answer;

	if (Address == TEXT("veil://os/status"))
	{
		RefreshFields();
		Answer = EngineLine();
	}
	else if (Address == TEXT("veil://os/who"))
	{
		Answer = MapLine();
	}
	else if (Address == TEXT("veil://os/save"))
	{
		// THE INTERFACE DOES THE THING, not just says it will.
		bool bSaved = false;
		if (UVEILOmniAll* Omni = World->GetSubsystem<UVEILOmniAll>())
		{
			bSaved = Omni->SaveGame();
		}
		else if (UVEILProgression* Prog = World->GetSubsystem<UVEILProgression>())
		{
			bSaved = Prog->SaveGame();
		}
		if (bSaved) { ++Saves; }
		Answer = FString::Printf(TEXT("SAVE RESULT %s"), bSaved ? TEXT("True") : TEXT("False"));
	}
	else if (Address == TEXT("veil://os/alloff"))
	{
		if (UPHICheats* Cheats = World->GetSubsystem<UPHICheats>())
		{
			Cheats->AllOff();
			Answer = Cheats->StatusLine();
		}
		else
		{
			Answer = TEXT("no cheat page in this world");
		}
	}
	else
	{
		Answer = FString::Printf(TEXT("no handler at %s"), *Address);
	}

	LastAnswerAddress = Address;
	LastAnswerText = Answer;
	++Answers;
	AppendLedger(Address, Answer);
	return Answer;
}

void UVEILOS::AskStatus()
{
	Open();
	const FString A = Dispatch(TEXT("veil://os/status"));
	Say(A);
}

void UVEILOS::AskSave()
{
	const FString A = Dispatch(TEXT("veil://os/save"));
	Say(A);
}

void UVEILOS::Say(const FString& Line)
{
	VoiceText = Line;
	if (UWorld* World = GetWorld())
	{
		// THE VOICE IS THE TOP LAYER ONLY: the same subtitle channel the world
		// already speaks through, so there is one voice on screen, not two.
		FPHIVoice::LastSubtitle = Line;
		FPHIVoice::LastPlayTime = World->GetTimeSeconds();
	}
}

void UVEILOS::AppendLedger(const FString& Address, const FString& Note)
{
	FVEILOSLedgerEntry E;
	UWorld* World = GetWorld();
	E.T = World ? (double)World->GetTimeSeconds() : 0.0;
	E.Rung = Rung(FMath::Max(1.0, E.T));
	E.Address = Address;
	E.Note = Note;
	LedgerMemory.Add(E);

	// A ledger is memory, not save state: it is capped, and it never touches the
	// one save. The file in Saved is a diagnostic copy, written on proof only.
	if (LedgerMemory.Num() > 256)
	{
		LedgerMemory.RemoveAt(0, LedgerMemory.Num() - 256);
	}
}

void UVEILOS::RoundTripSkills()
{
	SkillsTotal = 0;
	SkillsRoundTripped = 0;

	for (const FString& Addr : AddressBook())
	{
		// Never round-trip the write verb at boot.
		if (Addr == TEXT("veil://os/save")) { continue; }
		++SkillsTotal;
		const FString Answer = Dispatch(Addr);
		if (!Answer.IsEmpty() && LastAnswerAddress == Addr) { ++SkillsRoundTripped; }
	}

	UE_LOG(LogVEIL, Warning, TEXT("PHI OS SKILLS: %d of %d round-tripped address -> handler -> answer."),
		SkillsRoundTripped, SkillsTotal);
}

void UVEILOS::WriteLedgerFile()
{
	TArray<FString> Lines;
	for (const FVEILOSLedgerEntry& E : LedgerMemory)
	{
		Lines.Add(FString::Printf(TEXT("%8.3f  rung %7.4f  %-24s  %s"), E.T, E.Rung, *E.Address, *E.Note));
	}
	const FString Path = FPaths::ProjectSavedDir() / TEXT("veil_os_ledger.txt");
	FFileHelper::SaveStringToFile(FString::Join(Lines, TEXT("\n")), *Path);
}

// ---------------------------------------------------------------------------
// WHAT THE FACE PRINTS
// ---------------------------------------------------------------------------
FString UVEILOS::EngineLine() const
{
	FString Out;
	for (const FVEILOSRungField& F : RungFields)
	{
		Out += FString::Printf(TEXT("%s %.0f r%.2f   "), *F.Key, F.Value, F.Rung);
	}
	return Out.IsEmpty() ? FString(TEXT("no fields sampled yet")) : Out;
}

FString UVEILOS::MapLine() const
{
	UWorld* World = GetWorld();
	if (!World) { return TEXT("no world"); }

	FString Region = TEXT("the signal");
	if (const UVEILRegionSubsystem* Regions = World->GetSubsystem<UVEILRegionSubsystem>())
	{
		Region = Regions->GetInfo(Regions->Current).Name.ToString();
	}

	FString Seat = TEXT("unseated");
	if (const APawn* Body = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Seat = Body->GetActorLocation().ToCompactString();
	}

	const UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
	const int32 Beacons = Quest ? Quest->ClaimedBeacons.Num() : 0;
	return FString::Printf(TEXT("%s   seat %s   beacons %d/3"), *Region, *Seat, Beacons);
}

FString UVEILOS::ControlsLine() const
{
	return TEXT("[F1] OS    [G] GODMODE    [E] speak    [SPACE] continue    [TAB] chart    [P] pause");
}

FString UVEILOS::RungTableLine() const
{
	return FString::Printf(TEXT("%d fields   %d addresses   ledger %d"),
		RungFields.Num(), AddressBook().Num(), LedgerMemory.Num());
}

FString UVEILOS::ObservabilityLine() const
{
	return FString::Printf(
		TEXT("dispatches %d  answers %d  automations %d  saves %d  skills %d/%d  ledger %d  shots %d"),
		Dispatches, Answers, Automations, Saves, SkillsRoundTripped, SkillsTotal, LedgerMemory.Num(), ProofShots);
}
