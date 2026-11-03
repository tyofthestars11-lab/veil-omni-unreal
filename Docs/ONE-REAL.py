#!/usr/bin/env python3
"""ONE, REAL - a whole-check that can only speak from a real tool read.

Tyree's rule, phase lock: only answer from a real tool output that actually ran.
Never invent actors, never invent a count, never assert a number this file wrote
itself. The DLAM ONE script pasted on 2026-11-03 did exactly that: it built its own
Python lists, asserted hub == 25 and printed "WHOLE: PASS" without touching Unreal.

This asks the running editor's own MCP server and prints what came back. If the
server does not answer, the answer is TOOL NOT CONNECTED, NO REAL READ, exit 2.
There is no path in this file that prints PASS without a parsed tool response.
"""
import json
import sys
import urllib.request

URL = "http://127.0.0.1:8000/mcp"


def rpc(payload, session=None, timeout=30):
    headers = {"Content-Type": "application/json",
               "Accept": "application/json, text/event-stream"}
    if session:
        headers["Mcp-Session-Id"] = session
    req = urllib.request.Request(URL, data=json.dumps(payload).encode(),
                                 headers=headers, method="POST")
    with urllib.request.urlopen(req, timeout=timeout) as r:
        sid = r.headers.get("Mcp-Session-Id") or session
        body = r.read().decode("utf-8", "replace")
    text = body.strip()
    if text.startswith("data:") or "\ndata:" in text:
        chunks = [l[5:].strip() for l in text.splitlines() if l.startswith("data:")]
        if chunks:
            text = chunks[-1]
    try:
        return json.loads(text), sid
    except json.JSONDecodeError:
        return {"raw": body}, sid


def main():
    try:
        init, sid = rpc({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                         "params": {"protocolVersion": "2025-06-18", "capabilities": {},
                                    "clientInfo": {"name": "one-real", "version": "1"}}},
                        timeout=15)
    except Exception as e:
        print("TOOL NOT CONNECTED, NO REAL READ")
        print("  tried: %s" % URL)
        print("  error: %s: %s" % (type(e).__name__, getattr(e, "reason", e)))
        print("  open Unreal with VEIL.uproject and VEILDistrict, MCP server running, then run this again.")
        return 2

    server = ((init.get("result") or {}).get("serverInfo") or {})
    print("REAL READ - connected to %s" % (server.get("name") or "unreal-mcp"))

    try:
        rpc({"jsonrpc": "2.0", "method": "notifications/initialized"}, sid)
    except Exception:
        pass

    listed, _ = rpc({"jsonrpc": "2.0", "id": 2, "method": "tools/list", "params": {}}, sid)
    tools = ((listed.get("result") or {}).get("tools") or [])
    names = [t.get("name", "") for t in tools]
    print("  tools advertised: %d" % len(names))

    target = next((n for n in names if n.endswith("find_actors")), None)
    if not target:
        print("  find_actors not advertised - the level cannot be read")
        print("TOOL NOT CONNECTED, NO REAL READ")
        return 2

    schema = next((t for t in tools if t.get("name") == target), {})
    args = {}
    for k in ((schema.get("inputSchema") or {}).get("required") or []):
        args[k] = [] if k == "collision_channels" else ""
    print("  reading the level through: %s" % target)

    try:
        out, _ = rpc({"jsonrpc": "2.0", "id": 3, "method": "tools/call",
                      "params": {"name": target, "arguments": args}}, sid)
    except Exception as e:
        print("  tool call failed: %s: %s" % (type(e).__name__, e))
        print("TOOL NOT CONNECTED, NO REAL READ")
        return 2

    if out.get("error"):
        print("  server error, verbatim: %s" % json.dumps(out["error"])[:300])
        print("TOOL NOT CONNECTED, NO REAL READ")
        return 2

    text = ""
    for c in ((out.get("result") or {}).get("content") or []):
        if isinstance(c, dict) and c.get("type") == "text":
            text += c.get("text", "")
    print("  real response: %d bytes" % len(text))

    n = None
    try:
        data = json.loads(text)
        if isinstance(data, dict):
            for key in ("count", "total", "num"):
                if isinstance(data.get(key), int):
                    n = data[key]
                    break
            if n is None:
                for key in ("actors", "results", "items"):
                    if isinstance(data.get(key), list):
                        n = len(data[key])
                        break
        elif isinstance(data, list):
            n = len(data)
    except json.JSONDecodeError:
        pass

    if n is None:
        print(text[:800])
        print("  no count could be read from the real payload - shown above, not guessed")
        return 3

    print("  ACTORS IN LEVEL: %d" % n)
    print("WHOLE: PASS (real read: %d actors via %s)" % (n, target))
    return 0


if __name__ == "__main__":
    sys.exit(main())
