"""Author the Hub in the editor map with the game's exact phi law.

law: Spacing = 100*phi, Reach(x) = Spacing * phi^x, golden angle 137.5077640500378
Every piece is tagged VEIL_AUTHORED (and VEIL_PAVING for the deck) so the runtime
builder can recognise the authored world instead of duplicating it.
"""
import math
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_build_log.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

PHI = 1.618033988749895
GOLDEN = 137.5077640500378
SPACING = 100.0 * PHI
MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
KIT = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/"
MAT = "/Game/VEIL/Materials/"
CLOTH = "/Game/VEIL/CC0/Materials/M_CC0_Cloth.M_CC0_Cloth"
GROUND = 182021.14
counts = {}


def reach(x):
    return SPACING * (PHI ** x)


def bump(k, n=1):
    counts[k] = counts.get(k, 0) + n


def clean():
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    removed = 0
    for a in actors:
        try:
            if a.actor_has_tag("VEIL_AUTHORED"):
                unreal.EditorLevelLibrary.destroy_actor(a)
                removed += 1
        except Exception:  # noqa: BLE001
            pass
    w("removed previous authored actors: %d" % removed)


def spawn():
    try:
        sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        if sub:
            return sub.spawn_actor_from_class
    except Exception:  # noqa: BLE001
        pass
    return unreal.EditorLevelLibrary.spawn_actor_from_class


def place(spawner, mesh_path, mat_path, x, y, z, width, height, yaw=0.0, tags=()):
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        w("MISSING %s" % mesh_path)
        bump("missing")
        return False
    mat = unreal.load_asset(mat_path) if mat_path else None
    ext = mesh.get_bounds().box_extent
    sx = width / max(max(ext.x, ext.y) * 2.0, 1.0)
    sz = height / max(ext.z * 2.0, 1.0)
    a = spawner(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0.0, yaw, 0.0))
    if not a:
        bump("missing")
        w("SPAWN FAILED %s" % mesh_path)
        return False
    c = a.static_mesh_component
    c.set_mobility(unreal.ComponentMobility.MOVABLE)   # must precede the mesh
    c.set_static_mesh(mesh)
    if mat:
        c.set_material(0, mat)
    a.set_actor_scale3d(unreal.Vector(sx, sx, sz))
    c.set_collision_profile_name("BlockAll")
    c.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    a.set_editor_property("tags", ["VEIL_AUTHORED"] + list(tags))
    return True


def main():
    if not unreal.EditorLevelLibrary.load_level(MAP):
        w("LEVEL LOAD FAILED")
        return
    w("level loaded")
    clean()
    sp = spawn()

    deck = GROUND + reach(0.986)      # 26 m terrace, exact law
    w("deck top %.2f (ground %.2f)" % (deck, GROUND))

    # terrace: 3x3 stone paving, solid
    for ix in (-1, 0, 1):
        for iy in (-1, 0, 1):
            if place(sp, KIT + "SM_Platform.SM_Platform", MAT + "M_VEIL_Stone.M_VEIL_Stone",
                     ix * 900.0, iy * 900.0, GROUND, 900.0, reach(0.986),
                     tags=("VEIL_PAVING",)):
                bump("paving")
    w("paving %s" % counts.get("paving", 0))

    # twelve gold pillars at Spacing*sqrt(i), inside the market
    for i in range(1, 13):
        a = math.radians(i * GOLDEN)
        r = SPACING * math.sqrt(i)
        if place(sp, KIT + "SM_AncientColumn_Big.SM_AncientColumn_Big",
                 MAT + "M_VEIL_Gold.M_VEIL_Gold",
                 r * math.cos(a), r * math.sin(a), deck,
                 reach(-0.4), reach(0.9), yaw=i * GOLDEN):
            bump("pillars")
    w("pillars %s" % counts.get("pillars", 0))

    # houses on the authority centre (900, 0)
    for i in range(1, 9):
        a = math.radians(i * GOLDEN)
        r = SPACING * PHI * math.sqrt(i)
        if place(sp, KIT + "SM_HlafDome.SM_HlafDome", MAT + "M_VEIL_Stone.M_VEIL_Stone",
                 900.0 + r * math.cos(a), r * math.sin(a), deck,
                 reach(-0.1), reach(0.55), yaw=i * GOLDEN):
            bump("houses")
    w("houses %s" % counts.get("houses", 0))

    # stalls, same authority centre, interleaved at r/phi, blue canvas
    for i in range(1, 9):
        a = math.radians(i * GOLDEN)
        r = SPACING * PHI * math.sqrt(i) / PHI
        if place(sp, KIT + "SM_ArchD.SM_ArchD", CLOTH,
                 900.0 + r * math.cos(a), r * math.sin(a), deck,
                 reach(-0.35), reach(0.32), yaw=i * GOLDEN + 90.0):
            bump("stalls")
    w("stalls %s" % counts.get("stalls", 0))

    # lanterns: signal shrines plus real point lights
    for i in range(1, 9):
        a = math.radians(i * GOLDEN)
        r = SPACING * PHI * PHI * PHI * PHI
        x, y = r * math.cos(a), r * math.sin(a)
        place(sp, KIT + "SM_LittleShrine.SM_LittleShrine",
              MAT + "M_VEIL_Signal.M_VEIL_Signal", x, y, deck, reach(-0.55), reach(0.5))
        lamp = sp(unreal.PointLight, unreal.Vector(x, y, deck + reach(0.62)),
                  unreal.Rotator(0.0, 0.0, 0.0))
        if lamp:
            comp = lamp.get_component_by_class(unreal.PointLightComponent)
            if comp:
                comp.set_mobility(unreal.ComponentMobility.MOVABLE)
                comp.set_intensity(1400.0)
                comp.set_attenuation_radius(2600.0)
                comp.set_light_color(unreal.LinearColor(1.0, 0.78, 0.42, 1.0))
                lamp.set_editor_property("tags", ["VEIL_AUTHORED"])
                bump("lanterns")
    w("lanterns %s" % counts.get("lanterns", 0))

    # five crystal peaks at the authority coordinate (2800, 500)
    for i in range(1, 6):
        a = math.radians(i * GOLDEN)
        r = SPACING * math.sqrt(i)
        if place(sp, KIT + "SM_AncientMonolith.SM_AncientMonolith",
                 MAT + "M_VEIL_Signal.M_VEIL_Signal",
                 2800.0 + r * math.cos(a), 500.0 + r * math.sin(a), GROUND,
                 reach(-0.5), reach(0.9), yaw=i * GOLDEN):
            bump("peaks")
    w("peaks %s" % counts.get("peaks", 0))

    ok = unreal.EditorLevelLibrary.save_current_level()
    w("save_current_level: %s" % bool(ok))
    w("actors now %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))
    total = sum(counts.get(k, 0) for k in ("paving", "pillars", "houses", "stalls", "peaks"))
    w("COUNTS %s" % counts)
    w("HUB AUTHORED TOTAL %d" % total)


try:
    main()
except Exception as err:  # noqa: BLE001
    w("EXCEPTION %r" % err)

w("BUILD DONE")
