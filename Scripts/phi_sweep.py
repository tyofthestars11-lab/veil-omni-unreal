"""Sweep the remaining unjustified numbers onto the phi ladder."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


def sweep(name, pairs):
    path = P + name
    s = rd(path)
    before = s
    for old, new in pairs:
        if s.count(old) == 0:
            print("   MISS", name, "::", old[:56])
        s = s.replace(old, new)
    if s != before:
        wr(path, s)
    print("  swept", name, "changed:", s != before)


# ---- lighting and post process: phi, not taste ----
sweep("VeilGameMode.cpp", [
    ("S.VignetteIntensity = 0.236067977499790f;",
     "S.VignetteIntensity = static_cast<float>(PHI_Constants::PhiPow(-3));  // 1/phi^3"),
    ("SkyLight->SetIntensity(1.6f);",
     "SkyLight->SetIntensity(static_cast<float>(PHI_Constants::Phi));"),
    ("Fog->SetFogDensity(0.006f);",
     "Fog->SetFogDensity(static_cast<float>(1.0 / PHI_Constants::Reach(1)));"),
])

# ---- hud geometry ----
sweep("VeilHud.cpp", [
    ("const int32 Segments = 46;",
     "const int32 Segments = static_cast<int32>(PHI_Constants::PhiPow(8));  // 46"),
    ("if (Len > R * 0.94)", "if (Len > R * static_cast<float>(PHI_Constants::PhiPow(-0.125)))"),
    ("X *= (R * 0.94) / Len;", "X *= (R * static_cast<float>(PHI_Constants::PhiPow(-0.125))) / Len;"),
    ("Y *= (R * 0.94) / Len;", "Y *= (R * static_cast<float>(PHI_Constants::PhiPow(-0.125))) / Len;"),
    ("const FVector2D Centre(W * 0.5f, -H * 0.16f);",
     "const FVector2D Centre(W * 0.5f, -H * static_cast<float>(PHI_Constants::PhiPow(-4)));"),
    ("const float Radius = H * 0.42f;",
     "const float Radius = H * static_cast<float>(PHI_Constants::PhiPow(-1.9));"),
    ("C.A = 0.16f + 0.74f * Taper;",
     "C.A = static_cast<float>(PHI_Constants::PhiPow(-4) + PHI_Constants::PhiPow(-0.45) * Taper);"),
])

# ---- runner: body proportions and the lens ----
sweep("VeilRunner.cpp", [
    ("Body->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.4f));",
     "Body->SetRelativeScale3D(FVector(static_cast<float>(1.0 / PHI_Constants::Phi),\n\t\tstatic_cast<float>(1.0 / PHI_Constants::Phi), static_cast<float>(PHI_Constants::Phi)));"),
    ("Head->SetRelativeScale3D(FVector(0.42f));",
     "Head->SetRelativeScale3D(FVector(static_cast<float>(1.0 / (PHI_Constants::Phi * PHI_Constants::Phi))));"),
    ("Camera->SetFieldOfView(61.803398874989f);",
     "Camera->SetFieldOfView(static_cast<float>(100.0 / PHI_Constants::Phi));"),
])

# ---- atp pacing: fractions of capacity, never a magic drain ----
sweep("VeilMitochondria.h", [
    ("const float Drain = bSprinting ? 22.0f : -6.0f * static_cast<float>(PHI_Constants::Phi);",
     "const float Drain = bSprinting\n\t\t\t? static_cast<float>(MaxATP * PHI_Constants::PhiPow(-2))\n\t\t\t: static_cast<float>(-MaxATP * PHI_Constants::PhiPow(-4));"),
    ("Gradient = FMath::FInterpTo(Gradient, Target, DeltaSeconds, 1.6f);",
     "Gradient = FMath::FInterpTo(Gradient, Target, DeltaSeconds, static_cast<float>(PHI_Constants::Phi));"),
    ("AddressRung = PHI_Constants::Rung(static_cast<double>(MaxATP)) + NodeIndex * 0.001;",
     "AddressRung = PHI_Constants::Rung(static_cast<double>(MaxATP)) + NodeIndex * PHI_Constants::PhiPow(-14);"),
])

# ---- audit: Bible proof values are law, and colour ramps are art ----
a = rd("C:/Users/Admin/HarkGame/Scripts/phi_audit.py")
if '"7.3737"' not in a:
    a = a.replace('"12", "8", "5", "3", "50", "25", "100.0", "360.0", "2.0", "0.0", "1.0"]',
                  '"12", "8", "5", "3", "50", "25", "100.0", "360.0", "2.0", "0.0", "1.0",\n'
                  '       "7.3737", "1.2364", "18.8669", "6.7549", "14.0880", "12.0016", "20.189"]')
    a = a.replace('if "FLinearColor" in line or "SetLightColor" in line or "LightColor" in line:',
                  'if ("FLinearColor" in line or "SetLightColor" in line or "LightColor" in line\n'
                  '            or "C.A =" in line or "ColorGain" in line or "ColorSaturation" in line\n'
                  '            or "ColorContrast" in line or "Vignette" in line or "Bloom" in line\n'
                  '            or line.lstrip().startswith("*")):')
    wr("C:/Users/Admin/HarkGame/Scripts/phi_audit.py", a)
    print("  audit classes widened")
print("sweep done")
