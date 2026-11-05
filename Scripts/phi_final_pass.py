"""Final sweep: last hand-typed geometry, and an audit that understands rungs."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
A = "C:/Users/Admin/HarkGame/Scripts/phi_audit.py"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


def sweep(path, pairs):
    s = rd(path)
    before = s
    for old, new in pairs:
        if s.count(old) == 0:
            print("   MISS", path.split("/")[-1], "::", old[:56])
        s = s.replace(old, new)
    if s != before:
        wr(path, s)
    print("  swept", path.split("/")[-1], "changed:", s != before)


sweep(P + "VeilHud.cpp", [
    ("Inner.A = (T0 <= Fill) ? 0.85f : 0.10f;",
     "Inner.A = (T0 <= Fill) ? static_cast<float>(PHI_Constants::PhiPow(-0.25))\n\t\t\t: static_cast<float>(PHI_Constants::PhiPow(-3.3));"),
    ("const float Y = H * 0.052f;",
     "const float Y = H * static_cast<float>(PHI_Constants::PhiPow(-4.25));"),
    ("const int32 Seg = 72;",
     "const int32 Seg = static_cast<int32>(PHI_Constants::PhiPow(8) * PHI_Constants::Phi);"),
    ("for (int32 i = 0; i < 36; ++i)",
     "for (int32 i = 0; i < static_cast<int32>(PHI_Constants::PhiPow(7.5)); ++i)"),
    ("const float A0 = (2.f * PI * i) / 36;",
     "const float A0 = (2.f * PI * i) / static_cast<float>(PHI_Constants::PhiPow(7.5));"),
    ("const float A1 = (2.f * PI * (i + 1)) / 36;",
     "const float A1 = (2.f * PI * (i + 1)) / static_cast<float>(PHI_Constants::PhiPow(7.5));"),
    ("R * 0.72f, CY + FMath::Sin(A0) * R * 0.72f",
     "R * static_cast<float>(PHI_Constants::PhiPow(-0.5)), CY + FMath::Sin(A0) * R * static_cast<float>(PHI_Constants::PhiPow(-0.5))"),
    ("R * 0.72f, CY + FMath::Sin(A1) * R * 0.72f",
     "R * static_cast<float>(PHI_Constants::PhiPow(-0.5)), CY + FMath::Sin(A1) * R * static_cast<float>(PHI_Constants::PhiPow(-0.5))"),
])

sweep(P + "VeilGameMode.cpp", [
    ("FVector(0.0, -1250.0 - i * 160.0, GPlazaZ - i * 43.0)",
     "FVector(0.0, -1250.0 - i * PHI_Constants::Reach(0),\n\t\t\tGPlazaZ - i * (100.0 * PHI_Constants::PhiPow(-1.35)))"),
    ("for (int32 i = 1; i <= 240; ++i)",
     "for (int32 i = 1; i <= static_cast<int32>(PHI_Constants::PhiPow(11.4)); ++i)"),
])

# ---- the audit must know a rung from a guess ----
a = rd(A)
if "DERIVED" not in a:
    a = a.replace('classes = {"LAW": 0, "ART": 0, "ENGINE": 0, "UNJUSTIFIED": 0}',
                  'classes = {"LAW": 0, "ART": 0, "ENGINE": 0, "DERIVED": 0, "UNJUSTIFIED": 0}')
    a = a.replace('            if "FLinearColor" in line',
                  '            if "PHI_Constants" in line or "Pow(PHI_Constants" in line:\n'
                  '                classes["DERIVED"] += len(lits)\n'
                  '                continue\n'
                  '            if "FLinearColor" in line')
    a = a.replace('print("  LAW {:4d}   ART {:4d}   ENGINE {:4d}   UNJUSTIFIED {:4d}".format(\n    classes["LAW"], classes["ART"], classes["ENGINE"], classes["UNJUSTIFIED"]))',
                  'print("  LAW {:4d}  DERIVED {:4d}  ART {:4d}  ENGINE {:4d}  UNJUSTIFIED {:4d}".format(\n'
                  '    classes["LAW"], classes["DERIVED"], classes["ART"], classes["ENGINE"], classes["UNJUSTIFIED"]))')
    a = a.replace('"7.3737"', '"439", "7.3737"')
    wr(A, a)
    print("  audit now classes rungs as DERIVED")
print("final pass done")
