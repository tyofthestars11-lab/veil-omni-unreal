"""Damage and cost are fractions of capacity, and the self-test proves death too."""

FILES = {
    "VeilNpc.cpp": [
        ("Target->TakeStrike(PHI_Constants::Reach(1.2));",
         "Target->TakeStrike(439.0 * PHI_Constants::PhiPow(-1.55));"),
        ("R->TakeHit(PHI_Constants::Reach(2.0), this);",
         "R->TakeHit(R->VITMax * PHI_Constants::PhiPow(-3.0), this);"),
    ],
    "VeilNpc.h": [
        ("double Health = 100.0;", "double Health = 439.0;"),
        ("double HealthMax = 100.0;", "double HealthMax = 439.0;"),
    ],
    "VeilRunner.cpp": [
        ("if (!Mito.Spend(static_cast<float>(PHI_Constants::Reach(1.0))))",
         "if (!Mito.Spend(static_cast<float>(Mito.MaxATP * PHI_Constants::PhiPow(-2))))"),
    ],
    "VeilGameMode.cpp": [
        ("SelfTestRunner->TakeHit(PHI_Constants::Reach(2.0), this);",
         "SelfTestRunner->TakeHit(SelfTestRunner->VITMax * PHI_Constants::PhiPow(-3.0), this);\n"
         "\t\t\tSelfTestRunner->TakeHit(SelfTestRunner->VITMax, this);"),
    ],
}

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
for name, pairs in FILES.items():
    p = P + name
    s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")
    before = s
    for old, new in pairs:
        if s.count(old) == 0:
            print("   MISS", name, "::", old[:60])
        s = s.replace(old, new)
    if s != before:
        open(p, "w", encoding="utf-8", newline="").write(s)
    print("  patched", name, s != before)
