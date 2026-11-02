import unreal

LOG = r'D:\VEIL-OMNI\proof\editor_load.txt'


def log(msg):
    print(msg)
    with open(LOG, 'a', encoding='utf-8') as f:
        f.write(str(msg) + '\n')


log('--- editor load check ---')
log('engine version: {}'.format(unreal.SystemLibrary.get_engine_version()))
log('project: {}'.format(unreal.SystemLibrary.get_project_directory()))
log('content: {}'.format(unreal.SystemLibrary.get_project_content_directory()))

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ok = les.load_level('/Game/Maps/VEILDistrict')
log('load VEILDistrict: {}'.format(ok))
if ok:
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    log('actors: {}'.format(len(actors)))
    spire = sum(1 for a in actors if a.get_actor_label().startswith('SPIRE_'))
    log('spire actors: {}'.format(spire))
log('DONE')
unreal.SystemLibrary.quit_editor()
