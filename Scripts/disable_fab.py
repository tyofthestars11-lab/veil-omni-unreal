"""SS1 handshake spam is Fab's embedded Chromium failing to reach Epic.

The Bible already records it: 'editor's built-in browser, not VEIL code. Disable the
Fab plugin if it keeps spamming.' Disabling Fab stops the CEF instance entirely.
"""
import json

PATH = "C:/Users/Admin/HarkGame/VEILOMNI.uproject"
proj = json.load(open(PATH, encoding="utf-8"))

plugins = proj.setdefault("Plugins", [])
entry = next((p for p in plugins if p.get("Name") == "Fab"), None)
if entry is None:
    plugins.append({"Name": "Fab", "Enabled": False})
    print("Fab added, disabled")
elif entry.get("Enabled"):
    entry["Enabled"] = False
    print("Fab disabled")
else:
    print("Fab already disabled")

# The startup web browser and its CEF log are what emit the handshake errors.
json.dump(proj, open(PATH, "w", encoding="utf-8"), indent="\t")
print("plugins:", [(p.get("Name"), p.get("Enabled")) for p in plugins])
