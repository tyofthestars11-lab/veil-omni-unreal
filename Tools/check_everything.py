# CHECK EVERYTHING: every place in the game that puts something in the world,
# and every problem the live level actually has.
#
# Two halves, because either alone lies:
#   1. SOURCE - every file and function containing a spawn call (C++ spawners, instanced
#      placement, BeginPlay builders) and every random call. Named file:line:function.
#   2. LEVEL - the saved map read back: duplicates, floating pieces, overlaps, and
#      anything without collision.
#
# Output: Docs\check_everything.txt   Nothing is modified.
import os
import re
import unreal

ROOT = r"D:\VEIL-OMNI"
SRC = os.path.join(ROOT, "Source")
OUT = os.path.join(ROOT, "Docs", "check_everything.txt")
MAP = "/Game/Maps/VEILDistrict"

SPAWN_PATTERNS = [
    ("SpawnActor", re.compile(r"SpawnActor<|SpawnActor\(|SpawnActorDeferred")),
    ("SpawnBlock/SpawnBlockRot", re.compile(r"SpawnBlock(Rot)?\(")),
    ("AddBlock/AddBlockYaw", re.compile(r"AddBlock(Yaw)?\(")),
    ("AddInstance", re.compile(r"AddInstance\(")),
    ("BeginPlay", re.compile(r"::BeginPlay\(")),
    ("OpenLevel/ServerTravel", re.compile(r"OpenLevel\(|ServerTravel\(")),
]
RNG = re.compile(r"FRandomStream|FMath::Rand|RandRange|FMath::FRand|rand\(\)|random\.")


def scan_source():
    hits = []
    func = "?"
    for dirpath, _, files in os.walk(SRC):
        for fn in sorted(files):
            if not fn.endswith((".cpp", ".h")):
                continue
            path = os.path.join(dirpath, fn)
            with open(path, "r", errors="ignore") as f:
                for i, line in enumerate(f, 1):
                    m = re.match(r"^[A-Za-z_].*?::([A-Za-z_0-9]+)\s*\(", line)
                    if m:
                        func = m.group(1)
                    for label, rx in SPAWN_PATTERNS:
                        if rx.search(line):
                            hits.append((label, os.path.relpath(path, ROOT), i, func,
                                         line.strip()[:120]))
                    if RNG.search(line):
                        hits.append(("RANDOM", os.path.relpath(path, ROOT), i, func,
                                     line.strip()[:120]))
    return hits


def scan_level():
    # A LOAD THAT FAILED IS NOT AN EMPTY LEVEL. The first run of this audit reported
    # "0 static meshes" because load_level returned False and the scan read the default
    # empty world - a clean report of nothing, which is worse than an error. It fails
    # loudly now instead of lying quietly.
    if not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP):
        unreal.log_error("[VEIL CHECK] COULD NOT LOAD " + MAP + " - level scan skipped")
        return [], [], [], [], []
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    pieces = []
    for a in actors:
        if not isinstance(a, unreal.StaticMeshActor):
            continue
        comp = a.static_mesh_component
        mesh = comp.static_mesh if comp else None
        if not mesh:
            continue
        # A COMPONENT HAS NO 'bounds' IN THE PYTHON API. The first version assumed it did
        # and the whole level half of this audit died on an AttributeError - the report
        # then said "0 static meshes", which reads as a clean world. Actor bounds are the
        # callable one: get_actor_bounds returns origin and extent in world space.
        origin, extent = a.get_actor_bounds(False)
        try:
            collision = str(comp.get_collision_enabled())
        except Exception:
            collision = "unknown"
        pieces.append({
            "label": a.get_actor_label(),
            "mesh": mesh.get_name(),
            "path": mesh.get_path_name(),
            "loc": a.get_actor_location(),
            "box": (origin.x, origin.y, origin.z, extent.x, extent.y, extent.z),
            "collision": collision,
            "tags": [str(t) for t in a.tags],
        })

    # DUPLICATES: same mesh, same place (within 1 cm).
    seen = {}
    dupes = []
    for p in pieces:
        key = (p["mesh"], round(p["loc"].x), round(p["loc"].y), round(p["loc"].z))
        if key in seen:
            dupes.append((seen[key], p))
        else:
            seen[key] = p

    # FLOATING, WITHOUT ASKING THE ENGINE FOR A TRACE.
    #
    # Three attempts at a Python line trace each died on a different missing symbol, and a
    # test that cannot run is not a test. Support is pure geometry instead: a piece stands
    # if the ground is directly under it, or if another piece's top is directly under its
    # bottom within 100 cm - measured from bounds, which are already read.
    GROUND_Z = 100.0        # the plaza the level is built on, in cm
    TOLERANCE = 100.0       # a kerb step: closer than this counts as standing on it

    def xy_overlap(a, b):
        ax, ay, _, aex, aey, _ = a["box"]
        bx, by, _, bex, bey, _ = b["box"]
        return (min(ax + aex, bx + bex) - max(ax - aex, bx - bex) > 1.0
                and min(ay + aey, by + bey) - max(ay - aey, by - bey) > 1.0)

    floating = []
    for p in pieces:
        _, _, oz, _, _, ez = p["box"]
        bottom = oz - ez
        supported = bottom <= GROUND_Z + TOLERANCE
        if not supported:
            for o in pieces:
                if o is p or not xy_overlap(p, o):
                    continue
                top = o["box"][2] + o["box"][5]
                if abs(top - bottom) <= TOLERANCE:
                    supported = True
                    break
        if not supported:
            floating.append((p, bottom - GROUND_Z))

    # OVERLAP: axis-aligned bounds intersecting between two different meshes.
    overlaps = []
    n = len(pieces)
    for i in range(n):
        ax, ay, az, aex, aey, aez = pieces[i]["box"]
        for j in range(i + 1, n):
            bx, by, bz, bex, bey, bez = pieces[j]["box"]
            dx = min(ax + aex, bx + bex) - max(ax - aex, bx - bex)
            dy = min(ay + aey, by + bey) - max(ay - aey, by - bey)
            dz = min(az + aez, bz + bez) - max(az - aez, bz - bez)
            if dx > 100.0 and dy > 100.0 and dz > 100.0:
                overlaps.append((pieces[i], pieces[j], dx * dy * dz / 1e6))

    no_collision = [p for p in pieces if "NoCollision" in p["collision"]]
    return pieces, dupes, floating, overlaps, no_collision


