"""Enable the engine Water plugin and wire the new code into the game project.

Everything here is engine-local: the Water plugin ships with UE 5.8, so no account
and no Fab download are involved.
"""
import json
import re

P = "C:/Users/Admin/HarkGame/"
UPROJECT = P + "VEILOMNI.uproject"
GM = P + "Source/VEILOMNI/VeilGameMode.cpp"
RH = P + "Source/VEILOMNI/VeilRunner.h"
RC = P + "Source/VEILOMNI/VeilRunner.cpp"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# ---------------- 1. Water plugin, engine-local ----------------
proj = json.loads(rd(UPROJECT))
plugins = proj.setdefault("Plugins", [])
have = {p.get("Name") for p in plugins}
added = []
for name in ("Water", "Niagara"):
    if name not in have:
        plugins.append({"Name": name, "Enabled": True})
        added.append(name)
wr(UPROJECT, json.dumps(proj, indent="\t"))
print("uproject plugins added:", added if added else "none (already enabled)")

# ---------------- 2. GameMode: water + controller + HUD ----------------
g = rd(GM)
if '#include "VeilWater.h"' not in g:
    g = g.replace('#include "VeilGameMode.h"',
                  '#include "VeilGameMode.h"\n'
                  '#include "VeilPlayerController.h"\n'
                  '#include "VeilHud.h"\n'
                  '#include "VeilRunner.h"\n'
                  '#include "VeilWater.h"', 1)

if "PlayerControllerClass = AVeilPlayerController::StaticClass();" not in g:
    g = re.sub(r"(AVeilGameMode::AVeilGameMode\(\)\s*\n\{\n)",
               r"\1\tPlayerControllerClass = AVeilPlayerController::StaticClass();\n"
               r"\tHUDClass = AVeilHud::StaticClass();\n"
               r"\tDefaultPawnClass = AVeilRunner::StaticClass();\n", g, count=1)

if "VeilWater::BuildBibleWater" not in g:
    g = g.replace("\tBuildTerrain();\n", "\tBuildTerrain();\n\tVeilWater::BuildBibleWater(GetWorld());\n", 1)
wr(GM, g)
print("gamemode wired: controller", "PlayerControllerClass" in g,
      "| hud", "HUDClass" in g, "| water", "BuildBibleWater" in g)

# ---------------- 3. Runner: Bible movement numbers + ATP ----------------
h = rd(RH)
if '#include "VeilMitochondria.h"' not in h:
    h = h.replace('#include "CoreMinimal.h"', '#include "CoreMinimal.h"\n#include "VeilMitochondria.h"', 1)

if "FVeilMitochondria Mito;" not in h:
    h = h.replace("\tAVeilRunner();\n",
                  "\tAVeilRunner();\n\n"
                  "\t// Bible movement: walk 439, sprint 750, jump 185, glide sink -50, gravity -980.\n"
                  "\tFVeilMitochondria Mito;\n"
                  "\tbool bSprint = false;\n"
                  "\tbool bGlide = false;\n"
                  "\tvoid SetSprinting(bool bOn) { bSprint = bOn; }\n", 1)

if "virtual void Tick(float DeltaSeconds) override;" not in h:
    h = h.replace("\tvoid LookUp(float Value);\n",
                  "\tvoid LookUp(float Value);\n\tvirtual void Tick(float DeltaSeconds) override;\n", 1)
wr(RH, h)

c = rd(RC)
c = c.replace("GetCharacterMovement()->MaxWalkSpeed = 618.f;",
              "GetCharacterMovement()->MaxWalkSpeed = 439.f;   // Bible walk")
c = c.replace("GetCharacterMovement()->JumpZVelocity = 618.f;",
              "GetCharacterMovement()->JumpZVelocity = 185.f;  // Bible jump\n"
              "\tGetCharacterMovement()->GravityScale = 1.f;    // -980 cm/s^2")

if "AVeilRunner::Tick" not in c:
    c += """
void AVeilRunner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move)
	{
		return;
	}

	// Sprint is 750, walk is 439, and the number is never interpolated by taste.
	Move->MaxWalkSpeed = bSprint ? 750.f : 439.f;

	Mito.Tick(DeltaSeconds, bSprint);

	// The glide: holding jump in the air sinks at -50, not at terminal velocity.
	const bool bHeld = bPressedJump || bGlide;
	if (Move->IsFalling() && bHeld && Move->Velocity.Z < -50.f)
	{
		FVector V = Move->Velocity;
		V.Z = -50.f;
		Move->Velocity = V;
	}
}
"""
wr(RC, c)
print("runner wired: walk 439", "439.f" in c, "| jump 185", "185.f" in c, "| tick", "AVeilRunner::Tick" in c)
print("apply_addons complete")
