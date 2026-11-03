#include "AI/VEILNPC.h"
void AVEILNPC::BeginPlay() { Super::BeginPlay(); }
void AVEILNPC::ThinkLive(float DeltaTime)
{
    if (DeltaTime <= 0.f) return;
    UE_LOG(LogTemp, VeryVerbose, TEXT("VEIL NPC %s thinking live"), *NPCName);
}
