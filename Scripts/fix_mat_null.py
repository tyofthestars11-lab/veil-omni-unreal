"""Mat() must survive a null path: FitPart passes one for the Spire plinth."""

path = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n")

sig = "UMaterialInterface* AVeilGameMode::Mat(const TCHAR* Path)\n{\n"
if "if (!Path)" in s:
    print("guard already present")
else:
    if s.count(sig) != 1:
        raise SystemExit("ANCHOR MISS: Mat signature")
    s = s.replace(sig, sig + "\tif (!Path)\n\t{\n\t\treturn nullptr;\n\t}\n", 1)
    open(path, "w", encoding="utf-8", newline="").write(s)
    print("null guard added to Mat()")
