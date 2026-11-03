"""Build VEIL through the OPEN editor's MCP hand, save the level, capture the viewport."""
import base64
import json
import re
import urllib.request

URL = "http://127.0.0.1:8000/mcp"


def rpc(payload, sid=None):
    headers = {"Content-Type": "application/json",
               "Accept": "application/json, text/event-stream"}
    if sid:
        headers["Mcp-Session-Id"] = sid
    req = urllib.request.Request(URL, data=json.dumps(payload).encode(),
                                 headers=headers, method="POST")
    with urllib.request.urlopen(req, timeout=240) as r:
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


def call_tool(toolset, tool, arguments, sid=None):
    if sid is None:
        _, sid = rpc({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                      "params": {"protocolVersion": "2025-06-18", "capabilities": {},
                                 "clientInfo": {"name": "buffy-dlam", "version": "1"}}})
        rpc({"jsonrpc": "2.0", "method": "notifications/initialized"}, sid)
    out, sid = rpc({"jsonrpc": "2.0", "id": 2, "method": "tools/call",
                    "params": {"name": "call_tool",
                               "arguments": {"toolset_name": toolset, "tool_name": tool,
                                             "arguments": arguments}}}, sid)
    content = out.get("result", {}).get("content", [])
    return (content[0].get("text") if content else json.dumps(out)), sid


