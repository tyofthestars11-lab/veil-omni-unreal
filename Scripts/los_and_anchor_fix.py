"""Line of sight must not be blocked by the target's own body; the anchor must be last."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


r = P + "VeilRunner.cpp"
s = rd(r)
pairs = [
    # geometry only: a pawn trace would be blocked by the very body being struck
    ("GetWorld()->LineTraceSingleByChannel(Hit, Eye, It->GetActorLocation(), ECC_Visibility, Params)",
     "GetWorld()->LineTraceSingleByChannel(Hit, Eye, It->GetActorLocation(), ECC_WorldStatic, Params)"),
    # control rotation is not applied to the body in every path; the body's own facing is
    ("const FVector Forward = FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X);",
     "const FVector Forward = GetActorForwardVector();"),
]
for old, new in pairs:
    if old in s:
        s = s.replace(old, new, 1)
        print("runner:", old[:46], "->", new[:46])
    else:
        print("runner: MISS", old[:46])
wr(r, s)

g = P + "VeilGameMode.cpp"
s = rd(g)
old = "\t\tSelfTestRunner->RespawnAt = SelfTestRunner->GetActorLocation();"
new = ("\t\t// The slot owns the position if it has one; the live body only anchors\n"
       "\t\t// after that placement, so death returns to the ground the player stood on.\n"
       "\t\tif (SelfTestRunner->bHasSaved)\n"
       "\t\t{\n"
       "\t\t\tSelfTestRunner->SetActorLocation(SelfTestRunner->RespawnAt);\n"
       "\t\t}\n"
       "\t\tSelfTestRunner->RespawnAt = SelfTestRunner->GetActorLocation();")
if old in s:
    s = s.replace(old, new, 1)
    print("game mode: respawn anchored after the saved placement")
else:
    print("game mode: MISS anchor line")

old = "\t\t\t\tC->SetControlRotation((It->GetActorLocation() - R->GetActorLocation()).Rotation());"
new = ("\t\t\t\tC->SetControlRotation((It->GetActorLocation() - R->GetActorLocation()).Rotation());\n"
       "\t\t\t\tR->SetActorRotation((It->GetActorLocation() - R->GetActorLocation()).Rotation());")
if old in s and "R->SetActorRotation((It->GetActorLocation()" not in s:
    s = s.replace(old, new, 1)
    print("proof: body faces its target")
wr(g, s)
print("los and anchor fix done")
