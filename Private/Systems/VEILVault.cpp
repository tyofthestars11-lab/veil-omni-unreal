#include "VEILVault.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"

// ---------------- Anchor ----------------
AVEILVaultAnchor::AVEILVaultAnchor()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionProfileName(TEXT("BlockAll")); // so the Countess' interact sweep hits it
	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(RootComponent);
	Glow->SetLightColor(FLinearColor(1.f, 0.78f, 0.23f));
	Glow->SetIntensity(0.f); // lights up when aligned
	Glow->SetAttenuationRadius(500.f);
}

void AVEILVaultAnchor::Interact_Implementation(AActor* /*Instigator*/)
{
	if (bAligned) { return; }
	bAligned = true;
	Glow->SetIntensity(8000.f);
	if (Vault) { Vault->NotifyAnchorAligned(); }
}

// ---------------- Vault ----------------
AVEILVault::AVEILVault()
{
	Lattice = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lattice"));
	RootComponent = Lattice;
	Lattice->SetCollisionProfileName(TEXT("BlockAll")); // grounded, solid
	Core = CreateDefaultSubobject<UPointLightComponent>(TEXT("Core"));
	Core->SetupAttachment(RootComponent);
	Core->SetRelativeLocation(FVector(0.f, 0.f, AnchorHeight));
	Core->SetLightColor(FLinearColor(1.f, 0.78f, 0.23f)); // gold
	Core->SetIntensity(5000.f);
	Core->SetAttenuationRadius(2500.f);
	AnchorClass = AVEILVaultAnchor::StaticClass();
}

void AVEILVault::OnConstruction(const FTransform& T)
{
	Super::OnConstruction(T);
	if (LatticeMesh) { Lattice->SetStaticMesh(LatticeMesh); }
}

// Regular tetrahedron vertices (1,1,1) (1,-1,-1) (-1,1,-1) (-1,-1,1), normalised:
// the angle between any two is acos(-1/3) = 109.471221 degrees.
FVector AVEILVault::TetraDirection(int32 i)
{
	static const FVector V[4] = { FVector(1,1,1), FVector(1,-1,-1), FVector(-1,1,-1), FVector(-1,-1,1) };
	return V[FMath::Clamp(i, 0, 3)].GetSafeNormal();
}

void AVEILVault::BeginPlay()
{
	Super::BeginPlay();
	Anchors.Reset();
	for (int32 i = 0; i < 4; ++i)
	{
		const FVector Offset = TetraDirection(i) * AnchorRadius;
		const FVector Pos = GetActorLocation() + FVector(0.f, 0.f, AnchorHeight) + Offset;
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AVEILVaultAnchor* A = GetWorld()->SpawnActor<AVEILVaultAnchor>(AnchorClass ? *AnchorClass : AVEILVaultAnchor::StaticClass(), Pos, FRotator::ZeroRotator, P);
		if (A)
		{
			A->Vault = this;
			A->Index = i;
			if (AnchorMesh) { A->FindComponentByClass<UStaticMeshComponent>()->SetStaticMesh(AnchorMesh); }
			Anchors.Add(A);
		}
	}
}

int32 AVEILVault::AlignedCount() const
{
	int32 N = 0;
	for (const AVEILVaultAnchor* A : Anchors) { if (A && A->bAligned) { ++N; } }
	return N;
}

float AVEILVault::OpenFactor() const
{
	return (AlignedCount() == 4) ? 1.f : 0.f;
}

void AVEILVault::NotifyAnchorAligned() { TryOpen(); }

void AVEILVault::TryOpen()
{
	if (bOpen) { return; }
	if (OpenFactor() >= 1.f && bArtifactBuilt)
	{
		bOpen = true;
		Core->SetIntensity(30000.f);
		OnVaultOpened.Broadcast();
	}
}
