"""130000 sun + 3.4 sky blew the town white. Dial it back."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_light_town2.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/VEIL/Maps/Lv_Veil_Town"
unreal.EditorLevelLibrary.load_level(MAP)

for a in unreal.EditorLevelLibrary.get_all_level_actors():
    cls = a.get_class().get_name()
    label = a.get_actor_label()
    if cls == "DirectionalLight":
        c = a.get_component_by_class(unreal.DirectionalLightComponent)
        if label == "VEIL_Fill":
            c.set_intensity(4500.0)
            w("fill 4500")
        else:
            c.set_intensity(52000.0)
            a.set_actor_rotation(unreal.Rotator(-42.0, 35.0, 0.0), False)
            w("sun 52000")
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
    elif cls == "SkyLight":
        c = a.get_component_by_class(unreal.SkyLightComponent)
        c.set_intensity(1.5)
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
        w("skylight 1.5")
    elif cls == "ExponentialHeightFog":
        c = a.get_component_by_class(unreal.ExponentialHeightFogComponent)
        try:
            c.set_editor_property("fog_density", 0.008)
            w("fog density 0.008")
        except Exception as err:  # noqa: BLE001
            w("fog tweak failed %r" % err)

saved = unreal.EditorLevelLibrary.save_current_level()
w("save: %s" % bool(saved))
w("LIGHT2 DONE")
