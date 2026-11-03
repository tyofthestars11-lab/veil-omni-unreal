"""Find the accepted execute_tool convention inside the editor's sandbox."""
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

def attempt(fn):
    try:
        return {"ok": fn()}
    except Exception as e:
        return {"err": type(e).__name__ + ": " + str(e)[:600]}

def run():
    name = "editor_toolset.toolsets.scene.SceneTools.get_current_level"
    return {
        "json_str": attempt(lambda: execute_tool(name, "{}")),
        "empty_dict": attempt(lambda: execute_tool(name, {})),
        "short_name": attempt(lambda: execute_tool("SceneTools.get_current_level", {})),
        "no_args": attempt(lambda: execute_tool(name)),
    }
'''

print(call_tool("editor_toolset.toolsets.programmatic.ProgrammaticToolset",
                "execute_tool_script", {"script": SCRIPT}))
