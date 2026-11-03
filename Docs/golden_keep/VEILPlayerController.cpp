#include "Game/VEILPlayerController.h"
void AVEILPlayerController::BeginPlay() { Super::BeginPlay(); SetInputMode(FInputModeGameOnly()); bShowMouseCursor = false; }
