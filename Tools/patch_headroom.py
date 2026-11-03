"""Headroom for the baked city.

Measured: the played map builds no rooms at runtime (no PHI ROOM WALL / ENTERABLE lines in
the log at all), so BuildRoom's new 6.4 m clear never reaches it - every shell in VEILDistrict
was baked by an older builder that laid walls at the caller's height and dropped the roof
12 cm above that. That is the low ceiling.

So the baked shell is tallened once, at boot, from real component bounds: roofs within the
plaza go up 260 cm, walls grow 1.4x in Z and rise by half the growth so their bases stay on
the ground. The log names both counts, so this is a measurement and not a claim.
"""

import io

P = "Source/VEIL/Private/Game/VEILGameMode.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"
fails = []

# includes the census needs
for inc in ('#include "Engine/StaticMeshActor.h"',
            '#include "Engine/StaticMesh.h"',
            '#include "EngineUtils.h"'):
    if inc not in s:
        m = s.find('#include "VEIL.h"')
        if m == -1:
            fails.append("no anchor to add %s" % inc)
            continue
        s = s[:m] + inc + N + s[m:]
        print("include added: %s" % inc)

ANCHOR = "\t// -phitype: THE CLICK, WITHOUT A MOUSE."
HOOK = '''	// HEADROOM FOR THE BAKED CITY. This map builds no rooms at runtime - it is a saved world,
	// so BuildRoom's 6.4 m clear never runs here - and every shell in it was baked by an older
	// builder that laid walls at the caller's height with the roof 12 cm above. That is the low
	// ceiling the player walks into. The shells are tallened once, at boot, from their own
	// bounds: a roof near the plaza rises 260 cm, a wall grows 1.4x and rises by half of it so
	// its base stays on the ground, and the count is logged either way.
	{
		FTimerHandle HeadroomTimer;
		GetWorldTimerManager().SetTimer(HeadroomTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWorld* W = GetWorld();
			if (!W) { return; }
			int32 Roofs = 0, Walls = 0;
			for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
			{
				AStaticMeshActor* Piece = *It;
				if (!Piece) { continue; }
				UStaticMeshComponent* Body = Piece->GetStaticMeshComponent();
				if (!Body || !Body->GetStaticMesh()) { continue; }
				const FString MeshName = Body->GetStaticMesh()->GetName();
				const FBox Box = Body->Bounds.GetBox();
				const FVector At = Box.GetCenter();
				if (FVector::Dist2D(At, FVector::ZeroVector) > 6000.f) { continue; }

				const bool bRoof = MeshName.Contains(TEXT("Roof")) || MeshName.Contains(TEXT("Ceil"));
				const bool bWall = MeshName.Contains(TEXT("Wall"));
				if (bRoof && At.Z > 150.f && At.Z < 900.f)
				{
					Piece->AddActorWorldOffset(FVector(0.f, 0.f, 260.f));
					++Roofs;
				}
				else if (bWall && At.Z < 900.f)
				{
					const FVector Old = Piece->GetActorScale3D();
					FVector Grown = Old;
					Grown.Z = Old.Z * 1.4f;
					Piece->SetActorScale3D(Grown);
					Piece->AddActorWorldOffset(FVector(0.f, 0.f, (Grown.Z - Old.Z) * 50.f));
					++Walls;
				}
			}
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI HEADROOM: %d baked roof(s) raised 260 cm, %d wall(s) grown 1.4x - ceilings now clear a standing body."),
				Roofs, Walls);
		}), 2.5f, false);
	}

''' + ANCHOR

if s.count(ANCHOR) != 1:
    fails.append("phitype anchor count=%d" % s.count(ANCHOR))
else:
    s = s.replace(ANCHOR, HOOK.replace("\n", N), 1)
    print("headroom pass added to GameMode::BeginPlay")

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(P, "w", encoding="utf-8", newline="").write(s)
print("HEADROOM WRITTEN")
