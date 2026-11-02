import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\proof\loop_probe.txt"

KEYS = ["KEEPER", "SEER", "CARTO", "FRAGMENT", "CACHE", "WARDEN", "ANCHOR", "VAULT", "SIGNAL", "PLAYERSTART"]

lines = []


def log(m):
    lines.append(m)
    unreal.log("LOOP " + m)


try:
    try:
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP)
    except Exception as e:
        log("LOAD failed %r" % (e,))

    sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = sub.get_all_level_actors()
    log("TOTAL_ACTORS %d" % len(actors))

    buckets = {}
    for a in actors:
        try:
            label = str(a.get_actor_label())
            cls = a.get_class().get_name()
        except Exception:
            continue
        up = label.upper()
        for k in KEYS:
            if k in up:
                buckets.setdefault(k, []).append("%s [%s]" % (label, cls))

    for k in KEYS:
        found = buckets.get(k, [])
        log("%s count=%d %s" % (k, len(found), found[:8]))

    # Vault anchor wiring: does the Vault have its 4 anchors in the level?
    vaults = [a for a in actors if isinstance(a, unreal.VEILVault)] if hasattr(unreal, "VEILVault") else []
    log("VEILVAULT_CLASS_FOUND %d" % len(vaults))
    anchors = [a for a in actors if a.get_class().get_name() in ("VEILVaultAnchor",)]
    log("VAULT_ANCHORS_IN_LEVEL %d" % len(anchors))
except Exception as e:
    log("FATAL %r" % (e,))
finally:
    with open(OUT, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    unreal.log("LOOP probe written")
