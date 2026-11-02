import math
import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\proof\build_loop_result.txt"
PHI = (1.0 + math.sqrt(5.0)) / 2.0
GOLDEN_DEG = 360.0 / (PHI * PHI)

SPHERE = "/Game/StarterContent/Shapes/Shape_Sphere"
CUBE = "/Game/StarterContent/Shapes/Shape_Cube"
CYL = "/Game/StarterContent/Shapes/Shape_Cylinder"

lines = []


def log(m):
    lines.append(m)
    unreal.log("BUILD " + m)


def load(path):
    try:
        a = unreal.load_asset(path)
        return a
    except Exception:
        return None


def dress(actor, mesh_path, scale):
    try:
        comp = actor.get_component_by_class(unreal.StaticMeshComponent)
        m = load(mesh_path)
        if comp and m:
            comp.set_static_mesh(m)
        actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    except Exception as e:
        log("DRESS failed %r" % (e,))


try:
    try:
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP)
        log("LOAD ok")
    except Exception as e:
        log("LOAD failed %r" % (e,))

    sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = sub.get_all_level_actors()

    vault = None
    beacon_spots = []
    for a in actors:
        try:
            label = str(a.get_actor_label())
        except Exception:
            continue
        if label == "VEILVault":
            vault = a
        if label.startswith("Beacon_") and "Light_" not in label:
            beacon_spots.append((label, a.get_actor_location()))

    origin = vault.get_actor_location() if vault else unreal.Vector(0, 0, -80.9)
    log("VAULT at %s beacons=%s" % ((origin.x, origin.y, origin.z), [b[0] for b in beacon_spots]))

    # 10 fragments on the golden-angle spiral, r_i = 618 * phi^(i/5)
    for i in range(10):
        th = math.radians(i * GOLDEN_DEG)
        r = 618.0 * (PHI ** (i / 5.0))
        pos = unreal.Vector(origin.x + r * math.cos(th), origin.y + r * math.sin(th), 100.0)
        f = sub.spawn_actor_from_class(unreal.VEILFragment, pos, unreal.Rotator(0, 0, 0))
        if f:
            f.set_actor_label("FRAGMENT_%02d" % i)
            dress(f, SPHERE, 0.45)
            log("FRAGMENT_%02d at (%.2f, %.2f, 100.0) r=%.2f" % (
                i, pos.x, pos.y, r))

    # Forge beside the Vault
    forge = sub.spawn_actor_from_class(
        unreal.VEILForge, unreal.Vector(origin.x, origin.y - 382.0, 100.0), unreal.Rotator(0, 0, 0))
    if forge:
        forge.set_actor_label("VEILForge")
        dress(forge, CUBE, 2.0)
        log("FORGE at (0.00, -382.00, 100.00)")

    # Wave spawner at the plaza
    spawner = sub.spawn_actor_from_class(
        unreal.VEILWaveSpawner, unreal.Vector(origin.x, origin.y - 618.0, 100.0), unreal.Rotator(0, 0, 0))
    if spawner:
        spawner.set_actor_label("VEILWaveSpawner")
        dress(spawner, CYL, 1.0)
        log("SPAWNER at (0.00, -618.00, 100.00)")

    # WARDEN PRIME in the cache arena
    warden = sub.spawn_actor_from_class(
        unreal.OMNIWardenPrime, unreal.Vector(0.0, -6200.0, 100.0), unreal.Rotator(0, 0, 0))
    if warden:
        warden.set_actor_label("WARDEN_PRIME")
        log("WARDEN at (0.00, -6200.00, 100.00)")

    # Functional beacons at the four statue points (statues themselves stay)
    made = 0
    for label, loc in beacon_spots:
        b = sub.spawn_actor_from_class(unreal.VEILBeacon, loc, unreal.Rotator(0, 0, 0))
        if b:
            b.set_actor_label("BEACON_" + label.replace("Beacon_", ""))
            dress(b, CYL, 1.2)
            made += 1
            log("BEACON %s at (%.1f, %.1f, %.1f)" % (b.get_actor_label(), loc.x, loc.y, loc.z))
    log("BEACONS made=%d" % made)

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
    unreal.log("BUILD result written")
