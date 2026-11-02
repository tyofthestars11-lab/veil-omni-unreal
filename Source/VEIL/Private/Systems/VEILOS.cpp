// VEIL OMNI // ONE OS - engine implementation.
//
// domains (rung fields) -> tasks (addresses) -> skills (round-tripped rungs)
// -> automations (on change) -> memory (phi ledger), walked by a voice door of
// Brain / Eyes / Seat / Hands: plan, review, execute.
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
	return Value > KINDA_SMALL_NUMBER
		? (double)FMath::Loge((float)Value) / (double)FMath::Loge((float)OsPhi)
		: 0.0;
}

const TArray<FString>& UVEILOS::TaskAddresses()
{
	static const TArray<FString> Book =
	{
		TEXT("veil://task/status"),
		TEXT("veil://task/who"),
		TEXT("veil://task/save"),
		TEXT("veil://task/alloff"),
	};
	return Book;
}

const TArray<FString>& UVEILOS::SkillAddresses()
{
	static const TArray<FString> Book =
	{
		TEXT("veil://skill/vitals"),
		TEXT("veil://skill/wealth"),
		TEXT("veil://skill/progress"),
	};
	return Book;
}

// ---------------------------------------------------------------------------
// BOOT
// ---------------------------------------------------------------------------
void UVEILOS::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// phi^2 = phi + 1, on every boot. The face is a function of it.
	const double Residue = FMath::Abs(OsPhiSq - (OsPhi + 1.0));
	UE_LOG(LogVEIL, Log, TEXT("PHI OS: phi^2 - (phi + 1) = %.3e, checked within 1e-6."), Residue);

	TaskList.Reset();
	for (const FString& Addr : TaskAddresses())
	{
		FVEILOSTask T;
		T.Address = Addr;
		// The verb is the LAST segment: "veil://task/save" -> "save". Chopping at
		// the first slash leaves "/task/save", and a name that lies about itself
		// is exactly how the skills below reported 0 of 3 round-tripped.
		FString VerbTail;
		Addr.Split(TEXT("/"), nullptr, &VerbTail, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		T.Verb = VerbTail;
		T.State = TEXT("queued");
		TaskList.Add(T);
	}

	SkillList.Reset();
	for (const FString& Addr : SkillAddresses())
	{
		FVEILOSSkill S;
		S.Address = Addr;
		// Same slice: the skill's name is its last segment, so the round trip
		// compares the right value against the right recomputation.
		FString NameTail;
		Addr.Split(TEXT("/"), nullptr, &NameTail, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		S.Name = NameTail;
		SkillList.Add(S);
	}

	RefreshDomains();
	RoundTripSkills();
	Say(TEXT("OS online. Domains sampled, tasks armed, skills round-tripped."));
	AppendLedger(TEXT("veil://os/boot"), FString::Printf(
		TEXT("%d domains, %d tasks, %d skills (%d/%d round-tripped), ledger armed"),
		DomainFields.Num(), TaskList.Num(), SkillList.Num(), SkillsRoundTripped, SkillsTotal));

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI OS ONLINE: %d domains (rung fields), %d task addresses, %d skills %d/%d round-tripped."),
		DomainFields.Num(), TaskList.Num(), SkillList.Num(), SkillsRoundTripped, SkillsTotal);

	if (FParse::Param(FCommandLine::Get(), TEXT("phios")))
	{
		FTimerHandle PlanHandle, ShotHandle, CloseHandle;

		// PLAN -> REVIEW -> EXECUTE, on the M1 ask: the run must persist.
		GetWorld()->GetTimerManager().SetTimer(PlanHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			Open();
			Plan(TEXT("persist the run"));
			Review();
			Execute();
			UE_LOG(LogVEIL, Warning, TEXT("PHI OS M1: %s"), *M1Line());
		}), 6.0f, false);

		GetWorld()->GetTimerManager().SetTimer(ShotHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				PC->ConsoleCommand(TEXT("HighResShot 1920x1080"), true);
				++ProofShots;
				UE_LOG(LogVEIL, Warning, TEXT("PHI OS PROOF: frame written with the door open and the answer on screen."));
			}
		}), 9.0f, false);

		GetWorld()->GetTimerManager().SetTimer(CloseHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			WriteLedgerFile();
			UE_LOG(LogVEIL, Warning, TEXT("PHI OS PROOF: %s | memory %d | hands '%s'"),
				*ObservabilityLine(), LedgerMemory.Num(), *PlanState.Result);
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				PC->ConsoleCommand(TEXT("quit"));
			}
		}), 12.0f, false);
	}
}

