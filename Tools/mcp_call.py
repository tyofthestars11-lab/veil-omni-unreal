#!/usr/bin/env python3
"""Talk to the Unreal editor's own MCP server (loopback only).

The ModelContextProtocol plugin serves the agent-facing MCP spec on
http://127.0.0.1:8000/mcp. This is the hand that lets an outside agent read
and act inside the running editor instead of guessing what stands in a level.

Usage:
  mcp_call.py tools/list
  mcp_call.py tools/call '{"name":"list_toolsets","arguments":{}}'
  mcp_call.py resources/list
"""
import json
import sys
import urllib.request

URL = "http://127.0.0.1:8000/mcp"


def rpc(payload, session=None):
    headers = {
        "Content-Type": "application/json",
        "Accept": "application/json, text/event-stream",
    }
    if session:
        headers["Mcp-Session-Id"] = session
    req = urllib.request.Request(URL, data=json.dumps(payload).encode(), headers=headers, method="POST")
    with urllib.request.urlopen(req, timeout=120) as r:
        sid = r.headers.get("Mcp-Session-Id") or session
        body = r.read().decode()
    # server answers either bare JSON or an SSE stream; take the last data line
    text = body.strip()
    if text.startswith("event:") or "\ndata:" in text or text.startswith("data:"):
        chunks = [l[5:].strip() for l in text.splitlines() if l.startswith("data:")]
        if chunks:
            text = chunks[-1]
    try:
        return json.loads(text), sid
    except json.JSONDecodeError:
        return {"raw": body}, sid


def main():
    method = sys.argv[1] if len(sys.argv) > 1 else "tools/list"
    params = json.loads(sys.argv[2]) if len(sys.argv) > 2 else {}

    _, sid = rpc({
        "jsonrpc": "2.0", "id": 1, "method": "initialize",
        "params": {
            "protocolVersion": "2025-06-18", "capabilities": {},
            "clientInfo": {"name": "buffy", "version": "1"},
        },
    })
    # required by the spec before any other request
    rpc({"jsonrpc": "2.0", "method": "notifications/initialized"}, sid)

    out, _ = rpc({"jsonrpc": "2.0", "id": 2, "method": method, "params": params}, sid)
    # full output, never truncated: captures carry a base64 image and a cut
    # payload is unparseable JSON
    print(json.dumps(out, indent=1))


if __name__ == "__main__":
    main()
