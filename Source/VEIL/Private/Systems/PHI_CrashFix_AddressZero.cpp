#include "Systems/PHI_CrashFix_AddressZero.h"
#include "Game/PHI_GameState_Fixed.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
bool UPHI_CrashFix_AddressZero::IsValidPawnForGodmode(APawn* Pawn)
{
    if (!Pawn) { return false; }
    if (!Pawn->IsValidLowLevel()) { return false; }
    if (Pawn->IsPendingKillPending()) { return false; }
    AController* Ctrl = Pawn->GetController();
    if (!Ctrl) { return false; }
    if (!Ctrl->IsValidLowLevel()) { return false; }
    if (ACharacter* Char = Cast<ACharacter>(Pawn))
    {
        UCharacterMovementComponent* Move = Char->GetCharacterMovement();
        if (!Move) { return false; }
        if (!Move->IsValidLowLevel()) { return false; }
    }
    return true;
}
bool UPHI_CrashFix_AddressZero::IsValidWorldForProgression(UWorld* World)
{
    if (!World) { return false; }
    if (!World->IsValidLowLevel()) { return false; }

    if (!World->GetGameState()) { return false; }
    return true;
}
void UPHI_CrashFix_AddressZero::GuardedAdvanceProgression(UObject* WorldContext, int32 Stage, float Percent)
{
    if (!WorldContext) { return; }
    if (!WorldContext->IsValidLowLevel()) { return; }
    UWorld* World = WorldContext->GetWorld();
    if (!IsValidWorldForProgression(World)) { return; }
    AGameStateBase* GS = UGameplayStatics::GetGameState(WorldContext);
    if (!GS) { return; }
    if (!GS->IsValidLowLevel()) { return; }
    if (APHI_GameState_Fixed* PHI = Cast<APHI_GameState_Fixed>(GS))
    {
        if (!PHI->IsValidLowLevel()) { return; }
        PHI->AdvanceProgression(Stage, Percent);
    }
}
