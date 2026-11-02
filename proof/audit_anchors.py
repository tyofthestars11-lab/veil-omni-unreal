"""Audit VEILDistrict against the design's anchor list.

Run inside the editor:
  UnrealEditor-Cmd.exe D:\\VEIL-OMNI\\VEIL.uproject -run=pythonscript -script="D:\\VEIL-OMNI\\proof\\audit_anchors.py"
Writes proof/anchor_audit.txt and prints the same lines.
"""
import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\proof\anchor_audit.txt"

# (label, x, y, z, tolerance_cm, what we expect to find)
ANCHORS = [
    ("Vault",         0.0,    0.0,  -80.9, 250.0, "AVEILVault"),
    ("Countess",    809.0, -309.0,   64.7, 250.0, "Countess pawn / hero placement"),
    ("RUNNER start",  0.0,    0.0,  100.0, 350.0, "player start / runner"),
    ("Keeper",      900.0,    0.0,    0.0, 500.0, "guiding statue"),
    ("Seer",       -450.0,  780.0,    0.0, 500.0, "guiding statue"),
    ("Cartographer", -4150.0, -780.0, 0.0, 600.0, "guiding statue"),
    ("Fragment 1", 2800.0,  500.0,    0.0, 500.0, "AVEILFragment"),
    ("Fragment 2", -2000.0, -900.0,   0.0, 500.0, "AVEILFragment"),
    ("Fragment 3", 600.0,  -2100.0,   0.0, 500.0, "AVEILFragment"),
    ("Cache",       0.0,  -6200.0,    0.0, 700.0, "cache / quest reward"),
]

lines = []


def log(msg):
    lines.append(msg)
    print(msg)


les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not les.load_level(MAP):
    log("FAILED to load %s" % MAP)
else:
    log("Loaded %s" % MAP)
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    log("Actors in level: %d" % len(actors))
    log("")

    positions = []
    for a in actors:
        try:
            loc = a.get_actor_location()
            positions.append((a.get_actor_label(), a.get_class().get_name(), loc))
        except Exception:
            continue

    hits, misses = 0, 0
    for label, ax, ay, az, tol, expect in ANCHORS:
        best = None
        best_d = 1e18
        for name, cls, loc in positions:
            d = ((loc.x - ax) ** 2 + (loc.y - ay) ** 2 + (loc.z - az) ** 2) ** 0.5
            if d < best_d:
                best_d, best = d, (name, cls, loc)
        if best and best_d <= tol:
            hits += 1
            log("MATCH  %-13s -> %s (%s) at (%.1f, %.1f, %.1f)  d=%.1f"
                % (label, best[0], best[1], best[2].x, best[2].y, best[2].z, best_d))
        else:
            misses += 1
            if best:
                log("MISS   %-13s expected %s near (%.0f, %.0f, %.0f); nearest is %s (%s) at "
                    "(%.1f, %.1f, %.1f) d=%.1f"
                    % (label, expect, ax, ay, az, best[0], best[1],
                       best[2].x, best[2].y, best[2].z, best_d))
            else:
                log("MISS   %-13s no actors at all" % label)

    log("")
    log("RESULT: %d/%d anchors present in the saved level" % (hits, len(ANCHORS)))
    log("")
    log("--- class census ---")
    counts = {}
    for _n, cls, _l in positions:
        counts[cls] = counts.get(cls, 0) + 1
    for cls in sorted(counts, key=lambda c: -counts[c]):
        log("  %-40s %d" % (cls, counts[cls]))

with open(OUT, "w") as fh:
    fh.write("\n".join(lines) + "\n")
print("wrote " + OUT)
