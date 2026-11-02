#!/usr/bin/env python3
"""Capture the active Unreal editor window to a PNG.

The Slate inspector hands back a base64 PNG inside the MCP text payload; this
writes it to disk so the picture can be looked at instead of described.
"""
import base64
import json
import sys

import mcp_call


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else "proof/editor_now.png"
    _, sid = mcp_call.rpc({
        "jsonrpc": "2.0", "id": 1, "method": "initialize",
        "params": {
            "protocolVersion": "2025-06-18", "capabilities": {},
            "clientInfo": {"name": "buffy", "version": "1"},
        },
    })
    mcp_call.rpc({"jsonrpc": "2.0", "method": "notifications/initialized"}, sid)
    out, _ = mcp_call.rpc({
        "jsonrpc": "2.0", "id": 2, "method": "tools/call",
        "params": {
            "name": "call_tool",
            "arguments": {
                "toolset_name": "SlateInspectorToolset.SlateInspectorToolset",
                "tool_name": "Screenshot",
                "arguments": {"ref": ""},
            },
        },
    }, sid)

    text = out["result"]["content"][0]["text"]
    payload = json.loads(text)["returnValue"]
    data = payload.get("data", "")
    with open(out_path, "wb") as handle:
        handle.write(base64.b64decode(data))
    print(f"wrote {out_path} ({len(data) * 3 // 4} bytes)")


if __name__ == "__main__":
    main()
