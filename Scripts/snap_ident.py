"""A snap that cannot name its mesh cannot be fixed."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")
L = s.split("\n")

idx = None
for i, line in enumerate(L):
    if "VEIL SNAP DOWN" in line:
        idx = i
        break

if idx is None:
    print("no snap log found; nothing to name")
    raise SystemExit(0)

print("=== snap log context ===")
for j in range(max(0, idx - 8), min(len(L), idx + 4)):
    print(f"  {j + 1}: {L[j]}")

# which function encloses it?
enclosing = ""
for j in range(idx, -1, -1):
    if "::" in L[j] and "(" in L[j] and L[j].rstrip().endswith(")"):
        enclosing = L[j].strip()
        break
print(f"\n  enclosing: {enclosing}")

if "FitPart" not in enclosing:
    print("  not inside FitPart: not patching (needs a different variable)")
    raise SystemExit(0)

indent = L[idx][: len(L[idx]) - len(L[idx].lstrip())]
if "VEIL SNAP: piece mesh" in s:
    print("  already named")
    raise SystemExit(0)

L.insert(idx, f'{indent}UE_LOG(LogVEILWorld, Warning, TEXT("VEIL SNAP: piece mesh %s"), MeshPath);')
open(p, "w", encoding="utf-8", newline="").write("\n".join(L))
print("  patched: each snap now names its mesh")
