# Code the Fab library into the world: the Spire (2800, 500) is the combat
# region and it was bare ground. SICKA's 215 ancient meshes and Iceland's 14
# mountains are already on disk, imported from Fab, and unused.
#
# Placement is Tyree's geometry, not a grid: step * 137.507764 degrees, radius
# growing by phi. Nothing is placed within 900 cm of the fragment at the centre.
import math
import unreal

LOG = r'D:\VEIL-OMNI\proof\dress_spire.txt'
GOLDEN_DEG = 137.507764
PHI = 1.618033988749895
CX, CY = 2800.0, 500.0
CLEAR = 900.0


def log(msg):
    print(msg)
    with open(LOG, 'a', encoding='utf-8') as f:
        f.write(str(msg) + '\n')


unreal.EditorLevelLibrary.load_level('/Game/Maps/VEILDistrict')
reg = unreal.AssetRegistryHelpers.get_asset_registry()


def meshes_under(folder):
    out = []
    for a in reg.get_assets_by_path(folder, recursive=True):
        try:
            if a.asset_class_path.asset_name != 'StaticMesh':
                continue
        except Exception:
            continue
        out.append(a.get_asset())
    return [m for m in out if m]


sicka = meshes_under('/Game/Sicka_EnvPack2')
iceland = meshes_under('/Game/Iceland_Environment')
log('found sicka={} iceland={}'.format(len(sicka), len(iceland)))

# Prefer structure over debris, so the Spire reads as architecture.
want = ('Column', 'Arch', 'Decor', 'Pillar', 'Wall', 'Stair', 'Floor', 'Temple', 'Gate')
structs = [m for m in sicka if any(w in m.get_name() for w in want)]
debris = [m for m in sicka if 'Debris' in m.get_name()]
structs.sort(key=lambda m: m.get_name())
debris.sort(key=lambda m: m.get_name())
log('structure meshes={} debris={}'.format(len(structs), len(debris)))


def spawn_mesh(mesh, loc, rot, label):
    """StaticMeshActor + explicit mesh: spawn_actor_from_object silently
    returns None for registry assets, which is why the first pass placed 0."""
    a = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, loc, rot)
    if not a:
        return None
    comp = a.get_editor_property('static_mesh_component')
    comp.set_editor_property('mobility', unreal.ComponentMobility.STATIC)
    comp.set_static_mesh(mesh)
    a.set_actor_label(label)
    return a

placed = 0
spawned_paths = []

# 1. Mountains frame the region from far out: a boundary you can see.
for i, m in enumerate(sorted(iceland, key=lambda x: x.get_name())):
    ang = math.radians(i * GOLDEN_DEG)
    radius = 7200.0 + (i % 3) * 900.0
    loc = unreal.Vector(CX + math.cos(ang) * radius, CY + math.sin(ang) * radius, -200.0)
    a = spawn_mesh(m, loc, unreal.Rotator(0, (i * GOLDEN_DEG) % 360.0, 0), 'SPIRE_MOUNTAIN_{:02d}'.format(i + 1))
    if a:
        placed += 1
        spawned_paths.append(m.get_path_name())

# 2. Ancient structures on a golden spiral around the Spire's heart.
if structs:
    for i in range(56):
        ang = math.radians(i * GOLDEN_DEG)
        radius = CLEAR + (PHI ** (i / 9.0)) * 260.0
        if radius > 4600.0:
            radius = 4600.0
        m = structs[i % len(structs)]
        loc = unreal.Vector(CX + math.cos(ang) * radius, CY + math.sin(ang) * radius, 0.0)
        rot = unreal.Rotator(0.0, (i * GOLDEN_DEG) % 360.0, 0.0)
        a = spawn_mesh(m, loc, rot, 'SPIRE_STRUCT_{:02d}'.format(i + 1))
        if a:
            placed += 1
            spawned_paths.append(m.get_path_name())

# 3. Debris between them: the region looks lived in, not swept.
if debris:
    for i in range(34):
        ang = math.radians((i + 0.5) * GOLDEN_DEG)
        radius = CLEAR + 320.0 + (PHI ** (i / 12.0)) * 180.0
        m = debris[i % len(debris)]
        loc = unreal.Vector(CX + math.cos(ang) * radius, CY + math.sin(ang) * radius, 0.0)
        rot = unreal.Rotator(0.0, (i * 47.0) % 360.0, (i * 13.0) % 20.0)
        a = spawn_mesh(m, loc, rot, 'SPIRE_DEBRIS_{:02d}'.format(i + 1))
        if a:
            placed += 1
            spawned_paths.append(m.get_path_name())

saved = unreal.EditorLevelLibrary.save_current_level()
log('PLACED {} | SAVE {}'.format(placed, saved))
for p in spawned_paths[:6]:
    log('  ' + p)

# Confirm by counting what is in the level now.
count = 0
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_actor_label().startswith('SPIRE_'):
        count += 1
log('SPIRE ACTORS IN LEVEL: {}'.format(count))
log('DONE')
