"""A local named Mesh hides ACharacter::Mesh."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilRunner.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")
old = """		if (USkeletalMesh* Mesh = Avatar->GetSkeletalMeshAsset())
		{
			const FBoxSphereBounds B = Mesh->GetBounds();"""
new = """		if (USkeletalMesh* Skinned = Avatar->GetSkeletalMeshAsset())
		{
			const FBoxSphereBounds B = Skinned->GetBounds();"""
if old in s:
    s = s.replace(old, new, 1)
    open(p, "w", encoding="utf-8", newline="").write(s)
    print("runner: local renamed, no member hidden")
else:
    print("already renamed or anchor changed")
