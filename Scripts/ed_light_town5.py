"""120000 clips the sunlit stone at a pinned +2 bias; 16000 was night. Split it."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_light_town5.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

unreal.EditorLevelLibrary.load_level("/Game/VEIL/Maps/Lv_Veil_Town")

for a in unreal.EditorLevelLibrary.get_all_level_actors():
    cls = a.get_class().get_name()
    label = a.get_actor_label()
    if cls == "DirectionalLight":
        c = a.get_component_by_class(unreal.DirectionalLightComponent)
        if label == "VEIL_Fill":
            c.set_intensity(9000.0)
            w("fill 9000")
        else:
            c.set_intensity(42000.0)
            w("sun 42000")
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
    elif cls == "SkyLight":
        c = a.get_component_by_class(unreal.SkyLightComponent)
        c.set_intensity(1.3)
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
        w("skylight 1.3")

w("save: %s" % bool(unreal.EditorLevelLibrary.save_current_level()))
w("LIGHT5 DONE")
