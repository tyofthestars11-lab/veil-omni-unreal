import io

ROOT = r"D:\VEIL-OMNI" + "\\"

def rd(p):
    return io.open(ROOT + p, encoding="utf-8", newline=None).read()

def wr(p, s):
    io.open(ROOT + p, "w", encoding="utf-8", newline="\r\n").write(s)

# ------------------------------------------------------------------ header
p = "Source/VEIL/Public/World/VEILWilds.h"
t = rd(p)
start = t.find("/**\r\n * A MINE.")
if start < 0:
    start = t.find("/**\n * A MINE.")
end = t.find("/**\n * A CAMP IS NOT A SPAWN POINT")
if end < 0:
    end = t.find("/**\r\n * A CAMP IS NOT A SPAWN POINT")

NEW_CLASS = '''/**
 * A MINE IS A PLACE WITH A NAME, NOT A CONE ON A HILL.
 *
 * Every mine in the wilds is authored: it has a name, an owner, an ore, a depth
 * and a reserve, and it is built as a headframe - four posts, two beams, a
 * shaft, and the seam itself piled at the mouth. Nothing here is a placeholder
 * standing in for a mine, and nothing is rolled at random: the twenty-four
 * names below are the world's mines, and a run always finds the same ones.
 *
 * Owning matters. The owner is whose ground it is, so stripping a seam is a
 * claim on someone's property, and the reply names them.
 */
UCLASS()
class VEIL_API AVEILMineNode : public AActor
{
	GENERATED_BODY()

public:
	AVEILMineNode();
	virtual void Tick(float DeltaSeconds) override;

	/** Give the mine the identity of its entry in the mine book. */
	void Init(int32 InIndex);

	UFUNCTION()
	void OnEnter(UPrimitiveComponent* OverlappedComp, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& Sweep);

	UPROPERTY() USceneComponent* Root = nullptr;
	UPROPERTY() UPointLightComponent* Lamp = nullptr;
	UPROPERTY() USphereComponent* Trigger = nullptr;

	const FString& MineName() const { return Name; }
	const FString& MineOwner() const { return Owner; }
	const FString& MineOre() const { return Ore; }
	float MineDepth() const { return DepthM; }
	int32 MineReserve() const { return Reserve; }

private:
	/** Posts, beams, shaft and seam - the headframe, built once. */
	void BuildHead();

	FString Name;
	FString Owner;
	FString Ore;
	FLinearColor OreTint = FLinearColor(0.35f, 0.62f, 1.f);
	float DepthM = 0.f;
	int32 Reserve = 0;
	int32 Gold = 40;
	int32 Index = 0;
	float RespawnAt = 0.f;
	bool bSpent = false;
};

'''
if start >= 0 and end > start:
    t = t[:start] + NEW_CLASS + t[end:]
    wr(p, t)
    print("header: mine is a named place")
else:
    print("header: MINE BLOCK NOT FOUND", start, end)

# ------------------------------------------------------------------ cpp
p = "Source/VEIL/Private/World/VEILWilds.cpp"
t = rd(p)

