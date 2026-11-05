"""Histogram auto-exposure normalises every lighting change away. Lock it."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old = "\tS.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;"
new = "\tS.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;"

if new in s:
    print("already manual")
elif old in s:
    s = s.replace(old, new, 1)
    open(GM, "w", encoding="utf-8", newline="").write(s)
    print("exposure locked to manual")
else:
    print("MISS exposure method")
