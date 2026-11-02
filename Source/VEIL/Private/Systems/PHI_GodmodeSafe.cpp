#include "Systems/PHI_GodmodeSafe.h"
#include "GameFramework/Pawn.h"
void UPHI_GodmodeSafe::SetGodmode(APawn* Pawn, bool Enable)
{
    if (!Pawn) { return; }
    if (!Pawn->IsValidLowLevel()) { return; }
    if (Pawn->IsPendingKillPending()) { return; }
    Pawn->SetCanBeDamaged(!Enable);
}
bool UPHI_GodmodeSafe::IsGodmode(APawn* Pawn)
{
    if (!Pawn) { return false; }
    if (!Pawn->IsValidLowLevel()) { return false; }
    return !Pawn->CanBeDamaged();
}
