import base64, json, os, re, urllib.request
URL = "http://127.0.0.1:8000/mcp"; SID = None
def rpc(p, sid=None):
    h = {"Content-Type": "application/json", "Accept": "application/json, text/event-stream"}
    if sid: h["Mcp-Session-Id"] = sid
    r = urllib.request.Request(URL, data=json.dumps(p).encode(), headers=h, method="POST")
    with urllib.request.urlopen(r, timeout=120) as resp:
        sid = resp.headers.get("Mcp-Session-Id") or sid; body = resp.read().decode()
    t = body.strip()
    if "data:" in t:
        c = [l[5:].strip() for l in t.splitlines() if l.startswith("data:")]
        if c: t = c[-1]
    try: return json.loads(t), sid
    except Exception: return {"raw": body}, sid
def call(ts, tool, args):
    global SID
    if SID is None:
        _, SID = rpc({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                      "params": {"protocolVersion": "2025-06-18", "capabilities": {},
                                 "clientInfo": {"name": "buffy", "version": "1"}}})
        rpc({"jsonrpc": "2.0", "method": "notifications/initialized"}, SID)
    o, SID = rpc({"jsonrpc": "2.0", "id": 2, "method": "tools/call",
                  "params": {"name": "call_tool", "arguments": {"toolset_name": ts,
                             "tool_name": tool, "arguments": args}}}, SID)
    c = o.get("result", {}).get("content", [])
    return c[0].get("text") if c else json.dumps(o)
TRANS = {"location": {"x": 0.0, "y": -2400.0, "z": 800.0},
         "rotation": {"pitch": -4.0, "yaw": 90.0, "roll": 0.0},
         "scale": {"x": 1.0, "y": 1.0, "z": 1.0}}
print("SETCAM:", call("EditorToolset.EditorAppToolset", "SetCameraTransform", {"transform": TRANS})[:120])
print("GETCAM:", call("EditorToolset.EditorAppToolset", "GetCameraTransform", {})[:300])
t = call("EditorToolset.EditorAppToolset", "CaptureViewport",
         {"captureTransform": TRANS, "annotations": {"gridSpacing": 0.0}, "bShowUI": False})
img = None
for m in re.finditer(r"[A-Za-z0-9+/=]{20000,}", t):
    try: img = base64.b64decode(m.group(0)); break
    except Exception: pass
if img:
    os.makedirs("Docs/shots", exist_ok=True)
    open("Docs/shots/hub_cam.png", "wb").write(img)
    print("WROTE Docs/shots/hub_cam.png")
else:
    print("CAPTURE ERROR:", t[:160].replace("\n", " "))
