#include "Systems/PHIOMNI.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/PHIMito.h"
#include "VEIL.h"
#include "Systems/VEILPhi.h"

namespace
{
	// 1, 1/phi, 1/phi^2 ... normalised to sum to one. The field is weighted by
	// the same law that weights everything else here, not by equal coin flips.
	void Normalise(TArray<float>& Amplitudes)
	{
		float Sum = 0.f;
		for (float& A : Amplitudes) { Sum += A; }
		if (Sum <= 0.f) { return; }
		for (float& A : Amplitudes) { A /= Sum; }
	}

	TArray<float> PhiWeights(int32 Count)
	{
		TArray<float> W;
		for (int32 i = 0; i < Count; ++i)
		{
			W.Add(FMath::Pow(UPHIMito::Phi, -(float)i));
		}
		Normalise(W);
		return W;
	}

	// The golden hash: a deterministic function of the site's name and how many
	// times it has been measured. Golden-ratio addition is the most uniformly
	// distributed sequence there is, so two neighbouring sites never resolve to
	// the same branch by accident.
	float GoldenHash(FName Id, int32 Observations)
	{
		const uint32 NameHash = GetTypeHash(Id);
		double X = FMath::Fmod((double)NameHash * 0.6180339887498949
			+ (double)(Observations + 1) * 0.3819660112501051, 1.0);
		if (X < 0.0) { X += 1.0; }
		return (float)X;
	}
}

void UPHIOMNI::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	Seed();
}

void UPHIOMNI::Seed()
{
	Sites.Reset();

	// THE FIELD IS SEEDED ONCE, AS A WHOLE. Each site names the states the world
	// is genuinely holding at the same time - not a plan with a current step.
	auto Add = [this](const TCHAR* Id, std::initializer_list<const TCHAR*> States,
		const FVector& Where, float Radius)
	{
		FPHIOmniSite Site;
		Site.Id = FName(Id);
		for (const TCHAR* S : States) { Site.States.Add(FString(S)); }
		Site.Amplitudes = PhiWeights(Site.States.Num());
		Site.Where = Where;
		Site.Radius = Radius;
		Sites.Add(Site);
	};

	Add(TEXT("keeper"), {
		TEXT("counting the lamps"),
		TEXT("listening for the Signal"),
		TEXT("waiting for a runner who is already walking")
	}, FVector(0.f, -2600.f, 0.f), 2200.f);

	Add(TEXT("nest"), {
		TEXT("trading salvage"),
		TEXT("burying an eleventh name"),
		TEXT("hearing the hum change key")
	}, FVector(-450.f, 780.f, 0.f), 2600.f);

	Add(TEXT("spire"), {
		TEXT("sealed and quiet"),
		TEXT("decoding a dead channel"),
		TEXT("letting one corridor be walked wrong")
	}, FVector(2800.f, 500.f, 0.f), 2600.f);

	Add(TEXT("deep"), {
		TEXT("pooling"),
		TEXT("wearing a coat"),
		TEXT("answering")
	}, FVector(0.f, 0.f, 0.f), 3000.f);

	Add(TEXT("weather"), {
		TEXT("the quiet hours"),
		TEXT("the high sun"),
		TEXT("ash falling without wind")
	}, FVector(0.f, 0.f, 0.f), 12000.f);      // a wide radius: the sky is observed from anywhere

	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI: field seeded - %d site(s) held open at once. %s"),
		Sites.Num(), *OmniLine());
}

int32 UPHIOMNI::Measure(const FPHIOmniSite& Site)
{
	if (Site.States.Num() == 0) { return INDEX_NONE; }
	const float Pick = GoldenHash(Site.Id, Site.Observations);

	// Walk the amplitude weights: the golden position lands in the highest
	// amplitude bin most often, which is what "most probable" has to mean if the
	// weights are real.
	float Acc = 0.f;
	for (int32 i = 0; i < Site.Amplitudes.Num(); ++i)
	{
		Acc += Site.Amplitudes[i];
		if (Pick < Acc) { return i; }
	}
	return Site.Amplitudes.Num() - 1;
}

bool UPHIOMNI::Observe(FName SiteId)
{
	for (FPHIOmniSite& Site : Sites)
	{
		if (Site.Id != SiteId || Site.Collapsed != INDEX_NONE) { continue; }

		const int32 Chosen = Measure(Site);
		if (!Site.States.IsValidIndex(Chosen)) { return false; }

		Site.Collapsed = Chosen;
		++Site.Observations;

		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI: %s collapses -> \"%s\" (weight %.3f, measured at rung %.4f)."),
			*Site.Id.ToString(), *Site.States[Chosen], Site.Amplitudes[Chosen],
			FMath::Loge(FMath::Max(Site.Amplitudes[Chosen], 1.e-6f)) / FMath::Loge(UPHIMito::Phi));
		return true;
	}
	return false;
}

int32 UPHIOMNI::ObserveWhere(const FVector& WorldLocation)
{
	int32 Fell = 0;
	for (FPHIOmniSite& Site : Sites)
	{
		if (Site.Collapsed != INDEX_NONE) { continue; }
		if (FVector::Dist2D(WorldLocation, Site.Where) <= Site.Radius)
		{
			if (Observe(Site.Id)) { ++Fell; }
		}
	}
	return Fell;
}

void UPHIOMNI::Tick(float DeltaTime)
{
	// ONE PASS PER PHI BEAT. Every site is evaluated in the same pass, from the
	// same field, in the same loop - so no site can be decided "after" another
	// and no ordering gets baked into the world by the update itself.
	SincePass += DeltaTime;
	const float Beat = 1.f / UPHIMito::Phi;
	if (SincePass < Beat) { return; }
	SincePass = 0.f;
	++Passes;

	UWorld* W = GetWorld();
	const APawn* Pawn = W ? UGameplayStatics::GetPlayerPawn(W, 0) : nullptr;
	if (!Pawn) { return; }

	const int32 Fell = ObserveWhere(Pawn->GetActorLocation());
	if (Fell > 0 || Passes % 8 == 0)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI: pass %d - %d state(s) collapsed by the runner standing here. %s"),
			Passes, Fell, *OmniLine());
	}
}

FString UPHIOMNI::StateOf(FName SiteId) const
{
	for (const FPHIOmniSite& Site : Sites)
	{
		if (Site.Id != SiteId) { continue; }
		if (Site.Collapsed == INDEX_NONE) { return TEXT("-"); }
		return Site.States.IsValidIndex(Site.Collapsed) ? Site.States[Site.Collapsed] : TEXT("-");
	}
	return TEXT("-");
}

FString UPHIOMNI::OmniLine() const
{
	int32 States = 0, Collapsed = 0;
	for (const FPHIOmniSite& Site : Sites)
	{
		States += Site.States.Num();
		if (Site.Collapsed != INDEX_NONE) { ++Collapsed; }
	}
	const float Coherence = Sites.Num() > 0 ? (float)Collapsed / (float)Sites.Num() : 0.f;
	return FString::Printf(TEXT("OMNI  %d sites  %d states  %d measured  coherence %.3f"),
		Sites.Num(), States, Collapsed, Coherence);
}
