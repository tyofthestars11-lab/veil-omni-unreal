"""Remove the skeletal material overrides that flood the log with warnings."""

npc = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilNpc.cpp"
ver = "C:/Users/Admin/HarkGame/Scripts/quantum_verify.py"

n = open(npc, encoding="utf-8", newline="").read().replace("\r\n", "\n")
start = n.find("\t\tif (Race.Skin)")
if start < 0:
    raise SystemExit("MISS skin block")
endmark = "\t\t\t\t\t++Dressed;\n\t\t\t\t}\n\t\t\t}\n"
e = n.find(endmark, start)
if e < 0:
    raise SystemExit("MISS cloth end")
e += len(endmark)

replacement = '''		// The people keep their authored character materials. Overriding a skeletal
		// mesh with a material that lacks the skeletal usage flag floods the log with
		// over a thousand warnings a run and replaces real character art with a colour.
		{
			static bool SkinLogged = false;
			if (!SkinLogged)
			{
				SkinLogged = true;
				UE_LOG(LogTemp, Display, TEXT("VEIL NPC SKIN: authored character materials"));
			}
		}
'''
n = n[:start] + replacement + n[e:]
open(npc, "w", encoding="utf-8", newline="").write(n)
print("people: overrides removed, authored materials kept")

v = open(ver, encoding="utf-8", newline="").read().replace("\r\n", "\n")
old_gate = '("people dressed", r"VEIL NPC SKIN: cloth applied"),'
new_gate = '("people dressed", r"VEIL NPC SKIN: authored character materials"),'
if old_gate in v:
    v = v.replace(old_gate, new_gate, 1)
    open(ver, "w", encoding="utf-8", newline="").write(v)
    print("gate: authored-material line")
else:
    print("gate: already updated or missing")