spec_anchor = "\t/** Rung of a quantity: ln(x) / ln(phi). Logged so the body can be read as a place. */"
SPEC = '''\t/**
	 * THE MINE BOOK. Twenty-four authored seams, each with an owner and an ore.
	 * A run always finds the same mines in the same places: these are the claims
	 * of the world, not a scatter of geometry that happens to be mineable.
	 */
	struct FWileSpec
	{
		const TCHAR* Name;
		const TCHAR* Owner;
		const TCHAR* Ore;
		float DepthM;
		int32 Reserve;
		FLinearColor Tint;
	};

	const FWileSpec GMineBook[] =
	{
		{ TEXT("IRONWIDOW"),   TEXT("Keeper's Compact"),    TEXT("iron"),      180.f,  620, FLinearColor(0.62f, 0.64f, 0.70f) },
		{ TEXT("COPPERMAW"),   TEXT("Keeper's Compact"),    TEXT("copper"),    240.f,  540, FLinearColor(0.85f, 0.48f, 0.22f) },
		{ TEXT("GREYSPINE"),   TEXT("Keeper's Compact"),    TEXT("iron"),      310.f,  700, FLinearColor(0.55f, 0.57f, 0.63f) },
		{ TEXT("LAMPLESS"),    TEXT("Keeper's Compact"),   TEXT("coal"),      420.f,  900, FLinearColor(0.22f, 0.22f, 0.26f) },
		{ TEXT("SALTGRACE"),   TEXT("Seer's Nest"),         TEXT("salt"),      120.f,  380, FLinearColor(0.92f, 0.93f, 0.95f) },
		{ TEXT("TIDEVANE"),    TEXT("Seer's Nest"),         TEXT("tide-glass"),210.f,  460, FLinearColor(0.30f, 0.78f, 0.86f) },
		{ TEXT("CLIFFSONG"),   TEXT("Seer's Nest"),         TEXT("salt"),      260.f,  420, FLinearColor(0.88f, 0.92f, 0.98f) },
		{ TEXT("RAINHOLLOW"),  TEXT("Seer's Nest"),         TEXT("tide-glass"),150.f,  390, FLinearColor(0.26f, 0.70f, 0.80f) },
		{ TEXT("BLACKGLASS"),  TEXT("Cartographer's Guild"),TEXT("obsidian"),  340.f,  520, FLinearColor(0.16f, 0.14f, 0.22f) },
		{ TEXT("MERIDIAN"),    TEXT("Cartographer's Guild"),TEXT("iron"),      280.f,  610, FLinearColor(0.58f, 0.60f, 0.66f) },
		{ TEXT("THEODOLITE"),  TEXT("Cartographer's Guild"),TEXT("obsidian"),  390.f,  480, FLinearColor(0.18f, 0.16f, 0.26f) },
		{ TEXT("FARSHORE"),    TEXT("Cartographer's Guild"),TEXT("salt"),      470.f,  560, FLinearColor(0.90f, 0.91f, 0.93f) },
		{ TEXT("EMBERVENT"),   TEXT("Countess"),            TEXT("ember"),     520.f,  760, FLinearColor(0.95f, 0.36f, 0.18f) },
		{ TEXT("MARROWLIGHT"), TEXT("Countess"),            TEXT("ember"),     610.f,  680, FLinearColor(0.92f, 0.30f, 0.14f) },
		{ TEXT("CINDERCLAIM"), TEXT("Countess"),            TEXT("coal"),      430.f,  820, FLinearColor(0.20f, 0.19f, 0.23f) },
		{ TEXT("REDHAND"),     TEXT("Countess"),            TEXT("copper"),    350.f,  600, FLinearColor(0.88f, 0.42f, 0.20f) },
		{ TEXT("HOLLOWDEEP"),  TEXT("the Warden"),          TEXT("void"),      980.f,  980, FLinearColor(0.42f, 0.20f, 0.68f) },
		{ TEXT("SILENTSEAM"),  TEXT("the Warden"),          TEXT("void"),      720.f,  840, FLinearColor(0.38f, 0.18f, 0.62f) },
		{ TEXT("LASTLIGHT"),   TEXT("the Warden"),          TEXT("obsidian"),  660.f,  700, FLinearColor(0.20f, 0.18f, 0.28f) },
		{ TEXT("THE ONE DOOR"),TEXT("the Warden"),          TEXT("void"),     1200.f, 1400, FLinearColor(0.50f, 0.24f, 0.76f) },
		{ TEXT("PALEACT"),     TEXT("the Hollow"),          TEXT("salt"),      300.f,  300, FLinearColor(0.86f, 0.88f, 0.90f) },
		{ TEXT("LISTENER'S REST"), TEXT("the Hollow"),      TEXT("obsidian"),  380.f,  340, FLinearColor(0.17f, 0.16f, 0.24f) },
		{ TEXT("BONECHOIR"),   TEXT("the Hollow"),          TEXT("void"),      540.f,  400, FLinearColor(0.36f, 0.22f, 0.58f) },
		{ TEXT("THE LONG QUIET"), TEXT("the Hollow"),       TEXT("coal"),      460.f,  380, FLinearColor(0.19f, 0.19f, 0.22f) },
	};
	constexpr int32 GMineBookCount = UE_ARRAY_COUNT(GMineBook);

''' + spec_anchor
if "GMineBook" not in t:
    t = t.replace(spec_anchor, SPEC, 1)
    print("cpp: mine book of 24 authored seams")
