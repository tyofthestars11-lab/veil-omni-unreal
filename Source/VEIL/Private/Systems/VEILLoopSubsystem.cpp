#include "Systems/VEILLoopSubsystem.h"
#include "Systems/VEILCampaign.h"

void UVEILLoopSubsystem::SetStage(EVEILStage S)
{
	if (Stage == S) { return; }
	Stage = S;
	OnStageChanged.Broadcast(S);
}

void UVEILLoopSubsystem::SignalFound()
{
	if (Stage == EVEILStage::FindSignal) { SetStage(EVEILStage::Fragments); }
}

void UVEILLoopSubsystem::CollectFragment()
{
	if (Stage == EVEILStage::FindSignal) { SetStage(EVEILStage::Fragments); }
	if (Fragments >= FragmentsNeeded) { return; }
	++Fragments;
	OnFragmentCollected.Broadcast(Fragments);
}

bool UVEILLoopSubsystem::BuildArtifact()
{
	if (Fragments < FragmentsNeeded || Stage >= EVEILStage::Artifact) { return false; }
	SetStage(EVEILStage::Artifact);
	// Artifact built: move straight on to the defence.
	SetStage(EVEILStage::FinalWave);
	return true;
}

bool UVEILLoopSubsystem::StartFinalWave()
{
	return Stage == EVEILStage::FinalWave; // already armed by BuildArtifact
}

void UVEILLoopSubsystem::WaveCleared()
{
	if (UVEILCampaignSubsystem* Camp = GetWorld() ? GetWorld()->GetSubsystem<UVEILCampaignSubsystem>() : nullptr) { Camp->NoteWave(); }
	if (Stage != EVEILStage::FinalWave) { return; }
	WavesCleared = FMath::Min(WavesCleared + 1, TotalWaves);
}

void UVEILLoopSubsystem::BossDefeated()
{
	if (UVEILCampaignSubsystem* Camp = GetWorld() ? GetWorld()->GetSubsystem<UVEILCampaignSubsystem>() : nullptr) { Camp->NoteKill(true); }
	if (Stage != EVEILStage::FinalWave || WavesCleared < TotalWaves) { return; }
	bBossDefeated = true;
	SetStage(EVEILStage::Beacon);
}

bool UVEILLoopSubsystem::ActivateBeacon()
{
	if (Stage != EVEILStage::Beacon) { return false; }
	SetStage(EVEILStage::Won);
	return true;
}

float UVEILLoopSubsystem::ProgressPercent() const
{
	switch (Stage)
	{
	case EVEILStage::FindSignal: return 0.f;
	case EVEILStage::Fragments:  return 5.f + 55.f * Fragments / FragmentsNeeded;               // 5..60
	case EVEILStage::Artifact:   return 65.f;
	case EVEILStage::FinalWave:  return 70.f + 25.f * (WavesCleared + (bBossDefeated ? 1 : 0)) / (TotalWaves + 1); // 70..95
	case EVEILStage::Beacon:     return 95.f;
	default:                     return 100.f;
	}
}

// Fibonacci waves: 3, 5, 8, 13, 21
int32 UVEILLoopSubsystem::DronesForWave(int32 WaveIndex)
{
	static const int32 F[5] = { 3, 5, 8, 13, 21 };
	return F[FMath::Clamp(WaveIndex, 0, 4)];
}


