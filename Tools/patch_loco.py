"""Apply the movement fixes to VEILCharacter.cpp, or abort without writing.

Why a script: this file is CRLF and the patch tool kept rejecting multi-hunk
edits on it. Every replacement below asserts its own match count first, so a
mismatch aborts the whole write instead of leaving a half-edited pawn.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
failures = []


def rep(old, new, count=1):
    global text
    old = old.replace("\n", NL)
    new = new.replace("\n", NL)
    found = text.count(old)
    if found != count:
        failures.append("%d (want %d): %s" % (found, count, old.split(NL)[0].strip()[:70]))
        return
    text = text.replace(old, new, count)


# 1. A CONVERSATION ONLY HOLDS THE BODY WHILE A QUESTION IS OPEN.
rep("\treturn Dlg && Dlg->IsActive();",
    "\t// ONLY A WAITING ANSWER HOLDS THE BODY. This used to return true whenever ANY\n"
    "\t// conversation was active - and the district plays ambient overheard lines at\n"
    "\t// boot (\"PHI LIVING: overheard - DAL ...\"), so Move() zeroed its own axis and\n"
    "\t// returned before the input ever reached the movement component. The runner\n"
    "\t// stood still for a whole session with every key correctly bound: that is\n"
    "\t// exactly \"he cannot move\". Walking away from a line is allowed; walking away\n"
    "\t// from a question is not.\n"
    "\treturn Dlg && Dlg->IsChoosing();")

# 2. THE BODY'S STATE, ONCE, FIVE SECONDS IN - no keypress required.
rep("\tHealth = MaxHealth;\n\tOnHealthChanged.Broadcast(Health, MaxHealth);",
    "\tHealth = MaxHealth;\n"
    "\tOnHealthChanged.Broadcast(Health, MaxHealth);\n"
    "\n"
    "\t// PROVE THE BODY CAN MOVE WITHOUT A KEYPRESS. Five seconds in, the log prints\n"
    "\t// what the movement component is set to: mode, allowed speed, whether input is\n"
    "\t// ignored, which controller owns him and whether a story holds him.\n"
    "\t{\n"
    "\t\tFTimerHandle LocoDiag;\n"
    "\t\tGetWorldTimerManager().SetTimer(LocoDiag, [this]()\n"
    "\t\t{\n"
    "\t\t\tconst UCharacterMovementComponent* M = GetCharacterMovement();\n"
    "\t\t\tUE_LOG(LogVEIL, Warning,\n"
    "\t\t\t\tTEXT(\"PHI LOCO STATE: mode=%d maxWalk=%.0f ignored=%d controller=%s story=%d vel=%s loc=%s\"),\n"
    "\t\t\t\tM ? (int32)M->MovementMode : -1, M ? M->MaxWalkSpeed : -1.f,\n"
    "\t\t\t\tIsMoveInputIgnored() ? 1 : 0,\n"
    "\t\t\t\tController ? *Controller->GetClass()->GetName() : TEXT(\"none\"),\n"
    "\t\t\t\tIsStoryHoldingInput() ? 1 : 0,\n"
    "\t\t\t\tM ? *M->Velocity.ToCompactString() : TEXT(\"-\"),\n"
    "\t\t\t\t*GetActorLocation().ToCompactString());\n"
    "\t\t}, 5.f, false);\n"
    "\t}")

# 3. WHAT THE MOVE ACTION ACTUALLY RECEIVED, every two seconds while held.
rep("\tconst FVector2D Axis = MoveAxis;",
    "\tconst FVector2D Axis = MoveAxis;\n"
    "\t// SAY WHAT THE BODY RECEIVED: axis, mode, allowed speed, ignored, story.\n"
    "\tif (!Axis.IsNearlyZero() && GetWorld())\n"
    "\t{\n"
    "\t\tstatic float MoveDiagClock = 0.f;\n"
    "\t\tMoveDiagClock += GetWorld()->GetDeltaSeconds();\n"
    "\t\tif (MoveDiagClock >= 2.f)\n"
    "\t\t{\n"
    "\t\t\tMoveDiagClock = 0.f;\n"
    "\t\t\tconst UCharacterMovementComponent* Diag = GetCharacterMovement();\n"
    "\t\t\tUE_LOG(LogVEIL, Warning,\n"
    "\t\t\t\tTEXT(\"PHI MOVE DIAG: axis=(%.2f,%.2f) mode=%d maxWalk=%.0f ignored=%d story=%d accel=%s vel=%s\"),\n"
    "\t\t\t\tAxis.X, Axis.Y, Diag ? (int32)Diag->MovementMode : -1,\n"
    "\t\t\t\tDiag ? Diag->MaxWalkSpeed : -1.f, IsMoveInputIgnored() ? 1 : 0,\n"
    "\t\t\t\tIsStoryHoldingInput() ? 1 : 0,\n"
    "\t\t\t\tDiag ? *Diag->GetCurrentAcceleration().ToCompactString() : TEXT(\"-\"),\n"
    "\t\t\t\tDiag ? *Diag->Velocity.ToCompactString() : TEXT(\"-\"));\n"
    "\t\t}\n"
    "\t}")

if failures:
    print("ABORTED - nothing written. Misses:")
    for f in failures:
        print("  " + f)
    raise SystemExit(1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d chars, %d replacements" % (PATH, len(text), 3))
