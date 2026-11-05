"""Import the generated cues into /Game/VEIL/Audio as real SoundWave assets."""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "ed_audio_log.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

DEST = "/Game/VEIL/Audio"
tasks = []
for name in ("veil_strike.wav", "veil_beacon.wav", "veil_step.wav"):
    src = os.path.join(HERE, "audio", name)
    if not os.path.exists(src):
        w("MISSING SOURCE %s" % src)
        continue
    task = unreal.AssetImportTask()
    task.filename = src
    task.destination_path = DEST
    task.destination_name = os.path.splitext(name)[0]
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.factory = unreal.SoundFactory()
    tasks.append(task)

if tasks:
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    w("imported %d tasks" % len(tasks))

for name in ("veil_strike", "veil_beacon", "veil_step"):
    path = "%s/%s" % (DEST, name)
    asset = unreal.load_asset(path)
    w("%s -> %s" % (path, "OK" if asset else "MISSING"))

w("AUDIO DONE")
