import unreal

try:
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    les.editor_request_end_play()
    unreal.log("END PLAY requested via LevelEditorSubsystem.editor_request_end_play()")
except Exception as e:
    unreal.log_error("END PLAY failed: %r" % (e,))
