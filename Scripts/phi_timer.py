"""The proof timer must be phi-derived like everything else."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")
old = "&AVeilGameMode::AiTestReport, 7.0f, false"
new = "&AVeilGameMode::AiTestReport, static_cast<float>(PHI_Constants::PhiPow(4.045)), false"
if old in s:
    s = s.replace(old, new)
    open(p, "w", encoding="utf-8", newline="").write(s)
    print("proof timer converted to PhiPow(4.045) = 7.0s")
else:
    print("already converted")