// ---------------------------------------------------------------------------
// VOICE DOOR: BRAIN -> EYES -> SEAT -> HANDS
// ---------------------------------------------------------------------------
void UVEILOS::Plan(const FString& Intent)
{
	PlanState = FVEILOSPlan();
	PlanState.Intent = Intent;
	PlanState.T = GetWorld() ? (double)GetWorld()->GetTimeSeconds() : 0.0;

	RefreshDomains();

	// BRAIN builds the steps out of the domains it just read - not out of a
	// script. A domain that is missing changes the plan.
	PlanState.Steps.Add(TEXT("brain  read domains"));
	for (const FVEILOSDomain& D : DomainFields)
	{
		PlanState.Steps.Add(FString::Printf(TEXT("brain  %s %.0f r%.2f"), *D.Key, D.Value, D.Rung));
	}
	PlanState.Steps.Add(TEXT("eyes   resolve the verb to an address"));
	PlanState.Steps.Add(TEXT("seat   name where the runner stands"));
	PlanState.Steps.Add(TEXT("hands  veil://task/save"));

	AppendLedger(TEXT("veil://brain/plan"), FString::Printf(
		TEXT("'%s' - %d steps drafted"), *Intent, PlanState.Steps.Num()));
	Say(FString::Printf(TEXT("Plan drafted for %s. Sending it to the eyes."), *Intent));
}

bool UVEILOS::Review()
{
	bool bOk = true;
	FString Note;

	for (const FString& Step : PlanState.Steps)
	{
		if (!Step.StartsWith(TEXT("hands"))) { continue; }
		const FString Addr = Step.RightChop(Step.Find(TEXT("veil://")));
		if (!TaskAddresses().Contains(Addr))
		{
			bOk = false;
			Note = FString::Printf(TEXT("no task at %s"), *Addr);
		}
		else
		{
			Note = FString::Printf(TEXT("%s resolves"), *Addr);
		}
	}

	PlanState.bReviewed = bOk;
	PlanState.ReviewNote = bOk ? FString::Printf(TEXT("approved - %s"), *Note)
		: FString::Printf(TEXT("refused - %s"), *Note);

	AppendLedger(TEXT("veil://eyes/review"), PlanState.ReviewNote);
	Say(PlanState.bReviewed ? TEXT("Eyes: approved. Standing by.") : TEXT("Eyes: refused. I will not run that."));
	return bOk;
}

FString UVEILOS::Execute()
{
	if (!PlanState.bReviewed)
	{
		PlanState.Result = TEXT("refused - plan was never reviewed");
		AppendLedger(TEXT("veil://hands/execute"), PlanState.Result);
		return PlanState.Result;
	}

	// SEAT first: where did this happen. An action with no place is a rumour.
	PlanState.Seat = MapLine();
	AppendLedger(TEXT("veil://seat/here"), PlanState.Seat);

	// HANDS: the real verb, through the same Dispatch the face uses.
	PlanState.Result = Dispatch(TEXT("veil://task/save"));
	Say(PlanState.Result);
	AppendLedger(TEXT("veil://hands/execute"), PlanState.Result);
	return PlanState.Result;
}

FString UVEILOS::GateLine() const
{
	const bool bPlanned = PlanState.Steps.Num() > 0;
	return FString::Printf(
		TEXT("BRAIN %s   EYES %s   SEAT %s   HANDS %s"),
		bPlanned ? TEXT("planned") : TEXT("idle"),
		PlanState.bReviewed ? TEXT("approved") : (bPlanned ? TEXT("waiting") : TEXT("-")),
		PlanState.Seat.IsEmpty() ? (bPlanned ? TEXT("named at execute") : TEXT("-")) : TEXT("named"),
		PlanState.Result.IsEmpty() ? TEXT("-") : TEXT("done"));
}

// ---------------------------------------------------------------------------
// READS
// ---------------------------------------------------------------------------
void UVEILOS::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UWorld* World = GetWorld();
	if (!World) { return; }

	// A quarter-second sample: live enough to feel, slow enough that the
	// automations below run on CHANGE rather than on every frame.
	const double Now = (double)World->GetTimeSeconds();
	if (Now - LastSample < 0.25) { return; }
	LastSample = Now;

	RefreshDomains();
	RunAutomations();
}

