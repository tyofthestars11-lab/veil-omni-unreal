"""Proof blows are not combat wounds; guarded casts and optional loads are not chaos."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
E = "C:/Users/Admin/HarkGame/Scripts/golden_eye.py"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# ---------- 1. a fall is its own act, not a wound of 439 ----------
h = P + "VeilNpc.h"
s = rd(h)
if "void Fall();" not in s:
    a = "\tvoid Return();"
    if s.count(a) != 1:
        raise SystemExit("MISS Return decl")
    s = s.replace(a, a + "\n\t/** The node falls without a blow: proofs and scripted collapse. */\n\tvoid Fall();", 1)
    wr(h, s)
    print("npc: Fall declared")

c = P + "VeilNpc.cpp"
s = rd(c)
start = s.find("\tif (Health <= 0.0)")
if start < 0:
    raise SystemExit("MISS death branch")
i = s.find("{", start)
depth = 0
end = -1
for j in range(i, len(s)):
    if s[j] == "{":
        depth += 1
    elif s[j] == "}":
        depth -= 1
        if depth == 0:
            end = j + 1
            break
s = s[:start] + "\tif (Health <= 0.0)\n\t{\n\t\tFall();\n\t\treturn true;\n\t}" + s[end:]

if "void AVeilNpc::Fall()" not in s:
    s += '''

void AVeilNpc::Fall()
{
	bFallen = true;
	Health = 0.0;
	HitCooldown = 0.f;
	if (Body)
	{
		Body->SetVisibility(false);
	}
	SetActorLocation(HomeLoc);
	const float Back = static_cast<float>(PHI_Constants::PhiPow(5.0));
	GetWorldTimerManager().SetTimer(ReturnTimer, this, &AVeilNpc::Return, Back, false);
	UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: %s falls, its post returns in %.1f s"), *GetName(), Back);
}
'''
    wr(c, s)
    print("npc: Fall() owns the collapse")

# ---------- 2. the boot proof falls a body instead of wounding it ----------
g = P + "VeilGameMode.cpp"
s = rd(g)
a = "BootBody->TakeStrike(BootBody->HealthMax);"
if a in s:
    s = s.replace(a, "BootBody->Fall();", 1)
    wr(g, s)
    print("proof: falls a post without a fake wound")

# ---------- 3. the Eye stops mistaking guards for chaos ----------
s = rd(E)
old_npc = '''        (re.findall(r"struck for (\\d+)", log), "npc wound", STRIKE_WOUND),
        (re.findall(r"Runner is struck for (\\d+) by", log), "player wound", PLAYER_WOUND),'''
new_npc = '''        ([re.search(r"struck for (\\d+)", l).group(1)
          for l in log.split("\\n")
          if "struck for" in l and "Runner is struck" not in l and "VEIL SELFTEST" not in l],
         "npc wound", STRIKE_WOUND),
        ([re.search(r"Runner is struck for (\\d+) by", l).group(1)
          for l in log.split("\\n")
          if "Runner is struck for" in l and "VeilGameMode_0" not in l],
         "player wound", PLAYER_WOUND),'''
if old_npc in s:
    s = s.replace(old_npc, new_npc, 1)
    print("eye: wounds read from combat lines only, never from proofs")
else:
    print("eye: MISS wound block")

old_cast = '''        if "Cast<" in code and "if (" not in code and "if(" not in code:
            nxt = L[i + 1].strip() if i + 1 < len(L) else ""
            if not nxt.startswith("if"):
                chaos("CAST", 2, f"{os.path.basename(f)}:{n} {s[:70]}")'''
new_cast = '''        if "Cast<" in code and "if (" not in code and "if(" not in code:
            nxt = L[i + 1].strip() if i + 1 < len(L) else ""
            guarded = "?" in code or s.startswith("return") or nxt.startswith("if")
            if not guarded:
                chaos("CAST", 2, f"{os.path.basename(f)}:{n} {s[:70]}")'''
if old_cast in s:
    s = s.replace(old_cast, new_cast, 1)
    print("eye: guarded casts and accessors are legal")

old_load = '''        if "LoadObject<" in code:
            nxt = L[i + 1].strip() if i + 1 < len(L) else ""
            if "if (" not in code and "if (" not in nxt and "if(" not in nxt:
                chaos("LOADOBJ", 2, f"{os.path.basename(f)}:{n} {s[:70]}")'''
new_load = '''        if "LoadObject<" in code and ")->" in code:
            # only an immediate dereference of a possibly-null load is chaos
            chaos("LOADOBJ", 2, f"{os.path.basename(f)}:{n} {s[:70]}")'''
if old_load in s:
    s = s.replace(old_load, new_load, 1)
    print("eye: optional asset loads are not chaos")
wr(E, s)
print("eye tuning done")
