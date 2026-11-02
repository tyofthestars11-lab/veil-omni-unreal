import unreal, os
unreal.EditorLevelLibrary.load_level("/Game/Maps/VEILDistrict")
actors = unreal.EditorLevelLibrary.get_all_level_actors()
basic = 0
total = 0
for a in actors:
    total += 1
    if isinstance(a, unreal.StaticMeshActor):
        mesh = a.static_mesh_component.static_mesh
        if mesh and "BasicShapes" in mesh.get_path_name():
            basic += 1
with open(r"D:\VEIL\Saved\Logs\district_state.txt", "w") as f:
    f.write("DISTRICT actors=%d basicshapes=%d\n" % (total, basic))