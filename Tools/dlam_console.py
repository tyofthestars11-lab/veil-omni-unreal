"""Type into the Unreal editor's own console through the Slate hand."""
import base64
import json
import os
import re
import time
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


snap = call("SlateInspectorToolset.SlateInspectorToolset", "Snapshot",
            {"maxDepth": 26, "bIncludeSourceLocations": False})
print("SNAPSHOT CHARS:", len(snap))
lines = snap.replace("\\n", "\n").splitlines()
hits = [l for l in lines if "console" in l.lower() or "cmd" in l.lower()]
print("CONSOLE-LINE HITS:", len(hits))
for l in hits[:8]:
    print("  ", l[:220])

refs = []
for l in hits:
    for pat in (r'"ref"\s*:\s*"([^"]+)"', r'ref["\s:=]+([A-Za-z0-9_.:\-]+)', r'\bid["\s:=]+([A-Za-z0-9_.:\-]+)'):
        refs.extend(re.findall(pat, l))
print("REFS:", refs[:8])

CMD = 'py exec(open(r"D:/VEIL-OMNI/Docs/VEIL_FIX.py").read())'
typed = None
for ref in refs:
    out = call("SlateInspectorToolset.SlateInspectorToolset", "Type",
               {"ref": ref, "text": CMD, "submit": True})
    print("TYPE", ref, "->", out[:200].replace("\n", " "))
    if "error" not in out.lower():
        typed = ref
        break
print("TYPED INTO:", typed)

time.sleep(8)

# what did the editor log?
logs = sorted(
    [p for p in os.listdir("Saved/Logs") if p.lower().endswith(".log")],
    key=lambda p: os.path.getmtime(os.path.join("Saved/Logs", p)))
if logs:
    latest = os.path.join("Saved/Logs", logs[-1])
    print("LATEST LOG:", latest)
    with open(latest, "r", errors="ignore") as f:
        tail = f.readlines()[-400:]
    fix = [l.strip() for l in tail if "VEILFIX" in l]
    print("VEILFIX LINES:", len(fix))
    for l in fix[-6:]:
        print("  ", l[:200])

TRANS = {"location": {"x": 0.0, "y": -2400.0, "z": 800.0},
         "rotation": {"pitch": -4.0, "yaw": 90.0, "roll": 0.0},
         "scale": {"x": 1.0, "y": 1.0, "z": 1.0}}
cap = call("EditorToolset.EditorAppToolset", "CaptureViewport",
           {"captureTransform": TRANS, "annotations": {}, "bShowUI": False})
img = None
for m in re.finditer(r"[A-Za-z0-9+/=]{20000,}", cap):
    try:
        img = base64.b64decode(m.group(0))
        break
    except Exception:
        continue
if img:
    os.makedirs("Docs/shots", exist_ok=True)
    with open("Docs/shots/console_fix.png", "wb") as f:
        f.write(img)
    print("WROTE Docs/shots/console_fix.png")
else:
    print("CAPTURE FAIL:", cap[:200].replace("\n", " "))
