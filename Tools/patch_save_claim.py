import io

ROOT = r"D:\VEIL-OMNI" + "\\"
log = []

def rd(p):
    return io.open(ROOT + p, encoding="utf-8", newline=None).read()

def wr(p, s):
    io.open(ROOT + p, "w", encoding="utf-8", newline="\r\n").write(s)

def sub(path, old, new, tag):
    t = rd(path)
    if new in t:
        log.append(tag + " (already)")
        return
    if old not in t:
        log.append(tag + " >>> MISSED")
        return
    wr(path, t.replace(old, new, 1))
    log.append(tag)

CH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
HH = "Source/VEIL/Public/Character/VEILCharacter.h"
QC = "Source/VEIL/Private/Systems/OMNIQuestSystem.cpp"
WC = "Source/VEIL/Private/World/VEILWilds.cpp"
WH = "Source/VEIL/Public/World/VEILWilds.h"

# ---------------------------------------------------------------- character
sub(HH,
'''	// ---- Input handlers ----
	void Move(const FInputActionValue& Value);''',
'''	/** Claim the nearest named seam within arm's length. Public because the interact
	 *  key and the veil.claim console command must run the SAME code, or the test
	 *  proves something the player cannot do. */
	void ClaimNearestSeam();

	// ---- Input handlers ----
	void Move(const FInputActionValue& Value);''',
"character.h claim api")

sub(CH,
'''#include "EngineUtils.h"
#include "Systems/OMNIQuestSystem.h"''',
'''#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "Systems/OMNIQuestSystem.h"''',
"character includes console")

sub(CH,
'''	// A SEAM IS CLAIMED BY PRESSING THE KEY, not by walking near it. The nearest
	// unclaimed mine within arm's length is what this press is about; the quest book
	// decides whether it was already taken, so the gold can never be paid twice.
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AVEILWildsPopulation> It(World); It; ++It)
		{
			AVEILMineNode* Mine = It->NearestUnclaimedMine(GetActorLocation(), 700.f);
			if (!Mine) { break; }
			UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
			if (Quest && Quest->ClaimMine(FName(*Mine->MineName()), Mine->MineOwner(), Mine->ClaimValue(), 24))
			{
				Mine->Claim(this);
			}
			break;
		}
	}

	// A BEACON IS FORGED''',
'''	// A SEAM IS CLAIMED BY PRESSING THE KEY, not by walking near it.
	ClaimNearestSeam();

	// A BEACON IS FORGED''',
"character interact calls claim")

sub(CH,
'''void AVEILCharacter::ToggleGodMode()''',
'''void AVEILCharacter::ClaimNearestSeam()
{
	UWorld* World = GetWorld();
	if (!World || !IsAlive()) { return; }

	for (TActorIterator<AVEILWildsPopulation> It(World); It; ++It)
	{
		AVEILMineNode* Mine = It->NearestUnclaimedMine(GetActorLocation(), 700.f);
		if (!Mine) { break; }
		UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
		if (Quest && Quest->ClaimMine(FName(*Mine->MineName()), Mine->MineOwner(), Mine->ClaimValue(), 24))
		{
			Mine->Claim(this);
		}
		else if (Quest)
		{
			// Standing on ground already taken is worth saying out loud, or the key
			// looks broken when the seam is simply someone's already.
			UE_LOG(LogVEIL, Warning, TEXT("PHI CLAIM: %s is already the runner's - nothing paid."),
				*Mine->MineName());
		}
		return;
	}

	UE_LOG(LogVEIL, Warning, TEXT("PHI CLAIM: no named seam within 7 m of %s - nothing to take."),
		*GetActorLocation().ToCompactString());
}

// A KEYBOARD TEST THAT IS THE SAME CODE AS THE KEY. veil.claim runs the exact
// function the interact key runs, so what a run proves here a player can do.
static FAutoConsoleCommandWithWorld GClaimSeam(
	TEXT("veil.claim"),
	TEXT("Claim the nearest named seam within 7 m (identical to pressing E)."),
	FConsoleCommandWithWorldDelegate::CreateStatic([](UWorld* World)
	{
		if (!World) { return; }
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (AVEILCharacter* Runner = Cast<AVEILCharacter>(PC->GetPawn())) { Runner->ClaimNearestSeam(); }
		}
	}));

void AVEILCharacter::ToggleGodMode()''',
"character claim impl + command")

# ---------------------------------------------------------------- save on claim
sub(QC,
'''	AwardXP(60, TEXT("a named seam claimed"));
	PersistToBook();''',
'''	AwardXP(60, TEXT("a named seam claimed"));
	PersistToBook();

	// A CLAIM IS WRITTEN NOW, not at exit. The book only reached the file when the
	// world shut down cleanly, so a closed window lost every seam taken; and the
	// path is printed, because "no save" and "saved somewhere else" look identical
	// from the outside.
	if (UVEILProgression* Book = GetWorld() ? GetWorld()->GetSubsystem<UVEILProgression>() : nullptr)
	{
		Book->PullFromQuest();
		const bool bWrote = Book->SaveGame();
		UE_LOG(LogVEIL, Warning, TEXT("PHI SAVE CLAIM: %s written=%d seams=%d gold=%d at %s."),
			*MineName.ToString(), bWrote ? 1 : 0, ClaimedMines.Num(), Gold,
			*FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()));
	}''',
"quest.cpp save on claim")

sub(QC,
'''#include "Systems/OMNIQuestSystem.h"''',
'''#include "Misc/Paths.h"
#include "Systems/OMNIQuestSystem.h"''',
"quest.cpp includes")

# ---------------------------------------------------------------- autosave
sub(WH,
'''	/** One a second is enough to point at the next seam. */
	float TrackClock = 0.f;''',
'''	/** One a second is enough to point at the next seam. */
	float TrackClock = 0.f;
	/** The run is written every fifteen seconds, so no session dies unwritten. */
	float SaveClock = 0.f;''',
"wilds.h save clock")

sub(WC,
'''		TrackClock += DeltaSeconds;
		if (TrackClock >= 1.f)''',
'''		TrackClock += DeltaSeconds;
		SaveClock += DeltaSeconds;
		if (SaveClock >= 15.f)
		{
			SaveClock = 0.f;
			if (UVEILProgression* Book = World->GetSubsystem<UVEILProgression>())
			{
				if (const UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>())
				{
					Book->Level = Quest->Level;
					Book->XP = Quest->XP;
					Book->Gold = Quest->Gold;
					Book->Fragments = Quest->Fragments;
					Book->ClaimedMines = Quest->ClaimedMines;
				}
				const bool bWrote = Book->SaveGame();
				UE_LOG(LogVEIL, Log, TEXT("PHI AUTOSAVE: wrote=%d level=%d gold=%d seams=%d."),
					bWrote ? 1 : 0, Book->Level, Book->Gold, Book->ClaimedMines.Num());
			}
		}
		if (TrackClock >= 1.f)''',
"wilds.cpp autosave")

sub(WC,
'''#include "Systems/OMNIQuestSystem.h"''',
'''#include "Systems/OMNIQuestSystem.h"
#include "Systems/VEILProgression.h"''',
"wilds.cpp includes progression")

print("\n".join(log))
