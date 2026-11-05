"""Build the Hub to the painting: pavement, pitched houses, glyphs, stairs, Spire.

Everything is real kit geometry with real materials, authored in the editor and
saved into Lv_Mountain_Tops_VT. No primitives, no runtime spawns.
"""
import math
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_v2_log.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

PHI = 1.618033988749895
GOLD = 137.5077640500378
SP = 100.0 * PHI
MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
PROPS = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/"
MOD = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/"
V = "/Game/VEIL/Materials/"
C = "/Game/VEIL/CC0/Materials/"
GROUND = 182021.14
count = {}


def reach(x):
    return SP * (PHI ** x)


def bump(k, n=1):
    count[k] = count.get(k, 0) + n


def clean():
    n = 0
    for a in unreal.EditorLevelLibrary.get_all_level_actors():
        try:
            if a.actor_has_tag("VEIL_AUTHORED"):
                unreal.EditorLevelLibrary.destroy_actor(a)
                n += 1
        except Exception:  # noqa: BLE001
            pass
    w("cleared %d authored actors" % n)


def spawner():
    try:
        s = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        if s:
            return s.spawn_actor_from_class
    except Exception:  # noqa: BLE001
        pass
    return unreal.EditorLevelLibrary.spawn_actor_from_class


def place(sp, mesh_path, mat_path, x, y, z, width, height, yaw=0.0, pitch=0.0, tags=()):
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        w("MISSING %s" % mesh_path)
        bump("missing")
        return False
    mat = unreal.load_asset(mat_path) if mat_path else None
    e = mesh.get_bounds().box_extent
    sx = width / max(max(e.x, e.y) * 2.0, 1.0)
    sz = height / max(e.z * 2.0, 1.0)
    a = sp(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(pitch, yaw, 0.0))
    if not a:
        bump("missing")
        return False
    c = a.static_mesh_component
    c.set_mobility(unreal.ComponentMobility.MOVABLE)
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
        w("LOAD FAILED")
        return
    w("level loaded")
    clean()
    sp = spawner()
    deck = GROUND + reach(0.986)
    w("ground %.2f deck %.2f" % (GROUND, deck))

    # ---- pavement: stone, reads as paving
    for ix in (-1, 0, 1):
        for iy in (-1, 0, 1):
            if place(sp, PROPS + "SM_Platform.SM_Platform", C + "M_CC0_Pavement.M_CC0_Pavement",
                     ix * 900.0, iy * 900.0, GROUND, 900.0, reach(0.986), tags=("VEIL_PAVING",)):
                bump("paving")

    # ---- pillars with phi glyph panels
    for i in range(1, 13):
        a = math.radians(i * GOLD)
        r = SP * math.sqrt(i)
        x, y = r * math.cos(a), r * math.sin(a)
        if place(sp, PROPS + "SM_AncientColumn_Big.SM_AncientColumn_Big",
                 V + "M_VEIL_Gold.M_VEIL_Gold", x, y, deck, reach(-0.4), reach(0.9),
                 yaw=i * GOLD):
            bump("pillars")
        if place(sp, PROPS + "SM_Platform.SM_Platform", V + "M_VEIL_Signal.M_VEIL_Signal",
                 x, y, deck + reach(0.72), 120.0, 8.0, yaw=i * GOLD):
            bump("glyphs")

    # ---- houses: stone walls and a pitched roof, not a golden dome
    for i in range(1, 9):
        a = math.radians(i * GOLD)
        r = SP * PHI * math.sqrt(i)
        hx, hy = 900.0 + r * math.cos(a), r * math.sin(a)
        half = reach(-0.25)
        walls = 0
        for dx, dy, yaw in ((0, half, 0), (0, -half, 0), (half, 0, 90), (-half, 0, 90)):
            if place(sp, MOD + "SM_Wall1_Small.SM_Wall1_Small",
                     C + "M_CC0_Sandstone.M_CC0_Sandstone",
                     hx + dx, hy + dy, deck, half * 2.4, reach(0.42), yaw=yaw):
                walls += 1
        if walls == 4:
            bump("houses")
        bump("walls", walls)
        # pitched roof: timber planks laid over the walls
        if place(sp, PROPS + "SM_Platform.SM_Platform", C + "M_CC0_RoofPlanks.M_CC0_RoofPlanks",
                 hx, hy, deck + reach(0.45), half * 3.0, 60.0,
                 yaw=i * GOLD, pitch=17.0324):
            bump("roofs")

    # ---- stalls: blue canvas
    for i in range(1, 9):
        a = math.radians(i * GOLD)
        r = SP * PHI * math.sqrt(i) / PHI
        if place(sp, PROPS + "SM_ArchD.SM_ArchD", C + "M_CC0_Cloth.M_CC0_Cloth",
                 900.0 + r * math.cos(a), r * math.sin(a), deck,
                 reach(-0.35), reach(0.32), yaw=i * GOLD + 90.0):
            bump("stalls")

    # ---- lanterns with real lights
    for i in range(1, 9):
        a = math.radians(i * GOLD)
        r = SP * PHI * PHI * PHI * PHI
        x, y = r * math.cos(a), r * math.sin(a)
        place(sp, PROPS + "SM_LittleShrine.SM_LittleShrine", V + "M_VEIL_Signal.M_VEIL_Signal",
              x, y, deck, reach(-0.55), reach(0.5))
        lamp = sp(unreal.PointLight, unreal.Vector(x, y, deck + reach(0.62)),
                  unreal.Rotator(0.0, 0.0, 0.0))
        if lamp:
            comp = lamp.get_component_by_class(unreal.PointLightComponent)
            if comp:
                comp.set_mobility(unreal.ComponentMobility.MOVABLE)
                comp.set_intensity(1400.0)
                comp.set_attenuation_radius(2600.0)
                comp.set_light_color(unreal.LinearColor(1.0, 0.75, 0.35, 1.0))
                lamp.set_editor_property("tags", ["VEIL_AUTHORED"])
                bump("lanterns")

    # ---- crystal peaks at authority (2800, 500)
    for i in range(1, 6):
        a = math.radians(i * GOLD)
        r = SP * math.sqrt(i)
        if place(sp, PROPS + "SM_AncientMonolith.SM_AncientMonolith",
                 V + "M_VEIL_Signal.M_VEIL_Signal",
                 2800.0 + r * math.cos(a), 500.0 + r * math.sin(a), GROUND,
                 reach(-0.5), reach(1.1), yaw=i * GOLD):
            bump("peaks")

    # ---- stairs down from the terrace edge
    for i in range(1, 7):
        if place(sp, PROPS + "SM_StairSmall.SM_StairSmall", C + "M_CC0_Sandstone.M_CC0_Sandstone",
                 0.0, -1250.0 - i * reach(-0.2), deck - i * reach(0.05),
                 reach(-0.2), reach(-0.05)):
            bump("stairs")

    # ---- the great Spire on the horizon at authority (0, 3500)
    sx0, sy0 = 0.0, 3500.0
    for tier in range(5):
        h = reach(0.5) + tier * reach(0.05)
        width = reach(0.55) - tier * reach(-0.05)
        if place(sp, PROPS + "SM_AncientColumn_Big.SM_AncientColumn_Big",
                 V + "M_VEIL_Gold.M_VEIL_Gold", sx0, sy0,
                 GROUND + PHI_Constants_z(tier, h), width, h, yaw=tier * GOLD):
            bump("spire")

    ok = unreal.EditorLevelLibrary.save_current_level()
    w("save: %s" % bool(ok))
    w("actors now %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))
    w("COUNTS %s" % count)


def PHI_Constants_z(tier, h):
    return tier * h


try:
    main()
except Exception as err:  # noqa: BLE001
    w("EXCEPTION %r" % err)

w("V2 DONE")
