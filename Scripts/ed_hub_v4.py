"""V4 hub, authored from the kit's real shapes.

The earlier passes scaled every mesh uniformly on X and Y, so a 900x500 floor
slab became a 520x520 square, a 1474x1905 platform became a huge lump, and the
roofs were pitched monsters. This pass sizes each axis independently and uses
SM_Floor1_Big for paving and roofs, SM_AncientColumn_Big for the colonnade.

Plaza: radius 2500 paved disc, terrace body beneath it, colonnade at 1700,
lanterns at 1200, market arches at 2200, eight gabled houses at 2900, the
signal stone at (0, 1500), peaks at (2800, 500), Spire at (0, 3500).
"""
import math
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_v4_log.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

PHI = 1.618033988749895
SP = 100.0 * PHI
GOLD = 137.5077640500378
MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
PROPS = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/"
MOD = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/"
V = "/Game/VEIL/Materials/"
C = "/Game/VEIL/CC0/Materials/"
GROUND = 182021.14
PITCH = 17.0324
DECK = GROUND + 161.8033988749895 * (PHI ** 0.986)
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


def place(sp, mesh_path, mat_path, x, y, base_z, wx, wy, h,
          yaw=0.0, pitch=0.0, tags=(), centre_z=None):
    """Each axis is sized independently; base_z is where the piece stands."""
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        w("MISSING %s" % mesh_path)
        bump("missing")
        return False
    mat = unreal.load_asset(mat_path) if mat_path else None
    e = mesh.get_bounds().box_extent
    sx = wx / max(e.x * 2.0, 1.0)
    sy = wy / max(e.y * 2.0, 1.0)
    sz = h / max(e.z * 2.0, 1.0)
    z = base_z + h * 0.5 if centre_z is None else centre_z
    a = sp(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(pitch, yaw, 0.0))
    if not a:
        bump("missing")
        return False
    comp = a.static_mesh_component
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_static_mesh(mesh)
    if mat:
        comp.set_material(0, mat)
    a.set_actor_scale3d(unreal.Vector(sx, sy, sz))
    comp.set_collision_profile_name("BlockAll")
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    a.set_editor_property("tags", ["VEIL_AUTHORED"] + list(tags))
    return True


def light(sp, x, y, z, intensity=2800.0, radius=3600.0, colour=(1.0, 0.76, 0.42)):
    lamp = sp(unreal.PointLight, unreal.Vector(x, y, z), unreal.Rotator(0.0, 0.0, 0.0))
    if not lamp:
        return False
    comp = lamp.get_component_by_class(unreal.PointLightComponent)
    if not comp:
        return False
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_intensity(intensity)
    comp.set_attenuation_radius(radius)
    comp.set_light_color(unreal.LinearColor(colour[0], colour[1], colour[2], 1.0))
    comp.set_cast_shadows(True)
    lamp.set_editor_property("tags", ["VEIL_AUTHORED"])
    return True


FLOORS = [MOD + "SM_Floor1_Big.SM_Floor1_Big", MOD + "SM_Floor2_Big.SM_Floor2_Big",
          MOD + "SM_Floor3_Big.SM_Floor3_Big", MOD + "SM_Floor4_Big.SM_Floor4_Big",
          MOD + "SM_Floor5_Big.SM_Floor5_Big"]