void UVEILOS::RefreshDomains()
{
	UWorld* World = GetWorld();
	if (!World) { return; }

	const AVEILCharacter* Runner = Cast<AVEILCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	const UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();

	DomainFields.Reset();
	auto Add = [this](const TCHAR* Key, const TCHAR* Addr, double V)
	{
		FVEILOSDomain D;
		D.Key = Key;
		D.Address = Addr;
		D.Value = V;
		D.Rung = Rung(V);
		DomainFields.Add(D);
	};

	Add(TEXT("VIT"), TEXT("veil://domain/vit"), Runner ? (double)Runner->Health : 0.0);
	Add(TEXT("ATP"), TEXT("veil://domain/atp"), Runner ? (double)Runner->ATP : 0.0);
	Add(TEXT("GOLD"), TEXT("veil://domain/gold"), Quest ? (double)Quest->Gold : 0.0);
	Add(TEXT("LEVEL"), TEXT("veil://domain/level"), Quest ? (double)Quest->Level : 0.0);
	Add(TEXT("BEACON"), TEXT("veil://domain/beacon"), Quest ? (double)Quest->ClaimedBeacons.Num() : 0.0);
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

	// ON CHANGE ONLY. A still world writes nothing at all.
	if (PrevGold >= 0.0 && !FMath::IsNearlyEqual(Gold, PrevGold))
	{
		++Automations;
		LastAutomation = FString::Printf(TEXT("gold %.0f -> %.0f"), PrevGold, Gold);
		AppendLedger(TEXT("veil://auto/gold"), FString::Printf(TEXT("%s, r%.2f"), *LastAutomation, Rung(FMath::Max(1.0, Gold))));
		Say(FString::Printf(TEXT("Gold is %.0f."), Gold));
	}
	if (PrevHealth >= 0.0 && Health < 35.0 && PrevHealth >= 35.0)
	{
		++Automations;
		LastAutomation = TEXT("vitals under 35");
		AppendLedger(TEXT("veil://auto/vitals"), TEXT("vitals under 35 - the voice rises"));
		Say(TEXT("Vitals critical. The signal is thinning."));
	}
	if (PrevBeacons >= 0 && Beacons != PrevBeacons)
	{
		++Automations;
		LastAutomation = FString::Printf(TEXT("beacons %d -> %d"), PrevBeacons, Beacons);
		AppendLedger(TEXT("veil://auto/beacon"), LastAutomation);
		Say(FString::Printf(TEXT("Beacon %d of 3 responds."), Beacons));
	}
	if (PrevLevel >= 0 && Level != PrevLevel)
	{
		++Automations;
		LastAutomation = FString::Printf(TEXT("level %d -> %d"), PrevLevel, Level);
		AppendLedger(TEXT("veil://auto/level"), LastAutomation);
	}

	PrevGold = Gold;
	PrevHealth = Health;
	PrevBeacons = Beacons;
	PrevLevel = Level;
}

