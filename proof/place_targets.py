import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\proof\place_result.txt"
VAULT_TARGET = unreal.Vector(0.0, 0.0, -80.9)
COUNTESS_TARGET = unreal.Vector(809.0, -309.0, 64.7)
RUNNER_TARGET = unreal.Vector(0.0, 0.0, 100.0)

lines = []


def log(msg):
    lines.append(msg)
    unreal.log("PLACE " + msg)


def vec(v):
    return "(%s, %s, %s)" % (round(float(v.x), 2), round(float(v.y), 2), round(float(v.z), 2))


def flush():
    try:
        with open(OUT, "w", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
    except Exception as e:
        unreal.log_error("PLACE result file failed: %r" % (e,))


try:
    try:
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP)
        log("LOAD ok")
    except Exception as e:
        log("LOAD failed %r" % (e,))

    sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = sub.get_all_level_actors()

    vault = None
    countesses = []
    runner = None
    starts = []
    for a in actors:
        try:
            label = a.get_actor_label()
        except Exception:
            continue
        upper = label.upper()
        if label == "VEILVault":
            vault = a
        if "COUNTESS" in upper:
            countesses.append((label, a))
        if "RUNNER" in upper:
            runner = a
        if isinstance(a, unreal.PlayerStart):
            starts.append(label)

    log("BEFORE vault=%s countess=%d runner=%s playerstarts=%d %s" % (
        vault.get_actor_label() if vault else "NONE", len(countesses),
        runner.get_actor_label() if runner else "NONE", len(starts), starts))

    if vault is None:
        log("VAULT NOT FOUND")
    else:
        vault.set_actor_location(VAULT_TARGET, False, True)
        log("VAULT %s at %s" % (vault.get_actor_label(), vec(vault.get_actor_location())))

    if len(countesses) == 1:
        label, act = countesses[0]
        act.set_actor_location(COUNTESS_TARGET, False, True)
        log("COUNTESS %s at %s" % (label, vec(act.get_actor_location())))
    elif not countesses:
        log("COUNTESS NOT FOUND")
    else:
        log("COUNTESS COUNT %d - not moved" % len(countesses))

    if runner is None:
        runner = sub.spawn_actor_from_class(
            unreal.PlayerStart, RUNNER_TARGET, unreal.Rotator(0.0, 90.0, 0.0))
        runner.set_actor_label("RUNNER")
        log("RUNNER ADDED %s at %s" % (runner.get_actor_label(), vec(runner.get_actor_location())))
    else:
        runner.set_actor_location(RUNNER_TARGET, False, True)
        log("RUNNER EXISTED %s at %s" % (runner.get_actor_label(), vec(runner.get_actor_location())))

    save_ok = False
    try:
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        save_ok = bool(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP))
        log("SAVE save_map %s" % save_ok)
    except Exception as e:
        log("SAVE save_map failed %r" % (e,))
        try:
            unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_all_dirty_levels()
            save_ok = True
        except Exception as e2:
            log("SAVE fallback failed %r" % (e2,))
    log("SAVE RESULT %s" % ("True" if save_ok else "False"))
except Exception as e:
    log("FATAL %r" % (e,))
finally:
    flush()
    unreal.log("PLACE result written to %s" % OUT)
