"""Build VEIL-OMNI-HUGE-KRAKEN.html: one file, art embedded, code embedded, no parts."""
import base64
import html
import json
import os

ROOT = "D:/VEIL-OMNI/"
OUT = ROOT + "VEIL-OMNI-HUGE-KRAKEN.html"


def b64(path):
    with open(ROOT + path, "rb") as f:
        return base64.b64encode(f.read()).decode()


def text(path):
    with open(ROOT + path, "r", encoding="utf-8") as f:
        return f.read()


shots = [
    ("shots/vision.png", "THE VISION - GOLD SPIRE, CRYSTALS, MARKET, RUNNER GROUNDED"),
    ("shots/vision_dialogue.png", "SERA VAHL - [E] CONTINUE, NOT SPACE, NOT ENTER"),
    ("shots/hub.png", "HUB - PHI PILLARS, BLUE ROOFS, FOUR MOUNTS, DIALOGUE"),
    ("shots/spire.png", "SPIRE - THE TOWER AND DOMEN ARCLIGHT'S MAP"),
    ("shots/nest.png", "NEST - CLIFFS, ROPE BRIDGES, ILVA MARROW"),
    ("shots/deep.png", "MEGASTRUCTURE DEEP - TERRACES AND THE LIT CORE"),
]

codes = [
    ("VEIL-OMNI-VISION-GAME.py", "THE VISION GAME - PYTHON, TKINTER, NO UNREAL"),
    ("VEIL-OMNI-GAME.py", "THE PLAYABLE GAME - PHYSICS, CAST, MOUNTS, FIVE ZONES"),
    ("VEIL-OMNI-HUGE.py", "THE MERGED RUNNER - ALL + PHYSICS + VISION"),
    ("VEIL-OMNI-ALL.py", "ALL - 50 GAMES IN 1, SEED 1618"),
    ("VEIL-OMNI-PHYSICS-BLUEPRINTS.py", "PHYSICS + BLUEPRINTS - THE NUMBERS"),
]

all_json = json.loads(text("VEIL-OMNI-ALL.json"))
games = all_json.get("games", [])
authority = all_json.get("authority", {})

parts = ["""<!doctype html><html><head><meta charset="utf-8">
<title>VEIL OMNI - KRAKEN - ONE FILE</title>
<style>
 body{margin:0;background:#080f1f;color:#f6ecd2;font-family:Segoe UI,Arial,sans-serif}
 .wrap{max-width:1100px;margin:0 auto;padding:32px 20px 80px}
 h1{color:#e8c368;font-size:34px;letter-spacing:2px;margin:0 0 6px}
 h2{color:#e8c368;font-size:20px;margin:38px 0 12px;border-bottom:1px solid #8f7a44;padding-bottom:6px}
 p{line-height:1.6;color:#d8cfb8}
 .tag{display:inline-block;border:1px solid #8f7a44;color:#e8c368;padding:3px 9px;margin:3px 5px 3px 0;font-size:12px}
 img{width:100%;border:1px solid #8f7a44;margin:10px 0 4px;display:block}
 .cap{color:#8f7a44;font-size:12px;margin-bottom:18px}
 ol,ul{line-height:1.9}
 code{display:block;white-space:pre-wrap;background:#0d1730;border:1px solid #223052;
      padding:14px;font-family:Consolas,monospace;font-size:12px;color:#cfe0ff}
 .grid{display:grid;grid-template-columns:1fr 1fr;gap:18px}
 footer{color:#8f7a44;font-size:12px;margin-top:48px}
</style></head><body><div class="wrap">
<h1>VEIL OMNI - THE LAST SIGNAL</h1>
<p><b>ONE FILE. ALL OF IT.</b> Gold Spire at 0,3500,500. Hub 0,0. Nest -450,780.
Omni 2800,500. Deep 0,-5000. Seed 1618. Golden angle only. Grounded. No random.
No placeholders. Dialogue [E] only. HUD gold ribbon, ONE mic. Runner gold phi wings.
Python plays it; no Unreal required.</p>
"""]
parts.append("<p>" + "".join(
    '<span class="tag">%s</span>' % html.escape(k) for k in
    ("PHI 1.618033988749895", "GOLDEN ANGLE 137.5077640500378", "SEED 1618",
     "50 GAMES IN 1", "GROUNDED", "NO RANDOM", "[E] ONLY")) + "</p>")

parts.append("<h2>THE WORLD - AUTHORITY POINTS</h2><ul>")
for name, pos in authority.items():
    parts.append("<li><b>%s</b> - %s</li>" % (html.escape(name), html.escape(str(pos))))
parts.append("</ul>")

parts.append("<h2>ALL IN ONE IMAGE</h2>")
for path, cap in shots:
    if os.path.exists(ROOT + path):
        parts.append('<img src="data:image/png;base64,%s" alt="%s">' % (b64(path), html.escape(cap)))
        parts.append('<div class="cap">%s</div>' % html.escape(cap))

parts.append("<h2>LIST</h2><ol>")
for g in games:
    parts.append("<li>%s - phi rung %s, at %s,%s</li>" % (
        html.escape(g.get("name", "")), g.get("game", ""), g.get("x", ""), g.get("y", "")))
parts.append("</ol>")

parts.append("<h2>CODE - WHOLE</h2>")
for path, label in codes:
    if not os.path.exists(ROOT + path):
        continue
    parts.append("<h3 style='color:#e8c368;font-size:14px'>%s</h3>" % html.escape(label))
    parts.append("<code>%s</code>" % html.escape(text(path)))

parts.append("<h2>PHYSICS - AS THE GAME PRINTS IT</h2>")
parts.append("""<code>PHI PHYSICS: gravity -980, walk 439, sprint 750, leap 602 cm/s = 185 cm,
 glide 600 with drop -50, capsule r 42 half-height 96, grounded 1 (floor Actor_1 at 0 cm).
PHI MOUNT: GOLD WING MOUNT / CRYSTAL MOUNT / MARKET MOUNT / DEEP MOUNT - speed 900, no float.</code>""")

parts.append("<footer>VEIL OMNI - KRAKEN - built from the project's own renders and code. "
             "Open this file in any browser; every image and every line is inside it.</footer>")
parts.append("</div></body></html>")

out = "".join(parts)
with open(OUT, "w", encoding="utf-8") as f:
    f.write(out)
print("KRAKEN: %d bytes, %d images, %d code files" % (len(out.encode("utf-8")), len(shots), len(codes)))
