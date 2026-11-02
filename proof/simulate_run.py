import math

PHI = 1.618033988749895
GA = 137.50776405
WORLD_SCALE = 46.97871376
PLAZA = (0.0, 0.0)
ANCH = [(x * WORLD_SCALE, y * WORLD_SCALE) for (x, y) in
        [(0, 0), (-450, 780), (2800, 500), (-4150, -780), (0, -6200)]]
REG = ["THE HUB", "THE NEST", "THE SPIRE", "OMNI DISTRICT", "THE DEEP"]
KINDS = ["kills", "fragments", "voices", "caches", "waves", "beacons"]
STEPS = {
    "kills":     [3, 6, 10, 15, 21, 28, 36, 45, 55, 66],
    "fragments": [1, 1, 2, 2, 2, 3, 3, 3, 3, 3],
    "voices":    [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
    "caches":    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
    "waves":     [1, 2, 3, 4, 5, 5, 5, 5, 5, 5],
    "beacons":   [1, 1, 1, 2, 2, 2, 3, 3, 3, 3],
}
SUPPLY = {"kills": 10**9, "fragments": 3, "voices": 10**9, "caches": 1, "waves": 5, "beacons": 3}
ACTION = {"kills": 14, "fragments": 30, "voices": 22, "caches": 95, "waves": 260, "beacons": 70}
WALK = 600.0

acts = []
for slot in range(10):
    for r in range(5):
        k = KINDS[(slot + r) % 6]
        ang = math.radians(slot * GA)
        rad = 700 + 210 * slot
        acts.append({"i": len(acts), "slot": slot, "region": r, "kind": k,
                     "req": STEPS[k][slot],
                     "x": ANCH[r][0] + math.cos(ang) * rad,
                     "y": ANCH[r][1] + math.sin(ang) * rad,
                     "done": False})

print("=" * 78)
print("VEIL OMNI - SIMULATED RUN (no engine, real constants)")
print("=" * 78)

# ---- 1. board audit -------------------------------------------------------
bad = [a for a in acts if a["req"] > SUPPLY[a["kind"]]]
print("ACTIVITY BOARD")
print("  activities: %d   regions: %d   slots: %d" % (len(acts), len(REG), 10))
print("  unreachable by supply: %d%s" % (len(bad), "" if not bad else "  <-- BUG"))
reach = max(math.hypot(a["x"] - ANCH[a["region"]][0], a["y"] - ANCH[a["region"]][1]) for a in acts)
far = max(math.hypot(a["x"], a["y"]) for a in acts)
print("  marker spread per region: %.0f cm .. %.0f cm" % (700, reach))
print("  farthest marker from plaza: %.0f cm (%.1f km)" % (far, far / 100000.0))
for k in KINDS:
    print("  %-9s supply %-4s  max ask %-3d  closes %d" %
          (k, SUPPLY[k] if SUPPLY[k] < 10**8 else "inf", max(STEPS[k]),
           sum(1 for a in acts if a["kind"] == k)))

# ---- 2. story -----------------------------------------------------------------
print()
print("THE SIX ACTS (as the campaign gates them)")
t = 0.0
events = []
def ev(name, secs, why):
    global t
    t += secs
    events.append((t, name, why))

ev("ACT I   count at the Keeper", 60 + 900 / WALK, "walk 9 m to the Keeper beacon, forge it")
ev("ACT II  three fragments", 3 * (ACTION["fragments"] + 1800 / WALK), "recover the three fragments of the voice")
ev("ACT III the Seer", 40 + 1200 / WALK, "walk the shelf, hear the ash read")
ev("ACT IV  the cache", 40 + 6200 * WORLD_SCALE / WALK + ACTION["caches"], "cross the district to the survey cache")
ev("ACT V   the Warden", 5 * ACTION["waves"] + 120, "hold five waves, put WARDEN PRIME down")
ev("ACT VI  the last beacon", 60 + 2400 / WALK, "light the final beacon - THE SIGNAL IS WHOLE")
for at, name, why in events:
    print("  %6.1f min  %-28s %s" % (at / 60.0, name, why))
end_act = t
print("  story length before activities: %.1f min" % (end_act / 60.0))

# ---- 3. the run ---------------------------------------------------------------
print()
print("THE RUN")
counters = dict.fromkeys(KINDS, 0)
pos = PLAZA
walked = 0.0
clock = end_act
closed = 0
log = []
while closed < len(acts):
    open_acts = [a for a in acts if not a["done"]]
    a = min(open_acts, key=lambda a: math.hypot(a["x"] - pos[0], a["y"] - pos[1]))
    if counters[a["kind"]] < a["req"]:
        need = a["req"] - counters[a["kind"]]
        counters[a["kind"]] += need
        cost = need * ACTION[a["kind"]]
        clock += cost
    d = math.hypot(a["x"] - pos[0], a["y"] - pos[1])
    walked += d
    clock += d / WALK
    pos = (a["x"], a["y"])
    a["done"] = True
    closed += 1
    log.append((clock, closed, a["kind"], a["region"], d))

for clock_s, n, kind, region, d in log:
    if n <= 5 or n % 10 == 0 or n >= len(acts) - 2:
        print("  %7.1f min  %2d/50  %-9s %-14s reached from %.0f cm" % (clock_s / 60.0, n, kind, REG[region], d))
print("  ..." if len(log) > 8 else "")
print()
print("RESULT")
print("  activities closed:        %d / %d" % (closed, len(acts)))
print("  total run time:           %.1f min (%.1f h)" % (clock / 60.0, clock / 3600.0))
print("  distance walked:          %.0f m (%.1f km)" % (walked / 100.0, walked / 100000.0))
print("  counters:                 " + "  ".join("%s=%d" % (k, counters[k]) for k in KINDS))
print("  ending condition:         beacons %d/3 and warden %s -> %s" %
      (counters["beacons"], "down", "COMPLETE" if counters["beacons"] >= 3 and counters["kills"] > 0 else "NOT MET"))
print("  phi check:                phi^2 - phi - 1 = %.3e" % (PHI * PHI - PHI - 1.0))
print("  golden-angle closure:     360 / 137.50776405 = %.6f (continued fraction of phi)" % (360 / GA))
