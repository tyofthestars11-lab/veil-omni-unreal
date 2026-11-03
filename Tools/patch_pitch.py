"""Golden pitch on the runner's movement directions, about RightVector.

17.0324 deg: cos 0.9561, sin 0.2929. The rotation is about RightVector because UpVector
is yaw - rotating about it turns W sideways, which is exactly the bug this replaces.
Applied to the same two vectors the Enhanced Input path feeds AddMovementInput, so the
keyboard, the stick and the proof all travel the one direction.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = (
    "\tconst FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);\n"
    "\tconst FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);\n"
    "\tconst FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);\n"
    "\tAddMovementInput(Forward, Axis.Y);\n"
    "\tAddMovementInput(Right, Axis.X);\n"
    "}"
)
NEW = (
    "\tconst FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);\n"
    "\tconst FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);\n"
    "\tconst FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);\n"
    "\t// GOLDEN PITCH, AROUND RIGHTVECTOR. 17.0324 deg - cos 0.9561, sin 0.2929 - so a\n"
    "\t// forward push is 95.6% of the heading and 29.3% of the pitch. UpVector is yaw\n"
    "\t// and rotates W sideways, which is why this uses the right axis. Every path\n"
    "\t// (keys, stick, the proof) arrives here, so there is one direction, not two.\n"
    "\tstatic const FVector PitchAxis = FVector::RightVector;\n"
    "\tAddMovementInput(Forward.RotateAngleAxis(-17.0324f, PitchAxis), Axis.Y);\n"
    "\tAddMovementInput(Right.RotateAngleAxis(-17.0324f, PitchAxis), Axis.X);\n"
    "}"
)

o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - count=%d" % src.count(o))
    raise SystemExit(1)
io.open(PATH, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - movement uses golden pitch about RightVector" % PATH)
