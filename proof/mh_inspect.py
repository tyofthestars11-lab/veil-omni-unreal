# VEIL - CAN A METAHUMAN BODY ACTUALLY MOVE HERE?
#
# The claim to test: "the built MetaHuman shipped without a locomotion graph". If its mesh
# component carries an anim class, a MetaHuman NPC can stand and breathe today. If it does
# not, a MetaHuman NPC is a frozen statue and the Polyart body stays the only animated
# humanoid the project owns.
#
# Reads only. Nothing is saved, spawned or changed.
import unreal

BP_PATHS = [
    "/Game/MetaHumans/MH_Runner_Built/MH_Runner/BP_MH_Runner.BP_MH_Runner_C",
    "/Game/MetaHumans/MH_Runner_Built/MH_Runner/BP_MH_Runner",
]
BODY = "/Game/MetaHumans/MH_Runner_Built/MH_Runner/Body/SKM_MH_Runner_BodyMesh"


def log(msg):
    unreal.log("[VEIL MH] " + str(msg))


def main():
    cls = None
    for path in BP_PATHS:
        try:
            cls = unreal.load_class(None, path)
        except Exception as exc:  # noqa: BLE001
            log("load_class {} raised {}".format(path, exc))
            cls = None
        log("load_class {} -> {}".format(path, cls))
        if cls:
            break

    if not cls:
        log("no blueprint class resolved - cannot test the anim class this way")
    else:
        cdo = unreal.get_default_object(cls)
        mesh = cdo.get_editor_property("mesh") if cdo else None
        if mesh:
            skel_mesh = mesh.get_editor_property("skeletal_mesh_asset")
            anim_class = mesh.get_editor_property("anim_class")
            log("BP mesh = {}".format(skel_mesh.get_path_name() if skel_mesh else "None"))
            log("BP anim_class = {}".format(anim_class.get_path_name() if anim_class else "None"))
        else:
            log("BP has no mesh component on its CDO")

    body = unreal.EditorAssetLibrary.load_asset(BODY)
    if body:
        skel = body.get_editor_property("skeleton")
        log("body mesh skeleton = {}".format(skel.get_path_name() if skel else "None"))
    else:
        log("could not load " + BODY)


main()
