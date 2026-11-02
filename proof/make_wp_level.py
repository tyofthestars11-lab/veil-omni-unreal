# VEIL - BIRTH THE DISTRICT AS A WORLD PARTITION WORLD
#
# /Game/Maps/VEILDistrict is a dense level: 1,151 actors, only 179 external. The vision's first
# requirement is one persistent World Partition world streaming by grid cells, and an existing
# dense level cannot be converted safely by script. But this district is PROCEDURAL - the builder
# raises all of it - so the honest move is to create a level from the engine's OpenWorld template
# (which is born partitioned, with HLOD layers) and bake the same builder into it.
#
# Writes progress to Saved/Logs/wp_build.txt as it goes, because a script that only reports at the
# end loses everything when one call throws.
import unreal

OUT = r"D:\VEIL-OMNI\Saved\Logs\wp_build.txt"
NEW_MAP = "/Game/Maps/VEILDistrictWP"
TEMPLATE = "/Engine/Maps/Templates/OpenWorld"


def say(msg):
    line = "[VEIL WPB] " + str(msg)
    print(line)
    try:
        with open(OUT, "a", encoding="utf-8") as f:
            f.write(line + "\n")
    except Exception:  # noqa: BLE001
        pass


def main():
    open(OUT, "w", encoding="utf-8").close()
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(NEW_MAP):
        say("{} already exists - loading it instead of recreating".format(NEW_MAP))
        say("load_level -> {}".format(les.load_level(NEW_MAP)))
    else:
        try:
            ok = les.new_level_from_template(NEW_MAP, TEMPLATE)
            say("new_level_from_template({}, OpenWorld) -> {}".format(NEW_MAP, ok))
        except Exception as exc:  # noqa: BLE001
            say("new_level_from_template raised: {}".format(exc))
            return

    actors_before = len(unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors())
    say("actors in the fresh level: {}".format(actors_before))

    # Bake the district with the same builder the district map uses.
    builder = None
    try:
        builder = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(
            unreal.VEILWorldBuilder, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        say("builder spawned: {}".format(builder is not None))
        if builder:
            builder.bake_to_level()
            say("bake_to_level returned")
    except Exception as exc:  # noqa: BLE001
        say("bake raised: {}".format(exc))

    if builder:
        try:
            unreal.get_editor_subsystem(unreal.EditorActorSubsystem).destroy_actor(builder)
            say("builder removed")
        except Exception as exc:  # noqa: BLE001
            say("could not remove builder: {}".format(exc))

    actors_after = len(unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors())
    say("actors after bake: {}".format(actors_after))

    try:
        saved = bool(les.save_current_level())
        say("save_current_level -> {}".format(saved))
    except Exception as exc:  # noqa: BLE001
        say("save raised: {}".format(exc))

    say("done")


main()
