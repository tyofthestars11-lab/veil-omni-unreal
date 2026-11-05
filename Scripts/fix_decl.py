"""Remove the leftover tail of the camera declaration my patch left behind."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")

stray = ("\t\t\t}\n"
         "\t\t\t\tAnchor[Idx].Y - FMath::Sin(YawRad) * Back,\n"
         "\t\t\t\tGroundZ + PHI_Constants::Reach(UpPow[Idx]));\n")

if "VEIL SHOT CLAMP" not in s:
    print("MISS clamp block")
elif stray in s:
    s = s.replace(stray, "\t\t\t}\n", 1)
    open(GM, "w", encoding="utf-8", newline="").write(s)
    print("stray declaration tail removed")
else:
    print("no stray tail found")
