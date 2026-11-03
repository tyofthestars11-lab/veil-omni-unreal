#include "Systems/VEILRegionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Systems/VEILLoopSubsystem.h"
#include "VEIL.h"

FVEILRegionInfo UVEILRegionSubsystem::GetInfo(EVEILRegion R) const
{
	FVEILRegionInfo I;
	const float S = WorldScale;
	switch (R)
	{
	// Centres sit on the world's own landmarks, in metres x WorldScale. The old values
	// ((0,150), (-150,20), (0,-160), (150,20)) put every region ~15 km outside a level that
	// spans about 74 m, so RegionAt() answered "Megastructure Deep" everywhere the runner
	// could actually walk. These are the anchors from the design:
	//   Keeper 900,0        -> Hub
	//   Seer -450,780       -> The Nest
	//   Spire zone 0,3500   -> The Spire (Hark's law: the landmark is not the Hub)
	//   fragment 600,-2100  -> Omni District (the Cache at 0,-6200 also resolves here)
	case EVEILRegion::Hub:
		I.Name = FText::FromString(TEXT("Hub"));
		I.Center = FVector2D(9.f, 0.f) * S; I.Palette = FLinearColor(0.85f, 0.63f, 0.36f);
		I.MusicCue = TEXT("Explore_Hub"); I.FragmentSites = 0; break;
	case EVEILRegion::Nest:
		I.Name = FText::FromString(TEXT("The Nest"));
		I.Center = FVector2D(-4.5f, 7.8f) * S; I.Palette = FLinearColor(0.25f, 0.75f, 0.35f);
		I.MusicCue = TEXT("Explore_Nest"); I.FragmentSites = 3; break;
	case EVEILRegion::Spire:
		I.Name = FText::FromString(TEXT("The Spire"));
		// 0,3500, not 2800,500: the Spire is its own zone on the district's north edge.
		I.Center = FVector2D(0.f, 35.f) * S; I.Palette = FLinearColor(0.35f, 0.66f, 1.f);
		I.MusicCue = TEXT("Combat_Spire"); I.FragmentSites = 2; break;
	case EVEILRegion::OmniDistrict:
		I.Name = FText::FromString(TEXT("Omni District"));
		I.Center = FVector2D(6.f, -21.f) * S; I.Palette = FLinearColor(1.f, 0.24f, 0.94f);
		I.MusicCue = TEXT("Explore_Omni"); I.FragmentSites = 3; break;
	case EVEILRegion::MegastructureDeep:
		I.Name = FText::FromString(TEXT("Megastructure Deep"));
		I.Center = FVector2D(0.f, 0.f); I.Palette = FLinearColor(1.f, 0.77f, 0.23f);
		I.MusicCue = TEXT("Combat_Deep"); I.FragmentSites = 2; break;
	}
	return I;
}

// Megastructure Deep is the Vault arena inside Omni District: the inner disc, and only
// while the final defence is running. Outside that, the nearest landmark centre wins.
EVEILRegion UVEILRegionSubsystem::RegionAt(const FVector& P) const
{
	const FVector2D XY(P.X, P.Y);
	const UVEILLoopSubsystem* Loop = GetWorld() ? GetWorld()->GetSubsystem<UVEILLoopSubsystem>() : nullptr;
	const bool bDefenceUnderway = Loop && Loop->Stage >= EVEILStage::FinalWave;
	if (bDefenceUnderway && XY.Size() < DeepRadiusCm)
	{
		return EVEILRegion::MegastructureDeep;
	}
	EVEILRegion Best = EVEILRegion::Hub;
	float BestD = TNumericLimits<float>::Max();
	for (EVEILRegion R : { EVEILRegion::Hub, EVEILRegion::Nest, EVEILRegion::Spire, EVEILRegion::OmniDistrict })
	{
		const float D = FVector2D::Distance(XY, GetInfo(R).Center);
		if (D < BestD) { BestD = D; Best = R; }
	}
	return Best;
}

bool UVEILRegionSubsystem::IsCombatRegion(EVEILRegion R)
{
	return R == EVEILRegion::Spire || R == EVEILRegion::MegastructureDeep;
}

void UVEILRegionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	PollTimer = 0.f;
	if (const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		// Resolve the seat the runner actually wakes in, so the region map is visible
		// in the log instead of being assumed from the default.
		Current = RegionAt(Pawn->GetActorLocation());
		UE_LOG(LogVEIL, Log, TEXT("VEIL REGION: start = %s at %s"),
			*GetInfo(Current).Name.ToString(), *Pawn->GetActorLocation().ToCompactString());
	}
}

void UVEILRegionSubsystem::Tick(float DeltaTime)
{
	PollTimer -= DeltaTime;
	if (PollTimer > 0.f) { return; }
	PollTimer = 0.25f;
	const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn) { return; }
	const EVEILRegion R = RegionAt(Pawn->GetActorLocation());
	if (R != Current)
	{
		Current = R;
		UE_LOG(LogVEIL, Log, TEXT("VEIL REGION -> %s at %s"),
			*GetInfo(R).Name.ToString(), *Pawn->GetActorLocation().ToCompactString());
		OnRegionChanged.Broadcast(R); // drive the region banner and crossfade the music
	}
}


