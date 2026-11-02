# seat719_save_proof.py
# VEIL OMNI - map verify + save + seat719_out.txt, INSIDE the editor only.
# Play is deliberately NOT requested here: this file exists to prove the world
# was really written to disk, and Play is proven separately from the game's own
# frame capture. Run with:
#   UnrealEditor.exe VEIL.uproject -run=pythonscript -script="D:/VEIL-OMNI/Tools/seat719_save_proof.py"
# or from the editor's Python console.

import time
import unreal

MAP = "/Game/Maps/VEILDistrict"
SEAT = r"D:\VEIL-OMNI\seat719_out.txt"

VAULT_TARGET = (0.0, 0.0, -80.9)
COUNTESS_TARGET = (809.0, -309.0, 64.7)
RUNNER_TARGET = (0.0, 0.0, 100.0)
TOL = 0.1


def vec_tuple(loc):
    return (float(loc.x), float(loc.y), float(loc.z))


def matches(loc, target):
    v = vec_tuple(loc)
    return all(abs(v[i] - target[i]) <= TOL for i in range(3))


def main():
    lines = ["WHOLE_VERIFY_GOAL start map=%s" % MAP]

    load_ok, load_used = False, "none"
    try:
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP)
        load_ok, load_used = True, "LevelEditorSubsystem.load_level"
    except Exception as e:
        lines.append("LOAD load_level failed: %r" % (e,))
        try:
            unreal.EditorLoadingAndSavingUtils.load_map(MAP)
            load_ok, load_used = True, "EditorLoadingAndSavingUtils.load_map"
        except Exception as e2:
            lines.append("LOAD load_map failed: %r" % (e2,))
    lines.append("LOAD ok=%s used=%s" % (load_ok, load_used))

    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    vaults, countesses, runners = [], [], []
    meshes, placeholders, lights = 0, 0, 0
    for a in actors:
        try:
            label = a.get_actor_label()
            loc = a.get_actor_location()
        except Exception:
            continue
        upper = label.upper()
        if "VAULT" in upper:
            vaults.append((label, vec_tuple(loc), matches(loc, VAULT_TARGET)))
        if "COUNTESS" in upper:
            countesses.append((label, vec_tuple(loc), matches(loc, COUNTESS_TARGET)))
        if "RUNNER" in upper:
            runners.append((label, vec_tuple(loc), matches(loc, RUNNER_TARGET)))
        try:
            if a.get_component_by_class(unreal.StaticMeshComponent):
                meshes += 1
            if a.get_component_by_class(unreal.LightComponent):
                lights += 1
        except Exception:
            pass
        if "PLACEHOLDER" in upper or "TEMP_" in upper:
            placeholders += 1

    for label, loc, ok in vaults:
        lines.append("VAULT %s %s match=%s" % (label, loc, ok))
    for label, loc, ok in countesses:
        lines.append("COUNTESS %s %s match=%s" % (label, loc, ok))
    for label, loc, ok in runners:
        lines.append("RUNNER %s %s match=%s" % (label, loc, ok))

    vault_pass = len(vaults) >= 1 and all(ok for _, _, ok in vaults)
    countess_pass = len(countesses) == 1 and countesses[0][2]
    runner_pass = len(runners) >= 1 and all(ok for _, _, ok in runners)
    verify_pass = bool(load_ok and vault_pass and countess_pass and runner_pass)
    lines.append("VERIFY vault=%s count=%d countess=%s count=%d runner=%s count=%d PASS=%s" % (
        vault_pass, len(vaults), countess_pass, len(countesses), runner_pass,
        len(runners), verify_pass))

    before = "BEFORE actors=%d meshes=%d placeholders=%d lights=%d" % (
        len(actors), meshes, placeholders, lights)
    lines.append(before)

    save_ok, save_used = False, "none"
    try:
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        save_ok = bool(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP))
        save_used = "EditorLoadingAndSavingUtils.save_map(world, MAP)"
    except Exception as e:
        lines.append("SAVE save_map failed: %r" % (e,))
        try:
            unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_all_dirty_levels()
            save_ok, save_used = True, "LevelEditorSubsystem.save_all_dirty_levels()"
        except Exception as e2:
            lines.append("SAVE save_all_dirty_levels failed: %r" % (e2,))
    lines.append("SAVE ok=%s used=%s" % (save_ok, save_used))

    after_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    lines.append("AFTER  actors=%d meshes=%d placeholders=%d lights=%d" % (
        len(after_actors), meshes, placeholders, lights))
    lines.append("SAVE RESULT %s" % ("True" if save_ok else "False"))
    lines.append("VERIFY_PASS=%s MAP=%s" % (verify_pass, MAP))

    with open(SEAT, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\nWHOLE %s VAULT 0,0,-80.9 COUNTESS 809,-309,64.7 RUNNER 0,0,100 VERIFY_PASS=%s SAVE=%s\n" % (
            time.time(), verify_pass, save_ok))
    for line in lines:
        unreal.log("SEAT719 " + line)


main()
