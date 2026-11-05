"""V6 hub: modular, natural scale, standing above the measured rock.

Two hard measurements drive this pass:

  * the map's own rock runs 182041 .. 189158 inside +/-2.8 km of the origin,
    highest at (700, 2100), so a town at the old 182281 deck was buried in it;
  * the Ancient Kit is a modular set: floors 900x500x20, walls 905x50x400,
    columns 70x70x530, shrine 189x190x251, monolith 233x91x301.

Everything here is placed at scale 1 on the kit's own grid, anchored by its own
height, so nothing is stretched and nothing floats. The deck stands at 189400 on
a stone mesa carried down to 181800, below the lowest rock.
"""
import math
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_v6_log.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

PHI = 1.618033988749895
SP = 100.0 * PHI                       # 161.8033988749895
MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
PROPS = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/"
MOD = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/"
V = "/Game/VEIL/Materials/"
C = "/Game/VEIL/CC0/Materials/"
ROCK_LOW = 182041.0
ROCK_HIGH = 189158.0
DECK = 189400.0
BASE = 181800.0
PITCH = 17.0324
count = {}


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


def put(sp, mesh_path, mat_path, x, y, base_z, yaw=0.0, pitch=0.0,
        tags=(), centre_z=None, sx=1.0, sy=1.0, sz=1.0):
    """Natural-size placement: the mesh's own height decides where it stands."""
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        w("MISSING %s" % mesh_path)
        bump("missing")
        return False
    mat = unreal.load_asset(mat_path) if mat_path else None
    e = mesh.get_bounds().box_extent
    h = max(e.z * 2.0, 1.0) * sz
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


def block(sp, mesh_path, mat_path, x, y, base_z, wx, wy, h, yaw=0.0, tags=()):
    """For the few pieces that must be a given size (the mesa, the terrace rim)."""
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        bump("missing")
        return False
    mat = unreal.load_asset(mat_path) if mat_path else None
    e = mesh.get_bounds().box_extent
    a = sp(unreal.StaticMeshActor, unreal.Vector(x, y, base_z + h * 0.5),
           unreal.Rotator(0.0, yaw, 0.0))
    if not a:
        bump("missing")
        return False
    comp = a.static_mesh_component
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_static_mesh(mesh)
    if mat:
        comp.set_material(0, mat)
    a.set_actor_scale3d(unreal.Vector(wx / max(e.x * 2, 1.0), wy / max(e.y * 2, 1.0),
                                      h / max(e.z * 2, 1.0)))
    comp.set_collision_profile_name("BlockAll")
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    a.set_editor_property("tags", ["VEIL_AUTHORED"] + list(tags))
    return True


