"""Mat() must reject a null path before it dereferences it."""

path = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n")

COMMENT = "\t// Real photoscanned CC0 art replaces the flat palette wherever it exists, so the"
GUARD = "\tif (!Path)\n\t{\n\t\treturn nullptr;\n\t}\n"

first_brace = "UMaterialInterface* AVeilGameMode::Mat(const TCHAR* Path)\n{\n"

if s.count(first_brace + GUARD) == 1 or s.count(first_brace + "\tif (!Path)") == 1:
    print("guard already first")
else:
    if s.count(first_brace) != 1:
        raise SystemExit("ANCHOR MISS: Mat opening")
    if s.count(COMMENT) != 1:
        raise SystemExit("ANCHOR MISS: CC0 comment")
    # insert immediately after the opening brace, before the comment
    s = s.replace(first_brace, first_brace + GUARD, 1)
    # drop the redundant later guard so the function reads straight
    late = "\n\tif (!Path)\n\t{\n\t\treturn nullptr;\n\t}\n\treturn LoadObject<UMaterialInterface>(nullptr, Path);"
    if late in s:
        s = s.replace(late, "\n\treturn LoadObject<UMaterialInterface>(nullptr, Path);", 1)
    open(path, "w", encoding="utf-8", newline="").write(s)
    print("guard moved to the top of Mat()")
