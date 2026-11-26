"""Regenerate docs/PHI_APPLIED.md from the build. Ruling 5: docs are derived, not the source.

Every row carries its provenance: a key in docs/world_layout.json, or a line printed by the game.
If a number has no source, it does not belong in this document.
"""
import io, json, re, sys, datetime, os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
J = json.load(io.open(os.path.join(ROOT, "docs/world_layout.json"), encoding="utf-8"))
LOG = sys.argv[1] if len(sys.argv) > 1 else ""
raw = io.open(LOG, encoding="utf-8", errors="replace").read() if LOG else ""

def grab(pattern, default=""):
    m = re.search(pattern, raw)
    return m.group(1).strip() if m else default

spire = grab(r"VEIL GEN spire: at=\(([-\d]+), ([-\d]+)\) steps=(\d+) height_cm=(\d+) base_w_cm=(\d+)")
spire_at = ""
if spire:
    m = re.search(r"VEIL GEN spire: at=\(([-\d]+), ([-\d]+)\)", raw)
    spire_at = "(%s, %s)" % (m.group(1), m.group(2))
rings = re.findall(r"VEIL GEN ring=(\d+) r_cm=([\d.]+) inner_cm=([\d.]+) count=(\d+) placed=(\d+)", raw)
peoples = grab(r"VEIL PEOPLES: ([^\n\r]+)")
pcheck = grab(r"VEIL PEOPLES CHECK: ([^\n\r]+)")
check = grab(r"VEIL CHECK (pass=\d+ fail=\d+ instances=\d+)")
frog = grab(r"VEIL FROG CHECK: ([^\n\r]+)")
mat = grab(r"VEIL MATERIAL: ([^\n\r]+)")
refl = grab(r"VEIL REFLECTION: ([^\n\r]+)")
hash_ = grab(r"VEIL HASH (crc=\S+ instances=\d+)")

today = datetime.date.today().isoformat()
L = []
L.append("# PHI APPLIED — generated from the build")
L.append("")
L.append("**This file is derived.** It is written by `Scripts/gen_phi_applied.py` from")
L.append("`docs/world_layout.json` and the run log; it is not a spec and nothing is copied into it by")
L.append("hand. Ruling 5 (2026-11-26): the build is the authority, the docs follow it. The previous")
L.append("hand-written version of this file claimed \"1200 wide, 5000 tall at (0, 3500)\" — the size was")
L.append("right by luck, the coordinate was the bible's and the generator never built it.")
L.append("")
L.append("Generated %s from `%s`." % (today, os.path.basename(LOG) if LOG else "(no log given)"))
L.append("")
L.append("## The world ladder")
L.append("")
L.append("| number | value | source |")
L.append("|---|---|---|")
L.append("| seed | %s | `seed` |" % J.get("seed"))
L.append("| phi | %s | `phi` |" % J.get("phi"))
L.append("| golden angle | %s deg | `golden_angle_deg` |" % J.get("golden_angle_deg"))
L.append("| base ring | %s cm | `base_ring_cm` |" % J.get("base_ring_cm"))
L.append("| rings | %s | `ring_count` |" % J.get("ring_count"))
L.append("| ring names | %s | `ring_names` |" % ", ".join(J.get("ring_names", [])))
L.append("| ring counts | %s | `counts` |" % J.get("counts"))
L.append("")
for n, r, i, c, p in rings:
    L.append("- ring %s: r_cm=%s inner_cm=%s count=%s placed=%s" % (n, r, i, c, p))
L.append("")
L.append("## The Spire (Ruling 5)")
L.append("")
L.append("| number | value | source |")
L.append("|---|---|---|")
L.append("| position | %s cm | `VEIL GEN spire` |" % (spire_at or "?"))
L.append("| height | %s cm | `spire_height_cm` |" % J.get("spire_height_cm"))
L.append("| base width | %s cm | `spire_base_width_cm` |" % J.get("spire_base_width_cm"))
L.append("| steps | %s | `spire_steps` |" % J.get("spire_steps"))
L.append("")
L.append("10,590 = 4,045 x phi^2: the Spire stands on its own ring's outer radius. The bible's")
L.append("(0, 3500) is superseded and carries a note, not a rebuild. `AVeilHubGameMode::BuildSpire()`,")
L.append("the hand-built tower at (0, 9000) that was never called, is deleted.")
L.append("")
L.append("## What the run printed")
L.append("")
L.append("| check | result |")
L.append("|---|---|")
L.append("| generator validate | `%s` |" % (check or "not run"))
L.append("| golden frame | `%s` |" % (frog or "not run"))
L.append("| census | `%s` |" % (peoples or "not run"))
L.append("| census accounting | `%s` |" % (pcheck or "not run"))
L.append("| placement hash | `%s` |" % (hash_ or "—"))
L.append("| plaza reflection | `%s` |" % (refl or "—"))
L.append("")
if mat:
    L.append("Materials: `%s`" % mat)
    L.append("")
L.append("## Not in this document")
L.append("")
L.append("Motion, lens and HUD numbers live in `Source/VEIL/VeilCanon.h`, where the game reads them.")
L.append("They are not repeated here: a second copy is the failure this file just had.")
L.append("")
io.open(os.path.join(ROOT, "docs/PHI_APPLIED.md"), "w", encoding="utf-8", newline="\n").write("\n".join(L))
print("wrote docs/PHI_APPLIED.md from", os.path.basename(LOG))
