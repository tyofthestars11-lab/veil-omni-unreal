#!/usr/bin/env python3
"""Inventory of the live VEILDistrict, read through the editor's MCP hand.

Every line here is the editor's answer, not a guess from source.
"""
import json
import subprocess
import sys

MCP = "Tools/mcp_call.py"
SCENE = "editor_toolset.toolsets.scene.SceneTools"

CLASSES = [
    ("Vault", "/Script/VEIL.VEILVault"),
    ("Anchor", "/Script/VEIL.VEILVaultAnchor"),
    ("Runner", "/Script/VEIL.VEILCharacter"),
    ("Countess", "/Script/VEIL.VEILCountessCharacter"),
    ("Machine", "/Script/VEIL.VEILFactionAgent"),
    ("Enemy", "/Script/VEIL.VEILEnemy"),
    ("Beacon", "/Script/VEIL.VEILBeacon"),
    ("Fragment", "/Script/VEIL.VEILFragment"),
    ("OMNIFragment", "/Script/VEIL.OMNIFragment"),
    ("Cache", "/Script/VEIL.OMNICache"),
    ("QuestGiver", "/Script/VEIL.OMNIQuestGiver"),
    ("WardenPrime", "/Script/VEIL.OMNIWardenPrime"),
    ("Forge", "/Script/VEIL.VEILForge"),
    ("PhiForge", "/Script/VEIL.PhiForge"),
    ("Portal", "/Script/VEIL.PhiPortal"),
    ("Clue", "/Script/VEIL.EnvironmentalClue"),
    ("PlayerStart", "/Script/Engine.PlayerStart"),
    ("StaticMesh", "/Script/Engine.StaticMeshActor"),
    ("Light", "/Script/Engine.PointLight"),
]


def rpc(method, args=None):
    payload = json.dumps({"name": "call_tool",
                          "arguments": {"toolset_name": SCENE, "tool_name": method,
                                        "arguments": args or {}}})
    out = subprocess.run([sys.executable, MCP, "tools/call", payload],
                         capture_output=True, text=True, timeout=300).stdout
    d = json.loads(out)
    txt = "".join(c.get("text", "") for c in d.get("result", {}).get("content", []))
    try:
        return json.loads(txt)
    except Exception:
        return txt


def find(cls):
    r = rpc("find_actors", {"name": "", "tag": "", "collision_channels": [],
                            "actor_type": {"refPath": cls}})
    v = r.get("returnValue") if isinstance(r, dict) else None
    return v if isinstance(v, list) else []


def main():
    print("LEVEL:", rpc("get_current_level").get("returnValue"))
    total = 0
    for label, cls in CLASSES:
        actors = find(cls)
        total += len(actors)
        if not actors:
            print(f"  {label:<13} 0")
            continue
        print(f"  {label:<13} {len(actors)}")
        for a in actors[:10]:
            name = a["refPath"].split(".")[-1]
            print(f"      {name}")
    print("total named-class actors:", total)


if __name__ == "__main__":
    main()
