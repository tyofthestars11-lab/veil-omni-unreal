"""Author the Hub on the real map inside the Unreal editor, with flushed evidence."""
import math
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_build_log.txt")


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


open(OUT, "w", encoding="utf-8").close()

import unreal  # noqa: E402

PHI = 1.618033988749895
GOLDEN = 137.5077640500378
MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
KIT = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/"
MAT = "/Game/VEIL/Materials/"
CLOTH = "/Game/VEIL/CC0/Materials/M_CC0_Cloth.M_CC0_Cloth"
GROUND = 182021.14
SPACING = 100.0 * PHI

counts = {"slabs": 0, "pillars": 0, "houses": 0, "stalls": 0, "lanterns": 0, "missing": 0}


def spacing(x):
    return SPACING * (PHI ** (x - 1.0))


def spawner():
    try:
        sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        if sub:
            return sub.spawn_actor_from_class
    except Exception as err:  # noqa: BLE001
        w("subsystem unavailable: %r" % err)
    return unreal.EditorLevelLibrary.spawn_actor_from_class


def place(spawn, mesh_path, mat_path, x, y, z, width, height, yaw=0.0):
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        counts["missing"] += 1
        w("MISSING %s" % mesh_path)
        return False
    mat = unreal.load_asset(mat_path) if mat_path else None
    b = mesh.get_bounds()
    ext = b.box_extent
    sx = width / max(max(ext.x, ext.y) * 2.0, 1.0)
    sz = height / max(ext.z * 2.0, 1.0)
    actor = spawn(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0.0, yaw, 0.0))
    if not actor:
        counts["missing"] += 1
        w("SPAWN FAILED %s" % mesh_path)
        return False
    comp = actor.static_mesh_component
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_static_mesh(mesh)
    if mat:
        comp.set_material(0, mat)
    actor.set_actor_scale3d(unreal.Vector(sx, sx, sz))
    comp.set_collision_profile_name("BlockAll")
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    return True


def main():
    w("start")
    try:
        opened = unreal.EditorLevelLibrary.load_level(MAP)
        w("load_level: %s" % bool(opened))
    except Exception as err:  # noqa: BLE001
        w("load_level FAILED %r" % err)
        return
    try:
        world = unreal.EditorLevelLibrary.get_editor_world()
        w("world: %s" % (world.get_name() if world else "none"))
    except Exception as err:  # noqa: BLE001
        w("world FAILED %r" % err)

    spawn = spawner()
    deck = GROUND + spacing(0.5)
    w("deck %.2f" % deck)

    for ix in (-1, 0, 1):
        for iy in (-1, 0, 1):
            if place(spawn, KIT + "SM_Platform.SM_Platform", MAT + "M_VEIL_Stone.M_VEIL_Stone",
                     ix * 900.0, iy * 900.0, GROUND, 900.0, spacing(0.42)):
                counts["slabs"] += 1
    w("slabs %d" % counts["slabs"])

    for i in range(1, 13):
        a = math.radians(i * GOLDEN)
        r = SPACING * math.sqrt(i)
        if place(spawn, KIT + "SM_AncientColumn_Big.SM_AncientColumn_Big",
                 MAT + "M_VEIL_Gold.M_VEIL_Gold",
                 r * math.cos(a), r * math.sin(a), deck,
                 spacing(-0.4), spacing(0.9), yaw=i * GOLDEN):
            counts["pillars"] += 1
    w("pillars %d" % counts["pillars"])

    for i in range(1, 9):
        a = math.radians(i * GOLDEN)
        r = SPACING * PHI * math.sqrt(i)
        if place(spawn, KIT + "SM_HlafDome.SM_HlafDome", MAT + "M_VEIL_Stone.M_VEIL_Stone",
                 900.0 + r * math.cos(a), r * math.sin(a), deck,
                 spacing(-0.1), spacing(0.55), yaw=i * GOLDEN):
            counts["houses"] += 1
    w("houses %d" % counts["houses"])

    for i in range(1, 9):
        a = math.radians(i * GOLDEN)
        r = SPACING * PHI * math.sqrt(i) / PHI
        if place(spawn, KIT + "SM_ArchD.SM_ArchD", CLOTH,
                 900.0 + r * math.cos(a), r * math.sin(a), deck,
                 spacing(-0.35), spacing(0.32), yaw=i * GOLDEN + 90.0):
            counts["stalls"] += 1
    w("stalls %d" % counts["stalls"])

    for i in range(1, 9):
        a = math.radians(i * GOLDEN)
        r = SPACING * (PHI ** 3.0)
        x, y = r * math.cos(a), r * math.sin(a)
        place(spawn, KIT + "SM_LittleShrine.SM_LittleShrine",
              MAT + "M_VEIL_Signal.M_VEIL_Signal", x, y, deck, spacing(-0.55), spacing(0.5))
        lamp = spawn(unreal.PointLight, unreal.Vector(x, y, deck + spacing(0.62)),
                     unreal.Rotator(0.0, 0.0, 0.0))
        if lamp:
            comp = lamp.get_component_by_class(unreal.PointLightComponent)
            if comp:
                comp.set_mobility(unreal.ComponentMobility.MOVABLE)
                comp.set_intensity(1400.0)
                comp.set_attenuation_radius(2600.0)
                comp.set_light_color(unreal.LinearColor(1.0, 0.78, 0.42, 1.0))
                counts["lanterns"] += 1
    w("lanterns %d" % counts["lanterns"])

    try:
        saved = unreal.EditorLevelLibrary.save_current_level()
        w("save_current_level: %s" % bool(saved))
    except Exception as err:  # noqa: BLE001
        w("save FAILED %r" % err)

    try:
        w("actors now %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))
    except Exception as err:  # noqa: BLE001
        w("count actors FAILED %r" % err)

    w("COUNTS %s" % counts)


try:
    main()
except Exception as err:  # noqa: BLE001
    w("EXCEPTION %r" % err)

w("BUILD DONE")
