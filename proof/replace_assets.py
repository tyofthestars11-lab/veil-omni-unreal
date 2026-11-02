import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\proof\replace_assets_result.txt"
SHARD = "/Game/ParagonGreystone/FX/Meshes/Heroes/Greystone/Abilities/SM_Greystone_Statue_Chunks_shard%d"
SHRINE = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_LittleShrine"
MONOLITH = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientMonolith"
COLUMN = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big"
MURDOCK = "/Game/ParagonMurdock/Characters/Heroes/Murdock/Meshes/Murdock"

lines = []


def log(m):
    lines.append(m)
    unreal.log("ASSETS " + m)


def load(path):
    try:
        a = unreal.load_asset(path)
        return a
    except Exception as e:
        log("LOAD FAIL %s %r" % (path, e))
        return None


def set_static(actor, path, scale):
    m = load(path)
    if not m:
        return False
    try:
        comp = actor.get_component_by_class(unreal.StaticMeshComponent)
        if not comp:
            log("NO SMC on %s" % actor.get_actor_label())
            return False
        comp.set_static_mesh(m)
        if scale:
            actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
        return True
    except Exception as e:
        log("SET FAIL %s %r" % (path, e))
        return False


try:
    try:
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP)
        log("LOAD ok")
    except Exception as e:
        log("LOAD failed %r" % (e,))

    sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    ok = 0
    for a in sub.get_all_level_actors():
        try:
            label = str(a.get_actor_label())
        except Exception:
            continue
        if label.startswith("FRAGMENT_"):
            idx = int(label.split("_")[1]) % 10
            if set_static(a, SHARD % (idx + 1), 0.8):
                ok += 1
                log("%s -> shard%d" % (label, idx + 1))
        elif label == "VEILForge":
            if set_static(a, SHRINE, 1.6):
                ok += 1
                log("VEILForge -> SM_LittleShrine")
        elif label.startswith("BEACON_"):
            if set_static(a, MONOLITH, 1.4):
                ok += 1
                log("%s -> SM_AncientMonolith" % label)
        elif label == "VEILWaveSpawner":
            if set_static(a, COLUMN, 1.0):
                ok += 1
                log("VEILWaveSpawner -> SM_AncientColumn_Big")
        elif label == "WARDEN_PRIME":
            m = load(MURDOCK)
            if m:
                try:
                    sk = a.get_component_by_class(unreal.SkeletalMeshComponent)
                    if sk:
                        try:
                            sk.set_editor_property("skeletal_mesh_asset", m)
                        except Exception:
                            sk.set_skeletal_mesh(m)
                        ok += 1
                        log("WARDEN_PRIME -> Murdock skeletal mesh")
                    else:
                        log("WARDEN no skeletal component")
                except Exception as e:
                    log("WARDEN set failed %r" % (e,))
            else:
                log("WARDEN mesh load failed")

    log("SET %d meshes" % ok)
    save_ok = False
    try:
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        save_ok = bool(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP))
    except Exception as e:
        log("SAVE failed %r" % (e,))
    log("SAVE RESULT %s" % ("True" if save_ok else "False"))
except Exception as e:
    log("FATAL %r" % (e,))
finally:
    with open(OUT, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    unreal.log("ASSETS done")
