"""Whole world through the editor hand: five zones, cast, save, per-zone shots."""
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


BUILD = r'''
import json, math

GOLDEN = 137.5077640500378
PHI = 1.618033988749895
ADD = "editor_toolset.toolsets.scene.SceneTools.add_to_scene_from_asset"
TAG = "editor_toolset.toolsets.actor.ActorTools.add_tag"
FIND = "editor_toolset.toolsets.scene.SceneTools.find_actors"
LABEL = "editor_toolset.toolsets.actor.ActorTools.set_label"
GET = "editor_toolset.toolsets.actor.ActorTools.get_label"
CUBE = "/Engine/BasicShapes/Cube.Cube"
CYL = "/Engine/BasicShapes/Cylinder.Cylinder"
CONE = "/Engine/BasicShapes/Cone.Cone"
SPH = "/Engine/BasicShapes/Sphere.Sphere"

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

def count(tag):
    return len(find(tag))

def run():
    out = {"placed": 0, "hub": count("VEIL_HUB"), "nest": 0, "spire": count("VEIL_SPIRE"),
           "omni": 0, "deep": 0, "cast": 0}

    # NEST at (-450, 780): terraces, falls, crystals
    NX, NY = -450.0, 780.0
    if count("VEIL_NEST") < 16:
        for i in range(8):
            place(CUBE, "VEIL_NEST_terrace_%02d" % i, "VEIL_NEST",
                  (NX - 700 + i * 260, NY + 500, 2400 + i * 220), (0, i * 9, 0), (5, 5, 4))
            out["placed"] += 1
        for i in range(4):
            place(CUBE, "VEIL_NEST_fall_%02d" % i, "VEIL_NEST",
                  (NX - 1600 + i * 360, NY + 900, 2000), (0, i * 14, 0), (0.7, 7, 38))
            out["placed"] += 1
        for i in range(4):
            place(CONE, "VEIL_NEST_crystal_%02d" % i, "VEIL_NEST",
                  (NX + 900 - i * 300, NY - 700 + i * 260, 700), (0, i * 61, 0), (3, 3, 14))
            out["placed"] += 1
    out["nest"] = count("VEIL_NEST")

    # SPIRE at (0, 3500): fins and rings around the existing stack
    if count("VEIL_SPIRE") < 10:
        for i in range(8):
            ang = math.radians(i * 45 + 12)
            place(CUBE, "VEIL_SPIRE_fin_%02d" % i, "VEIL_SPIRE",
                  (math.cos(ang) * 1150, 3500 + math.sin(ang) * 1150, 2400),
                  (0, i * 45, 0), (2.4, 1.0, 22))
            out["placed"] += 1
    out["spire"] = count("VEIL_SPIRE")

    # OMNI at (2800, 500): towers, bands, crystals
    if count("VEIL_OMNI") < 18:
        for i in range(9):
            ang = math.radians(i * GOLDEN + 10)
            rad = 1200 + 500 * (i % 3)
            x, y = 2800 + math.cos(ang) * rad, 500 + math.sin(ang) * rad
            h = 16 + 7 * (i % 4)
            place(CUBE, "VEIL_OMNI_tower_%02d" % i, "VEIL_OMNI", (x, y, h * 50),
                  (0, i * 45, 0), (7, 7, h))
            place(CUBE, "VEIL_OMNI_band_%02d" % i, "VEIL_OMNI", (x, y, h * 100 + 120),
                  (0, i * 45, 0), (7.6, 7.6, 0.9))
        for i in range(6):
            ang = math.radians(i * 60)
            place(CONE, "VEIL_OMNI_crystal_%02d" % i, "VEIL_OMNI",
                  (2800 + math.cos(ang) * 2200, 500 + math.sin(ang) * 2200, 800),
                  (0, i * 60, 0), (3.4, 3.4, 16))
    out["omni"] = count("VEIL_OMNI")

    # DEEP at (0, -5000): ring, crown, five spires, bridges
    if count("VEIL_DEEP") < 12:
        place(CYL, "VEIL_DEEP_ring", "VEIL_DEEP", (0, -5000, 9600), (0, 0, 0), (62, 62, 2.4))
        place(CONE, "VEIL_DEEP_crown", "VEIL_DEEP", (0, -5000, 11200), (0, 0, 0), (26, 26, 34))
        for i in range(5):
            ang = math.radians(i * 72 + 18)
            place(CONE, "VEIL_DEEP_spire_%02d" % i, "VEIL_DEEP",
                  (math.cos(ang) * 2600, -5000 + math.sin(ang) * 2600, 2600),
                  (0, i * 72, 0), (7, 7, 52))
        for i in range(4):
            ang = math.radians(i * 90 + 40)
            place(CUBE, "VEIL_DEEP_bridge_%02d" % i, "VEIL_DEEP",
                  (math.cos(ang) * 1300, -5000 + math.sin(ang) * 1300, 2400),
                  (0, i * 90 + 40, 0), (26, 0.5, 0.25))
    out["deep"] = count("VEIL_DEEP")

    # CAST at the law's addresses
    seats = [
        ("Keeper", 900, 0, 0, CYL, (2.2, 2.2, 12)),
        ("Seer", -450, 780, 0, CYL, (1.8, 1.8, 10)),
        ("Cartographer", -4150, -780, 0, CYL, (1.8, 1.8, 10)),
        ("Countess", 809, -309, 64.7, CYL, (2.0, 2.0, 11)),
        ("Vault", 0, 0, -80.9, CUBE, (10, 10, 8)),
        ("Cache", 0, -6200, 0, CUBE, (8, 8, 10)),
        ("Beacon1", 2800, 500, 0, CYL, (1.2, 1.2, 30)),
        ("Beacon2", -2000, -900, 0, CYL, (1.2, 1.2, 30)),
        ("Beacon3", 600, -2100, 0, CYL, (1.2, 1.2, 30)),
    ]
    have = set()
    for a in find("VEIL_CAST"):
        try:
            have.add(str(rv(call(GET, {"actor": a}))))
        except Exception:
            pass
    for name, x, y, z, mesh, scale in seats:
        label = "VEIL_CAST_" + name
        if label in have:
            continue
        place(mesh, label, "VEIL_CAST", (x, y, z), (0, 0, 0), scale)
        out["placed"] += 1
    out["cast"] = count("VEIL_CAST")
    return out
'''

