import unreal


def main():
    unreal.log("[T] start")
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    ok = les.load_level("/Game/Maps/VEILDistrict")
    unreal.log("[T] load=%s" % ok)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    unreal.log("[T] actors=%d" % len(actors))
    cols = 0
    for a in actors:
        if not isinstance(a, unreal.StaticMeshActor):
            continue
        m = a.static_mesh_component.static_mesh if a.static_mesh_component else None
        if m and m.get_name() == "SM_AncientColumn_Big":
            cols += 1
    unreal.log("[T] columns=%d" % cols)


main()
