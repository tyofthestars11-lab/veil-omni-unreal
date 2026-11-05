"""Pull real CC0 meshes from Poly Haven (no account, no key) into the project.

Poly Haven models are photoscanned props: rocks, cliffs, plants, wood, rope-like
gear. Their files API exposes a glTF package per resolution; every url in that
package is downloaded so the mesh arrives complete with its textures.
"""
import json
import os
import urllib.request

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
OUT = os.path.join(ROOT, "Assets", "CC0", "Models")
os.makedirs(OUT, exist_ok=True)
UA = {"User-Agent": "VEIL-OMNI/1.0 (project asset fetch)"}


def get_json(url, timeout=60):
    req = urllib.request.Request(url, headers=UA)
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return json.loads(r.read().decode("utf-8"))


def get_file(url, path, timeout=180):
    req = urllib.request.Request(url, headers=UA)
    with urllib.request.urlopen(req, timeout=timeout) as r:
        data = r.read()
    with open(path, "wb") as f:
        f.write(data)
    return len(data)


def walk_urls(node, acc):
    if isinstance(node, dict):
        if isinstance(node.get("url"), str):
            acc.append(node["url"])
        for v in node.values():
            walk_urls(v, acc)
    elif isinstance(node, list):
        for v in node:
            walk_urls(v, acc)


print("indexing polyhaven models...")
models = get_json("https://api.polyhaven.com/assets?type=models")
print("models available:", len(models))

WANT = {
    "cliff": ["cliff", "rock", "boulder", "stone"],
    "plant": ["plant", "tree", "bush", "grass", "fern"],
    "wood": ["wood", "plank", "log", "beam", "barrel"],
    "rope": ["rope", "chain", "net"],
    "bridge": ["bridge", "arch", "walkway"],
}

chosen = {}
for label, words in WANT.items():
    hits = []
    for key, meta in models.items():
        tags = {t.lower() for t in (meta.get("tags") or [])}
        name = key.lower()
        if any(w in tags for w in words) or any(w in name for w in words):
            hits.append(key)
    chosen[label] = hits[:3]
    print(f"  {label:8s} -> {chosen[label]}")

manifest = {"source": "https://polyhaven.com (CC0)", "models": []}
total = 0
for label, ids in chosen.items():
    for asset in ids:
        try:
            files = get_json("https://api.polyhaven.com/files/" + asset)
        except Exception as exc:
            print("  files failed", asset, exc)
            continue
        gltf = files.get("gltf") or {}
        node = gltf.get("1k") or gltf.get("2k") or next(iter(gltf.values()), None)
        urls = []
        walk_urls(node, urls)
        if not urls:
            print("  no gltf package for", asset)
            continue
        folder = os.path.join(OUT, asset)
        os.makedirs(folder, exist_ok=True)
        got = 0
        for u in urls:
            name = u.split("?")[0].rstrip("/").split("/")[-1]
            if not name:
                continue
            try:
                total += get_file(u, os.path.join(folder, name))
                got += 1
            except Exception as exc:
                print("  file failed", name, exc)
        print(f"  saved {label:8s} {asset:30s} {got} files")
        manifest["models"].append({"category": label, "id": asset, "folder": os.path.relpath(folder, ROOT), "files": got})

with open(os.path.join(ROOT, "Assets", "CC0", "manifest_models.json"), "w", encoding="utf-8") as f:
    json.dump(manifest, f, indent=2)

print()
print(f"downloaded {len(manifest['models'])} real CC0 meshes, {total/1024/1024:.1f} MiB")