def main():
    hits = scan_source()
    pieces, dupes, floating, overlaps, no_collision = scan_level()
    if not pieces:
        unreal.log_error("[VEIL CHECK] level scan produced no pieces - report is source-only")

    lines = []
    lines.append("VEIL - CHECK EVERYTHING")
    lines.append("map under audit: " + MAP)
    lines.append("")
    lines.append("PART 1 - EVERY PLACE IN THE CODE THAT PUTS SOMETHING IN THE WORLD")
    by_file = {}
    for label, path, lineno, func, text in hits:
        by_file.setdefault(path, []).append((label, lineno, func, text))
    for path in sorted(by_file, key=lambda p: -len(by_file[p])):
        rows = by_file[path]
        counts = {}
        for label, _, _, _ in rows:
            counts[label] = counts.get(label, 0) + 1
        lines.append("  %-58s %s" % (path, ", ".join("%s x%d" % (k, v) for k, v in sorted(counts.items()))))
        seen_fn = set()
        for label, lineno, func, text in rows:
            key = (label, func)
            if key in seen_fn:
                continue
            seen_fn.add(key)
            lines.append("      %-28s %-34s %s:%d  %s" % (label, func, os.path.basename(path), lineno, text))

    random_hits = [h for h in hits if h[0] == "RANDOM"]
    lines.append("")
    lines.append("RANDOM CALLS FOUND: %d" % len(random_hits))
    for label, path, lineno, func, text in random_hits:
        lines.append("  %s:%d in %s  %s" % (path, lineno, func, text))

    lines.append("")
    lines.append("PART 2 - THE SAVED LEVEL")
    lines.append("  static mesh actors: %d" % len(pieces))
    lines.append("  duplicated (same mesh, same point): %d" % len(dupes))
    for a, b in dupes[:25]:
        lines.append("      %s and %s at (%.0f,%.0f,%.0f)" % (a["mesh"], b["mesh"],
                     a["loc"].x, a["loc"].y, a["loc"].z))
    lines.append("  floating (no ground within 80 cm): %d" % len(floating))
    for p, d in sorted(floating, key=lambda t: -t[1])[:25]:
        lines.append("      %-26s at (%.0f,%.0f,%.0f) gap=%.0f" % (p["mesh"], p["loc"].x,
                     p["loc"].y, p["loc"].z, d))
    lines.append("  overlapping pairs (bounds, >100 cm each axis): %d" % len(overlaps))
    for a, b, vol in sorted(overlaps, key=lambda t: -t[2])[:25]:
        lines.append("      %s x %s  %.1f m3 at (%.0f,%.0f,%.0f)" % (a["mesh"], b["mesh"], vol,
                     a["loc"].x, a["loc"].y, a["loc"].z))
    lines.append("  no collision: %d" % len(no_collision))
    for p in no_collision[:25]:
        lines.append("      %-26s at (%.0f,%.0f,%.0f)" % (p["mesh"], p["loc"].x, p["loc"].y, p["loc"].z))

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")

    unreal.log("[VEIL CHECK] wrote %s" % OUT)
    unreal.log("[VEIL CHECK] pieces=%d dupes=%d floating=%d overlaps=%d nocollision=%d random=%d"
               % (len(pieces), len(dupes), len(floating), len(overlaps), len(no_collision),
                  len(random_hits)))


main()
