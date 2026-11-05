"""Bodies were seated on rooftops and valley floor; probe only near the seat."""

npc = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilNpc.cpp"
run = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilRunner.cpp"

# 1. GroundZ: a short probe around the intended height, so a hall roof 500 above
#    cannot capture a body standing in the plaza.
n = open(npc, encoding="utf-8", newline="").read().replace("\r\n", "\n")
start = n.find("double AVeilNpc::GroundZ(")
if start < 0:
    raise SystemExit("MISS GroundZ")
brace = n.find("{", start)
depth = 0
end = -1
for k in range(brace, len(n)):
    if n[k] == "{":
        depth += 1
    elif n[k] == "}":
        depth -= 1
        if depth == 0:
            end = k
            break
if end < 0:
    raise SystemExit("MISS GroundZ end")

body = '''{
	// Probe a short window around the intended height. A long probe from high above
	// captures rooftops and leaves people standing on a hall roof or a valley floor.
	FHitResult Hit;
	const FVector From(X, Y, FromZ + PHI_Constants::Reach(0.5));
	const FVector To(X, Y, FromZ - PHI_Constants::Reach(0.75));
	if (GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility))
	{
		return Hit.ImpactPoint.Z + 2.0;
	}
	return FromZ;
}'''
n = n[:brace] + body + n[end + 1:]
open(npc, "w", encoding="utf-8", newline="").write(n)
print("people: seats probe only near their own height")

# 2. hide the placeholder primitives whenever the real skinned body is loaded
r = open(run, encoding="utf-8", newline="").read().replace("\r\n", "\n")
if "Avatar" in r and "SetHiddenInGame" not in r.split("BasicShapeMaterial")[0]:
    anchor = '\tUMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));'
    if anchor in r:
        guard = ('\t// The placeholder primitives exist only as a fallback. When the real\n'
                 '\t// skinned body is present they must not draw or carry placeholder material.\n'
                 '\tconst bool bHasSkin = Avatar && Avatar->GetSkeletalMeshAsset();\n'
                 '\tif (bHasSkin)\n'
                 '\t{\n'
                 '\t\tif (Body) { Body->SetHiddenInGame(true); Body->SetVisibility(false); }\n'
                 '\t\tif (Head) { Head->SetHiddenInGame(true); Head->SetVisibility(false); }\n'
                 '\t}\n')
        r = r.replace(anchor, guard + "\tUMaterialInterface* Base = bHasSkin ? nullptr : LoadObject<UMaterialInterface>(nullptr, TEXT(\"/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial\"));", 1)
        open(run, "w", encoding="utf-8", newline="").write(r)
        print("runner: primitives hidden when the real body is present")
    else:
        print("runner: MISS BasicShapeMaterial")
else:
    print("runner: skipped")
