"""Delete the duplicated camera-declaration tail by line position.

The pattern attempt failed because the stray lines do not carry the exact
leading tabs the pattern expected. Position and content decide it here.
"""
import io

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
with io.open(GM, encoding="utf-8", newline="") as f:
    raw = f.read()

crlf = "\r\n" in raw
text = raw.replace("\r\n", "\n")
lines = text.split("\n")

decl = "Anchor[Idx].Y - FMath::Sin(YawRad) * Back,"
idx = [i for i, ln in enumerate(lines) if ln.strip() == decl]
print("declaration tails found at lines:", [i + 1 for i in idx])

if len(idx) < 2:
    print("nothing duplicated, no change")
else:
    drop = set()
    for i in idx[1:]:
        drop.add(i)
        if i + 1 < len(lines) and "GroundZ + PHI_Constants::Reach(UpPow[Idx]));" in lines[i + 1]:
            drop.add(i + 1)
    kept = [ln for j, ln in enumerate(lines) if j not in drop]
    print("dropped lines:", sorted(x + 1 for x in drop))
    out = "\n".join(kept)
    if crlf:
        out = out.replace("\n", "\r\n")
    with io.open(GM, "w", encoding="utf-8", newline="") as f:
        f.write(out)
    print("rewritten")
