"""MEASURE THE MASSING. A slab and a building differ by one number: how tall they are.

Reads every static mesh actor in the level and reports the height distribution, so "flat grey
slabs" and "a city" are distinguishable in a log instead of an opinion. No guessing, no sampling.
"""
import unreal

MAP = "/Game/Maps/VEILDistrict"
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.load_level(MAP)


def log(m):
    unreal.log("[VEIL MASSING] " + str(m))


actors = [a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
          if a.get_class().get_name() == "StaticMeshActor"]
heights, flat, tall = [], 0, 0
for a in actors:
    try:
        origin, extent = a.get_actor_bounds(False)
        h = 2.0 * extent.z
        heights.append(h)
        if h < 30.0:
            flat += 1
        if h >= 300.0:
            tall += 1
    except Exception:
        continue
heights.sort()
n = len(heights)
if n:
    log("static meshes %d" % n)
    log("height cm: min %.0f  p25 %.0f  median %.0f  p75 %.0f  p95 %.0f  max %.0f" % (
        heights[0], heights[n // 4], heights[n // 2], heights[3 * n // 4],
        heights[min(n - 1, int(n * 0.95))], heights[-1]))
    log("flat (under 30 cm): %d  (%.1f%%)" % (flat, 100.0 * flat / n))
    log("building-scale (300 cm or more): %d  (%.1f%%)" % (tall, 100.0 * tall / n))

mats, comps = {}, 0
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    cls = a.get_class().get_name()
    comps += 1
    if cls != "StaticMeshActor":
        continue
    try:
        c = a.get_editor_property("static_mesh_component")
        for m in c.get_editor_property("override_materials"):
            if m:
                mats[m.get_name()] = mats.get(m.get_name(), 0) + 1
    except Exception:
        pass
for k in sorted(mats, key=lambda x: -mats[x])[:6]:
    log("material %-30s %d" % (k, mats[k]))
log("actors total %d" % comps)
