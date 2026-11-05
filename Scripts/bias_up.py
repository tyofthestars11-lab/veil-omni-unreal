"""Sun wattage stopped reaching the shot; the bias provably does. Two stops up."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old = "\tS.AutoExposureBias = 0.0f;"
new = "\tS.AutoExposureBias = 2.0f;"

if new in s:
    print("already +2")
elif old in s:
    s = s.replace(old, new, 1)
    open(GM, "w", encoding="utf-8", newline="").write(s)
    print("bias +2 stops")
else:
    print("MISS bias line")
