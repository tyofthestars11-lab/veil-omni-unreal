#include "Game/PHI_GameState_Fixed.h"
#include "Engine/Engine.h"
APHI_GameState_Fixed::APHI_GameState_Fixed() : ProgressionPercent(0.f), CurrentStage(0) {}
void APHI_GameState_Fixed::AdvanceProgression(int32 Stage, float Percent)
{
    if (!IsValidLowLevel()) { return; }
    if (IsPendingKillPending()) { return; }
    if (Stage < CurrentStage) { return; }
    CurrentStage = Stage;
    ProgressionPercent = FMath::Clamp(Percent, 0.f, 100.f);
    UE_LOG(LogTemp, Log, TEXT("PHI Progress %d -> %.0f%%"), Stage, ProgressionPercent);
    if (GEngine)
    {
        if (GEngine->GetCurrentPlayWorld())
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Emerald, FString::Printf(TEXT("PHI %.0f%%"), ProgressionPercent));
        }
    }
}
void APHI_GameState_Fixed::AddGameplayNeed(FString Need)
{
    if (!IsValidLowLevel()) { return; }
    UE_LOG(LogTemp, Log, TEXT("PHI Gameplay Need: %s - interact climb push"), *Need);
}
