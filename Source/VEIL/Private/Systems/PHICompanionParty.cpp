// PHI companion party implementation.
#include "Systems/PHICompanionParty.h"

#include "AI/VEILCompanion.h"
#include "AIController.h"
#include "Character/VEILCharacter.h"
#include "Character/VEILCountessCharacter.h"
#include "Engine/Engine.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/PHISideQuests.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "VEIL.h"

static TAutoConsoleVariable<int32> CVarPhiParty(TEXT("phi.party"), 0,
	TEXT("Muster a test party: 1 sage ... 5 full party. Default 0 off."));

void UPHICompanionParty::TickWorld(float DeltaSeconds)
{
	(void)DeltaSeconds;
	Prune();
	if (!bMustered)
	{
		bMustered = true;
		const int32 Want = CVarPhiParty.GetValueOnGameThread() > 0
			? FMath::Clamp(CVarPhiParty.GetValueOnGameThread(), 0, 5) : 5;
		Muster(Want);
	}
	NoteRunner();
	FollowTick(DeltaSeconds);
	BeatsTick(DeltaSeconds);
}

int32 UPHICompanionParty::Count() const
{
	int32 n = 0;
	for (const TWeakObjectPtr<AVEILCompanion>& M : Members)
	{
		if (M.IsValid() && M->IsAlive()) { ++n; }
	}
	return n;
}

void UPHICompanionParty::Prune()
{
	for (int32 i = Members.Num() - 1; i >= 0; --i)
	{
		if (!Members[i].IsValid() || !Members[i]->IsAlive())
		{
			Members.RemoveAt(i);
		}
	}
}

AVEILCompanion* UPHICompanionParty::Recruit(uint8 Role, FName CallSign)
{
	Prune();
	if (Members.Num() >= 5)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI PARTY: five walk already - %s waits for an empty slot."), *CallSign.ToString());
		return nullptr;
	}
	UWorld* W = GetWorld();
	if (!W) return nullptr;
	APawn* Leader = UGameplayStatics::GetPlayerPawn(W, 0);
	FVector At = Leader ? Leader->GetActorLocation() - Leader->GetActorForwardVector() * 400.f : FVector::ZeroVector;
	At.Z += 100.f;
	FActorSpawnParameters PSP;
	PSP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AVEILCompanion* C = W->SpawnActor<AVEILCompanion>(AVEILCompanion::StaticClass(), At, FRotator::ZeroRotator, PSP);
	if (!C) return nullptr;
	C->ConfigureForRole((EVEILCompanionRole)(Role % 5), CallSign);
	Members.Add(C);
	UE_LOG(LogVEIL, Log, TEXT("PHI PARTY: %s mustered, party %d/5."), *CallSign.ToString(), Members.Num());
	return C;
}

void UPHICompanionParty::Muster(int32 Wanted)
{
	static const uint8 Roles[5] = { 0, 1, 2, 3, 4 };
	static const TCHAR* Names[5] = { TEXT("GR MUSE"), TEXT("DRONE"), TEXT("VESPER"), TEXT("QUILL"), TEXT("BASTION") };
	for (int32 i = 0; i < Wanted && i < 5; ++i)
	{
		Recruit(Roles[i], FName(Names[i]));
	}
}

void UPHICompanionParty::NotePlayable(APawn* P)
{
	if (!P) return;
	if (Cast<AVEILCountessCharacter>(P)) { CountessPawn = P; }
	else if (Cast<AVEILCharacter>(P)) { if (!RunnerPawn.IsValid()) { RunnerPawn = P; } }
	UE_LOG(LogVEIL, Log, TEXT("PHI PARTY: %s stands in the lead circle."), *P->GetName());
}

void UPHICompanionParty::NoteRunner()
{
	UWorld* W = GetWorld();
	if (!W) return;
	if (!RunnerPawn.IsValid())
	{
		if (APawn* P = UGameplayStatics::GetPlayerPawn(W, 0)) { NotePlayable(P); }
	}
}

bool UPHICompanionParty::PlayableAlive(APawn* P) const
{
	if (!IsValid(P)) return false;
	if (AVEILCharacter* R = Cast<AVEILCharacter>(P)) return R->IsAlive();
	if (AVEILCountessCharacter* C = Cast<AVEILCountessCharacter>(P)) return C->GetHealth() > 0.f;
	return true;
}

