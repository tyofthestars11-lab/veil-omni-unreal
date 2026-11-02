import unreal

SRC = r"D:\VEIL-OMNI\Content\Branding\VEIL_Logo.png"
DEST = "/Game/Branding"

try:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", SRC)
    task.set_editor_property("destination_path", DEST)
    task.set_editor_property("destination_name", "VEIL_Logo")
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    unreal.log("LOGO imported to %s/VEIL_Logo" % DEST)
except Exception as e:
    unreal.log_error("LOGO import failed: %r" % (e,))
