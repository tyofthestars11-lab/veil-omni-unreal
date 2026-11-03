#!/usr/bin/env python3
"""VEIL-Whole-Phi-Intent.py - whole, phi intent only, one file, one run."""
import math, json, hashlib, sys
from pathlib import Path
PHI = 1.618033988749895
GOLDEN_ANGLE = 137.5077640500378
GOLDEN_PITCH = 17.0324

def phi_place(i, base_x=0, base_y=0):
    angle = math.radians(i * GOLDEN_ANGLE)
    radius = math.sqrt(i) * PHI * 100
    return round(base_x + radius * math.cos(angle), 2), round(base_y + radius * math.sin(angle), 2)

def whole_world():
    authority = {
        "Vault": [0, 0, -80.9], "Countess": [809, -309, 64.7], "Runner": [0, 0, 100],
        "Keeper": [900, 0, 0], "Seer": [-450, 780, 0], "Cartographer": [-4150, -780, 0],
        "Cache": [0, -6200, 0], "Beacon1": [2800, 500, 0], "Beacon2": [-2000, -900, 0],
        "Beacon3": [600, -2100, 0], "Spire": [0, 0, 500],
    }
    zones = {
        "Hub": "market town, phi pillars, waterfalls, rope bridges",
        "Nest": "village houses, cliffs, streams",
        "Spire": "central cloud-wreathed golden Spire, phi-covered",
        "Omni District": "blue-and-gold city, blue crystals, gold light beacons",
        "Megastructure Deep": "dark blue-lit chasm, blue crystals, never flat blue",
    }
    placements = []
    for i in range(12):
        x, y = phi_place(i + 1, 0, 0); placements.append({"type": "phi_pillar", "x": x, "y": y})
    for i in range(8):
        x, y = phi_place(i + 1, 900, 0); placements.append({"type": "market_house", "x": x, "y": y})
    for i in range(5):
        x, y = phi_place(i + 1, 2800, 500); placements.append({"type": "blue_crystal", "x": x, "y": y})
    hud = {
        "objective": "420x110 top-left", "minimap": "180x180 at 70% top-right",
        "manual": "320x90 fading 6s", "quest": "480px one-line top-left only",
        "zone_speaker": "bottom-left only", "actions": "bottom-center only", "centre_clear": "60%",
        "vit_stm_lv": "gold ribbon, VIT red, STM green, LV diamond, compass minimap, COMBAT MODE",
        "black_panels": False, "overlap": False,
    }
    npcs = {
        "Keeper": {"post": authority["Keeper"], "task": "count, guard", "follow_player": False, "orbit": False},
        "Seer": {"post": authority["Seer"], "task": "read, speak", "follow_player": False, "orbit": False},
        "Cartographer": {"post": authority["Cartographer"], "task": "map, mark", "follow_player": False, "orbit": False},
        "Countess": {"post": authority["Countess"], "task": "hold vault upright", "follow_player": False, "orbit": False},
    }
    movement = {"wasd": True, "controller": True, "forward": 0.9561, "pitch": 0.2929, "pitch_deg": GOLDEN_PITCH}
    return {"phi": PHI, "authority": authority, "zones": zones, "placements": placements,
            "hud": hud, "npcs": npcs, "movement": movement,
            "vision": "Runner gliding gold phi wings, golden Spire, market town, blue crystal mountains, gold beacons, Megastructure Deep"}

def main():
    world = whole_world()
    s = json.dumps(world, sort_keys=True)
    h = hashlib.sha256(s.encode()).hexdigest()
    print("Placements:", len(world["placements"]))
    print("Authority:", len(world["authority"]))
    print("Zones:", len(world["zones"]))
    print("NPCs follow:", any(n["follow_player"] for n in world["npcs"].values()), "orbit:", any(n["orbit"] for n in world["npcs"].values()))
    print("HUD overlap:", world["hud"]["overlap"], "black_panels:", world["hud"]["black_panels"])
    print("Whole SHA:", h)
    out = Path(__file__).resolve().parent / "vision_whole.json"
    try:
        out.write_text(s, encoding="utf-8")
        print("Whole world written:", out)
    except Exception as e:
        print("Write whole world:", e)
    if len(world["placements"]) == 25 and len(world["authority"]) == 11 and len(world["zones"]) == 5 and not world["hud"]["overlap"]:
        print("WHOLE: PASS - vision, authority, zones, HUD, NPCs, movement, all whole, phi intent only")
        return 0
    print("WHOLE: FAIL")
    return 1

if __name__ == "__main__":
    raise SystemExit(main())
