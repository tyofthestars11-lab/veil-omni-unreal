"""Structural flaw scan of the live VEIL source. Evidence, not opinion."""

import glob
import os
import re

SRC = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
files = sorted(glob.glob(SRC + "*.cpp") + glob.glob(SRC + "*.h"))


def short(p):
    return os.path.basename(p)


def lines(p):
    return open(p, encoding="utf-8", errors="replace").read().replace("\r\n", "\n").split("\n")


dead, rawptr, casty, loado, divzero, passtrue, sweep, sound, todos = [], [], [], [], [], [], [], [], []
binds = []

for p in files:
    L = lines(p)
    for i, raw in enumerate(L):
        n = i + 1
        code = raw.split("//")[0]
        s = code.strip()
        if not s:
            continue
        if s in ("return;", "break;", "continue;", "return false;", "return true;", "return nullptr;"):
            for j in range(i + 1, min(i + 6, len(L))):
                nxt = L[j].strip()
                if not nxt:
                    continue
                if nxt == "}":
                    break
                indent = len(L[j]) - len(L[j].lstrip())
                if indent >= (len(raw) - len(raw.lstrip())) and not nxt.startswith("//"):
                    dead.append(f"{short(p)}:{n}  '{s}' then '{nxt[:70]}'")
                break
        if re.search(r"\b[A-Z]\w*\s*\*+\s*\w+\s*[=;]", code) and "(" not in s:
            prev = " ".join(L[max(0, i - 2):i])
            if "UPROPERTY" not in prev and "TWeakObjectPtr" not in raw and "class " not in raw:
                rawptr.append(f"{short(p)}:{n}  {s[:80]}")
        for m in re.finditer(r"Cast<([A-Za-z_]\w*)>\(", code):
            if "if (" not in code and "if(" not in code:
                casty.append(f"{short(p)}:{n}  {s[:80]}")
        if "LoadObject<" in code and "if (" not in code:
            loado.append(f"{short(p)}:{n}  {s[:80]}")
        for m in re.finditer(r"/\s*([A-Za-z_]\w*)\b", code):
            if m.group(1) not in ("1", "2", "phi", "PHI"):
                divzero.append(f"{short(p)}:{n}  {s[:80]}")
                break
        if "PASS" in raw and "UE_LOG" in raw and "?" not in raw:
            passtrue.append(f"{short(p)}:{n}  {s[:90]}")
        if "SetActorLocation" in code and "Sweep" not in code and "SetActorLocationAndRotation" not in code:
            sweep.append(f"{short(p)}:{n}  {s[:80]}")
        if "USoundBase" in raw or "PlaySound" in raw:
            sound.append(f"{short(p)}:{n}")
        if re.search(r"\b(TODO|FIXME|HACK|XXX)\b", raw):
            todos.append(f"{short(p)}:{n}  {s[:80]}")
        for m in re.finditer(r"Bind(?:Key|Action)\w*\(([^;]*)\)", code):
            binds.append(f"{short(p)}:{n}  {m.group(1)[:90]}")


def block(title, rows, limit=14):
    print(f"\n### {title}: {len(rows)}")
    for r in rows[:limit]:
        print("   ", r)
    if len(rows) > limit:
        print(f"    ... {len(rows) - limit} more")


block("DEAD CODE (statement after an unconditional exit)", dead)
block("RAW POINTER MEMBERS WITHOUT UPROPERTY (GC hazard)", rawptr)
block("Cast<> OUTSIDE A CONDITION (null deref risk)", casty)
block("LoadObject<> WITHOUT A GUARD", loado)
block("DIVISION BY A VARIABLE", divzero)
block("UNCONDITIONAL 'PASS' LOGS (self-fulfilling gate)", passtrue)
block("SetActorLocation WITHOUT SWEEP (teleport through geometry)", sweep)
block("AUDIO REFERENCES", sound)
block("TODO CLASS MARKERS", todos)
print(f"\n### INPUT BINDINGS: {len(binds)}")
for b in binds:
    print("   ", b)
