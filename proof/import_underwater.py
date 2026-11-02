import unreal

SRC = r"D:\VEIL-OMNI\proof\underwater\sub akvo.obj"
DEST = "/Game/Underwater"

try:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", SRC)
    task.set_editor_property("destination_path", DEST)
    task.set_editor_property("destination_name", "SM_SubAkvo")
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    paths = task.get_editor_property("imported_object_paths")
    unreal.log("UNDERWATER imported: %s" % [str(p) for p in paths])
except Exception as e:
    unreal.log_error("UNDERWATER import failed: %r" % (e,))
