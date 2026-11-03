"""DLAM editor test, whole: Hub first, count 25, Spire, save, viewport capture.

Talks to the open editor's MCP server only. No headless process, no C++ edits.
Exit code 0 = count matched and a viewport image was written.
"""
import base64
import json
import os
import re
import sys
import urllib.request

URL = "http://127.0.0.1:8000/mcp"
OUT = "Docs/shots/hub.png"


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


SID = None


def call(toolset, tool, arguments):
    global SID
    if SID is None:
        _, SID = rpc({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                      "params": {"protocolVersion": "2025-06-18", "capabilities": {},
                                 "clientInfo": {"name": "buffy-dlam", "version": "1"}}})
        rpc({"jsonrpc": "2.0", "method": "notifications/initialized"}, SID)
    out, SID = rpc({"jsonrpc": "2.0", "id": 2, "method": "tools/call",
                    "params": {"name": "call_tool",
                               "arguments": {"toolset_name": toolset, "tool_name": tool,
                                             "arguments": arguments}}}, SID)
    content = out.get("result", {}).get("content", [])
    return content[0].get("text") if content else json.dumps(out)


BUILD = r'''
import json, math

GOLDEN = 137.5077640500378
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
CUBE = "/Engine/BasicShapes/Cube.Cube"
CYL = "/Engine/BasicShapes/Cylinder.Cylinder"
CONE = "/Engine/BasicShapes/Cone.Cone"
SPH = "/Engine/BasicShapes/Sphere.Sphere"

def find(tag):
    try:
        return rv(call(FIND, {"name": "", "tag": tag, "collision_channels": []})) or []
    except Exception:
        return []

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

def run():
    out = {"removed": 0, "placed": 0, "hub": 0, "errors": []}
    for tag in ("VEIL_HUB", "VEIL_SPIRE"):
        for a in find(tag):
            try:
                call(DEL, {"actor": a}); out["removed"] += 1
            except Exception as e:
                out["errors"].append(str(e)[:60])

    # 12 phi pillars at the Hub, golden angle, no random
    for i in range(12):
        ang = math.radians(i * GOLDEN)
        r = math.sqrt(i + 1) * 1.618033988749895 * 100
        place(CYL, "VEIL_HUB_phi_pillar_%02d" % i, "VEIL_HUB",
              (math.cos(ang) * r, math.sin(ang) * r, 300), (0, i * 30, 0), (2.0, 2.0, 16))
        out["placed"] += 1

    # 8 market houses, offset east
    for i in range(8):
        ang = math.radians(i * GOLDEN)
        r = math.sqrt(i + 1) * 1.618033988749895 * 100
        place(CUBE, "VEIL_HUB_market_house_%02d" % i, "VEIL_HUB",
              (900 + math.cos(ang) * r, math.sin(ang) * r, 260), (0, i * 30, 0), (6, 6, 5))
        out["placed"] += 1

    # 5 blue crystals
    for i in range(5):
        ang = math.radians(i * 72)
        place(CONE, "VEIL_HUB_blue_crystal_%02d" % i, "VEIL_HUB",
              (math.cos(ang) * 1600, math.sin(ang) * 1600, 500), (0, i * 72, 0), (3, 3, 12))
        out["placed"] += 1

    # the Spire on the law's address, 500 up
    place(CYL, "VEIL_SPIRE_base", "VEIL_SPIRE", (0, 3500, 2100), (0, 0, 0), (30, 30, 30))
    place(CYL, "VEIL_SPIRE_tier2", "VEIL_SPIRE", (0, 3500, 4900), (0, 22, 0), (20, 20, 28))
    place(CONE, "VEIL_SPIRE_crown", "VEIL_SPIRE", (0, 3500, 7600), (0, 0, 0), (13, 13, 26))
    place(SPH, "VEIL_SPIRE_orb", "VEIL_SPIRE", (0, 3500, 9200), (0, 0, 0), (5, 5, 5))
    out["placed"] += 4
    out["hub"] = len(find("VEIL_HUB"))
    return out
'''


CAPTURE_VARIANTS = [
    {"captureTransform": {"location": {"x": 0.0, "y": -2600.0, "z": 900.0},
                          "rotation": {"pitch": -6.0, "yaw": 90.0, "roll": 0.0},
                          "scale": {"x": 1.0, "y": 1.0, "z": 1.0}},
     "annotations": False, "bShowUI": False},
    {"captureTransform": {"location": {"x": 0.0, "y": -2600.0, "z": 900.0},
                          "rotation": {"pitch": -6.0, "yaw": 90.0, "roll": 0.0}},
     "annotations": False, "bShowUI": False},
    {"captureTransform": {"location": {"x": 0, "y": -2600, "z": 900},
                          "rotation": {"pitch": -6, "yaw": 90, "roll": 0},
                          "scale": {"x": 1, "y": 1, "z": 1}}},
    {"captureTransform": None},
]


def decode(text):
    for m in re.finditer(r"[A-Za-z0-9+/=]{20000,}", text):
        try:
            return base64.b64decode(m.group(0))
        except Exception:
            continue
    return None


def main():
    build_txt = call("editor_toolset.toolsets.programmatic.ProgrammaticToolset",
                     "execute_tool_script", {"script": BUILD})
    print("BUILD:", build_txt[:600])

    count_txt = call("editor_toolset.toolsets.programmatic.ProgrammaticToolset",
                     "execute_tool_script",
                     {"script": 'import json\ndef run():\n'
                                '    r = execute_tool("editor_toolset.toolsets.scene.SceneTools.find_actors", '
                                'json.dumps({"name": "", "tag": "VEIL_HUB", "collision_channels": []}))\n'
                                '    return {"hub": len(r["returnValue"])}'})
    print("COUNT:", count_txt[:200])

    save_txt = call("editor_toolset.toolsets.asset.AssetTools", "save_assets",
                    {"asset_paths": ["/Game/Maps/VEILDistrict"]})
    print("SAVE:", save_txt[:200])

    image = None
    for i, variant in enumerate(CAPTURE_VARIANTS):
        txt = call("EditorToolset.EditorAppToolset", "CaptureViewport", variant)
        image = decode(txt)
        if image:
            print("CAPTURE: variant %d ok" % i)
            break
        print("CAPTURE variant %d error: %s" % (i, txt[:220].replace("\n", " ")))

    if image is None:
        txt = call("EditorToolset.EditorAppToolset", "CaptureEditorImage", {})
        image = decode(txt)
        if image:
            print("CAPTURE: fell back to CaptureEditorImage (whole editor window)")

    if image:
        os.makedirs(os.path.dirname(OUT), exist_ok=True)
        with open(OUT, "wb") as f:
            f.write(image)
        print("WROTE", OUT)

    hub = None
    for token in ("\"hub\":", "hub"):
        if token in count_txt:
            m = re.search(r'hub\D{0,6}(\d+)', count_txt)
            if m:
                hub = int(m.group(1))
                break
    ok = (hub == 25 and image is not None)
    print("RESULT:", "PASS" if ok else "FAIL", "hub=%s capture=%s" % (hub, bool(image)))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