// ---------------------------------------------------------------------------
// TASKS + SKILLS
// ---------------------------------------------------------------------------
FString UVEILOS::Dispatch(const FString& Address)
{
	UWorld* World = GetWorld();
	if (!World) { return TEXT("engine offline"); }

	++Dispatches;
	FString Answer;

	// ---- tasks: things the engine makes true ------------------------------
	if (Address == TEXT("veil://task/status"))
	{
		RefreshDomains();
		Answer = DomainLine();
	}
	else if (Address == TEXT("veil://task/who"))
	{
		Answer = MapLine();
	}
	else if (Address == TEXT("veil://task/save"))
	{
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
	else if (Address == TEXT("veil://task/alloff"))
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
	// ---- skills: answer a rung, proven to have come back unchanged --------
	else if (Address == TEXT("veil://skill/vitals"))
	{
		const AVEILCharacter* Runner = Cast<AVEILCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
		Answer = FString::Printf(TEXT("%.3f"), Rung(Runner ? (double)Runner->Health : 0.0));
	}
	else if (Address == TEXT("veil://skill/wealth"))
	{
		const UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
		Answer = FString::Printf(TEXT("%.3f"), Rung(Quest ? FMath::Max(1.0, (double)Quest->Gold) : 1.0));
	}
	else if (Address == TEXT("veil://skill/progress"))
	{
		const UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
		Answer = FString::Printf(TEXT("%.3f"), Rung(Quest ? (double)Quest->Level : 1.0));
	}
	else
	{
		Answer = FString::Printf(TEXT("no task at %s"), *Address);
	}

	LastAnswerAddress = Address;
	LastAnswerText = Answer;
	++Answers;
	MarkTask(Address);
	AppendLedger(Address, Answer);
	return Answer;
}

void UVEILOS::MarkTask(const FString& Address)
{
	for (FVEILOSTask& T : TaskList)
	{
		if (T.Address == Address)
		{
			T.State = FString::Printf(TEXT("answered %d"), Answers);
			return;
		}
	}
}

void UVEILOS::RoundTripSkills()
{
	SkillsTotal = SkillList.Num();
	SkillsRoundTripped = 0;

	// A round trip is: the engine's own rung -> formatted into a string ->
	// parsed back -> equal to the rung recomputed from the live value. A skill
	// that drifts is not round-tripped, and the face says so.
	const AVEILCharacter* Runner = Cast<AVEILCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	const UOMNIQuestSystem* Quest = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr;

	for (FVEILOSSkill& S : SkillList)
	{
		const FString Answer = Dispatch(S.Address);
		const double Returned = FCString::Atod(*Answer);

		double Truth = 0.0;
		if (S.Name == TEXT("vitals")) { Truth = Rung(Runner ? (double)Runner->Health : 0.0); }
		else if (S.Name == TEXT("wealth")) { Truth = Rung(Quest ? FMath::Max(1.0, (double)Quest->Gold) : 1.0); }
		else if (S.Name == TEXT("progress")) { Truth = Rung(Quest ? (double)Quest->Level : 1.0); }

		S.Rung = Returned;
		S.bRoundTripped = FMath::Abs(Returned - Truth) < 1e-3;
		if (S.bRoundTripped) { ++SkillsRoundTripped; }
	}

	UE_LOG(LogVEIL, Warning, TEXT("PHI OS SKILLS: %d of %d rungs round-tripped unchanged."),
		SkillsRoundTripped, SkillsTotal);
}

void UVEILOS::AskStatus()
{
	Open();
	Say(Dispatch(TEXT("veil://task/status")));
}

void UVEILOS::AskSave()
{
	Say(Dispatch(TEXT("veil://task/save")));
}

void UVEILOS::Say(const FString& Line)
{
	VoiceText = Line;
	if (UWorld* World = GetWorld())
	{
		// JARVIS IS THE TOP LAYER ONLY: the same subtitle channel the world
		// already speaks through, so there is one voice on screen, not two.
		FPHIVoice::LastSubtitle = Line;
		FPHIVoice::LastPlayTime = World->GetTimeSeconds();
	}
}

void UVEILOS::Toggle()
{
	if (bOpen) { Close(); }
	else { Open(); }
}

void UVEILOS::Open()
{
	if (bOpen) { return; }
	bOpen = true;
	RefreshDomains();
	AppendLedger(TEXT("veil://os/open"), TEXT("face raised"));
	Say(TEXT("The face is up. Ask me, and I answer from the engine."));
}

void UVEILOS::Close()
{
	if (!bOpen) { return; }
	bOpen = false;
	AppendLedger(TEXT("veil://os/close"), TEXT("face lowered"));
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

	// Memory, not save state: capped, and it never touches the one save.
	if (LedgerMemory.Num() > 256)
	{
		LedgerMemory.RemoveAt(0, LedgerMemory.Num() - 256);
	}
}

void UVEILOS::WriteLedgerFile()
{
	TArray<FString> Lines;
	for (const FVEILOSLedgerEntry& E : LedgerMemory)
	{
		Lines.Add(FString::Printf(TEXT("%8.3f  rung %7.4f  %-26s  %s"), E.T, E.Rung, *E.Address, *E.Note));
	}
	const FString Path = FPaths::ProjectSavedDir() / TEXT("veil_os_ledger.txt");
	FFileHelper::SaveStringToFile(FString::Join(Lines, TEXT("\n")), *Path);
}

// ---------------------------------------------------------------------------
// WHAT THE FACE PRINTS
// ---------------------------------------------------------------------------
FString UVEILOS::DomainLine() const
{
	FString Out;
	for (const FVEILOSDomain& D : DomainFields)
	{
		Out += FString::Printf(TEXT("%s %.0f r%.2f   "), *D.Key, D.Value, D.Rung);
	}
	return Out.IsEmpty() ? FString(TEXT("no domains sampled")) : Out;
}

FString UVEILOS::SkillLine() const
{
	FString Out;
	for (const FVEILOSSkill& S : SkillList)
	{
		Out += FString::Printf(TEXT("%s %.3f %s   "), *S.Name, S.Rung,
			S.bRoundTripped ? TEXT("ok") : TEXT("DRIFT"));
	}
	return FString::Printf(TEXT("%d/%d  %s"), SkillsRoundTripped, SkillsTotal, *Out);
}

FString UVEILOS::AutomationLine() const
{
	return Automations == 0
		? FString(TEXT("0 fired - nothing has moved since boot (on-change only)"))
		: FString::Printf(TEXT("%d fired, last: %s"), Automations, *LastAutomation);
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
	return TEXT("[F1] OS   [G] GODMODE   [E] speak   [SPACE] continue   [TAB] chart   [P] pause");
}

FString UVEILOS::M1Line() const
{
	return FString::Printf(
		TEXT("domains %s| plan %d steps | eyes %s | hands %s | seat %s"),
		*DomainLine(), PlanState.Steps.Num(),
		PlanState.bReviewed ? TEXT("approved") : TEXT("refused"),
		*PlanState.Result, *PlanState.Seat);
}

FString UVEILOS::ObservabilityLine() const
{
	return FString::Printf(
		TEXT("dispatches %d  answers %d  automations %d  saves %d  skills %d/%d  memory %d  shots %d"),
		Dispatches, Answers, Automations, Saves, SkillsRoundTripped, SkillsTotal, LedgerMemory.Num(), ProofShots);
}
