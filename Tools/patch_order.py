"""One layout authority, and the three things the cook found.

Chaos had a cause: the builder raises a district, then the sanctum raises a deck at the same
centre, then the concept world raises its own plaza, market, falls, bridges and crystals over
both, then the vista scatters terraces at radii the market already occupies. Four authors, one
patch of ground.

So:
  1. the concept composition pass stops building - the builder owns the district layout;
  2. every vista element gets its own radius band, outside the market ring;
  3. components disallow Nanite, because the kit material is not flagged for it and the cooked
     build fell back to Default Material (the flat grey in the package);
  4. the shipped default map is the district that is actually tuned;
  5. the art directories cook always, so the Polyart body texture survives packaging.
"""

import io
import os

fails = []

# ---------------------------------------------------------------- 1. one builder
GM = "Source/VEIL/Private/Game/VEILGameMode.cpp"
if os.path.exists(GM):
    s = io.open(GM, encoding="utf-8", newline="").read()
    N = "\r\n" if "\r\n" in s else "\n"
    if "PHI COMPOSITION: concept pass skipped" not in s:
        old = "\t\t\t\t\t\tConcept->Build();"
        new = ("\t\t\t\t\t\t// ONE COMPOSITION PASS, NOT TWO. This raised its own plaza, market," + N +
               "\t\t\t\t\t\t// falls, bridges and crystals on top of the district the builder had just" + N +
               "\t\t\t\t\t\t// raised at the same coordinates: props interpenetrating props, which is" + N +
               "\t\t\t\t\t\t// what reads as chaos. The builder owns the layout now." + N +
               "\t\t\t\t\t\tUE_LOG(LogVEIL, Warning," + N +
               "\t\t\t\t\t\t\tTEXT(\"PHI COMPOSITION: concept pass skipped - the builder owns the district layout.\"));")
        if s.count(old) == 1:
            s = s.replace(old, new, 1)
            io.open(GM, "w", encoding="utf-8", newline="").write(s)
            print("concept world: composition pass disabled (second layout removed)")
        else:
            fails.append("concept anchor count=%d" % s.count(old))

# ---------------------------------------------------------------- 2. radius bands
WB = "Source/VEIL/Private/World/VEILWorldBuilder.cpp"
if os.path.exists(WB):
    s = io.open(WB, encoding="utf-8", newline="").read()
    N = "\r\n" if "\r\n" in s else "\n"
    bands = [
        ("const float Rad = 2300.f * FMath::Pow(PhiV, (float)T);",
         "const float Rad = 3600.f * FMath::Pow(PhiV, (float)T);", "terraces -> 3600 cm band"),
        ("const float Rad = 3400.f * FMath::Pow(PhiV, (float)(i % 3) * 0.25f);",
         "const float Rad = 5600.f * FMath::Pow(PhiV, (float)(i % 3) * 0.25f);", "falls -> 5600 cm band"),
        ("const float Rad = 5200.f * FMath::Pow(PhiV, (float)(i % 5) * 0.20f);",
         "const float Rad = 7800.f * FMath::Pow(PhiV, (float)(i % 5) * 0.20f);", "crystals -> 7800 cm band"),
        ("const float Rad = 1100.f * FMath::Pow(PhiV, (float)i);",
         "const float Rad = 1500.f * FMath::Pow(PhiV, (float)i);", "phi pillars -> 1500 cm band"),
    ]
    for old, new, label in bands:
        if s.count(old) == 1:
            s = s.replace(old, new, 1)
            print(label)
        else:
            fails.append("%s count=%d" % (label, s.count(old)))

    # 3. no Nanite on components: the kit material is not flagged for it
    if "bDisallowNanite" not in s:
        old = "\tComp->SetStaticMesh(Mesh);"
        new = ("\t// THE KIT MATERIAL IS NOT FLAGGED FOR NANITE. The cooked build reported" + N +
               "\t// \"MI_Modular1 missing usage flag Nanite! Default Material will be used in game\", so" + N +
               "\t// every wall in the package rendered as engine grey. Disallowing Nanite on the" + N +
               "\t// component keeps the authored material on the mesh that is actually drawn." + N +
               "\tComp->bDisallowNanite = true;" + N +
               old)
        if s.count(old) >= 1:
            s = s.replace(old, new, 1)
            io.open(WB, "w", encoding="utf-8", newline="").write(s)
            print("components disallow Nanite (kit material stays authored in the cook)")
        else:
            fails.append("SetStaticMesh anchor count=%d" % s.count(old))
    else:
        io.open(WB, "w", encoding="utf-8", newline="").write(s)

# ---------------------------------------------------------------- 4. shipped map
ENG = "Config/DefaultEngine.ini"
if os.path.exists(ENG):
    s = io.open(ENG, encoding="utf-8", newline="").read()
    N = "\r\n" if "\r\n" in s else "\n"
    import re
    before = re.findall(r"(GameDefaultMap|EditorStartupMap)=([^\r\n]+)", s)
    s = re.sub(r"(GameDefaultMap=)[^\r\n]+", r"\1/Game/Maps/VEILDistrict.VEILDistrict", s)
    io.open(ENG, "w", encoding="utf-8", newline="").write(s)
    print("shipped map was %s -> GameDefaultMap=/Game/Maps/VEILDistrict" % (before or "unset"))

# ---------------------------------------------------------------- 5. cook the art
GAME = "Config/DefaultGame.ini"
s = io.open(GAME, encoding="utf-8", newline="").read() if os.path.exists(GAME) else ""
N = "\r\n" if "\r\n" in s else "\n"
add = ""
for path in ("/Game/Polyart", "/Game/Sicka_EnvPack2", "/Game/PHI"):
    line = '+DirectoriesToAlwaysCook=(Path="%s")' % path
    if line not in s:
        add += line + N
if add:
    section = "[/Script/UnrealEd.ProjectPackagingSettings]"
    if section not in s:
        s = s + N + section + N
    s = s + add
    io.open(GAME, "w", encoding="utf-8", newline="").write(s)
    print("always-cook directories added: Polyart, Sicka kit, PHI palette")
else:
    print("always-cook directories already set")

if fails:
    print("FAILURES:", fails)
    raise SystemExit(1)
print("ORDER WRITTEN")