else:
    print("cpp: mine book already present")

# replace the constructor + Init + Tick block
start = t.find("AVEILMineNode::AVEILMineNode()")
end = t.find("// ---------------------------------------------------------------------------\r\n// THE POPULATION")
if end < 0:
    end = t.find("// ---------------------------------------------------------------------------\n// THE POPULATION")

NEW_MINE = r'''AVEILMineNode::AVEILMineNode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("MineRoot"));
	SetRootComponent(Root);

	Lamp = CreateDefaultSubobject<UPointLightComponent>(TEXT("Lamp"));
	Lamp->SetupAttachment(Root);
	Lamp->SetLightColor(OreTint);
	Lamp->SetIntensity(3200.f);
	Lamp->SetAttenuationRadius(1600.f);
	Lamp->SetCastShadows(false);
	Lamp->SetAffectGlobalIllumination(false);

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Root);
	Trigger->InitSphereRadius(360.f);
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);
}

void AVEILMineNode::Init(int32 InIndex)
{
	Index = InIndex;
	const FWileSpec& Spec = GMineBook[FMath::Abs(Index) % GMineBookCount];

	// The identity IS the mine. Name, owner, ore, depth and reserve come from the
	// book, so the same seam is the same seam on every run.
	Name = Spec.Name;
	Owner = Spec.Owner;
	Ore = Spec.Ore;
	DepthM = Spec.DepthM;
	Reserve = Spec.Reserve;
	OreTint = Spec.Tint;
	Gold = FMath::Max(15, Reserve / 10);

	if (Trigger && !Trigger->OnComponentBeginOverlap.IsAlreadyBound(this, &AVEILMineNode::OnEnter))
	{
		Trigger->OnComponentBeginOverlap.AddDynamic(this, &AVEILMineNode::OnEnter);
	}
	if (Lamp)
	{
		Lamp->SetLightColor(OreTint);
	}
	BuildHead();

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI MINE %02d %s - %s, %s seam, %0.f m down, reserve %d, worth %d gold (rung %.2f) at %s."),
		Index, *Name, *Owner, *Ore, DepthM, Reserve, Gold, RungOf((float)Gold),
		*GetActorLocation().ToCompactString());
}

void AVEILMineNode::BuildHead()
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	USceneComponent* Attach = GetRootComponent();
	if (!Cube || !Attach) { return; }

	auto Part = [this, Attach](UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FRotator& Rot, bool bSolid)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		if (!C) { return (UStaticMeshComponent*)nullptr; }
		C->SetupAttachment(Attach);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeRotation(Rot);
		C->SetRelativeScale3D(Scale);
		C->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		C->RegisterComponent();
		return C;
	};

	// FOUR POSTS AND TWO BEAMS: a headframe, not a rock. The posts are solid, so
	// the mouth of the mine is something the runner can walk around.
	const float PostH = 4.2f;
	for (int32 i = 0; i < 4; ++i)
	{
		const float SX = (i & 1) ? 1.f : -1.f;
		const float SY = (i & 2) ? 1.f : -1.f;
		Part(Cube, FVector(SX * 130.f, SY * 110.f, PostH * 50.f), FVector(0.34f, 0.34f, PostH), FRotator::ZeroRotator, true);
	}
	Part(Cube, FVector(0.f, 115.f, 400.f), FVector(3.4f, 0.30f, 0.30f), FRotator::ZeroRotator, true);
	Part(Cube, FVector(0.f, -115.f, 400.f), FVector(3.4f, 0.30f, 0.30f), FRotator::ZeroRotator, true);
	Part(Cube, FVector(0.f, 0.f, 430.f), FVector(0.30f, 2.6f, 0.30f), FRotator::ZeroRotator, false);

	// THE SHAFT: a dark mouth in the ground, not a hole the runner falls through.
	if (Cylinder)
	{
		UStaticMeshComponent* Mouth = Part(Cylinder, FVector(0.f, 0.f, 12.f), FVector(2.2f, 2.2f, 0.10f), FRotator::ZeroRotator, false);
		if (Mouth)
		{
			if (UMaterialInterface* Deep = LoadObject<UMaterialInterface>(nullptr, WildsCrystalPath))
			{
				Mouth->SetMaterial(0, Deep);
			}
		}
	}

	// THE SEAM: three boulders of the ore it actually yields, tinted by the book.
	for (int32 i = 0; i < 3; ++i)
	{
		const float A = FMath::DegreesToRadians(i * WildsGoldenDeg);
		UStaticMeshComponent* Rock = Part(Cube,
			FVector(FMath::Cos(A) * 210.f, FMath::Sin(A) * 190.f, 55.f + 12.f * i),
			FVector(1.5f - 0.2f * i, 1.3f + 0.15f * i, 1.1f - 0.1f * i),
			FRotator(0.f, i * 37.f, 0.f), false);
		if (Rock)
		{
			if (UMaterialInterface* OreMat = LoadObject<UMaterialInterface>(nullptr, WildsCrystalPath))
			{
				Rock->SetMaterial(0, OreMat);
			}
			Rock->SetCustomPrimitiveDataFloat(0, OreTint.R);
			Rock->SetCustomPrimitiveDataFloat(1, OreTint.G);
			Rock->SetCustomPrimitiveDataFloat(2, OreTint.B);
		}
	}
}

void AVEILMineNode::OnEnter(UPrimitiveComponent* OverlappedComp, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (bSpent) { return; }
	bSpent = true;
	RespawnAt = GetWorld() ? GetWorld()->GetTimeSeconds() + 120.f : 0.f;

	if (UOMNIQuestSystem* Q = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
	{
		Q->Gold += Gold;
		Q->AwardXP(45, TEXT("an ore seam stripped"));
	}
	SetActorHiddenInGame(false);
	// The seam does not disappear: it is someone's mine. It goes dark and comes
	// back, and the line names whose ground was worked.
	UE_LOG(LogVEIL, Warning, TEXT("PHI MINE: %02d %s stripped - %s %s on %s ground, +%d gold."),
		Index, *Name, *Ore, *Ore, *Owner, Gold);
}

void AVEILMineNode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bSpent || !GetWorld()) { return; }
	if (GetWorld()->GetTimeSeconds() < RespawnAt) { return; }
	bSpent = false;
	UE_LOG(LogVEIL, Log, TEXT("PHI MINE: %02d %s re-formed."), Index, *Name);
}

'''
if start >= 0 and end > start:
    t = t[:start] + NEW_MINE + t[end:]
    print("cpp: mine built as a headframe with identity")
else:
    print("cpp: MINE BLOCK BOUNDS NOT FOUND", start, end)

# Populate call: Init(m) not Init(m, gold)
t = t.replace("Node->Init(m, 30 + 10 * (m % 4));", "Node->Init(m);", 1)

# placement: keep the golden spiral but say what it is
t = t.replace("\t// MINES: ore seams on the same spiral, closer in, between the camps.",
              "\t// MINES: the world's own claims, on the golden spiral between the camps.\n"
              "\t// The angle and radius are the address; the identity comes from the book.", 1)

wr(p, t)
print("cpp: placement comment + Init(m)")
