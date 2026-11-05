"""Dress the plaza: real prop meshes, discovered by name, placed deterministically.

The first pass died because the asset scan also returned MaterialInstance assets
named after meshes and then asked one for its bounds. Discovery is now class
filtered, and it looks in every prop pack the project actually has.
"""
import math
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_dress_log.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

PHI = 1.618033988749895
GOLD = 137.5077640500378
MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
ROOTS = ["/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh",
         "/Game/VEIL/CC0/Meshes",
         "/Game/Sicka_EnvPack2/Sicka_Scifi_Interior/Mesh"]
V = "/Game/VEIL/Materials/"
C = "/Game/VEIL/CC0/Materials/"
DECK = 189400.0
TAG = "VEIL_DRESS"
KEYS = ["barrel", "crate", "box", "pot", "urn", "basket", "statue", "table",
        "cart", "wheel", "bridge", "boulder", "rock", "sack", "bag", "chest"]
count = {}


def bump(k, n=1):
    count[k] = count.get(k, 0) + n


def clean():
    n = 0
    for a in unreal.EditorLevelLibrary.get_all_level_actors():
        try:
            if a.actor_has_tag(TAG):
                unreal.EditorLevelLibrary.destroy_actor(a)
                n += 1
        except Exception:  # noqa: BLE001
            pass
    w("cleared %d dress actors" % n)


def spawner():
    try:
        s = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        if s:
            return s.spawn_actor_from_class
    except Exception:  # noqa: BLE001
        pass
    return unreal.EditorLevelLibrary.spawn_actor_from_class


def discover():
    """Only real StaticMeshes, grouped by keyword."""
    found = {k: [] for k in KEYS}
    seen = set()
    for root in ROOTS:
        for path in unreal.EditorAssetLibrary.list_assets(root, recursive=True,
                                                         include_folder=False):
            try:
                obj = unreal.load_asset(path)
            except Exception:  # noqa: BLE001
                continue
            if not obj or obj.get_class().get_name() != "StaticMesh":
                continue
            pkg = path.split(".")[0]
            if pkg in seen:
                continue
            name = pkg.rsplit("/", 1)[-1].lower()
            for k in KEYS:
                if k in name:
                    found[k].append(path)
                    seen.add(pkg)
                    break
    return found


def put(sp, mesh_path, mat_path, x, y, base_z, yaw=0.0, scale=1.0):
    mesh = unreal.load_asset(mesh_path)
    if not mesh or mesh.get_class().get_name() != "StaticMesh":
        bump("missing")
        return False
    mat = unreal.load_asset(mat_path) if mat_path else None
    e = mesh.get_bounds().box_extent
    a = sp(unreal.StaticMeshActor, unreal.Vector(x, y, base_z + e.z * scale),
           unreal.Rotator(0.0, yaw, 0.0))
    if not a:
        bump("missing")
        return False
    comp = a.static_mesh_component
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_static_mesh(mesh)
    if mat:
        comp.set_material(0, mat)
    a.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    comp.set_collision_profile_name("BlockAll")
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    a.set_editor_property("tags", [TAG])
    return True


def ring(i, r0, r1, offset=0.0):
    a = math.radians(((i * GOLD) + offset) % 360.0)
    r = r0 + (r1 - r0) * (((i * 0.618033988749895) + 0.5) % 1.0)
    return r * math.cos(a), r * math.sin(a)


def main():
    if not unreal.EditorLevelLibrary.load_level(MAP):
        w("LOAD FAILED")
        return
    clean()
    sp = spawner()
    w("deck %.0f" % DECK)

    kit = discover()
    for k in KEYS:
        w("%-8s %d: %s" % (k, len(kit[k]),
                           ", ".join(p.rsplit("/", 1)[-1].split(".")[0] for p in kit[k][:5])))

    barrels = kit["barrel"] + kit["crate"] + kit["box"] + kit["chest"] + kit["sack"] + kit["bag"]
    for i in range(30):
        if not barrels:
            break
        x, y = ring(i, 1850.0, 2450.0, 18.0)
        if put(sp, barrels[i % len(barrels)], None, x, y, DECK, yaw=(i * 47.0) % 360.0):
            bump("barrels+crates")

    pots = kit["pot"] + kit["urn"] + kit["basket"] + kit["table"]
    for i in range(22):
        if not pots:
            break
        x, y = ring(i, 1300.0, 1600.0, 40.0)
        if put(sp, pots[i % len(pots)], None, x, y, DECK, yaw=(i * 61.0) % 360.0):
            bump("pots+tables")

    accents = kit["statue"] + kit["cart"] + kit["wheel"]
    for i in range(8):
        if not accents:
            break
        x, y = ring(i, 2500.0, 2850.0, 90.0)
        if put(sp, accents[i % len(accents)], C + "M_CC0_RoofPlanks.M_CC0_RoofPlanks",
               x, y, DECK, yaw=(i * 73.0) % 360.0):
            bump("carts+statues")

    rocks = kit["boulder"] + kit["rock"]
    for i in range(20):
        if not rocks:
            break
        x, y = ring(i, 3300.0, 3540.0, 7.0)
        if put(sp, rocks[i % len(rocks)], C + "M_CC0_Rock.M_CC0_Rock", x, y, DECK - 60.0,
               yaw=(i * 113.0) % 360.0, scale=1.7):
            bump("rim rocks")

    for i, bp in enumerate(kit["bridge"][:2]):
        x, y = ring(i, 4000.0, 4200.0, 225.0)
        if put(sp, bp, C + "M_CC0_RoofPlanks.M_CC0_RoofPlanks", x, y, DECK, yaw=225.0):
            bump("bridge")

    ok = unreal.EditorLevelLibrary.save_current_level()
    w("save: %s" % bool(ok))
    w("actors now %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))
    w("DRESS COUNTS %s" % count)


try:
    main()
except Exception as err:  # noqa: BLE001
    w("EXCEPTION %r" % err)

w("DRESS DONE")
