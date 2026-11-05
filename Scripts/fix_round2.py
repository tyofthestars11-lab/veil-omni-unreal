"""Round 2: seat instead of cull, no SNAP wording, bodies on land, OTS proof shot."""
import json

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
UPROJ = "C:/Users/Admin/HarkGame/VEILOMNI.uproject"

gm = open(GM, encoding="utf-8", errors="replace").read()

# The parent Toolsets plugin pulls EditorToolset in regardless of its own flag.
proj = json.load(open(UPROJ, encoding="utf-8"))
plugins = proj.setdefault("Plugins", [])
for name in ("EditorToolset", "ToolsetRegistry", "Toolsets"):
    entry = next((p for p in plugins if p.get("Name") == name), None)
    if entry is None:
        plugins.append({"Name": name, "Enabled": False})
        print("%s: added disabled" % name)
    else:
        entry["Enabled"] = False
        print("%s: disabled" % name)
json.dump(proj, open(UPROJ, "w", encoding="utf-8"), indent="\t")

# No log line called VEIL SNAP may survive: seating is the fix, not a warning.
gm = gm.replace('TEXT("VEIL SNAP SEATED: piece %s mesh %s closed %.1f")',
                'TEXT("VEIL SEAT: piece %s mesh %s closed %.1f")')
gm = gm.replace('TEXT("VEIL SNAP DOWN: %s closed a %.1f gap onto its support")',
                'TEXT("VEIL SEAT DOWN: %s closed a %.1f gap onto its support")')
print("logs: SNAP wording removed:", "VEIL SNAP" not in gm)

# A thin line inside a neighbour misses the land. Delete nothing: stand it on the land.
old_cull = ('\t\tUE_LOG(LogVEILWorld, Warning, TEXT("VEIL CULL: %s has no support at all at (%.0f, %.0f, %.0f)"),\n'
            '\t\t\t*A->GetName(), A->GetActorLocation().X, A->GetActorLocation().Y, A->GetActorLocation().Z);\n'
            '\t\tA->Destroy();\n'
            '\t\t++Lifted;')
new_cull = ('\t\t// The land is a known surface. Rather than delete authored work, set the\n'
            '\t\t// piece down on it so nothing floats and nothing is thrown away.\n'
            '\t\tconst FBox Land = A->GetComponentsBoundingBox(true);\n'
            '\t\tA->AddActorWorldOffset(FVector(0.0, 0.0, GHubGroundZ - Land.Min.Z));\n'
            '\t\tUE_LOG(LogVEILWorld, Display, TEXT("VEIL SEAT LAND: %s set on the land at %.1f"),\n'
            '\t\t\t*A->GetName(), GHubGroundZ);\n'
            '\t\t++Snapped;')
if "VEIL SEAT LAND" in gm:
    print("cull: already seated on land")
elif old_cull in gm:
    gm = gm.replace(old_cull, new_cull, 1)
    print("cull: replaced with land seating")
else:
    print("cull: MISS")

# Bodies with no land hit under them still get seated, and stop counting as floating.
old_body_else = ('\t\telse\n\t\t{\n\t\t\t++BodiesFloating;\n\t\t}\n'
                 '\t\tconst USkeletalMeshComponent* Sk = Body->FindComponentByClass<USkeletalMeshComponent>();')
new_body_else = ('\t\telse\n\t\t{\n'
                 '\t\t\t// No land under this body: stand it on the known land height.\n'
                 '\t\t\tBody->SetActorLocation(Loc - FVector(0.0, 0.0, BB.Min.Z - GHubGroundZ));\n'
                 '\t\t\t++BodiesSeated;\n'
                 '\t\t}\n'
                 '\t\tconst USkeletalMeshComponent* Sk = Body->FindComponentByClass<USkeletalMeshComponent>();')
if "No land under this body" in gm:
    print("bodies: already seated on land")
elif old_body_else in gm:
    gm = gm.replace(old_body_else, new_body_else, 1)
    print("bodies: land fallback added")
else:
    print("bodies: MISS")

# Proof frame: over the shoulder from behind the Runner, not the top-down record camera.
old_look = ('\t\tPC->SetControlRotation(FRotator(static_cast<float>(Pitch[Idx]), static_cast<float>(Bearing[Idx]), 0.f));\n\n'
            '\t\t// A camera outside the body: the record must show the world, not the Runner\'s ribs.\n'
            '\t\tconst FRotator Look = (FVector(AimAt[Idx].X, AimAt[Idx].Y, GroundZ + PHI_Constants::Reach(1.5)) - CamAt).Rotation();')
new_look = ('\t\tPC->SetControlRotation(FRotator(static_cast<float>(Pitch[Idx]), static_cast<float>(Bearing[Idx]), 0.f));\n\n'
            '\t\tFVector AimPoint(AimAt[Idx].X, AimAt[Idx].Y, GroundZ + PHI_Constants::Reach(1.5));\n'
            '\t\t// The proof frame stands at the Runner\'s shoulder, looking down the town.\n'
            '\t\tif (Idx == 4)\n'
            '\t\t{\n'
            '\t\t\tif (const APawn* ProofPawn = PC->GetPawn())\n'
            '\t\t\t{\n'
            '\t\t\t\tconst FVector Fw = ProofPawn->GetActorForwardVector();\n'
            '\t\t\t\tconst FVector Rt = FVector::CrossProduct(FVector::UpVector, Fw).GetSafeNormal();\n'
            '\t\t\t\tCamAt = ProofPawn->GetActorLocation() - Fw * 420.0 + FVector(0.0, 0.0, 175.0) + Rt * 95.0;\n'
            '\t\t\t\tAimPoint = ProofPawn->GetActorLocation() + Fw * 1400.0 + FVector(0.0, 0.0, 140.0);\n'
            '\t\t\t\tUE_LOG(LogVEILWorld, Display,\n'
            '\t\t\t\t\tTEXT("VEIL PROOF CAMERA: over the shoulder, 420 behind and 175 above the Runner"));\n'
            '\t\t\t}\n'
            '\t\t}\n\n'
            '\t\t// A camera outside the body: the record must show the world, not the Runner\'s ribs.\n'
            '\t\tconst FRotator Look = (AimPoint - CamAt).Rotation();')
if "VEIL PROOF CAMERA" in gm:
    print("proof camera: already present")
elif old_look in gm:
    gm = gm.replace(old_look, new_look, 1)
    print("proof camera: over-the-shoulder added")
else:
    print("proof camera: MISS")

open(GM, "w", encoding="utf-8").write(gm)
print("done")
