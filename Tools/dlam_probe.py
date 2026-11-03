"""One script through the editor's own MCP hand: read the level, place a probe."""
import json
import urllib.request

URL = "http://127.0.0.1:8000/mcp"


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


def call_tool(toolset, tool, arguments):
    _, sid = rpc({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                  "params": {"protocolVersion": "2025-06-18", "capabilities": {},
                             "clientInfo": {"name": "buffy-dlam", "version": "1"}}})
    rpc({"jsonrpc": "2.0", "method": "notifications/initialized"}, sid)
    out, _ = rpc({"jsonrpc": "2.0", "id": 2, "method": "tools/call",
                  "params": {"name": "call_tool",
                             "arguments": {"toolset_name": toolset, "tool_name": tool,
                                           "arguments": arguments}}}, sid)
    content = out.get("result", {}).get("content", [])
    return content[0].get("text") if content else json.dumps(out)


SCRIPT = r'''
import json

def call(name, args):
    r = execute_tool(name, json.dumps(args))
    if isinstance(r, str):
        try:
            r = json.loads(r)
        except Exception:
            pass
    return r

def run():
    lvl = call("editor_toolset.toolsets.scene.SceneTools.get_current_level", {})
    made = call("editor_toolset.toolsets.scene.SceneTools.add_to_scene_from_asset",
        {"asset_path": "/Engine/BasicShapes/Cube.Cube",
         "name": "VEIL_PY_PROBE",
         "xform": {"location": {"x": 0, "y": 0, "z": 300},
                   "rotation": {"pitch": 0, "yaw": 0, "roll": 0},
                   "scale": {"x": 2, "y": 2, "z": 2}}})
    return {"level": lvl, "made": made}
'''

print(call_tool("editor_toolset.toolsets.programmatic.ProgrammaticToolset",
                "execute_tool_script", {"script": SCRIPT}))
