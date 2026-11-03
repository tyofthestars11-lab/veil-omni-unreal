"""Re-probe the editor hand: current CaptureViewport schema and capture attempts."""
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


def decode(text):
    for m in re.finditer(r"[A-Za-z0-9+/=]{20000,}", text):
        try:
            return base64.b64decode(m.group(0))
        except Exception:
            continue
    return None


spec_text = call("describe_toolset", "", {}) if False else None
raw = call("describe_toolset", "", {}) if False else None

# describe through the meta tool name directly
out = call("", "describe_toolset", {"toolset_name": "EditorToolset.EditorAppToolset"})
try:
    spec = json.loads(out)
except Exception:
    spec = None
print("DESCRIBE OK:", bool(spec))
if spec:
    names = [t["name"] for t in spec.get("tools", [])]
    print("TOOLS:", ", ".join(n.split(".")[-1] for n in names))
    for t in spec.get("tools", []):
        if t["name"].endswith("CaptureViewport"):
            print("SCHEMA:", json.dumps(t.get("inputSchema", {}))[:1600])
