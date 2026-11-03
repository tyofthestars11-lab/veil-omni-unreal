"""Reconcile the Hub to 25 by label, save with the editor's own level path, capture."""
import base64
import json
import os
import re
import urllib.request

URL = "http://127.0.0.1:8000/mcp"
SID = None


def rpc(payload, sid=None):
    headers = {"Content-Type": "application/json",
               "Accept": "application/json, text/event-stream"}
    if sid:
        headers["Mcp-Session-Id"] = sid
    req = urllib.request.Request(URL, data=json.dumps(payload).encode(),
                                 headers=headers, method="POST")
    with urllib.request.urlopen(req, timeout=180) as r:
        sid = r.headers.get("Mcp-Session-Id") or sid
        body = r.read().decode()
    text = body.strip()
    if "data:" in text:
        chunks = [l[5:].strip() for l in text.splitlines() if l.startswith("data:")]
        if chunks:
            text = chunks[-1]
    try:
        return json.loads(text), sid
    except json.JSONDecodeError:
        return {"raw": body}, sid


def call(toolset, tool, arguments):
    global SID
    if SID is None:
        _, SID = rpc({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                      "params": {"protocolVersion": "2025-06-18", "capabilities": {},
                                 "clientInfo": {"name": "buffy", "version": "1"}}})
        rpc({"jsonrpc": "2.0", "method": "notifications/initialized"}, SID)
    out, SID = rpc({"jsonrpc": "2.0", "id": 2, "method": "tools/call",
                    "params": {"name": "call_tool",
                               "arguments": {"toolset_name": toolset, "tool_name": tool,
                                             "arguments": arguments}}}, SID)
    content = out.get("result", {}).get("content", [])
    return content[0].get("text") if content else json.dumps(out)


SCRIPT = r'''
import json, math

GOLDEN = 137.5077640500378
ADD = "editor_toolset.toolsets.scene.SceneTools.add_to_scene_from_asset"
TAG = "editor_toolset.toolsets.actor.ActorTools.add_tag"
FIND = "editor_toolset.toolsets.scene.SceneTools.find_actors"
LABEL = "editor_toolset.toolsets.actor.ActorTools.set_label"
GET = "editor_toolset.toolsets.actor.ActorTools.get_label"
CUBE = "/Engine/BasicShapes/Cube.Cube"
CYL = "/Engine/BasicShapes/Cylinder.Cylinder"
CONE = "/Engine/BasicShapes/Cone.Cone"

def call(n, a):
    return execute_tool(n, json.dumps(a))

def rv(r):
    if isinstance(r, dict):
        try:
            return r["returnValue"]
        except Exception:
            return r
    return r

def find(tag):
    try:
        return rv(call(FIND, {"name": "", "tag": tag, "collision_channels": []})) or []
    except Exception:
        return []

def label_of(a):
    try:
        return str(rv(call(GET, {"actor": a})))
    except Exception:
        return ""

def place(path, label, tag, loc, rot, scale):
    a = rv(call(ADD, {"asset_path": path, "name": label,
         "xform": {"location": {"x": loc[0], "y": loc[1], "z": loc[2]},
                   "rotation": {"pitch": rot[0], "yaw": rot[1], "roll": rot[2]},
                   "scale": {"x": scale[0], "y": scale[1], "z": scale[2]}}}))
    if a:
        try:
            call(TAG, {"actor": a, "tag": tag})
            call(LABEL, {"actor": a, "label": label})
        except Exception:
            pass
    return a

def wanted():
    specs = []
    for i in range(12):
        ang = math.radians(i * GOLDEN)
        r = math.sqrt(i + 1) * 1.618033988749895 * 100
        specs.append((CYL, "VEIL_HUB_phi_pillar_%02d" % i,
                      (math.cos(ang) * r, math.sin(ang) * r, 300), (0, i * 30, 0), (2.0, 2.0, 16)))
    for i in range(8):
        ang = math.radians(i * GOLDEN)
        r = math.sqrt(i + 1) * 1.618033988749895 * 100
        specs.append((CUBE, "VEIL_HUB_market_house_%02d" % i,
                      (900 + math.cos(ang) * r, math.sin(ang) * r, 260), (0, i * 30, 0), (6, 6, 5)))
    for i in range(5):
        ang = math.radians(i * 72)
        specs.append((CONE, "VEIL_HUB_blue_crystal_%02d" % i,
                      (math.cos(ang) * 1600, math.sin(ang) * 1600, 500), (0, i * 72, 0), (3, 3, 12)))
    return specs

def run():
    have = [label_of(a) for a in find("VEIL_HUB")]
    out = {"have": len(have), "added": 0, "labels": have[:3]}
    for spec in wanted():
        path, label, loc, rot, scale = spec
        if label in have:
            continue
        place(path, label, "VEIL_HUB", loc, rot, scale)
        out["added"] += 1
    out["hub"] = len(find("VEIL_HUB"))
    return out
'''

print("STOP PIE:", call("EditorToolset.EditorAppToolset", "StopPIE", {})[:120])
import time
time.sleep(3)
print("BUILD:", call("editor_toolset.toolsets.programmatic.ProgrammaticToolset",
                     "execute_tool_script", {"script": SCRIPT})[:300])

lvl = call("editor_toolset.toolsets.programmatic.ProgrammaticToolset", "execute_tool_script",
           {"script": 'import json\ndef run():\n'
                      '    return {"lvl": execute_tool("editor_toolset.toolsets.scene.SceneTools.get_current_level", "{}")}'})
print("LEVEL:", lvl[:200])

m = re.search(r"/Game/[A-Za-z0-9_/]+", lvl)
candidates = []
if m:
    base = m.group(0)
    candidates += [base + "." + base.rsplit("/", 1)[-1], base]
candidates += ["/Game/Maps/VEILDistrict.VEILDistrict", "/Game/Maps/VEILDistrict"]
seen, save_ok = set(), None
for c in candidates:
    if c in seen:
        continue
    seen.add(c)
    txt = call("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": [c]})
    print("SAVE", c, "->", txt[:120])
    if "true" in txt:
        save_ok = c
        break

TRANS = {"location": {"x": 0.0, "y": -3400.0, "z": 1100.0},
         "rotation": {"pitch": -3.0, "yaw": 90.0, "roll": 0.0},
         "scale": {"x": 1.0, "y": 1.0, "z": 1.0}}
cap = call("EditorToolset.EditorAppToolset", "CaptureViewport",
           {"captureTransform": TRANS, "annotations": {}, "bShowUI": False})
img = None
for mm in re.finditer(r"[A-Za-z0-9+/=]{20000,}", cap):
    try:
        img = base64.b64decode(mm.group(0))
        break
    except Exception:
        pass
os.makedirs("Docs/shots", exist_ok=True)
if img:
    with open("Docs/shots/hub.png", "wb") as f:
        f.write(img)
    print("WROTE Docs/shots/hub.png")
else:
    print("CAPTURE ERROR:", cap[:160].replace("\n", " "))
