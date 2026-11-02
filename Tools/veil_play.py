#!/usr/bin/env python3
"""Enter Play through the editor's MCP hand and take the photo from the game.

Proof rule: Play counts only when the picture comes from the running game.
Usage: python3 Tools/veil_play.py [prefix]
"""
import base64
import json
import math
import subprocess
import sys
import time

MCP = "Tools/mcp_call.py"
APP = "EditorToolset.EditorAppToolset"


def t(tool, args=None):
    payload = json.dumps({"name": "call_tool",
                          "arguments": {"toolset_name": APP, "tool_name": tool,
                                        "arguments": args or {}}})
    out = subprocess.run([sys.executable, MCP, "tools/call", payload],
                         capture_output=True, text=True, timeout=300).stdout
    d = json.loads(out)
    txt = "".join(c.get("text", "") for c in d.get("result", {}).get("content", []))
    try:
        return json.loads(txt)
    except Exception:
        return txt


def pose(loc, target):
    d = (target[0] - loc[0], target[1] - loc[1], target[2] - loc[2])
    yaw = math.degrees(math.atan2(d[1], d[0]))
    pitch = math.degrees(math.atan2(d[2], math.hypot(d[0], d[1])))
    return {"location": {"x": loc[0], "y": loc[1], "z": loc[2]},
            "rotation": {"pitch": pitch, "yaw": yaw, "roll": 0},
            "scale": {"x": 1, "y": 1, "z": 1}}


def capture(p, path):
    # both params must be sent: the binding treats "optional" ones as required
    r = t("CaptureViewport", {
        "captureTransform": p,
        "annotations": {"gridSpacing": 0, "maxLabelDistance": 0},
    })
    rv = r.get("returnValue") if isinstance(r, dict) else None
    if not rv:
        print("  capture failed:", str(r)[:200])
        return False
    data = rv["image"]["data"]
    with open(path, "wb") as f:
        f.write(base64.b64decode(data))
    print("  wrote", path, "from", json.dumps(rv["cameraLocation"]))
    return True


def main():
    prefix = sys.argv[1] if len(sys.argv) > 1 else "proof/play"
    print("starting PIE...")
    t("StartPIE", {"options": {}})
    for _ in range(30):
        time.sleep(2)
        if t("IsPIERunning").get("returnValue"):
            break
    print("PIE running:", t("IsPIERunning").get("returnValue"))
    time.sleep(9)  # world boot: camp wakes, runner lands, HUD comes up

    shots = [
        ("_high.png", pose((0, -40, 2200), (0, 0, 120))),      # over the plaza
        ("_runner.png", pose((900, -900, 320), (0, 0, 150))),  # behind the runner
        ("_close.png", pose((260, -260, 210), (0, 0, 140))),   # the runner's body
        ("_vault.png", pose((520, 520, 260), (0, 0, 60))),     # down at the Vault
    ]
    for suffix, p in shots:
        capture(p, prefix + suffix)

    print("editor image...")
    r = t("CaptureEditorImage")
    rv = r.get("returnValue") if isinstance(r, dict) else None
    if isinstance(rv, dict) and rv.get("data"):
        with open(prefix + "_editor.png", "wb") as f:
            f.write(base64.b64decode(rv["data"]))
        print("  wrote", prefix + "_editor.png")
    else:
        print("  editor image unavailable:", str(r)[:200])

    print("stopping PIE...")
    t("StopPIE")
    time.sleep(2)
    print("PIE running:", t("IsPIERunning").get("returnValue"))


if __name__ == "__main__":
    main()
