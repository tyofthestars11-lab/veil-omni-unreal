import base64, json, re, urllib.request
URL = "http://127.0.0.1:8000/mcp"
def rpc(p, sid=None):
    h = {"Content-Type": "application/json", "Accept": "application/json, text/event-stream"}
    if sid: h["Mcp-Session-Id"] = sid
    req = urllib.request.Request(URL, data=json.dumps(p).encode(), headers=h, method="POST")
    with urllib.request.urlopen(req, timeout=180) as r:
        sid = r.headers.get("Mcp-Session-Id") or sid; body = r.read().decode()
    t = body.strip()
    if "data:" in t:
        c = [l[5:].strip() for l in t.splitlines() if l.startswith("data:")]
        if c: t = c[-1]
    try: return json.loads(t), sid
    except Exception: return {"raw": body}, sid
def call(ts, tool, args, sid=None):
    if sid is None:
        _, sid = rpc({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                      "params": {"protocolVersion": "2025-06-18", "capabilities": {},
                                 "clientInfo": {"name": "buffy", "version": "1"}}})
        rpc({"jsonrpc": "2.0", "method": "notifications/initialized"}, sid)
    out, sid = rpc({"jsonrpc": "2.0", "id": 2, "method": "tools/call",
                    "params": {"name": "call_tool",
                               "arguments": {"toolset_name": ts, "tool_name": tool, "arguments": args}}}, sid)
    c = out.get("result", {}).get("content", [])
    return (c[0].get("text") if c else json.dumps(out)), sid
save, sid = call("editor_toolset.toolsets.asset.AssetTools", "save_assets",
                 {"asset_paths": ["/Game/Maps/VEILDistrict"]})
print("SAVE:", save[:300])
cap, sid = call("EditorToolset.EditorAppToolset", "CaptureViewport",
                {"captureTransform": {"location": {"x": 2400.0, "y": -2600.0, "z": 1700.0},
                                      "rotation": {"pitch": -12.0, "yaw": 42.0, "roll": 0.0},
                                      "scale": {"x": 1.0, "y": 1.0, "z": 1.0}},
                 "annotations": False, "bShowUI": False}, sid)
blob = None
for m in re.finditer(r"[A-Za-z0-9+/=]{20000,}", cap):
    blob = m.group(0); break
if blob:
    open("Saved/Screenshots/dlam_hand2.png", "wb").write(base64.b64decode(blob))
    print("CAPTURED dlam_hand2.png")
else:
    print("CAP FAIL:", cap[:300])
