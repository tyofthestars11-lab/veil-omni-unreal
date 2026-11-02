# buffy_verify_save.py - runs INSIDE Unreal Editor (Output Log: py "D:/VEIL-OMNI/Tools/buffy_verify_save.py")
# Load VEILDistrict, verify the three anchors, save the level, report whole, quit.
# Vault 0,0,-80.9 - Countess 809,-309,64.7 - RUNNER 0,0,100 - Floor 1618
import unreal

MAP = "/Game/Maps/VEILDistrict"

print("BUFFY LOAD BEGIN " + MAP)
unreal.EditorLoadingAndSavingUtils.load_map(MAP)
print("BUFFY LOAD OK " + MAP)

subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsys.get_all_level_actors()

vault = []
countess = []
runner = []
for a in actors:
    label = a.get_actor_label()
    loc = a.get_actor_location()
    if "Vault" in label:
        vault.append((label, loc))
        print(f"BUFFY VAULT {label} at {loc} - anchor 0,0,-80.9")
    if "Countess" in label:
        countess.append((label, loc))
        print(f"BUFFY COUNTESS {label} at {loc} - anchor 809,-309,64.7")
    if "RUNNER" in label:
        runner.append((label, loc))
        print(f"BUFFY RUNNER {label} at {loc} - anchor 0,0,100")

ok = (len(vault) >= 1 and len(countess) >= 1 and len(runner) >= 1)
print(f"BUFFY VERIFY vault={len(vault)} countess={len(countess)} runner={len(runner)} actors={len(actors)} -> {'WHOLE' if ok else 'INCOMPLETE'}")

# UE 5.8: save_map takes a World object, not an asset path string. The level
# editor subsystem saves the CURRENT level, which is the one just loaded.
saved = False
try:
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    saved = les.save_current_level()
except Exception as e:
    print("BUFFY SAVE EXCEPTION " + str(e))
print("BUFFY SAVE " + ("True" if saved else "False") + " " + MAP)

unreal.SystemLibrary.quit_editor()
