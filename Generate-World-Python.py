#!/usr/bin/env python3
# Generate-World-Python.py - Generate the world in Python, whole, over the vision.
import math, json
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
PHI = 1.618033988749895
def phi_place(i, bx=0, by=0):
    a = math.radians(i * 137.5077640500378); r = math.sqrt(i) * PHI * 100
    return bx + r * math.cos(a), by + r * math.sin(a)
fig, ax = plt.subplots(figsize=(12, 9))
ax.set_facecolor("#0b1e3a"); ax.set_xlim(-5000, 5000); ax.set_ylim(-7000, 5000); ax.set_aspect("equal"); ax.axis("off")
ax.set_title("VEIL OMNI - PHI WORLD, WHOLE", color="gold", fontsize=16)
zones = {"Hub": (0,0,"gold"), "Nest": (-2000,2000,"#8a5a2b"), "Spire": (0,0,"#ffd700"), "Omni District": (2800,500,"#1e90ff"), "Megastructure Deep": (0,-5000,"#0a2a6b")}
for name,(x,y,c) in zones.items():
    ax.add_patch(plt.Circle((x,y), 1200, color=c, alpha=0.15)); ax.text(x, y+1300, name, color="white", ha="center", fontsize=10)
placements=[]
for i in range(12):
    x,y = phi_place(i+1,0,0); placements.append(("phi_pillar",x,y)); ax.plot(x,y, marker="^", color="gold", markersize=12)
for i in range(8):
    x,y = phi_place(i+1,900,0); placements.append(("market_house",x,y)); ax.plot(x,y, marker="s", color="#d4a017", markersize=9)
for i in range(5):
    x,y = phi_place(i+1,2800,500); placements.append(("blue_crystal",x,y)); ax.plot(x,y, marker="D", color="#00bfff", markersize=14)
authority = {"Vault": (0,0), "Keeper": (900,0), "Spire": (0,0), "Beacon1": (2800,500), "Beacon2": (-2000,-900), "Beacon3": (600,-2100), "Runner": (0,0), "Countess": (809,-309), "Seer": (-450,780), "Cartographer": (-4150,-780), "Cache": (0,-6200)}
for name,(x,y) in authority.items():
    ax.plot(x,y, marker="*", color="white", markersize=10); ax.text(x+80, y+80, name, color="white", fontsize=8)
ax.plot([-2000,0,2800], [2000,0,500], color="#00bfff", linewidth=3, alpha=0.8)
ax.plot([0,900], [0,0], color="#c19a6b", linewidth=2)
ax.text(0, -6500, "Waterfalls + Rope Bridges + Blue Crystals + Gold Spire + Blue-and-Gold City - whole, not random", color="gold", ha="center")
plt.savefig("VEIL-OMNI-World.png", dpi=150, bbox_inches="tight")
world = {"placements": placements, "authority": authority, "zones": list(zones.keys())}
open("VEIL-OMNI-World.json","w").write(json.dumps(world, indent=2))
print(f"WHOLE: PASS - placements {len(placements)} authority {len(authority)} zones {len(zones)}")
