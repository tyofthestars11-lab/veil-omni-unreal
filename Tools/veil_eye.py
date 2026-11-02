#!/usr/bin/env python3
"""Read the live VEILDistrict through the editor's own MCP server.

READ is the whole point: these numbers come from the running editor, not from
source files or memory. Run:  python3 Tools/veil_eye.py
"""
import json
import subprocess
import sys

MCP = "Tools/mcp_call.py"
SCENE = "editor_toolset.toolsets.scene.SceneTools"
ACTOR = "editor_toolset.toolsets.actor.ActorTools"
ASSET = "editor_toolset.toolsets.asset.AssetTools"
APP = "EditorToolset.EditorAppToolset"


def tool(toolset, name, args=None):
    payload = json.dumps({
        "name": "call_tool",
        "arguments": {"toolset_name": toolset, "tool_name": name, "arguments": args or {}},
    })
    out = subprocess.run([sys.executable, MCP, "tools/call", payload],
                         capture_output=True, text=True, timeout=300).stdout
    d = json.loads(out)
    txt = "".join(c.get("text", "") for c in d.get("result", {}).get("content", []))
    try:
        return json.loads(txt)
    except Exception:
        return txt


def find(cls):
    r = tool(SCENE, "find_actors", {"name": "", "tag": "", "collision_channels": [],
                                    "actor_type": {"refPath": cls}})
    v = r.get("returnValue") if isinstance(r, dict) else None
    return v if isinstance(v, list) else []


def xform(ref):
    r = tool(ACTOR, "get_actor_transform", {"actor": {"refPath": ref}})
    return r.get("returnValue") if isinstance(r, dict) else None


def main():
    level = tool(SCENE, "get_current_level").get("returnValue")
    print("LEVEL          :", level)
    print("PIE running    :", tool(APP, "IsPIERunning").get("returnValue"))

    vaults = find("/Script/VEIL.VEILVault")
    runners = find("/Script/VEIL.VEILCharacter")
    machines = find("/Script/VEIL.VEILFactionAgent")
    print("VAULT actors   :", len(vaults))
    print("RUNNER actors  :", len(runners))
    print("MACHINE actors :", len(machines))

    for v in vaults:
        print("  vault at     :", json.dumps(xform(v["refPath"])))

    dirty = tool(ASSET, "is_dirty", {"asset_path": level})
    print("level dirty    :", json.dumps(dirty))


if __name__ == "__main__":
    main()
