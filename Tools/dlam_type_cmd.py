import base64, json, os, re, sys, time, urllib.request
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
                                 "clientInfo": {"name": "b", "version": "1"}}})
        rpc({"jsonrpc": "2.0", "method": "notifications/initialized"}, SID)
    o, SID = rpc({"jsonrpc": "2.0", "id": 2, "method": "tools/call",
                  "params": {"name": "call_tool", "arguments": {"toolset_name": ts,
                             "tool_name": tool, "arguments": args}}}, SID)
    c = o.get("result", {}).get("content", [])
    return c[0].get("text") if c else json.dumps(o)
S = "SlateInspectorToolset.SlateInspectorToolset"
cmd = sys.argv[1]
print("CLICK:", call(S, "Click", {"ref": "tb4"})[:80])
print("TYPE:", call(S, "Type", {"ref": "tb4", "text": cmd, "submit": True})[:80])
time.sleep(10)
logs = sorted([p for p in os.listdir("Saved/Logs") if p.endswith(".log")],
              key=lambda p: os.path.getmtime("Saved/Logs/" + p))
fix = []
if logs:
    tail = open("Saved/Logs/" + logs[-1], errors="ignore").readlines()[-800:]
    fix = [l.strip() for l in tail if "VEILFIX" in l or "LogPython: Error" in l]
print("LOG", logs[-1] if logs else None, "HITS", len(fix))
for l in fix[-10:]: print("  ", l[:190])
TRANS = {"location": {"x": 0.0, "y": -2400.0, "z": 800.0},
         "rotation": {"pitch": -4.0, "yaw": 90.0, "roll": 0.0},
         "scale": {"x": 1.0, "y": 1.0, "z": 1.0}}
cap = call("EditorToolset.EditorAppToolset", "CaptureViewport",
           {"captureTransform": TRANS, "annotations": {}, "bShowUI": False})
img = None
for m in re.finditer(r"[A-Za-z0-9+/=]{20000,}", cap):
    try: img = base64.b64decode(m.group(0)); break
    except Exception: pass
if img:
    open("Docs/shots/console_fix.png", "wb").write(img); print("WROTE console_fix.png")
else: print("CAP FAIL", cap[:120].replace("\n", " "))
