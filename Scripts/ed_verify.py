"""Read-only proof that the editor-authored Hub is stored in the saved map."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_verify.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
w("open map: %s" % bool(unreal.EditorLevelLibrary.load_level(MAP)))

actors = unreal.EditorLevelLibrary.get_all_level_actors()
w("total actors: %d" % len(actors))

meshes = [a for a in actors if isinstance(a, unreal.StaticMeshActor)]
lights = [a for a in actors if isinstance(a, unreal.PointLight)]
w("static mesh actors: %d" % len(meshes))
w("point lights: %d" % len(lights))

# the authored pieces sit above the landscape; report those and their collision
solid = 0
floating = 0
zmin, zmax = 1e12, -1e12
for a in meshes:
    loc = a.get_actor_location()
    if loc.z > 182000.0:
        zmin = min(zmin, loc.z)
        zmax = max(zmax, loc.z)
    comp = a.static_mesh_component
    if comp and comp.get_collision_profile_name() == "BlockAll":
        solid += 1

w("world pieces above ground: %d" % len([a for a in meshes if a.get_actor_location().z > 182000.0]))
w("pieces with BlockAll collision: %d" % solid)
if zmin < 1e11:
    w("piece Z range: %.1f .. %.1f" % (zmin, zmax))

named = {}
for a in meshes:
    loc = a.get_actor_location()
    if loc.z > 182100.0:
        named[a.get_actor_label()] = (round(loc.x), round(loc.y), round(loc.z))
for k in list(named)[:8]:
    w("piece %s at %s" % (k, named[k]))

w("VERIFY DONE")
