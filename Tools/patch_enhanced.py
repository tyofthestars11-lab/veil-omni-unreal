"""A checked cast at input setup is a crash, not a check.

Measured fatal: "Cast of InputComponent ...VEILCharacter_0.PawnInputComponent0 to
EnhancedInputComponent failed", Casts.cpp:10, during PlayerController.ClientRestart.
Cause: Config/DefaultInput.ini on disk had lost
DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent, so the pawn
was handed a plain UInputComponent. The ini is restored; this makes the code survive
the same regression with a named diagnosis instead of a dead process.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = "\tif (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))"
NEW = (
    "\t// A CHECKED CAST HERE IS A CRASH, NOT A CHECK. Enhanced Input lives in the ini\n"
    "\t// (DefaultInputComponentClass); with that key missing the pawn is handed a plain\n"
    "\t// UInputComponent and CastChecked kills the process at ClientRestart. Measured:\n"
    "\t// \"Cast of InputComponent ... to EnhancedInputComponent failed\", Casts.cpp:10.\n"
    "\tif (!PlayerInputComponent->IsA<UEnhancedInputComponent>())\n"
    "\t{\n"
    "\t\tUE_LOG(LogVEIL, Error,\n"
    "\t\t\tTEXT(\"PHI INPUT: pawn input component is %s, not EnhancedInputComponent - check Config/DefaultInput.ini DefaultInputComponentClass. Keys still bind; actions do not.\"),\n"
    "\t\t\t*PlayerInputComponent->GetClass()->GetName());\n"
    "\t}\n"
    "\tif (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))"
)

o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - count=%d" % src.count(o))
    raise SystemExit(1)
io.open(PATH, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - enhanced input cast is safe and self-reporting" % PATH)
