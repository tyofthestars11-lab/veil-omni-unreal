"""Create the project's own town level: no borrowed scenery, no rock in the floor."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_new_town.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/VEIL/Maps/Lv_Veil_Town"

try:
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    ok = les.new_level(MAP)
    w("new_level: %s" % ok)
except Exception as err:  # noqa: BLE001
    w("new_level EXCEPTION %r" % err)
    raise SystemExit

world = unreal.EditorLevelLibrary.get_editor_world()
spawn = unreal.EditorLevelLibrary.spawn_actor_from_class

try:
    sun = spawn(unreal.DirectionalLight, unreal.Vector(0, 0, 4000),
                unreal.Rotator(-38.0, 40.0, 0.0))
    comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    comp.set_intensity(90000.0)
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    try:
        comp.set_editor_property("atmosphere_sun_light", True)
    except Exception:  # noqa: BLE001
        pass
    w("sun spawned")
except Exception as err:  # noqa: BLE001
    w("sun EXCEPTION %r" % err)

for cls, name in ((unreal.SkyAtmosphere, "atmosphere"), (unreal.ExponentialHeightFog, "fog")):
    try:
        spawn(cls, unreal.Vector(0, 0, 0), unreal.Rotator(0.0, 0.0, 0.0))
        w("%s spawned" % name)
    except Exception as err:  # noqa: BLE001
        w("%s EXCEPTION %r" % (name, err))

try:
    sky = spawn(unreal.SkyLight, unreal.Vector(0, 0, 600), unreal.Rotator(0.0, 0.0, 0.0))
    sc = sky.get_component_by_class(unreal.SkyLightComponent)
    sc.set_intensity(2.6)
    sc.set_editor_property("real_time_capture", True)
    sc.set_mobility(unreal.ComponentMobility.MOVABLE)
    w("skylight spawned")
except Exception as err:  # noqa: BLE001
    w("skylight EXCEPTION %r" % err)

try:
    spawn(unreal.PlayerStart, unreal.Vector(0, 0, 300), unreal.Rotator(0.0, 90.0, 0.0))
    w("player start spawned")
except Exception as err:  # noqa: BLE001
    w("player start EXCEPTION %r" % err)

saved = unreal.EditorLevelLibrary.save_current_level()
w("save: %s" % bool(saved))
w("actors: %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))
w("NEW TOWN DONE")
