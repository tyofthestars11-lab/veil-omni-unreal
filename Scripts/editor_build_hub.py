"""Author the Hub in the Unreal Editor and save it into the map.

Run with: UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript=this -unattended -nosplash
No -nullrhi: without a render device the editor exits before scripts run.
"""
import math
import unreal

PHI = 1.618033988749895
GOLDEN = 137.5077640500378
MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
KIT = "/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/"
MAT = "/Game/VEIL/Materials/"
CLOTH = "/Game/VEIL/CC0/Materials/M_CC0_Cloth.M_CC0_Cloth"
GROUND_Z = 182021.14
COUNT = {"slabs": 0, "pillars": 0, "houses": 0, "stalls": 0, "lanterns": 0, "missing": 0}


def spacing(x):
    return 100.0 * PHI * (PHI ** (x - 1.0))


def log(msg):
    unreal.log("VEIL EDITOR: " + str(msg))


def place(mesh_path, mat_path, x, y, z, width, height, yaw=0.0, solid=True):
    mesh = unreal.load_asset(mesh_path)
    mat = unreal.load_asset(mat_path) if mat_path else None
    if not mesh:
        COUNT["missing"] += 1
        log("MISSING " + mesh_path)
        return None
    b = mesh.get_bounds()
    ext = b.box_extent
    sx = width / max(max(ext.x, ext.y) * 2.0, 1.0)
    sz = height / max(ext.z * 2.0, 1.0)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    a = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z),
                                      unreal.Rotator(0.0, yaw, 0.0))
    if not a:
        COUNT["missing"] += 1
        return None
    c = a.static_mesh_component
    c.set_mobility(unreal.ComponentMobility.MOVABLE)   # before the mesh, or it refuses
    c.set_static_mesh(mesh)
    if mat:
        c.set_material(0, mat)
    a.set_actor_scale3d(unreal.Vector(sx, sx, sz))
    if solid:
        c.set_collision_profile_name("BlockAll")
        c.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    else:
        c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    return a


def main():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not les.load_level(MAP):
        log("LEVEL LOAD FAILED")
        return
    log("level loaded")

    deck = GROUND_Z + spacing(0.5)

    for ix in (-1, 0, 1):
        for iy in (-1, 0, 1):
            if place(KIT + "SM_Platform.SM_Platform", MAT + "M_VEIL_Stone.M_VEIL_Stone",
                     ix * 900.0, iy * 900.0, GROUND_Z, 900.0, spacing(0.42)):
                COUNT["slabs"] += 1

    for i in range(1, 13):
        ang = math.radians(i * GOLDEN)
        r = 100.0 * PHI * math.sqrt(i)
        if place(KIT + "SM_AncientColumn_Big.SM_AncientColumn_Big",
                 MAT + "M_VEIL_Gold.M_VEIL_Gold",
                 r * math.cos(ang), r * math.sin(ang), deck,
                 spacing(-0.4), spacing(0.9), yaw=i * GOLDEN):
            COUNT["pillars"] += 1

    for i in range(1, 9):
        ang = math.radians(i * GOLDEN)
        r = 100.0 * PHI * PHI * math.sqrt(i)
        if place(KIT + "SM_HlafDome.SM_HlafDome", MAT + "M_VEIL_Stone.M_VEIL_Stone",
                 900.0 + r * math.cos(ang), r * math.sin(ang), deck,
                 spacing(-0.1), spacing(0.55), yaw=i * GOLDEN):
            COUNT["houses"] += 1

    for i in range(1, 9):
        ang = math.radians(i * GOLDEN)
        r = 100.0 * PHI * PHI * math.sqrt(i) / PHI
        if place(KIT + "SM_ArchD.SM_ArchD", CLOTH,
                 900.0 + r * math.cos(ang), r * math.sin(ang), deck,
                 spacing(-0.35), spacing(0.32), yaw=i * GOLDEN + 90.0):
            COUNT["stalls"] += 1

    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    for i in range(1, 9):
        ang = math.radians(i * GOLDEN)
        r = 100.0 * PHI * (PHI ** 3.0)
        x, y = r * math.cos(ang), r * math.sin(ang)
        place(KIT + "SM_LittleShrine.SM_LittleShrine", MAT + "M_VEIL_Signal.M_VEIL_Signal",
              x, y, deck, spacing(-0.55), spacing(0.5))
        lamp = actors.spawn_actor_from_class(unreal.PointLight,
                                            unreal.Vector(x, y, deck + spacing(0.62)))
        if lamp:
            comp = lamp.get_component_by_class(unreal.PointLightComponent)
            if comp:
                comp.set_mobility(unreal.ComponentMobility.MOVABLE)
                comp.set_intensity(1400.0)
                comp.set_attenuation_radius(2600.0)
                comp.set_light_color(unreal.LinearColor(1.0, 0.78, 0.42, 1.0))
                COUNT["lanterns"] += 1

    log("COUNTS %s" % COUNT)
    total = COUNT["slabs"] + COUNT["pillars"] + COUNT["houses"] + COUNT["stalls"]
    log("HUB AUTHORED %d pieces" % total)

    pkg = unreal.load_asset(MAP)
    ok = unreal.EditorLoadingAndSavingUtils.save_map(pkg, MAP)
    log("MAP SAVED" if ok else "MAP SAVE FAILED")
    log("DONE")


try:
    main()
except Exception as err:  # noqa: BLE001
    log("EXCEPTION " + repr(err))
finally:
    unreal.SystemLibrary.quit_editor()