def light(sp, x, y, z, intensity=3000.0, radius=3600.0, colour=(1.0, 0.76, 0.42)):
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
    w("rock %.0f..%.0f deck %.0f" % (ROCK_LOW, ROCK_HIGH, DECK))

    # ---------- the mesa: a stone plateau carried down past the lowest rock
    if block(sp, MOD + "SM_Floor1_Big.SM_Floor1_Big", C + "M_CC0_Rock.M_CC0_Rock",
             0.0, 0.0, BASE, 8200.0, 8200.0, DECK - BASE, tags=("VEIL_PAVING",)):
        bump("mesa")

    # ---------- paving: the kit's 900x500 floor modules, brick offset
    row = 0
    y = -3600.0
    while y <= 3600.0:
        off = 450.0 if row % 2 else 0.0
        x = -3600.0 + off
        while x <= 3600.0 + off:
            if (x ** 2 + y ** 2) ** 0.5 <= 3650.0:
                m = FLOORS[(row + int(x / 900)) % len(FLOORS)]
                if put(sp, m, C + "M_CC0_Pavement.M_CC0_Pavement",
                       x, y, DECK - 20.0, tags=("VEIL_PAVING",)):
                    bump("paving")
            x += 900.0
        y += 500.0
        row += 1

    # ---------- rim wall: the kit's 905x50x400 wall, one per bay, facing out
    bays = 24
    for i in range(bays):
        a = math.radians(i * (360.0 / bays))
        r = 3620.0
        if put(sp, MOD + "SM_Wall1_Big.SM_Wall1_Big",
               C + "M_CC0_Sandstone.M_CC0_Sandstone",
               r * math.cos(a), r * math.sin(a), DECK,
               yaw=i * (360.0 / bays) + 90.0, tags=("VEIL_PAVING",)):
            bump("rim")

    # ---------- arch gates at the four quarters
    for i in range(4):
        a = math.radians(i * 90.0)
        put(sp, PROPS + "SM_ArchD.SM_ArchD", C + "M_CC0_Sandstone.M_CC0_Sandstone",
            3550.0 * math.cos(a), 3550.0 * math.sin(a), DECK,
            yaw=i * 90.0 + 90.0)
        light(sp, 3550.0 * math.cos(a), 3550.0 * math.sin(a), DECK + 900.0, 2600.0, 3200.0)
    bump("gates", 4)

    # ---------- colonnade: the kit's 70x70x530 column, capital on top
    for i in range(12):
        a = math.radians(i * 30.0)
        x, y = 1700.0 * math.cos(a), 1700.0 * math.sin(a)
        if put(sp, PROPS + "SM_AncientColumn_Big.SM_AncientColumn_Big",
               V + "M_VEIL_Gold.M_VEIL_Gold", x, y, DECK, yaw=i * 30.0):
            bump("pillars")
        if put(sp, PROPS + "SM_AncientDecorArch.SM_AncientDecorArch",
               V + "M_VEIL_Gold.M_VEIL_Gold", x, y, DECK + 520.0, yaw=i * 30.0):
            bump("glyphs")
        light(sp, x, y, DECK + 700.0, 1700.0, 2500.0)

    # ---------- market stalls: the decor arch upright, cloth draped
    for i in range(8):
        a = math.radians(i * 45.0 + 22.5)
        if put(sp, PROPS + "SM_AncientDecorArch.SM_AncientDecorArch",
               C + "M_CC0_Cloth.M_CC0_Cloth",
               2150.0 * math.cos(a), 2150.0 * math.sin(a), DECK, yaw=i * 45.0 + 112.5):
            bump("stalls")

    # ---------- houses: 905x400 walls on a 900 square, gable at the bible pitch
    houses = 8
    for i in range(houses):
        ang = i * (360.0 / houses) + 22.5
        a = math.radians(ang)
        hx, hy = 2950.0 * math.cos(a), 2950.0 * math.sin(a)
        ca, sa = math.cos(a), math.sin(a)
        walls = 0
        for dx, dy, turn in ((0.0, -450.0, ang), (0.0, 450.0, ang),
                             (-450.0, 0.0, ang + 90.0), (450.0, 0.0, ang + 90.0)):
            px = hx + dx * ca - dy * sa
            py = hy + dx * sa + dy * ca
            if put(sp, MOD + "SM_Wall1_Big.SM_Wall1_Big",
                   C + "M_CC0_Sandstone.M_CC0_Sandstone", px, py, DECK, yaw=turn):
                walls += 1
        if walls == 4:
            bump("houses")
        bump("walls", walls)
        # gable: two 900x500 roof slabs against a ridge, at the bible pitch
        for side in (-1.0, 1.0):
            px = hx + side * 225.0 * ca
            py = hy + side * 225.0 * sa
            put(sp, MOD + "SM_Floor1_Big.SM_Floor1_Big",
                C + "M_CC0_RoofPlanks.M_CC0_RoofPlanks", px, py, DECK + 400.0,
                yaw=ang, pitch=-side * PITCH, centre_z=DECK + 430.0)
        light(sp, hx, hy, DECK + 260.0, 1400.0, 1900.0, (1.0, 0.82, 0.55))

    # ---------- lanterns: the kit's little shrine, lit
    for i in range(8):
        a = math.radians(i * 45.0 + 22.5)
        x, y = 1200.0 * math.cos(a), 1200.0 * math.sin(a)
        if put(sp, PROPS + "SM_LittleShrine.SM_LittleShrine",
               V + "M_VEIL_Signal.M_VEIL_Signal", x, y, DECK):
            bump("shrine")
        light(sp, x, y, DECK + 320.0)

    # ---------- the signal stone, off the spawn point
    if put(sp, PROPS + "SM_AncientMonolith.SM_AncientMonolith",
           V + "M_VEIL_Signal.M_VEIL_Signal", 0.0, 1500.0, DECK, yaw=PHI * 90.0):
        bump("monument")
    light(sp, 0.0, 1500.0, DECK + 420.0, 6000.0, 5200.0, (0.75, 0.92, 1.0))
    light(sp, 0.0, 0.0, DECK + 900.0, 3600.0, 4400.0)

    # ---------- stairs: the kit's three-step flight, stacked into a real flight
    for i in range(10):
        put(sp, PROPS + "SM_StairSmall.SM_StairSmall",
            C + "M_CC0_Sandstone.M_CC0_Sandstone",
            0.0, -(3550.0 + i * 140.0), DECK - 30.0 - i * 36.0)
    bump("stairs", 10)

    # ---------- the Spire: the beacon on the north edge of the plaza
    z = DECK
    for tier in range(5):
        width = 6.0 - tier
        if put(sp, PROPS + "SM_AncientColumn_Big.SM_AncientColumn_Big",
               V + "M_VEIL_Gold.M_VEIL_Gold", 0.0, 3350.0, z,
               yaw=tier * 36.0, sx=width, sy=width, sz=1.0):
            bump("spire")
        z += 530.0 * 0.94
    light(sp, 0.0, 3350.0, DECK + 2600.0, 12000.0, 9000.0, (1.0, 0.85, 0.55))

    # ---------- crystal peaks on the rock at authority (2800, 500)
    for i in range(5):
        a = math.radians(i * 72.0)
        r = 300.0 + i * 140.0
        if put(sp, PROPS + "SM_AncientMonolith.SM_AncientMonolith",
               V + "M_VEIL_Signal.M_VEIL_Signal",
               2800.0 + r * math.cos(a), 500.0 + r * math.sin(a), ROCK_LOW,
               yaw=i * 72.0, sx=3.0, sy=3.0, sz=2.4):
            bump("peaks")
        if i < 2:
            light(sp, 2800.0 + r * math.cos(a), 500.0 + r * math.sin(a), ROCK_LOW + 900.0,
                  4000.0, 4200.0, (0.7, 0.9, 1.0))

    ok = unreal.EditorLevelLibrary.save_current_level()
    w("save: %s" % bool(ok))
    w("actors now %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))
    w("COUNTS %s" % count)


try:
    main()
except Exception as err:  # noqa: BLE001
    w("EXCEPTION %r" % err)

w("V6 DONE")
