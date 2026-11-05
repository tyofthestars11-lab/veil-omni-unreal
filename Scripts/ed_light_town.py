"""The town level was lit like a cellar. Give it a sun, sky and a fill."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_light_town.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/VEIL/Maps/Lv_Veil_Town"
unreal.EditorLevelLibrary.load_level(MAP)

sun = None
sky = None
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    cls = a.get_class().get_name()
    if cls == "DirectionalLight" and sun is None and "_fill" not in a.get_actor_label():
        sun = a
    elif cls == "SkyLight" and sky is None:
        sky = a

if sun:
    c = sun.get_component_by_class(unreal.DirectionalLightComponent)
    c.set_intensity(130000.0)
    c.set_mobility(unreal.ComponentMobility.MOVABLE)
    sun.set_actor_rotation(unreal.Rotator(-42.0, 35.0, 0.0), False)
    try:
        c.set_editor_property("atmosphere_sun_light", True)
    except Exception:  # noqa: BLE001
        pass
    w("sun at 130000, rotation -42/35")
else:
    w("MISS sun")

if sky:
    c = sky.get_component_by_class(unreal.SkyLightComponent)
    c.set_intensity(3.4)
    try:
        c.set_editor_property("real_time_capture", True)
    except Exception:  # noqa: BLE001
        pass
    c.set_mobility(unreal.ComponentMobility.MOVABLE)
    w("skylight at 3.4")
else:
    w("MISS skylight")

# a cool fill from the opposite side so shadowed faces read as stone
fill = None
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_class().get_name() == "DirectionalLight" and a.get_actor_label() == "VEIL_Fill":
        fill = a
        break
if fill is None:
    fill = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0, 0, 2000), unreal.Rotator(-18.0, 215.0, 0.0))
    fill.set_actor_label("VEIL_Fill")
c = fill.get_component_by_class(unreal.DirectionalLightComponent)
c.set_intensity(14000.0)
c.set_mobility(unreal.ComponentMobility.MOVABLE)
try:
    c.set_editor_property("atmosphere_sun_light", False)
    c.set_editor_property("cast_shadows", False)
except Exception:  # noqa: BLE001
    pass
w("fill light 14000, no shadows")

saved = unreal.EditorLevelLibrary.save_current_level()
w("save: %s" % bool(saved))
w("LIGHT TOWN DONE")
