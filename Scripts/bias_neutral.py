"""Manual exposure at -(phi+1/phi) is two stops under. Neutral is the start point."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old = "\tS.AutoExposureBias = static_cast<float>(-(PHI_Constants::Phi + PHI_Constants::PhiPow(-1.0)));"
new = "\tS.AutoExposureBias = 0.0f;"

if new in s:
    print("already neutral")
elif old in s:
    s = s.replace(old, new, 1)
    open(GM, "w", encoding="utf-8", newline="").write(s)
    print("bias neutral (0 stops)")
else:
    print("MISS bias line")