FString UPHICompanionParty::LeadName() const
{
	UWorld* W = GetWorld();
	if (!W) return TEXT("none");
	if (const APlayerController* PC = W->GetFirstPlayerController())
	{
		if (const APawn* P = PC->GetPawn())
		{
			if (Cast<AVEILCountessCharacter>(P)) return TEXT("COUNTESS");
			return TEXT("RUNNER");
		}
	}
	return TEXT("none");
}

void UPHICompanionParty::CyclePlayable()
{
	UWorld* W = GetWorld();
	if (!W) return;
	APlayerController* PC = W->GetFirstPlayerController();
	if (!PC) return;
	APawn* Cur = PC->GetPawn();
	APawn* R = RunnerPawn.Get();
	APawn* C = CountessPawn.Get();
	APawn* Next = nullptr;
	if (Cur == R && PlayableAlive(C)) Next = C;
	else if (Cur == C && PlayableAlive(R)) Next = R;
	else Next = PlayableAlive(R) ? R : C;
	if (!Next || Next == Cur) return;
	if (Cur) { PC->UnPossess(); Cur->SpawnDefaultController(); }
	PC->Possess(Next);
	UE_LOG(LogVEIL, Log, TEXT("PHI PARTY LEAD: %s takes the lead."), *LeadName());
}

void UPHICompanionParty::FollowTick(float DT)
{
	static float Acc = 0.f;
	Acc += DT;
	if (Acc < 1.f) return;
	Acc = 0.f;
	UWorld* W = GetWorld();
	if (!W) return;
	const APlayerController* PC = W->GetFirstPlayerController();
	const APawn* Lead = PC ? PC->GetPawn() : nullptr;
	if (!Lead) return;
	APawn* Mates[2] = { RunnerPawn.Get(), CountessPawn.Get() };
	int32 Flank = 0;
	for (APawn* M : Mates)
	{
		if (!IsValid(M) || M == Lead || !PlayableAlive(M)) continue;
		if (!M->GetController()) { M->SpawnDefaultController(); }
		AAIController* AI = Cast<AAIController>(M->GetController());
		if (!AI) continue;
		// PHI LAW: flank formation, never the camera lane. Mates hold beside
		// the lead (right/left by member), out of the boom line behind.
		const float Side = (Flank++ % 2 == 0) ? 1.f : -1.f;
		const FVector FlankAt = Lead->GetActorLocation()
			- Lead->GetActorForwardVector() * 350.f
			+ Lead->GetActorRightVector() * 250.f * Side;
		if (FVector::Dist(M->GetActorLocation(), FlankAt) > 200.f)
		{
			AI->MoveToLocation(FlankAt, 80.f);
		}
	}
}

void UPHICompanionParty::Beat(bool& Flag, const TCHAR* Who, const TCHAR* Deed)
{
	if (Flag) return;
	Flag = true;
	UE_LOG(LogVEIL, Log, TEXT("PHI BEAT %s: %s"), Who, Deed);
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Cyan, FString::Printf(TEXT("%s: %s"), Who, Deed)); }
}

void UPHICompanionParty::BeatsTick(float DT)
{
	static float Acc = 0.f;
	Acc += DT;
	if (Acc < 2.f) return;
	Acc = 0.f;
	UWorld* W = GetWorld();
	if (!W) return;
	UOMNIQuestSystem* Q = W->GetSubsystem<UOMNIQuestSystem>();
	UPHISideQuests* S = W->GetSubsystem<UPHISideQuests>();
	if (S && S->Echoes() >= 8) { Beat(bVesper, TEXT("VESPER"), TEXT("First Blood - eight echoes down.")); }
	if (S && S->Beacons() >= 3) { Beat(bQuill, TEXT("QUILL"), TEXT("Kept Signals - all three beacons burn.")); }
	if (Q && Q->ForgeLevel >= 2) { Beat(bBastion, TEXT("BASTION"), TEXT("Forge Heart - the fire answers twice.")); }
	if (Q && (Q->RegionMap.Contains(TEXT("OMNIWorld")) || Q->RegionMap.Contains(TEXT("CountessRegion")))) { Beat(bHome, TEXT("COUNTESS"), TEXT("Home Ground - her region remembers her.")); }
}

