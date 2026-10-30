#include "VEILRegionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

FVEILRegionInfo UVEILRegionSubsystem::GetInfo(EVEILRegion R) const
{
	FVEILRegionInfo I;
	const float S = WorldScale;
	switch (R)
	{
	case EVEILRegion::Hub:
		I.Name = FText::FromString(TEXT("Hub"));
		I.Center = FVector2D(0.f, 150.f) * S; I.Palette = FLinearColor(0.85f, 0.63f, 0.36f);
		I.MusicCue = TEXT("Explore_Hub"); I.FragmentSites = 0; break;
	case EVEILRegion::Nest:
		I.Name = FText::FromString(TEXT("The Nest"));
		I.Center = FVector2D(-150.f, 20.f) * S; I.Palette = FLinearColor(0.25f, 0.75f, 0.35f);
		I.MusicCue = TEXT("Explore_Nest"); I.FragmentSites = 3; break;
	case EVEILRegion::Spire:
		I.Name = FText::FromString(TEXT("The Spire"));
		I.Center = FVector2D(0.f, -160.f) * S; I.Palette = FLinearColor(0.35f, 0.66f, 1.f);
		I.MusicCue = TEXT("Combat_Spire"); I.FragmentSites = 2; break;
	case EVEILRegion::OmniDistrict:
		I.Name = FText::FromString(TEXT("Omni District"));
		I.Center = FVector2D(150.f, 20.f) * S; I.Palette = FLinearColor(1.f, 0.24f, 0.94f);
		I.MusicCue = TEXT("Explore_Omni"); I.FragmentSites = 3; break;
	case EVEILRegion::MegastructureDeep:
		I.Name = FText::FromString(TEXT("Megastructure Deep"));
		I.Center = FVector2D(0.f, 0.f); I.Palette = FLinearColor(1.f, 0.77f, 0.23f);
		I.MusicCue = TEXT("Combat_Deep"); I.FragmentSites = 2; break;
	}
	return I;
}

// Same rule as the web game: inside 78 m of the centre is Megastructure Deep, else the nearest centre.
EVEILRegion UVEILRegionSubsystem::RegionAt(const FVector& P) const
{
	const FVector2D XY(P.X, P.Y);
	if (XY.Size() < 78.f * WorldScale) { return EVEILRegion::MegastructureDeep; }
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
		OnRegionChanged.Broadcast(R); // drive the region banner and crossfade the music
	}
}