CAMS = {
    "hub": ((0.0, -2400.0, 800.0), (-4.0, 90.0, 0.0)),
    "nest": ((-450.0, -1500.0, 900.0), (-5.0, 90.0, 0.0)),
    "spire": ((0.0, 900.0, 1400.0), (6.0, 90.0, 0.0)),
    "omni": ((2800.0, -1800.0, 900.0), (-4.0, 90.0, 0.0)),
    "deep": ((0.0, -8000.0, 1400.0), (-2.0, 90.0, 0.0)),
}


def xform(pos, rot):
    return {"location": {"x": pos[0], "y": pos[1], "z": pos[2]},
            "rotation": {"pitch": rot[0], "yaw": rot[1], "roll": rot[2]},
            "scale": {"x": 1.0, "y": 1.0, "z": 1.0}}


def main():
    call("EditorToolset.EditorAppToolset", "StopPIE", {})
    print("BUILD:", call("editor_toolset.toolsets.programmatic.ProgrammaticToolset",
                         "execute_tool_script", {"script": BUILD})[:400])

    save = call("editor_toolset.toolsets.asset.AssetTools", "save_assets",
                {"asset_paths": ["/Game/Maps/VEILDistrict.VEILDistrict"]})
    print("SAVE:", save[:120])

    os.makedirs("Docs/shots", exist_ok=True)
    for zone, (pos, rot) in CAMS.items():
        call("EditorToolset.EditorAppToolset", "SetCameraTransform", {"transform": xform(pos, rot)})
        txt = call("EditorToolset.EditorAppToolset", "CaptureViewport",
                   {"captureTransform": xform(pos, rot),
                    "annotations": {"gridSpacing": 0.0}, "bShowUI": False})
        img = None
        for m in re.finditer(r"[A-Za-z0-9+/=]{20000,}", txt):
            try:
                img = base64.b64decode(m.group(0))
                break
            except Exception:
                continue
        if img:
            path = "Docs/shots/%s.png" % zone
            with open(path, "wb") as f:
                f.write(img)
            print("SHOT", path)
        else:
            print("SHOT FAIL", zone, txt[:120].replace("\n", " "))


if __name__ == "__main__":
    main()