def main():
    if not unreal.EditorLevelLibrary.load_level(MAP):
        w("LOAD FAILED")
        return
    w("level loaded")
    clean()
    sp = spawner()
    w("ground %.2f deck %.2f" % (GROUND, DECK))

    # ---------- terrace body under the plaza (nothing floats)
    if place(sp, MOD + "SM_Floor1_Big.SM_Floor1_Big", C + "M_CC0_Rock.M_CC0_Rock",
             0.0, 0.0, DECK - 420.0, 5600.0, 5600.0, 420.0, tags=("VEIL_PAVING",)):
        bump("terrace")

    # ---------- paving: 900x500 slabs, brick-offset rows, cut to a disc
    row = 0
    y = -2500.0
    while y <= 2500.0:
        off = 450.0 if row % 2 else 0.0
        x = -2700.0 + off
        while x <= 2700.0 + off:
            if (x ** 2 + y ** 2) ** 0.5 <= 2560.0:
                m = FLOORS[(row + int(x / 900)) % len(FLOORS)]
                if place(sp, m, C + "M_CC0_Pavement.M_CC0_Pavement",
                         x, y, DECK - 60.0, 940.0, 520.0, 60.0, tags=("VEIL_PAVING",)):
                    bump("paving")
            x += 900.0
        y += 500.0
        row += 1

    # ---------- colonnade: slender columns on a ring, capital on each
    ring = 1700.0
    for i in range(12):
        a = math.radians(i * 30.0)
        x, y = ring * math.cos(a), ring * math.sin(a)
        if place(sp, PROPS + "SM_AncientColumn_Big.SM_AncientColumn_Big",
                 V + "M_VEIL_Gold.M_VEIL_Gold", x, y, DECK, 100.0, 100.0, 660.0):
            bump("pillars")
        if place(sp, PROPS + "SM_AncientDecorArch.SM_AncientDecorArch",
                 V + "M_VEIL_Gold.M_VEIL_Gold", x, y, DECK + 640.0,
                 260.0, 260.0, 130.0, yaw=i * 30.0):
            bump("glyphs")
        light(sp, x, y, DECK + 720.0, 1600.0, 2400.0)

    # ---------- market arches: tall, cloth-draped
    for i in range(8):
        a = math.radians(i * 45.0 + 22.5)
        r = 2200.0
        if place(sp, PROPS + "SM_ArchD.SM_ArchD", C + "M_CC0_Cloth.M_CC0_Cloth",
                 r * math.cos(a), r * math.sin(a), DECK,
                 360.0, 160.0, 640.0, yaw=i * 45.0 + 22.5 + 90.0):
            bump("stalls")

    # ---------- houses: four walls, a gable at the bible pitch, a lit doorway
    houses = 8
    for i in range(houses):
        ang = i * (360.0 / houses) + 22.5
        a = math.radians(ang)
        hx, hy = 2900.0 * math.cos(a), 2900.0 * math.sin(a)
        half = 300.0
        wall_h = 320.0
        walls = 0
        for dx, dy, wx, wy in ((0.0, -half, half * 2, 50.0), (0.0, half, half * 2, 50.0),
                               (-half, 0.0, 50.0, half * 2), (half, 0.0, 50.0, half * 2)):
            # the whole house is rotated to face the plaza
            ca, sa = math.cos(math.radians(ang)), math.sin(math.radians(ang))
            px = hx + dx * ca - dy * sa
            py = hy + dx * sa + dy * ca
            turn = ang + (90.0 if wx < wy else 0.0)
            if place(sp, MOD + "SM_Wall1_Small.SM_Wall1_Small",
                     C + "M_CC0_Sandstone.M_CC0_Sandstone",
                     px, py, DECK, wx, wy, wall_h, yaw=turn):
                walls += 1
        if walls == 4:
            bump("houses")
        bump("walls", walls)
        # gable: two slabs meeting at a ridge, thrown up at the bible pitch
        for side in (-1.0, 1.0):
            place(sp, MOD + "SM_Floor1_Big.SM_Floor1_Big",
                  C + "M_CC0_RoofPlanks.M_CC0_RoofPlanks",
                  hx + side * half * 0.5 * math.cos(math.radians(ang)),
                  hy + side * half * 0.5 * math.sin(math.radians(ang)),
                  DECK + wall_h + 30.0, half * 1.15, half * 2.1, 24.0,
                  yaw=ang, pitch=-side * PITCH, centre_z=DECK + wall_h + 30.0)
        light(sp, hx, hy, DECK + wall_h * 0.6, 1300.0, 1800.0, (1.0, 0.82, 0.55))

    # ---------- lanterns inside the colonnade
    for i in range(8):
        a = math.radians(i * 45.0 + 22.5)
        r = 1200.0
        x, y = r * math.cos(a), r * math.sin(a)
        if place(sp, PROPS + "SM_LittleShrine.SM_LittleShrine",
                 V + "M_VEIL_Signal.M_VEIL_Signal", x, y, DECK, 150.0, 150.0, 260.0):
            bump("shrine")
        light(sp, x, y, DECK + 300.0)

    # ---------- the signal stone, off the spawn point
    if place(sp, PROPS + "SM_AncientMonolith.SM_AncientMonolith",
             V + "M_VEIL_Signal.M_VEIL_Signal", 0.0, 1500.0, DECK, 220.0, 220.0, 620.0):
        bump("monument")
    light(sp, 0.0, 1500.0, DECK + 700.0, 6000.0, 5200.0, (0.75, 0.92, 1.0))
    light(sp, 0.0, 0.0, DECK + 900.0, 3600.0, 4400.0)

    # ---------- stairs down from the south rim of the terrace
    for i in range(8):
        if place(sp, PROPS + "SM_StairSmall.SM_StairSmall",
                 C + "M_CC0_Sandstone.M_CC0_Sandstone",
                 0.0, -(2620.0 + i * 260.0), DECK - (i + 1) * 55.0,
                 420.0, 280.0, 70.0):
            bump("stairs")

    # ---------- crystal peaks at authority (2800, 500)
    for i in range(5):
        a = math.radians(i * 72.0)
        r = 300.0 + i * 130.0
        if place(sp, PROPS + "SM_AncientMonolith.SM_AncientMonolith",
                 V + "M_VEIL_Signal.M_VEIL_Signal",
                 2800.0 + r * math.cos(a), 500.0 + r * math.sin(a), GROUND,
                 170.0, 170.0, 420.0 + i * 90.0, yaw=i * 72.0):
            bump("peaks")

    # ---------- the great Spire on the horizon at authority (0, 3500)
    z = GROUND
    for tier in range(5):
        h = 380.0 - tier * 30.0
        width = 520.0 - tier * 80.0
        if place(sp, PROPS + "SM_AncientColumn_Big.SM_AncientColumn_Big",
                 V + "M_VEIL_Gold.M_VEIL_Gold", 0.0, 3500.0, z,
                 width, width, h, yaw=tier * 36.0):
            bump("spire")
        z += h * 0.92
    light(sp, 0.0, 3500.0, GROUND + 1500.0, 9000.0, 7000.0, (1.0, 0.85, 0.55))

    ok = unreal.EditorLevelLibrary.save_current_level()
    w("save: %s" % bool(ok))
    w("actors now %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))
    w("COUNTS %s" % count)


try:
    main()
except Exception as err:  # noqa: BLE001
    w("EXCEPTION %r" % err)

w("V4 DONE")
