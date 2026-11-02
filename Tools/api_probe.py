# WHAT TRACE API DOES THIS BUILD ACTUALLY EXPOSE?
#
# Two guesses have already cost a run each (SystemLibrary, KismetSystemLibrary). This asks
# the engine instead of guessing, and prints every module-level name containing "race"
# plus the members of any library that carries one.
import unreal

names = sorted(n for n in dir(unreal) if "race" in n.lower())
unreal.log("[VEIL API] module names with 'trace': " + ", ".join(names))

for lib in ("SystemLibrary", "GameplayStatics", "MathLibrary", "EditorLevelLibrary",
            "EditorActorSubsystem", "UnrealEditorSubsystem", "Engine", "HitResult"):
    obj = getattr(unreal, lib, None)
    if obj is None:
        unreal.log("[VEIL API] %s: MISSING" % lib)
        continue
    members = sorted(m for m in dir(obj) if "trace" in m.lower() or "bounds" in m.lower())
    unreal.log("[VEIL API] %s: %s" % (lib, ", ".join(members) if members else "(none)"))
