#include "VeilPlayerController.h"
#include "VeilRunner.h"
#include "VeilSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"

const FVector AVeilPlayerController::KeeperAt(900.0, 0.0, 100.0);
const FVector AVeilPlayerController::SpireAt(0.0, 3500.0, 500.0);
const FVector AVeilPlayerController::DeepAt(0.0, -5000.0, 100.0);

FVector AVeilPlayerController::BeaconAt(int32 Index)
{
	switch (Index)
	{
	case 1: return FVector(-2000.0, -900.0, 100.0);
	case 2: return FVector(600.0, -2100.0, 100.0);
	default: return FVector(2800.0, 500.0, 100.0);
	}
}

AVeilPlayerController::AVeilPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = false;
}

void AVeilPlayerController::BeginPlay()
{
	Super::BeginPlay();
	LoadSignal();
}

AVeilRunner* AVeilPlayerController::Runner() const
{
	return Cast<AVeilRunner>(GetPawn());
}

void AVeilPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!InputComponent)
	{
		return;
	}
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AVeilPlayerController::OnInteract);
	InputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AVeilPlayerController::OnSprintStart);
	InputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &AVeilPlayerController::OnSprintStop);
	InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AVeilPlayerController::OnGlideStart);
	InputComponent->BindKey(EKeys::SpaceBar, IE_Released, this, &AVeilPlayerController::OnGlideStop);
	InputComponent->BindKey(EKeys::C, IE_Pressed, this, &AVeilPlayerController::OnCombatMode);
}

void AVeilPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	STM = FMath::Min(STMMax, STM + 12.f * DeltaSeconds);
	TickQuest();
	CheckBeacons();
}

void AVeilPlayerController::TickQuest()
{
	if (AVeilRunner* R = Runner())
	{
		bNearKeeper = FVector::Dist2D(R->GetActorLocation(), KeeperAt) < 520.0;
	}
}

void AVeilPlayerController::Speak(const TCHAR* Who, const TCHAR* Line)
{
	DialogueSpeaker = Who;
	DialogueLine = Line;
	bDialogueOpen = true;
	UE_LOG(LogTemp, Display, TEXT("VEIL SAY: %s - %s"), Who, Line);
}

void AVeilPlayerController::OnInteract()
{
	static const TCHAR* KeeperLines[3] = {
		TEXT("Stand still. I count the living first."),
		TEXT("You carry the Signal and no wings. That is the whole of the Path of Light."),
		TEXT("Three beacons sleep. Wake them and the Spire will know your name.")
	};

	if (bDialogueOpen)
	{
		++DialogueStep;
		if (DialogueStep < 3)
		{
			DialogueLine = KeeperLines[DialogueStep];
			return;
		}
		bDialogueOpen = false;
		DialogueStep = 0;
		bKeeperCounted = true;
		bQuestAccepted = true;
		UE_LOG(LogTemp, Display, TEXT("VEIL QUEST: the Keeper counted the Runner, beacons %d/%d"),
			BeaconsLit, BeaconsTotal);
		SaveSignal();
		return;
	}

	if (bNearKeeper)
	{
		Speech:
		Speak(TEXT("Keeper Sera Vahl"), KeeperLines[0]);
	}
}

void AVeilPlayerController::CheckBeacons()
{
	if (!bQuestAccepted)
	{
		return;
	}
	AVeilRunner* R = Runner();
	if (!R)
	{
		return;
	}
	const FVector At = R->GetActorLocation();
	for (int32 i = 0; i < 3; ++i)
	{
		if (bBeaconLit[i])
		{
			continue;
		}
		if (FVector::Dist2D(At, BeaconAt(i)) < 700.0)
		{
			bBeaconLit[i] = true;
			++BeaconsLit;
			UE_LOG(LogTemp, Display, TEXT("VEIL BEACON: beacon %d awake, %d/%d"), i + 1, BeaconsLit, BeaconsTotal);
			if (BeaconsLit >= BeaconsTotal)
			{
				UE_LOG(LogTemp, Display, TEXT("VEIL SIGNAL: three beacons awake, the Spire answers"));
			}
			SaveSignal();
		}
	}
}

void AVeilPlayerController::OnSprintStart()
{
	if (STM > 5.f)
	{
		if (AVeilRunner* R = Runner())
		{
			R->SetSprinting(true);
		}
	}
}

void AVeilPlayerController::OnSprintStop()
{
	STM = FMath::Max(0.f, STM - 3.f);
	if (AVeilRunner* R = Runner())
	{
		R->SetSprinting(false);
	}
}

void AVeilPlayerController::OnGlideStart()
{
	if (AVeilRunner* R = Runner())
	{
		R->bGlide = true;
	}
}

void AVeilPlayerController::OnGlideStop()
{
	if (AVeilRunner* R = Runner())
	{
		R->bGlide = false;
	}
}

void AVeilPlayerController::OnCombatMode()
{
	bCombat = !bCombat;
	UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: %s"), bCombat ? TEXT("on") : TEXT("off"));
}

void AVeilPlayerController::SaveSignal()
{
	UVeilSaveGame* S = Cast<UVeilSaveGame>(UGameplayStatics::CreateSaveGameObject(UVeilSaveGame::StaticClass()));
	if (!S)
	{
		return;
	}
	S->bKeeperCounted = bKeeperCounted;
	S->bQuestAccepted = bQuestAccepted;
	S->BeaconsLit = BeaconsLit;
	S->RunnerAt = Runner() ? Runner()->GetActorLocation() : FVector(0.0, 0.0, 100.0);
	if (AVeilRunner* R = Runner())
	{
		S->ATP = R->Mito.ATP;
		S->Organelles = R->Mito.Organelles;
		S->AddressRung = R->Mito.AddressRung;
	}
	UGameplayStatics::SaveGameToSlot(S, TEXT("VEILOMNI_Omni0"), 0);
	UE_LOG(LogTemp, Display, TEXT("VEIL SAVE: VEILOMNI_Omni0 written, beacons %d/%d"), BeaconsLit, BeaconsTotal);
}

void AVeilPlayerController::LoadSignal()
{
	if (!UGameplayStatics::DoesSaveGameExist(TEXT("VEILOMNI_Omni0"), 0))
	{
		UE_LOG(LogTemp, Display, TEXT("VEIL SAVE: no VEILOMNI_Omni0 yet, a new run begins"));
		return;
	}
	UVeilSaveGame* S = Cast<UVeilSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("VEILOMNI_Omni0"), 0));
	if (!S)
	{
		return;
	}
	Save = S;
	bKeeperCounted = S->bKeeperCounted;
	bQuestAccepted = S->bQuestAccepted;
	BeaconsLit = S->BeaconsLit;
	for (int32 i = 0; i < 3; ++i)
	{
		bBeaconLit[i] = (i < BeaconsLit);
	}
	if (AVeilRunner* R = Runner())
	{
		R->Mito.ATP = S->ATP;
		R->Mito.Organelles = S->Organelles;
		R->Mito.AddressRung = S->AddressRung;
	}
	UE_LOG(LogTemp, Display, TEXT("VEIL SAVE: VEILOMNI_Omni0 read, beacons %d/%d"), BeaconsLit, BeaconsTotal);
}
