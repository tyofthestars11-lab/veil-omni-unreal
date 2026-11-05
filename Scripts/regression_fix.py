"""Fix the two regressions the boot gate caught, and make the cull self-reporting."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
S = "C:/Users/Admin/HarkGame/Scripts/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# ---- 1. the Bible proof values are law, not rungs: restore them exactly ----
h = rd(P + "PHI_Constants.h")
if "ProofSpacing" not in h:
    h = h.replace("\tstatic constexpr double GoldenPitchDeg = 17.0324;",
                  "\tstatic constexpr double GoldenPitchDeg = 17.0324;\n"
                  "\t// Bible law: the proof spacing. GoldenLoc is proved at exactly this value.\n"
                  "\tstatic constexpr double ProofSpacing = 10.0;")
    wr(P + "PHI_Constants.h", h)
    print("header: ProofSpacing named")

c = rd(P + "PHI_Constants.cpp")
before = c
c = c.replace("const double ExpectedX[3] = { -PHI_Constants::Reach(-6.418), 1.2364, 18.8669 };",
              "const double ExpectedX[3] = { -7.3737, 1.2364, 18.8669 };")
c = c.replace("GoldenLoc(Indices[k], PHI_Constants::Reach(-5.785))",
              "GoldenLoc(Indices[k], PHI_Constants::ProofSpacing)")
if c != before:
    wr(P + "PHI_Constants.cpp", c)
print("proof spacing restored exactly:", c != before)

# ---- 2. counts must round, not truncate (this is what dropped 66 to 65) ----
for name in ("VeilGameMode.cpp", "VeilHud.cpp", "VeilWater.cpp"):
    p = P + name
    s = rd(p)
    n = s.count("static_cast<int32>(PHI_Constants::PhiPow")
    s = s.replace("static_cast<int32>(PHI_Constants::PhiPow", "FMath::RoundToInt(PHI_Constants::PhiPow")
    if n:
        wr(p, s)
    print("  rounded {} counts in {}".format(n, name))

# ---- 3. the audit must name what it culls ----
g = rd(P + "VeilGameMode.cpp")
old = """			&& A->GetActorLocation().Z > GHubGroundZ + 900.0)
		{
			A->Destroy();"""
new = """			&& A->GetActorLocation().Z > GHubGroundZ + 900.0)
		{
			UE_LOG(LogVEILWorld, Warning, TEXT("VEIL CULL: %s is hanging at (%.0f, %.0f, %.0f)"),
				*A->GetName(), A->GetActorLocation().X, A->GetActorLocation().Y, A->GetActorLocation().Z);
			A->Destroy();"""
if "VEIL CULL" not in g:
    assert g.count(old) == 1, "cull block anchor"
    g = g.replace(old, new)
    wr(P + "VeilGameMode.cpp", g)
print("cull self-reporting:", "VEIL CULL" in g)

# ---- 4. the audit keeps knowing law from guess ----
for name in ("phi_report.py", "phi_audit.py"):
    p = S + name
    s = rd(p)
    before = s
    s = s.replace("7.3737, 1.2364, 18.8669, 6.7549, -14.088, -12.0016,",
                  "7.3737, -7.3737, 1.2364, 18.8669, 6.7549, -14.088, -12.0016, 10.0,")
    s = s.replace('"7.3737", "1.2364", "18.8669", "6.7549", "14.0880", "12.0016", "20.189"]',
                  '"7.3737", "-7.3737", "1.2364", "18.8669", "6.7549", "14.0880", "12.0016", "20.189", "10.0"]')
    if s != before:
        wr(p, s)
    print("  audit law list updated:", name, s != before)
print("done")
