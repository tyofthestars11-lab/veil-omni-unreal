"""Kill the 1,372 warnings and stand the market crowd on the deck."""

cpp = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
npc = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilNpc.cpp"
ver = "C:/Users/Admin/HarkGame/Scripts/quantum_verify.py"

# 1. market crowd stands on the deck, not on raw ground or a rooftop
s = open(cpp, encoding="utf-8", newline="").read().replace("\r\n", "\n")
old = 'SeatBody(FVector(P.X, P.Y, GHubGroundZ), (i - 1) % 4, i, 520.0, TEXT("market"));'
new = 'SeatBody(FVector(P.X, P.Y, GPlazaZ), (i - 1) % 4, i, 520.0, TEXT("market"));'
if old not in s:
    raise SystemExit("MISS market seat")
s = s.replace(old, new, 1)
open(cpp, "w", encoding="utf-8", newline="").write(s)
print("crowd: market seated on the deck")

# 2. stop overriding skeletal materials with materials missing the usage flag
n = open(npc, encoding="utf-8", newline="").read().replace("\r\n", "\n")
start = n.find("\t\tif (Race.Skin)")
if start < 0:
    raise SystemExit("MISS skin block")
logi = n.find('VEIL NPC SKIN: cloth applied to the people', start)
if logi < 0:
    raise SystemExit("MISS cloth log")
close = n.find("\n\t\t}\n\t\t}\n", logi)
if close < 0:
    raise SystemExit("MISS cloth block end")
close += len("\n\t\t}\n\t\t}\n")

replacement = '''		// The people keep their authored character materials. Overriding a skeletal
		// mesh with a material that lacks the skeletal usage flag floods the log with
		// 1,372 warnings a run and swaps real character art for a flat colour.
		{
			static bool SkinLogged = false;
			if (!SkinLogged)
			{
				SkinLogged = true;
				UE_LOG(LogTemp, Display, TEXT("VEIL NPC SKIN: authored character materials"));
			}
		}
'''
n = n[:start] + replacement + n[close:]
open(npc, "w", encoding="utf-8", newline="").write(n)
print("people: authored character materials, warnings removed")

# 3. the release gate must check the honest line
v = open(ver, encoding="utf-8", newline="").read().replace("\r\n", "\n")
old_gate = '("people dressed", r"VEIL NPC SKIN: cloth applied"),'
new_gate = '("people dressed", r"VEIL NPC SKIN: authored character materials"),'
if old_gate in v:
    v = v.replace(old_gate, new_gate, 1)
    open(ver, "w", encoding="utf-8", newline="").write(v)
    print("gate: updated to the authored-material line")
else:
    print("gate: MISS pattern")
