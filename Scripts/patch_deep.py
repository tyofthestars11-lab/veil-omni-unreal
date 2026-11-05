import re

GP = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
NC = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilNpc.cpp"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


s = rd(GP)

# 1. globals: a snap gate and the deck height
if "GSnapEnabled" not in s:
    a = "\tint32 GSnapped = 0;"
    assert s.count(a) == 1, "GSnapped anchor"
    s = s.replace(a, a + "\n\tbool GSnapEnabled = true;\n\tdouble GPlazaZ = 0.0;")
    print("globals: GSnapEnabled, GPlazaZ")

# 2. the snap gate itself
old = "\tif (Loc.Z >= GHubGroundZ - 1.0)"
if old in s:
    s = s.replace(old, "\tif (GSnapEnabled && Loc.Z >= GHubGroundZ - 1.0)")
    print("snap gate: hub pieces keep the deck height")

# 3. the ring stands on the deck
br = "FVector(Centre.X + P.X, Centre.Y + P.Y, GHubGroundZ)"
if br in s:
    s = s.replace(br, "FVector(Centre.X + P.X, Centre.Y + P.Y, GPlazaZ)")
    print("ring stands on the deck")

# 4. the terrace: one flat stone plaza, laid on the real ground
hub = "void AVeilGameMode::BuildHub()\n{\n"
i = s.find(hub)
assert i >= 0, "BuildHub"
if "GPlazaZ = GHubGroundZ + 260.0" not in s:
    pre = (
        "void AVeilGameMode::BuildHub()\n{\n"
        "\t// The Bible Hub is a cliffside terrace. One flat stone deck is laid on the\n"
        "\t// real ground, and the whole market stands on that deck, never on a peak.\n"
        "\tGPlazaZ = GHubGroundZ + 260.0;\n"
        "\tGSnapEnabled = false;\n"
        "\tFitPart(MeshFloor, FVector(0.0, 0.0, GHubGroundZ), 2600.0, 260.0, "
        "FRotator::ZeroRotator, MatStone, true);\n"
    )
    s = s[:i] + pre + s[i + len(hub):]
    print("terrace deck laid: 26 m stone plaza")

s = s.replace("GHubGroundZ), 90.0, FMath::Min(H, 420.0),",
              "GPlazaZ), 90.0, FMath::Min(H, 420.0),")

# 5. snap is restored after the hub, so the regions still read the real land
hub_log = 'UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD HUB:'
if hub_log in s and "\tGSnapEnabled = true;" not in s:
    s = s.replace(hub_log, "\tGSnapEnabled = true;\n\tGPlazaZ = GHubGroundZ;\n" + hub_log, 1)
    print("snap restored after the hub")

# 6. the eye stands on the deck, inside the plaza edge
s = s.replace("GHubShift.Y - 3600.0, GHubGroundZ + 320.0",
              "GHubShift.Y - 1150.0, GHubGroundZ + 435.0")
s = s.replace("PC->SetControlRotation(FRotator(-7.f, 90.f, 0.f));",
              "PC->SetControlRotation(FRotator(-3.f, 90.f, 0.f));")
s = s.replace("CamY[5] = { -3600.0,", "CamY[5] = { -1150.0,")
s = s.replace("CamZ[5] = { 320.0,", "CamZ[5] = { 435.0,")
s = s.replace("CamPitch[5] = { -7.f,", "CamPitch[5] = { -3.f,")

# 7. the float audit only judges the plaza, where a hang is a bug
old_audit = "if (A && A->GetActorLocation().Z > GHubGroundZ + 1200.0)"
if old_audit in s:
    s = s.replace(
        old_audit,
        "if (A && FVector::Dist2D(A->GetActorLocation(), "
        "FVector(GHubShift.X, GHubShift.Y)) < 1600.0 && "
        "A->GetActorLocation().Z > GHubGroundZ + 900.0)")
    print("audit scoped to the plaza")

# 8. give the minds time to think before the first frame is taken
s = s.replace("&AVeilGameMode::TakeShot, 9.f, false)",
              "&AVeilGameMode::TakeShot, 16.f, false)")

wr(GP, s)

# 9. a role per people, so no two peoples behave alike
t = rd(NC)
anchor = "    Wait = 0.5f + 5.5f * Phase;"
if "case 1: Speed" not in t:
    assert t.count(anchor) == 1, "phase anchor"
    roles = (
        "    Wait = 0.5f + 5.5f * Phase;\n\n"
        "    // A role per people. Gold walk in slow procession, crystal drift wide,\n"
        "    // Deep patrol fast, the Pale Host hunts.\n"
        "    switch (RaceIndex)\n"
        "    {\n"
        "    case 1: Speed *= 0.55; Radius = 300.0; break;\n"
        "    case 2: Speed *= 0.85; Radius = 950.0; break;\n"
        "    case 3: Speed *= 1.45; Radius = 520.0; break;\n"
        "    case 4: Speed = 320.0; Radius = 150.0; break;\n"
        "    default: break;\n"
        "    }"
    )
    t = t.replace(anchor, roles)
    wr(NC, t)
    print("roles per people")

print("patch complete")