BUILD = r'''
import json, math

def call(name, args):
    return execute_tool(name, json.dumps(args))

def rv(r):
    if isinstance(r, dict):
        try:
            return r["returnValue"]
        except Exception:
            return r
    return r

ADD = "editor_toolset.toolsets.scene.SceneTools.add_to_scene_from_asset"
TAG = "editor_toolset.toolsets.actor.ActorTools.add_tag"
FIND = "editor_toolset.toolsets.scene.SceneTools.find_actors"
DEL = "editor_toolset.toolsets.scene.SceneTools.remove_from_scene"
LABEL = "editor_toolset.toolsets.actor.ActorTools.set_label"
SAVE = "editor_toolset.toolsets.asset.AssetTools.save_assets"
CUBE = "/Engine/BasicShapes/Cube.Cube"
CYL = "/Engine/BasicShapes/Cylinder.Cylinder"
CONE = "/Engine/BasicShapes/Cone.Cone"
SPH = "/Engine/BasicShapes/Sphere.Sphere"
out = {"level": None, "removed": 0, "placed": 0, "errors": [], "save": None}

def put(path, name, loc, rot, scale):
    try:
        a = rv(call(ADD, {"asset_path": path, "name": name,
              "xform": {"location": {"x": loc[0], "y": loc[1], "z": loc[2]},
                        "rotation": {"pitch": rot[0], "yaw": rot[1], "roll": rot[2]},
                        "scale": {"x": scale[0], "y": scale[1], "z": scale[2]}}}))
        if a:
            try:
                call(TAG, {"actor": a, "tag": "VEIL_PY"})
                call(LABEL, {"actor": a, "label": name})
            except Exception:
                pass
            out["placed"] += 1
        return a
    except Exception as e:
        out["errors"].append(name + ": " + str(e)[:90])
        return None

def run():
    out["level"] = rv(call("editor_toolset.toolsets.scene.SceneTools.get_current_level", {}))
    try:
        old_actors = rv(call(FIND, {"name": "", "tag": "VEIL_PY", "collision_channels": []})) or []
    except Exception as e:
        out["errors"].append("find: " + str(e)[:90])
        old_actors = []
    for a in old_actors:
        try:
            call(DEL, {"actor": a}); out["removed"] += 1
        except Exception as e:
            out["errors"].append("del: " + str(e)[:70])

    put(CYL, "VEIL_PY_SPIRE_base", (0, 3500, 1600), (0, 0, 0), (30, 30, 32))
    put(CYL, "VEIL_PY_SPIRE_tier2", (0, 3500, 4400), (0, 22, 0), (20, 20, 30))
    put(CYL, "VEIL_PY_SPIRE_tier3", (0, 3500, 6900), (0, 45, 0), (12, 12, 24))
    put(CONE, "VEIL_PY_SPIRE_crown", (0, 3500, 9500), (0, 0, 0), (13, 13, 26))
    put(SPH, "VEIL_PY_SPIRE_orb", (0, 3500, 11200), (0, 0, 0), (5, 5, 5))
    for i in range(8):
        ang = math.radians(i * 45 + 12)
        put(CUBE, "VEIL_PY_SPIRE_fin", (math.cos(ang) * 1150, 3500 + math.sin(ang) * 1150, 2400),
            (0, i * 45, 0), (2.4, 1.0, 22))

    for i in range(8):
        ang = math.radians(i * 45)
        cx, cy = math.cos(ang) * 1050, math.sin(ang) * 1050
        for side in (-1, 1):
            put(CYL, "VEIL_PY_ARCH_pier",
                (cx + math.cos(ang) * 350 * side, cy + math.sin(ang) * 350 * side, 450),
                (0, 0, 0), (1.1, 1.1, 9))
        put(CUBE, "VEIL_PY_ARCH_lintel", (cx, cy, 960), (0, i * 45 + 90, 0), (8.4, 1.5, 1.1))
        put(CUBE, "VEIL_PY_ARCH_key", (cx, cy, 1060), (0, i * 45 + 90, 0), (1.2, 1.2, 1.0))

    for i in range(12):
        ang = math.radians(i * 137.5077640500378)
        rad = 1700 + 700 * (i % 3)
        x, y = math.cos(ang) * rad, math.sin(ang) * rad
        h = 5 + 1.8 * (i % 4)
        put(CUBE, "VEIL_PY_STALL", (x, y, h * 50), (0, i * 30, 0), (8, 6, h))
        put(CUBE, "VEIL_PY_AWNING", (x, y, h * 100 + 260), (0, i * 30, 0), (10, 8, 0.5))
        put(CYL, "VEIL_PY_LANTERN", (x * 1.02, y * 1.02, 320), (0, 0, 0), (0.8, 0.8, 6.5))
        put(SPH, "VEIL_PY_LAMP", (x * 1.02, y * 1.02, 720), (0, 0, 0), (1.5, 1.5, 1.5))

    for i in range(6):
        ang = math.radians(i * 60 + 30)
        x1, y1 = math.cos(ang) * 2100, math.sin(ang) * 2100
        x2, y2 = math.cos(ang) * 3600, math.sin(ang) * 3600
        span = math.hypot(x2 - x1, y2 - y1)
        yaw = math.degrees(math.atan2(y2 - y1, x2 - x1))
        put(CUBE, "VEIL_PY_BRIDGE", ((x1 + x2) / 2, (y1 + y2) / 2, 460), (0, yaw, 0),
            (span / 100, 1.2, 0.35))

    for i, (fx, fy) in enumerate(((-2400, 1500), (-2600, 300), (2800, -1200), (2400, 1600))):
        put(CUBE, "VEIL_PY_FALL", (fx, fy, 2300), (0, i * 22, 0), (0.7, 9, 46))

    for i in range(6):
        ang = math.radians(i * 61)
        put(CONE, "VEIL_PY_CRYSTAL", (-450 + math.cos(ang) * 2600, 780 + math.sin(ang) * 2600, 900),
            (0, i * 61, 0), (3.4, 3.4, 18))

    for i in range(9):
        ang = math.radians(i * 137.5077640500378 + 10)
        rad = 1200 + 500 * (i % 3)
        x, y = 2800 + math.cos(ang) * rad, 500 + math.sin(ang) * rad
        h = 16 + 7 * (i % 4)
        put(CUBE, "VEIL_PY_OMNI_tower", (x, y, h * 50), (0, i * 45, 0), (7, 7, h))
        put(CUBE, "VEIL_PY_OMNI_band", (x, y, h * 100 + 120), (0, i * 45, 0), (7.6, 7.6, 0.9))

    put(CYL, "VEIL_PY_DEEP_ring", (0, -5000, 9600), (0, 0, 0), (62, 62, 2.4))
    put(CONE, "VEIL_PY_DEEP_crown", (0, -5000, 11200), (0, 0, 0), (26, 26, 34))
    for i in range(5):
        ang = math.radians(i * 72 + 18)
        put(CONE, "VEIL_PY_DEEP_spire",
            (math.cos(ang) * 2600, -5000 + math.sin(ang) * 2600, 2600), (0, i * 72, 0), (7, 7, 52))

    try:
        out["save"] = call(SAVE, {"assetPaths": ["/Game/Maps/VEILDistrict"]})
    except Exception as e:
        out["save_error"] = str(e)[:300]
    return out
'''

text, sid = call_tool("editor_toolset.toolsets.programmatic.ProgrammaticToolset",
                      "execute_tool_script", {"script": BUILD})
print("BUILD RESULT:", text[:1500])

CAPTURE = {"captureTransform": {"location": {"x": 2100, "y": -2300, "z": 1500},
                                "rotation": {"pitch": -14.0, "yaw": 47.0, "roll": 0.0},
                                "scale": {"x": 1.0, "y": 1.0, "z": 1.0}},
           "annotations": False, "bShowUI": False}
cap, _ = call_tool("EditorToolset.EditorAppToolset", "CaptureViewport", CAPTURE, sid)
if "NO IMAGE" in cap or "could not convert" in cap or "needs a default" in cap:
    cap, _ = call_tool("EditorToolset.EditorAppToolset", "CaptureEditorImage", {}, sid)
blob = None
for m in re.finditer(r"[A-Za-z0-9+/=]{20000,}", cap):
    blob = m.group(0)
    break
if blob:
    open("Saved/Screenshots/dlam_hand.png", "wb").write(base64.b64decode(blob))
    print("CAPTURED Saved/Screenshots/dlam_hand.png")
else:
    print("NO IMAGE:", cap[:400])
