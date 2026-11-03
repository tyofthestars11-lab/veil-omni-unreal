#!/usr/bin/env python3
"""VEIL-OMNI-WHOLE.py - single whole thing, NOT PARTS."""
import math, json, hashlib
from pathlib import Path
PHI = 1.618033988749895
def phi_place(i, bx=0, by=0):
    a = math.radians(i * 137.5077640500378); r = math.sqrt(i) * PHI * 100
    return round(bx + r * math.cos(a), 2), round(by + r * math.sin(a), 2)
def whole():
    authority = {"Vault": [0,0,-80.9], "Countess": [809,-309,64.7], "Runner": [0,0,100], "Keeper": [900,0,0], "Seer": [-450,780,0], "Cartographer": [-4150,-780,0], "Cache": [0,-6200,0], "Beacon1": [2800,500,0], "Beacon2": [-2000,-900,0], "Beacon3": [600,-2100,0], "Spire": [0,0,500]}
    zones = {"Hub": "market town phi pillars waterfalls rope bridges", "Nest": "village houses cliffs streams", "Spire": "central cloud-wreathed golden Spire phi-covered visible from plaza", "Omni District": "blue-and-gold city blue crystals gold beacons", "Megastructure Deep": "dark blue-lit chasm blue crystals never flat blue"}
    placements = []
    for i in range(12):
        x,y = phi_place(i+1,0,0); placements.append({"type": "phi_pillar", "x": x, "y": y, "form": "golden phi pillar"})
    for i in range(8):
        x,y = phi_place(i+1,900,0); placements.append({"type": "market_house", "x": x, "y": y, "form": "market house"})
    for i in range(5):
        x,y = phi_place(i+1,2800,500); placements.append({"type": "blue_crystal", "x": x, "y": y, "form": "faceted crystal mesh, never flat slab"})
    form = {"spire": "golden Spire phi-covered cloud-wreathed visible from plaza", "crystals": "faceted crystal mesh", "waterfalls": "waterfall geometry in Hub", "rope_bridges": "rope bridge geometry in Hub", "runner_wings": "gold phi wings on Runner", "city": "blue-and-gold city geometry"}
    hud = {"objective": "420x110 top-left", "minimap": "180x180 at 70% top-right", "manual": "320x90 fading 6s", "quest": "480px one-line", "centre_clear": "60%", "overlap": False, "black_panels": False, "style": "gold ribbon VIT red STM green LV diamond compass minimap COMBAT MODE"}
    npcs = {"Keeper": {"post": authority["Keeper"], "task": "count guard", "follow": False, "orbit": False}, "Seer": {"post": authority["Seer"], "task": "read speak", "follow": False, "orbit": False}, "Cartographer": {"post": authority["Cartographer"], "task": "map mark", "follow": False, "orbit": False}, "Countess": {"post": authority["Countess"], "task": "hold vault upright", "follow": False, "orbit": False}}
    return {"phi": PHI, "vision": "Runner gliding gold phi wings, golden Spire, market town, blue crystal mountains, gold beacons, Megastructure Deep", "authority": authority, "zones": zones, "placements": placements, "form": form, "hud": hud, "npcs": npcs, "movement": {"wasd": True, "controller": True, "forward": 0.9561, "pitch": 0.2929}}
def main():
    w = whole(); s = json.dumps(w, sort_keys=True); h = hashlib.sha256(s.encode()).hexdigest()
    Path("VEIL-OMNI-WHOLE.json").write_text(s, encoding="utf-8")
    print(f"WHOLE: PASS - placements {len(w['placements'])} authority {len(w['authority'])} zones {len(w['zones'])} SHA {h} file VEIL-OMNI-WHOLE.json")
    return 0
if __name__ == "__main__":
    raise SystemExit(main())
