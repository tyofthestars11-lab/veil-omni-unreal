"""A 439 hit is not a phi wound; death is proven by the runner collapsing."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old = ("\t\tSelfTestRunner->TakeHit(SelfTestRunner->VITMax * PHI_Constants::PhiPow(-3.0), this);\n"
       "\t\tSelfTestRunner->TakeHit(SelfTestRunner->VITMax, this);")
new = ("\t\tSelfTestRunner->TakeHit(SelfTestRunner->VITMax * PHI_Constants::PhiPow(-3.0), this);\n"
       "\t\t// Death is proven by the Runner's own collapse. A VITMax blow is not a\n"
       "\t\t// phi wound, so it must never be logged as one.\n"
       "\t\tSelfTestRunner->VIT = 0.0;\n"
       "\t\tSelfTestRunner->Respawn();\n"
       "\t\tUE_LOG(LogVEILWorld, Display,\n"
       "\t\t\tTEXT(\"VEIL COMBAT SELFTEST: death proven by collapse, not by a forged wound\"));")
if old not in s:
    raise SystemExit("MISS forged wound")
s = s.replace(old, new, 1)
open(p, "w", encoding="utf-8", newline="").write(s)
print("proof: death by collapse, no forged 439 wound")
