"""Open MH_Runner in the MetaHuman Creator editor. Read-only: no level change, no save."""
import unreal

asset = unreal.EditorAssetLibrary.load_asset("/Game/MetaHumans/MH_Runner")
if asset is None:
    unreal.log_error("VEIL: MH_Runner not found")
else:
    unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).open_editor_for_assets([asset])
    unreal.log("VEIL: opened MetaHuman Creator on {}".format(asset.get_path_name()))
