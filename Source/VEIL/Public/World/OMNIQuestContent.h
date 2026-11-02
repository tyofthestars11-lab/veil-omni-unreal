// OMNI RPG: PATH OF LIGHT - the quest's physical content.
//
// Statues that talk by standing close, fragments that gather by touch,
// the Seer's cache by the south wall. All overlap-driven: no input assets
// needed, the lead's presence (Runner or Countess) is the interaction.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OMNIQuestContent.generated.h"

class USphereComponent;
class UStaticMeshComponent;

// ---------------------------------------------------------------------------
// A talking statue: Keeper, Seer, Cartographer.
// ---------------------------------------------------------------------------
UCLASS()
class VEIL_API AOMNIQuestGiver : public AActor
{
	GENERATED_BODY()

public:
	AOMNIQuestGiver();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OMNI|Quest")
	FName GiverName = FName(TEXT("Keeper"));

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnTalkRadius(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	TObjectPtr<USphereComponent> TalkRadius;
	bool bSpokenThisVisit = false;
	// Talk brain: remembers the Runner, one greeting per visit, no spam.
	bool bMetRunner = false;
	float LastTalkWorldTime = -1000.f;
	UFUNCTION()
	void OnTalkEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

// ---------------------------------------------------------------------------
// A fragment of the broken signal: gold, turning, gathered by touch.
// ---------------------------------------------------------------------------
UCLASS()
class VEIL_API AOMNIFragment : public AActor
{
	GENERATED_BODY()

public:
	AOMNIFragment();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnTouched(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	TObjectPtr<UStaticMeshComponent> Body;
	TObjectPtr<USphereComponent> TouchRadius;
	float BaseZ = 0.f;
};

// ---------------------------------------------------------------------------
// A lore note: gold leaf that reads aloud, three pages, over a living world.
// ---------------------------------------------------------------------------
UCLASS()
class VEIL_API AOMNILoreNote : public AActor
{
 GENERATED_BODY()

public:
 AOMNILoreNote();
 const FString& GetTitle() const { return NoteTitle; }
 const TArray<FString>& GetPages() const { return NotePages; }
 FName GetVoiceId() const { return NoteVoice; }
 void SetText(const FString& Title, const TArray<FString>& Pages, FName VoiceId)
 {
  NoteTitle = Title; NotePages = Pages; NoteVoice = VoiceId;
 }

protected:
 virtual void BeginPlay() override;

private:
 UFUNCTION()
 void OnReadRadius(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
 const FHitResult& SweepResult);

 UPROPERTY(EditAnywhere, Category = "OMNI|Lore")
 FString NoteTitle = TEXT("Leaf");
 UPROPERTY(EditAnywhere, Category = "OMNI|Lore")
 TArray<FString> NotePages;
 UPROPERTY(EditAnywhere, Category = "OMNI|Lore")
 FName NoteVoice = FName(TEXT("V02"));
 TObjectPtr<UStaticMeshComponent> Leaf;
 TObjectPtr<USphereComponent> ReadRadius;
};

// ---------------------------------------------------------------------------
// The Seer's survey cache: green, glowing, at the district's south wall.
// ---------------------------------------------------------------------------
UCLASS()
class VEIL_API AOMNICache : public AActor
{
	GENERATED_BODY()

public:
	AOMNICache();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnOpened(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	TObjectPtr<USphereComponent> OpenRadius;
};

