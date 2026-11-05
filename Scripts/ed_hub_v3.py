"""V3 hub: base-anchored placement and a town that actually reads as a town.

Two faults are fixed here, both measurable:
  * every piece used to be spawned with its CENTRE at the deck, so half of each
    wall, column and house was buried in the ground;
  * the golden-angle spiral used SP*sqrt(i) radii, which packed a twelve-column
    colonnade, eight houses and eight stalls into a 560-unit circle.

Radii now follow phi powers on a real plaza: paving disc 1800, lantern ring 1150,
colonnade 1600, stalls 2050, houses 2700, horizon Spire at (0, 3500).
"""
import math
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_v3_log.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

PHI = 1.618033988749895
SP = 100.0 * PHI
MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
PROPS = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/"
MOD = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/"
V = "/Game/VEIL/Materials/"
C = "/Game/VEIL/CC0/Materials/"
GROUND = 182021.14
PITCH = 17.0324          # the bible roof pitch
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


def place(sp, mesh_path, mat_path, x, y, base_z, width, height,
          yaw=0.0, pitch=0.0, roll=0.0, tags=()):
    """x,y,base_z is the FOOTPRINT CORNER-FREE base: the piece stands on base_z."""
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        w("MISSING %s" % mesh_path)
        bump("missing")
        return False
    mat = unreal.load_asset(mat_path) if mat_path else None
    e = mesh.get_bounds().box_extent
    sx = width / max(max(e.x, e.y) * 2.0, 1.0)
    sz = height / max(e.z * 2.0, 1.0)
    c = base_z + height * 0.5          # centre so the piece stands, not sinks
    a = sp(unreal.StaticMeshActor, unreal.Vector(x, y, c),
           unreal.Rotator(pitch, yaw, roll))
    if not a:
        bump("missing")
        return False
    comp = a.static_mesh_component
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_static_mesh(mesh)
    if mat:
        comp.set_material(0, mat)
    a.set_actor_scale3d(unreal.Vector(sx, sx, sz))
    comp.set_collision_profile_name("BlockAll")
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    a.set_editor_property("tags", ["VEIL_AUTHORED"] + list(tags))
    return True


