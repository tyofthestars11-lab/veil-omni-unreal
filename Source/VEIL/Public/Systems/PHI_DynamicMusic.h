#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PHI_DynamicMusic.generated.h"
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VEIL_API UPHI_DynamicMusicComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPHI_DynamicMusicComponent();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PHI|Music")
    class USoundWave* ExploreStem;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PHI|Music")
    class USoundWave* TensionStem;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PHI|Music")
    class USoundWave* CombatStem;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PHI|Music")
    class USoundWave* DiscoveryStem;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PHI|Music")
    class USoundWave* TriumphStem;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PHI|Music")
    int32 MusicState;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PHI|Music")
    int32 Region;
    UFUNCTION(BlueprintCallable, Category="PHI|Music")
    void SetMusicState(int32 NewState);
    UFUNCTION(BlueprintCallable, Category="PHI|Music")
    void SetRegion(int32 NewRegion);
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
    virtual void BeginPlay() override;
private:
    TArray<class UAudioComponent*> LayerAudios;
    TArray<float> TargetVolumes;
    TArray<float> CurrentVolumes;
    float CrossfadeRate;
    void UpdateTargets();
    void PlayAllStems();
};