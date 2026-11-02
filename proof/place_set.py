import math
import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\proof\place_set_result.txt"
PHI = (1.0 + math.sqrt(5.0)) / 2.0
GOLDEN = 360.0 / (PHI * PHI)

SHIP = "/Game/Underwater/SM_SubAkvo"
COLUMN = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big"
DEBRIS = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientDebris"
ARCH = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientArchColumn"

lines = []


def log(m):
    lines.append(m)
    unreal.log("SET " + m)


def load(path):
    try:
        return unreal.load_asset(path)
    except Exception as e:
        log("load fail %s %r" % (path, e))
        return None


def spawn(mesh_path, loc, yaw, label, scale=1.0):
    m = load(mesh_path)
    if not m:
        return None
    a = sub.spawn_actor_from_class(unreal.StaticMeshActor, loc, unreal.Rotator(0.0, yaw, 0.0))
    if not a:
        return None
    comp = a.get_component_by_class(unreal.StaticMeshComponent)
    if comp:
        comp.set_static_mesh(m)
        comp.set_mobility(unreal.ComponentMobility.STATIC)
    a.set_actor_label(label)
    a.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    return a


try:
    try:
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP)
        log("LOAD ok")
    except Exception as e:
        log("LOAD failed %r" % (e,))

    sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    # the imported Fab ship, beached north of the plaza
    ship = spawn(SHIP, unreal.Vector(0.0, -1800.0, 100.0), GOLDEN, "SET_SubAkvo", 1.0)
    if ship:
        log("SHIP at (0.00, -1800.00, 100.00) yaw=%.4f" % GOLDEN)

    # sicka columns on the golden spiral around the plaza
    made = 0
    for i in range(8):
        th = math.radians(i * GOLDEN)
        r = 700.0 * (PHI ** (i / 6.0))
        loc = unreal.Vector(r * math.cos(th), r * math.sin(th), 100.0)
        s = 0.8 + 0.2 * (i % 3)
        a = spawn(COLUMN, loc, (i * GOLDEN) % 360.0, "SET_Column_%02d" % i, s)
        if a:
            made += 1
            log("COLUMN_%02d (%.2f, %.2f) yaw=%.2f scale=%.2f" % (
                i, loc.x, loc.y, (i * GOLDEN) % 360.0, s))

    # debris and one arch to break the plaza silhouette
    for i in range(5):
        th = math.radians(i * GOLDEN + 60.0)
        r = 1200.0 + 200.0 * i
        loc = unreal.Vector(r * math.cos(th), r * math.sin(th), 100.0)
        if spawn(DEBRIS, loc, (i * GOLDEN) % 360.0, "SET_Debris_%02d" % i, 1.2):
            made += 1
            log("DEBRIS_%02d (%.2f, %.2f)" % (i, loc.x, loc.y))

    if spawn(ARCH, unreal.Vector(-900.0, -900.0, 100.0), 90.0, "SET_Arch", 1.0):
        made += 1
        log("ARCH at (-900.00, -900.00)")

    log("PLACED %d pieces" % made)

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
    unreal.log("SET done")
