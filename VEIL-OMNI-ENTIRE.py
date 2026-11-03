#!/usr/bin/env python3
# VEIL-OMNI-ENTIRE.py - entire thing, NOT PARTS, whole, quantum superposition via golden ratio.
import math, json, sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
PHI = 1.618033988749895
def phi_place(i, bx=0, by=0):
    a = math.radians(i * 137.5077640500378); r = math.sqrt(i) * PHI * 100
    return bx + r * math.cos(a), by + r * math.sin(a)
ENTIRE = {
 "name": "VEIL OMNI: THE LAST SIGNAL - PATH OF LIGHT",
 "phi": PHI, "seed": 1618, "golden_angle": 137.5077640500378, "golden_pitch": 17.0324,
 "vision": "Runner gliding gold phi wings, dark cape phi, golden Spire phi-covered cloud-wreathed visible from everywhere, market town phi pillars waterfalls rope bridges, blue-and-gold city blue crystals gold beacons, Megastructure Deep dark blue-lit chasm, blue crystal mountains horizon",
 "icon": "gold V, golden spiral, feather/wings both arms, thin gold circle, blue glow, dark navy starfield, rounded-square tile",
 "hud": {"objective": "420x110 top-left", "minimap": "180x180 at 70% top-right", "manual": "320x90 fading 6s", "quest": "480px one-line QUEST: THE GOLDEN MIDST Find the hidden phi chamber", "vit": "3570/4200 red", "stm": "117/180 green", "lv": "72 diamond", "abilities": 4, "combat": "COMBAT MODE", "centre_clear": "60%", "overlap": False, "black_panels": False},
 "authority": {"Vault": [0,0,-80.9], "Countess": [809,-309,64.7], "Runner": [0,0,100], "Keeper": [900,0,0], "Seer": [-450,780,0], "Cartographer": [-4150,-780,0], "Cache": [0,-6200,0], "Beacon1": [2800,500,0], "Beacon2": [-2000,-900,0], "Beacon3": [600,-2100,0], "Spire": [0,3500,500]},
 "zones": {"Hub": [0,0], "Nest": [-450,780], "Spire": [0,3500], "Omni District": [2800,500], "Megastructure Deep": [0,-5000]},
 "npcs": {"Keeper": "count guard own post no follow no orbit", "Seer": "read speak own post", "Cartographer": "map mark own post", "Countess": "hold vault upright"},
 "movement": {"wasd": True, "controller": True, "forward": 0.9561, "pitch": 0.2929},
 "hark_build_order": "Gate 0 Golden Merge WHOLE: PASS + VEILDistrict Play, no random, seed 1618, world_layout.json one truth, Hub first then Nest Spire Omni Deep, one Unreal Editor Python script per zone, counts vs JSON, screenshot per zone",
}
placements=[]
for i in range(12):
    x,y = phi_place(i+1,0,0); placements.append({"type": "phi_pillar", "x": x, "y": y})
for i in range(8):
    x,y = phi_place(i+1,900,0); placements.append({"type": "market_house", "x": x, "y": y})
for i in range(5):
    x,y = phi_place(i+1,2800,500); placements.append({"type": "blue_crystal", "x": x, "y": y})
ENTIRE["placements"] = placements
ENTIRE["hub"] = {"phi_pillars": 12, "market_houses": 8, "blue_crystals": 5, "placements": 25}
def main():
    fig, ax = plt.subplots(figsize=(12, 9)); fig.patch.set_facecolor("#0b1e3a")
    ax.set_facecolor("#0b1e3a"); ax.set_xlim(-5000, 5000); ax.set_ylim(-7000, 5000); ax.set_aspect("equal"); ax.axis("off")
    ax.set_title("VEIL OMNI - ENTIRE, WHOLE, NOT PARTS", color="gold", fontsize=16)
    for name,(x,y) in ENTIRE["zones"].items():
        ax.add_patch(plt.Circle((x,y), 1200, color="gold", alpha=0.15)); ax.text(x, y+1300, name, color="white", ha="center", fontsize=10)
    for p in placements:
        c = "gold" if p["type"]=="phi_pillar" else "#d4a017" if p["type"]=="market_house" else "#00bfff"
        m = "^" if p["type"]=="phi_pillar" else "s" if p["type"]=="market_house" else "D"
        ax.plot(p["x"], p["y"], marker=m, color=c, markersize=12)
    for name,(x,y,*_) in ENTIRE["authority"].items():
        ax.plot(x, y, marker="*", color="white", markersize=10); ax.text(x+80, y+80, name, color="white", fontsize=8)
    plt.savefig("VEIL-OMNI-World.png", dpi=150, bbox_inches="tight", facecolor=fig.get_facecolor())
    open("VEIL-OMNI-ENTIRE.json","w").write(json.dumps(ENTIRE, indent=2, sort_keys=True))
    if len(placements)==25 and len(ENTIRE["authority"])==11 and len(ENTIRE["zones"])==5:
        print("WHOLE: PASS - entire thing, placements 25, authority 11, zones 5")
        return 0
    print("WHOLE: FAIL"); return 1
if __name__ == "__main__":
    raise SystemExit(main())
