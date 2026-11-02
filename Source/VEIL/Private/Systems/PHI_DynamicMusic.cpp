#include "Systems/PHI_DynamicMusic.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
UPHI_DynamicMusicComponent::UPHI_DynamicMusicComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    MusicState = 0;
    Region = 0;
    CrossfadeRate = 0.5f;
    ExploreStem = nullptr;
    TensionStem = nullptr;
    CombatStem = nullptr;
    DiscoveryStem = nullptr;
    TriumphStem = nullptr;
}
void UPHI_DynamicMusicComponent::BeginPlay()
{
    Super::BeginPlay();
    PlayAllStems();
    UpdateTargets();
}
void UPHI_DynamicMusicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    for (int32 i = 0; i < LayerAudios.Num(); i++)
    {
        if (LayerAudios[i] == nullptr) { continue; }
        if (i >= CurrentVolumes.Num() || i >= TargetVolumes.Num()) { continue; }
        float diff = TargetVolumes[i] - CurrentVolumes[i];
        float step = CrossfadeRate * DeltaTime;
        if (diff > step) { CurrentVolumes[i] += step; }
        else if (diff < -step) { CurrentVolumes[i] -= step; }
        else { CurrentVolumes[i] = TargetVolumes[i]; }
        LayerAudios[i]->SetVolumeMultiplier(CurrentVolumes[i]);
    }
}
void UPHI_DynamicMusicComponent::SetMusicState(int32 NewState)
{
    MusicState = NewState;
    UpdateTargets();
}
void UPHI_DynamicMusicComponent::SetRegion(int32 NewRegion)
{
    Region = NewRegion;
    UpdateTargets();
}
void UPHI_DynamicMusicComponent::UpdateTargets()
{
    TargetVolumes.Empty();
    TargetVolumes.Add(0.0f);
    TargetVolumes.Add(0.0f);
    TargetVolumes.Add(0.0f);
    TargetVolumes.Add(0.0f);
    TargetVolumes.Add(0.0f);
    if (MusicState == 0) { TargetVolumes[0] = 1.0f; }
    else if (MusicState == 1) { TargetVolumes[1] = 1.0f; TargetVolumes[0] = 0.3f; }
    else if (MusicState == 2) { TargetVolumes[2] = 1.0f; TargetVolumes[1] = 0.5f; }
    else if (MusicState == 3) { TargetVolumes[3] = 1.0f; TargetVolumes[0] = 0.4f; }
    else if (MusicState == 4) { TargetVolumes[4] = 1.0f; TargetVolumes[0] = 0.3f; }
    else if (MusicState == 5) { TargetVolumes[2] = 1.0f; TargetVolumes[4] = 0.6f; TargetVolumes[1] = 0.5f; }
    if (Region == 0) { if (TargetVolumes[0] < 0.6f) { TargetVolumes[0] = 0.6f; } }
    else if (Region == 1) { if (TargetVolumes[3] < 0.6f) { TargetVolumes[3] = 0.6f; } }
    else if (Region == 2) { if (TargetVolumes[1] < 0.6f) { TargetVolumes[1] = 0.6f; } }
    else if (Region == 3) { if (TargetVolumes[3] < 0.6f) { TargetVolumes[3] = 0.6f; } }
    else if (Region == 4) { if (TargetVolumes[1] < 0.6f) { TargetVolumes[1] = 0.6f; } }
    else if (Region == 5) { if (TargetVolumes[4] < 0.6f) { TargetVolumes[4] = 0.6f; } }
}
void UPHI_DynamicMusicComponent::PlayAllStems()
{
    LayerAudios.Empty();
    CurrentVolumes.Empty();
    USoundWave* stems[5] = { ExploreStem, TensionStem, CombatStem, DiscoveryStem, TriumphStem };
    for (int32 i = 0; i < 5; i++)
    {
        if (stems[i] == nullptr) { LayerAudios.Add(nullptr); CurrentVolumes.Add(0.0f); continue; }
        UAudioComponent* comp = UGameplayStatics::SpawnSound2D(GetWorld(), stems[i]);
        if (comp != nullptr) { comp->SetVolumeMultiplier(0.0f); }
        LayerAudios.Add(comp);
        CurrentVolumes.Add(0.0f);
    }
}
