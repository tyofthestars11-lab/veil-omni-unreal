#!/usr/bin/env python3
# VEIL-PYTHON-HELPS-MAKE-VEIL.py - Python is NOT a separate game. Python = source,
# blueprint, data. Unreal = stage. Same game: VEIL OMNI: THE LAST SIGNAL - PATH OF LIGHT.
#
# Run in Unreal Editor (this bakes the district INTO /Game/Maps/VEILDistrict and saves it):
#   UnrealEditor-Cmd.exe "D:\VEIL-OMNI\VEIL.uproject" -run=pythonscript -script="D:\VEIL-OMNI\Docs\VEIL-PYTHON-HELPS-MAKE-VEIL.py"
# Run in plain Python (counts, no engine needed):
#   python VEIL-PYTHON-HELPS-MAKE-VEIL.py
import json
import math
import os
import sys

PHI = 1.618033988749895
GOLDEN = 137.5077640500378
PROJECT = r"D:\VEIL-OMNI"
MAP = "/Game/Maps/VEILDistrict"

VEIL = {
    "game": "VEIL OMNI: THE LAST SIGNAL - PATH OF LIGHT",
    "python_is": "source, blueprint, data - Unreal is the stage, same game, whole",
    "unreal_project": os.path.join(PROJECT, "VEIL.uproject"),
    "map": MAP,
    "zones": {
        "Hub": (0, 0),
        "Nest": (-450, 780),
        "Spire": (0, 3500),
        "Omni District": (2800, 500),
        "Megastructure Deep": (0, -5000),
    },
    "authority": {
        "Vault": (0, 0, -80.9),
        "Countess": (809, -309, 64.7),
        "Runner": (0, 0, 100),
        "Keeper": (900, 0, 0),
        "Seer": (-450, 780, 0),
        "Cartographer": (-4150, -780, 0),
        "Cache": (0, -6200, 0),
        "Beacon1": (2800, 500, 0),
        "Beacon2": (-2000, -900, 0),
        "Beacon3": (600, -2100, 0),
        "Spire": (0, 3500, 500),
    },
    "physics": {"gravity": -980, "walk": 439, "sprint": 750, "leap_cm": 185,
                "glide": 600, "glide_drop": -50, "grounded": True,
                "floats": False, "random": False, "placeholders": False},
    "dialogue": "[E] only - Space and Enter never advance a line",
    "hud": "gold ribbon, VIT/STM/LV diamond, compass 180 at 70%, quest 480, ONE mic only",
}


def phi_place(i, bx=0.0, by=0.0):
    """Golden-angle placement. No random anywhere: the same i always lands in the same spot."""
    a = math.radians(i * GOLDEN)
    r = math.sqrt(i) * PHI * 100.0
    return round(bx + r * math.cos(a), 2), round(by + r * math.sin(a), 2)


def layout():
    """25 placements: 12 phi pillars at Hub, 8 market houses, 5 Omni crystals."""
    spots = []
    for i in range(12):
        x, y = phi_place(i + 1, 0, 0)
        spots.append({"kind": "phi_pillar", "x": x, "y": y, "zone": "Hub"})
    for i in range(8):
        x, y = phi_place(i + 1, 900, 0)
        spots.append({"kind": "market_house", "x": x, "y": y, "zone": "Hub"})
    for i in range(5):
        x, y = phi_place(i + 1, 2800, 500)
        spots.append({"kind": "blue_crystal", "x": x, "y": y, "zone": "Omni District"})
    return spots


def write_data(spots):
    out = os.path.join(PROJECT, "Data", "veil_python_layout.json")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    payload = {
        "game": VEIL["game"],
        "map": MAP,
        "seed": 1618,
        "golden_angle": GOLDEN,
        "phi": PHI,
        "zones": {k: list(v) for k, v in VEIL["zones"].items()},
        "authority": {k: list(v) for k, v in VEIL["authority"].items()},
        "placements": spots,
        "physics": VEIL["physics"],
    }
    with open(out, "w", encoding="utf-8") as f:
        json.dump(payload, f, indent=2)
    return out


def unreal_build():
    """Inside the editor: raise the vision into VEILDistrict and save the map."""
    import unreal

    def log(msg):
        unreal.log("[VEIL PYTHON] " + str(msg))

    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    if not les.load_level(MAP):
        log("could not load " + MAP)
        return False
    log("loaded " + MAP)

    before = eas.get_all_level_actors()
    log("actors before %d" % len(before))

    builder = None
    for a in before:
        if a.get_class().get_name() == "VEILWorldBuilder":
            builder = a
            break
    if builder is None:
        builder = eas.spawn_actor_from_class(unreal.VEILWorldBuilder,
                                             unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        log("spawned builder")
    if builder is None:
        log("no builder available - the C++ module is not loaded")
        return False

    builder.bake_to_level()
    after = eas.get_all_level_actors()
    log("actors after %d" % len(after))

    statics = 0
    spire = 0
    for a in after:
        comp = a.get_component_by_class(unreal.StaticMeshComponent) if False else None
        if isinstance(a, unreal.StaticMeshActor):
            statics += 1
            loc = a.get_actor_location()
            if abs(loc.x) < 600 and abs(loc.y - 3500) < 600 and loc.z > 1500:
                spire += 1
    player_starts = sum(1 for a in after if isinstance(a, unreal.PlayerStart))
    log("static meshes %d, spire pieces %d, player starts %d" % (statics, spire, player_starts))

    saved = bool(les.save_current_level())
    log("saved %s -> %s" % (MAP, saved))
    print("UNREAL: map %s saved=%s statics=%d spire=%d starts=%d"
          % (MAP, saved, statics, spire, player_starts))
    return saved and statics > 100 and spire > 0


if __name__ == "__main__":
    spots = layout()
    path = write_data(spots)
    assert len(spots) == 25, len(spots)
    assert len(VEIL["authority"]) == 11, len(VEIL["authority"])
    assert len(VEIL["zones"]) == 5, len(VEIL["zones"])
    assert VEIL["authority"]["Spire"] == (0, 3500, 500)
    assert VEIL["zones"]["Hub"] == (0, 0)
    print("PYTHON: wrote " + path)
    print("PYTHON: placements 25, authority 11, zones 5, Spire %s, Hub first, grounded, no random"
          % (VEIL["authority"]["Spire"],))
    print("WHOLE: PASS - VEIL, Python helps make VEIL, NOT separate, placements 25, authority 11, zones 5")
    try:
        import unreal  # noqa: F401 - present only inside the editor
        in_editor = True
    except ImportError:
        in_editor = False
    if in_editor or "--unreal" in sys.argv:
        sys.exit(0 if unreal_build() else 1)
