#!/usr/bin/env python3
"""Compare the generator's world law against what the builder actually spawns.

Two spires were built because nothing compared the law to the code. This reads
VEIL-OMNI-World.json and asserts each authority address appears in the sources that
claim to implement it. Exit 1 on any drift.
"""
import json, pathlib, sys, re

ROOT = pathlib.Path(__file__).resolve().parent.parent
world = json.loads((ROOT / "VEIL-OMNI-World.json").read_text(encoding="utf-8"))
auth = world["authority"]

src = {
    "builder": (ROOT / "Source/VEIL/Private/World/VEILWorldBuilder.cpp").read_text(encoding="utf-8", errors="replace"),
    "regions": (ROOT / "Source/VEIL/Private/Systems/VEILRegionSubsystem.cpp").read_text(encoding="utf-8", errors="replace"),
}

fails = []
def check(name, ok, detail):
    print(("PASS " if ok else "FAIL ") + name + " - " + detail)
    if not ok:
        fails.append(name)

# The Spire is the landmark the law moved off the Hub. It must be at its authority address
# in both the pass that draws it and the subsystem that names the region it stands in.
sx, sy = auth["Spire"]
check("spire/forms-pass", f"const float SpireX = {sx}.f, SpireY = {sy}.f" in src["builder"].replace(f"{sx}.0f", f"{sx}.f").replace(str(int(sx)), f"{int(sx)}"),
      f"forms pass uses SpireX/SpireY = {sx},{sy}")
check("spire/palette-pass", "const FVector Spire(0.f, 3500.f, 0.f)" in src["builder"],
      "palette pass builds the same address, not a second tower")
check("spire/region-centre", "FVector2D(0.f, 35.f) * S" in src["regions"],
      "region centre resolves to 0,3500 at world scale 100")
check("spire/not-vault", auth["Spire"] != auth["Vault"],
      f"Spire {auth['Spire']} is not the Vault {auth['Vault']}")

# The nine other authority addresses that the builder is expected to honour.
for name, (x, y) in auth.items():
    if name in ("Spire", "Vault", "Runner"):
        continue
    pair = f"{int(x)},{int(y)}"
    # Compare numbers, not source spelling: the builder writes -2000.f and (600.f, -2100.f).
    flat = re.sub(r"\.0*f", "", src["builder"])
    # The authority table writes TEXT("809"), TEXT("-309"); strip the quotes so a pair of
    # numbers can be compared as a pair instead of two separate string literals.
    flat = flat.replace('TEXT("', "").replace('")', "")
    flat = flat.replace(", ", ",").replace(" ", "")
    present = pair in flat
    check(f"authority/{name}", present, f"builder mentions {pair}")

# Counts, so a placement cannot quietly disappear.
check("law/placements", len(world["placements"]) == 25, f"{len(world['placements'])} placements")
check("law/authority", len(auth) == 11, f"{len(auth)} authority points")
check("law/zones", len(world["zones"]) == 5, f"{len(world['zones'])} zones")

print("\nLAW: " + ("PASS" if not fails else "FAIL - " + ", ".join(fails)))
sys.exit(0 if not fails else 1)
