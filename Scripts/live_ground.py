"""Build-time traces run before the landscape is registered; prove it in Play instead."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

# 1. drop the premature build-time floor trace
start = s.find("\t{\n\t\t// Proof, not assumption: what is actually under the plaza centre.")
endmark = ("\t\t\tUE_LOG(LogVEILWorld, Display, TEXT(\"VEIL HUB FLOOR TRACE: NOTHING under (0,0)\"));\n"
           "\t\t}\n\t}\n")
if start >= 0:
    e = s.find(endmark, start)
    if e < 0:
        raise SystemExit("MISS build trace end")
    s = s[:start] + s[e + len(endmark):]
    print("hub: removed premature build-time trace")

# 2. the record camera looks across the plaza, not down at its floor
pairs = [
    ("\tstatic const double BackPow[5] = { 0.9, 2.0, 2.0, 2.0, 2.0 };",
     "\tstatic const double BackPow[5] = { 3.566, 2.0, 2.0, 2.0, 2.0 };"),
    ("\tstatic const double UpPow[5] = { 2.6, 1.6, 1.6, 1.6, 1.6 };",
     "\tstatic const double UpPow[5] = { 3.566, 1.6, 1.6, 1.6, 1.6 };"),
    ("GroundZ + PHI_Constants::Reach(1.4)) - CamAt).Rotation();",
     "GroundZ + PHI_Constants::Reach(1.9)) - CamAt).Rotation();"),
]
for old, new in pairs:
    if old not in s:
        raise SystemExit("MISS " + old[:44])
    s = s.replace(old, new, 1)

# 3. prove the floor in Play, where the landscape is registered
old_trace = ("\t\tif (GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_WorldStatic))\n"
             "\t\t{\n"
             "\t\t\tGroundZ = Hit.ImpactPoint.Z;\n"
             "\t\t}")
new_trace = ("\t\tif (GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_WorldStatic))\n"
             "\t\t{\n"
             "\t\t\tGroundZ = Hit.ImpactPoint.Z;\n"
             "\t\t\tUE_LOG(LogVEILWorld, Display, TEXT(\"VEIL SHOT GROUND: %s at %.1f\"),\n"
             "\t\t\t\tIsValid(Hit.GetActor()) ? *Hit.GetActor()->GetName() : TEXT(\"landscape\"),\n"
             "\t\t\t\tHit.ImpactPoint.Z);\n"
             "\t\t}")
if old_trace not in s:
    raise SystemExit("MISS shot trace")
s = s.replace(old_trace, new_trace, 1)

open(p, "w", encoding="utf-8", newline="").write(s)
print("shot: live ground proof, camera across the plaza")
