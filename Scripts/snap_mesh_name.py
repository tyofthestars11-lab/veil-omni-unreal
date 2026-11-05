"""The audit pass knows the actor; make it say which mesh drifted."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

if "VEIL SNAP: piece" in s:
    print("already naming meshes")
    raise SystemExit(0)

L = s.split("\n")
for i, line in enumerate(L):
    if "VEIL SNAP DOWN" in line:
        indent = line[: len(line) - len(line.lstrip())]
        insert = [
            f"{indent}{{",
            f"{indent}\tUStaticMeshComponent* SMC = Cast<AStaticMeshActor>(A) ? Cast<AStaticMeshActor>(A)->GetStaticMeshComponent() : nullptr;",
            f'{indent}\tUE_LOG(LogVEILWorld, Warning, TEXT("VEIL SNAP: piece %s mesh %s gap %.1f"),',
            f'{indent}\t\t*A->GetName(), (SMC && SMC->GetStaticMesh()) ? *SMC->GetStaticMesh()->GetPathName() : TEXT("none"), Gap);',
            f"{indent}}}",
        ]
        L[i:i] = insert
        open(p, "w", encoding="utf-8", newline="").write("\n".join(L))
        print("audit: snapped pieces now name their mesh")
        break
else:
    print("no snap log found")