def light(sp, x, y, z, intensity=2600.0, radius=3400.0):
    lamp = sp(unreal.PointLight, unreal.Vector(x, y, z), unreal.Rotator(0.0, 0.0, 0.0))
    if not lamp:
        return False
    comp = lamp.get_component_by_class(unreal.PointLightComponent)
    if not comp:
        return False
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_intensity(intensity)
    comp.set_attenuation_radius(radius)
    comp.set_light_color(unreal.LinearColor(1.0, 0.76, 0.42, 1.0))
    comp.set_cast_shadows(True)
    lamp.set_editor_property("tags", ["VEIL_AUTHORED"])
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

    # ---------- plaza paving: a real disc of stone, 7x7 minus the corners
    tile = 600.0
    paved = 0
    for ix in range(-3, 4):
        for iy in range(-3, 4):
            x, y = ix * tile, iy * tile
            if math.hypot(x, y) > 2100.0:
                continue
            if place(sp, PROPS + "SM_Platform.SM_Platform",
                     C + "M_CC0_Pavement.M_CC0_Pavement",
                     x, y, deck - 24.0, tile - 6.0, 24.0, tags=("VEIL_PAVING",)):
                paved += 1
    bump("paving", paved)

    # ---------- colonnade: twelve columns on a ring, evenly spaced
    ring = 1600.0
    p_h = reach(2.4)
    p_w = reach(-0.4)
    for i in range(12):
        a = math.radians(i * 30.0)
        x, y = ring * math.cos(a), ring * math.sin(a)
        if place(sp, PROPS + "SM_AncientColumn_Big.SM_AncientColumn_Big",
                 V + "M_VEIL_Gold.M_VEIL_Gold", x, y, deck, p_w, p_h,
                 yaw=i * 30.0):
            bump("pillars")
        if place(sp, PROPS + "SM_Platform.SM_Platform", V + "M_VEIL_Signal.M_VEIL_Signal",
                 x, y, deck + p_h, 150.0, 12.0, yaw=i * 30.0):
            bump("glyphs")

    # ---------- houses: four walls and a real pitched gable, facing the plaza
    houses = 8
    for i in range(houses):
        a = math.radians(i * (360.0 / houses) + 22.5)
        hx, hy = 2700.0 * math.cos(a), 2700.0 * math.sin(a)
        yaw = i * (360.0 / houses) + 22.5 + 90.0
        half = 260.0
        wall_h = 300.0
        walls = 0
        for dx, dy, wy in ((0, -half, 0), (0, half, 0), (-half, 0, 90), (half, 0, 90)):
            if place(sp, MOD + "SM_Wall1_Small.SM_Wall1_Small",
                     C + "M_CC0_Sandstone.M_CC0_Sandstone",
                     hx + dx, hy + dy, deck, half * 2.0 + 60.0, wall_h, yaw=yaw + wy):
                walls += 1
        if walls == 4:
            bump("houses")
        bump("walls", walls)
        # gable: two slabs meeting at a ridge, at the bible pitch
        rise = half * math.tan(math.radians(PITCH))
        for sx, p in ((-half * 0.5, PITCH), (half * 0.5, -PITCH)):
            place(sp, PROPS + "SM_Platform.SM_Platform",
                  C + "M_CC0_RoofPlanks.M_CC0_RoofPlanks",
                  hx + sx, hy, deck + wall_h + rise - 26.0,
                  half * 1.35, 26.0, yaw=yaw, pitch=p)

    # ---------- market stalls on their own ring
    for i in range(8):
        a = math.radians(i * 45.0)
        r = 2050.0
        if place(sp, PROPS + "SM_ArchD.SM_ArchD", C + "M_CC0_Cloth.M_CC0_Cloth",
                 r * math.cos(a), r * math.sin(a), deck,
                 reach(-0.30), reach(0.36), yaw=i * 45.0 + 90.0):
            bump("stalls")

    # ---------- lanterns inside the colonnade
    for i in range(8):
        a = math.radians(i * 45.0 + 22.5)
        r = 1150.0
        x, y = r * math.cos(a), r * math.sin(a)
        if place(sp, PROPS + "SM_LittleShrine.SM_LittleShrine",
                 V + "M_VEIL_Signal.M_VEIL_Signal",
                 x, y, deck, reach(-0.55), reach(0.5)):
            bump("shrine")
        if light(sp, x, y, deck + reach(0.55)):
            bump("lanterns")

    # ---------- the signal stone at the centre of the plaza
    place(sp, PROPS + "SM_AncientMonolith.SM_AncientMonolith",
          V + "M_VEIL_Signal.M_VEIL_Signal", 0.0, 0.0, deck, reach(-0.55), reach(1.05))
    bump("monument")
    light(sp, 0.0, 0.0, deck + reach(1.1), 5200.0, 5200.0)

    # ---------- stairs down from the south rim
    rim = 2350.0
    for i in range(7):
        if place(sp, PROPS + "SM_StairSmall.SM_StairSmall",
                 C + "M_CC0_Sandstone.M_CC0_Sandstone",
                 0.0, -(rim + i * 230.0), deck - (i + 1) * 42.0,
                 240.0, 62.0):
            bump("stairs")

    # ---------- crystal peaks at authority (2800, 500)
    for i in range(5):
        a = math.radians(i * 72.0)
        r = 260.0 * (i + 1) * 0.5
        if place(sp, PROPS + "SM_AncientMonolith.SM_AncientMonolith",
                 V + "M_VEIL_Signal.M_VEIL_Signal",
                 2800.0 + r * math.cos(a), 500.0 + r * math.sin(a), GROUND,
                 reach(-0.45), reach(1.2), yaw=i * 72.0):
            bump("peaks")

    # ---------- the great Spire on the horizon at authority (0, 3500)
    for tier in range(5):
        h = reach(0.9) + tier * reach(0.16)
        width = reach(0.5) - tier * reach(-0.2)
        if place(sp, PROPS + "SM_AncientColumn_Big.SM_AncientColumn_Big",
                 V + "M_VEIL_Gold.M_VEIL_Gold", 0.0, 3500.0,
                 GROUND + tier * reach(0.9) * 0.92, width, h, yaw=tier * 36.0):
            bump("spire")

    ok = unreal.EditorLevelLibrary.save_current_level()
    w("save: %s" % bool(ok))
    w("actors now %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))
    w("COUNTS %s" % count)


try:
    main()
except Exception as err:  # noqa: BLE001
    w("EXCEPTION %r" % err)

w("V3 DONE")
