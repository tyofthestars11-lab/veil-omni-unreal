"""A piece the audit seated is repaired; only a piece left floating is chaos."""

p = "C:/Users/Admin/HarkGame/Scripts/golden_eye.py"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old = '''    gaps = [float(g) for g in re.findall(r"closed a ([0-9.]+) gap", log)]
    bad = [g for g in gaps if g > SANCTUARY]
    if bad:
        chaos("PLACEMENT", 2, f"{len(bad)} piece(s) authored above {SANCTUARY:.0f} of support, worst {max(bad):.0f}")'''

new = '''    gaps = [float(g) for g in re.findall(r"closed a ([0-9.]+) gap", log)]
    bad = [g for g in gaps if g > SANCTUARY]
    if bad:
        # The audit closes these by construction, so they are repaired noise, not chaos.
        # Chaos is reserved for a piece still floating when the world is seated.
        chaos("PLACEMENT", 1,
              f"{len(bad)} piece(s) were authored above {SANCTUARY:.0f} above support and seated, worst {max(bad):.0f}")

    seated = re.search(r"VEIL WORLD SEATED:.*missing=(\\d+)", log)
    if not seated:
        chaos("PLACEMENT", 2, "no seated audit line: support is unproven")
    elif int(seated.group(1)) != 0:
        chaos("PLACEMENT", 3, f"{seated.group(1)} piece(s) missing from the seated world")'''

if old in s:
    s = s.replace(old, new, 1)
    open(p, "w", encoding="utf-8", newline="").write(s)
    print("eye: repaired placements are info, unfixed float is chaos")
else:
    print("eye: MISS placement rule")
