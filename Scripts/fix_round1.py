"""Round 1: body audit, real seating traces, plugin disables, electric-blue VIT."""
import json
import re

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
HUD = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilHud.cpp"
UPROJ = "C:/Users/Admin/HarkGame/VEILOMNI.uproject"

# ---------- 2. experimental toolsets that throw on startup ----------
proj = json.load(open(UPROJ, encoding="utf-8"))
plugins = proj.setdefault("Plugins", [])
for name in ("EditorToolset", "ToolsetRegistry"):
    entry = next((p for p in plugins if p.get("Name") == name), None)
    if entry is None:
        plugins.append({"Name": name, "Enabled": False})
        print("%s: added disabled" % name)
    else:
        entry["Enabled"] = False
        print("%s: disabled" % name)
fab = next((p for p in plugins if p.get("Name") == "Fab"), None)
if fab is None:
    plugins.append({"Name": "Fab", "Enabled": False})
    print("Fab: added disabled")
else:
    fab["Enabled"] = False
    print("Fab: disabled")
json.dump(proj, open(UPROJ, "w", encoding="utf-8"), indent="\t")

# ---------- 4. VIT is electric blue, no red anywhere ----------
hud = open(HUD, encoding="utf-8", errors="replace").read()
hud = hud.replace(
    "\tconst FLinearColor VitRed(0.74f, 0.13f, 0.12f, 0.92f);",
    "\tconst FLinearColor VitBlue(0.247f, 0.663f, 1.0f, 1.0f);   // #3FA9FF, no red in VEIL")
hud = hud.replace("PC->VIT / FMath::Max(1.f, PC->VITMax), VitRed)",
                  "PC->VIT / FMath::Max(1.f, PC->VITMax), VitBlue)")
assert "VitRed" not in hud, "red still referenced"
open(HUD, "w", encoding="utf-8").write(hud)
print("HUD: VIT #3FA9FF, VitRed gone")

# ---------- 1. snap audit: bodies are skeletal, gaps need a long trace ----------
gm = open(GM, encoding="utf-8", errors="replace").read()

if '#include "Components/SkeletalMeshComponent.h"' not in gm:
    gm = gm.replace('#include "Components/SkyAtmosphereComponent.h"',
                    '#include "Components/SkyAtmosphereComponent.h"\n'
                    '#include "Components/SkeletalMeshComponent.h"', 1)
    print("include: skeletal mesh added")

# 17 cm reach can never find a 160 cm support.
old_trace = ("\t\tconst bool bFound = GetWorld()->LineTraceSingleByChannel(\n"
             "\t\t\tHit, Bottom, Bottom - FVector(0.0, 0.0, PHI_Constants::Reach(6)), ECC_Visibility, Q);")
new_trace = ("\t\tconst bool bFound = GetWorld()->LineTraceSingleByChannel(\n"
             "\t\t\tHit, Bottom, Bottom - FVector(0.0, 0.0, 20000.0), ECC_Visibility, Q);")
if new_trace in gm:
    print("trace: already long")
elif old_trace in gm:
    gm = gm.replace(old_trace, new_trace, 1)
    print("trace: reach(6) -> 20000")
else:
    print("trace: MISS")

# Bodies are skeletal: the static-mesh audit cannot describe them and must not drop them.
old_loop = ("\t\tif (!A)\n\t\t{\n\t\t\tcontinue;\n\t\t}\n"
            "\t\tconst FBox Bounds = A->GetComponentsBoundingBox(true);")
new_loop = ("\t\tif (!A || Cast<AVeilNpc>(A))\n\t\t{\n\t\t\tcontinue;          // bodies are audited below, with their skeletal meshes\n\t\t}\n"
            "\t\tconst FBox Bounds = A->GetComponentsBoundingBox(true);")
if "// bodies are audited below" in gm:
    print("loop: already skips npcs")
elif old_loop in gm:
    gm = gm.replace(old_loop, new_loop, 1)
    print("loop: NPCs excluded from static audit")
else:
    print("loop: MISS")

# Seating a piece is the fix, not a warning: keep Warning for real failures only.
old_w1 = '\t\t\t\tUE_LOG(LogVEILWorld, Warning, TEXT("VEIL SNAP: piece %s mesh %s gap %.1f"),'
new_w1 = '\t\t\t\tUE_LOG(LogVEILWorld, Display, TEXT("VEIL SNAP SEATED: piece %s mesh %s closed %.1f"),'
if new_w1 in gm:
    print("log1: already display")
elif old_w1 in gm:
    gm = gm.replace(old_w1, new_w1, 1)
    print("log1: seated, display")
else:
    print("log1: MISS")

old_w2 = '\t\t\tUE_LOG(LogVEILWorld, Warning, TEXT("VEIL SNAP DOWN: %s closed a %.1f gap onto its support"),'
new_w2 = '\t\t\tUE_LOG(LogVEILWorld, Display, TEXT("VEIL SNAP DOWN: %s closed a %.1f gap onto its support"),'
if new_w2 in gm:
    print("log2: already display")
elif old_w2 in gm:
    gm = gm.replace(old_w2, new_w2, 1)
    print("log2: display")
else:
    print("log2: MISS")

body_audit = '''	int32 BodiesSeated = 0;
	int32 BodiesFloating = 0;
	int32 BodiesMeshed = 0;
	for (TActorIterator<AVeilNpc> BodyIt(GetWorld()); BodyIt; ++BodyIt)
	{
		AVeilNpc* Body = *BodyIt;
		if (!Body)
		{
			continue;
		}
		const FVector Loc = Body->GetActorLocation();
		const FBox BB = Body->GetComponentsBoundingBox(true);
		FHitResult BodyHit;
		FCollisionQueryParams BQ(SCENE_QUERY_STAT(VeilBodySeat), false, Body);
		const bool bGround = GetWorld()->LineTraceSingleByChannel(
			BodyHit, Loc + FVector(0.0, 0.0, 200.0), Loc - FVector(0.0, 0.0, 20000.0), ECC_Visibility, BQ);
		if (bGround)
		{
			// Seat the soles, not the origin: bounds bottom goes onto the impact.
			Body->SetActorLocation(Loc - FVector(0.0, 0.0, BB.Min.Z - BodyHit.ImpactPoint.Z));
			++BodiesSeated;
		}
		else
		{
			++BodiesFloating;
		}
		const USkeletalMeshComponent* Sk = Body->FindComponentByClass<USkeletalMeshComponent>();
		if (Sk && Sk->GetSkeletalMeshAsset())
		{
			++BodiesMeshed;
		}
	}
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL BODY AUDIT: %d bodies traced to ground, %d floating, %d with real skeletal meshes"),
		BodiesSeated, BodiesFloating, BodiesMeshed);

'''

anchor = ('\tUE_LOG(LogVEILWorld, Display,\n'
          '\t\tTEXT("VEIL WORLD AUDIT: %d unsupported piece(s) removed, %d snapped down to support, deck at %.1f"),\n'
          '\t\tLifted, Snapped, GPlazaZ);')
if "VEIL BODY AUDIT" in gm:
    print("body audit: already present")
elif anchor in gm:
    gm = gm.replace(anchor, body_audit + anchor, 1)
    print("body audit: inserted")
else:
    print("body audit: MISS")

open(GM, "w", encoding="utf-8").write(gm)
print("done")
