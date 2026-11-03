import io

def rw(path, fn):
    s = io.open(path, encoding="utf-8", newline="").read()
    n = "\r\n" if "\r\n" in s else "\n"
    out = fn(s, n)
    io.open(path, "w", encoding="utf-8", newline="").write(out)
    return out


HUD_BLOCK = """\t// ONE, IN ONE PLACE. The type bar and the state line were two doors for one voice: a
\t// "click here to type" chip and a separate "ready - hold B to speak" line floating above
\t// it, both on screen at once, both about the same conversation. They are one control now:
\t// click it and you speak or type to ONE from that click, and nothing pops up until you do.
\t{
\t\tconst VEILTypeBar::FZone Zone = VEILTypeBar::ForScreen(Canvas->ClipX, Canvas->ClipY);
\t\tconst AVEILPlayerController* VPC = Cast<AVEILPlayerController>(PC);
\t\tconst bool bTypingNow = VPC && VPC->IsTypingToONE();
\t\tconst UPHIONE* One = W->GetSubsystem<UPHIONE>();
\t\tconst UVEILDialogue* DlgNow = W->GetSubsystem<UVEILDialogue>();
\t\tconst bool bBusy = DlgNow && DlgNow->IsActive();
\t\tconst EONEState OneState = One ? One->GetState() : EONEState::Ready;
\t\tconst bool bListening = OneState == EONEState::Listening;
\t\tconst bool bWorking = OneState == EONEState::Thinking;
\t\tconst bool bSpeaking = OneState == EONEState::Speaking;
\t\tFVeilUI::Chip(this, FLinearColor(0.035f, 0.045f, 0.075f, (bTypingNow || bListening) ? 0.92f : 0.60f),
\t\t\tZone.X, Zone.Y, Zone.W, Zone.H);
\t\tconst FLinearColor Edge(1.f, 0.78f, 0.30f, (bTypingNow || bListening) ? 0.95f : 0.42f);
\t\tDrawRect(Edge, Zone.X, Zone.Y, Zone.W, 1.4f);
\t\tDrawRect(Edge, Zone.X, Zone.Y + Zone.H - 1.4f, Zone.W, 1.4f);
\t\tconst FLinearColor Orb = bSpeaking ? FLinearColor(1.f, 0.85f, 0.35f)
\t\t\t: bWorking ? FLinearColor(0.45f, 0.95f, 0.85f)
\t\t\t: bListening ? FLinearColor(0.85f, 0.92f, 1.f)
\t\t\t: FLinearColor(0.30f, 0.34f, 0.44f);
\t\tDrawRect(Orb, Zone.X + 12.f, Zone.Y + 12.f, 6.f, 6.f);
\t\tconst TCHAR* Label = bTypingNow ? TEXT("TYPING TO ONE  -  ENTER SENDS, ESC CLOSES")
\t\t\t: bListening ? TEXT("ONE IS LISTENING  -  speak now, or type")
\t\t\t: bWorking ? TEXT("ONE IS WORKING")
\t\t\t: TEXT("ONE  -  click to speak or type");
\t\tDrawText(Label, (bTypingNow || bListening) ? FLinearColor(1.f, 0.90f, 0.55f) : FLinearColor(0.88f, 0.82f, 0.64f),
\t\t\tZone.X + 26.f, Zone.Y + 5.f, Med, 0.60f);
\t\tif (One && !bBusy)
\t\t{
\t\t\tconst FString OneLine = One->StateLine();
\t\t\tDrawText(OneLine, bSpeaking ? FLinearColor(1.f, 0.92f, 0.72f) : FLinearColor(0.82f, 0.88f, 1.f),
\t\t\t\tZone.X + 26.f, Zone.Y + 22.f, Med, 0.56f);
\t\t}
\t}"""


def hud(s, n):
    a = lambda t: t.replace("\n", n)
    start = s.find("// CLICK HERE TO TYPE TO ONE")
    assert start > 0, "hud start"
    end = s.find(a("\n\t}\n\n\tif (bWon)"), start)
    assert end > start, "hud end"
    return s[:start] + a(HUD_BLOCK) + s[end:]


CAST_BLOCK = """
\t// A PERSON, NOT A PLACEHOLDER. Twenty-four authored people across the four factions - the
\t// Ashfall crews, the Lantern Watch, the Quiet Court and the Hollow - each with a rank and a
\t// temperament. The row comes from the machine's own id, so a patrol is the same person every
\t// session and neighbours are never the same person. Temperament changes how it fights rather
\t// than labelling it: aggressive hits harder and closes faster, patient sees further and hits
\t// softer, curious watches and fires slower. Body scale and hue follow the row, so silhouette
\t// differs before anyone reads a name.
\t{
\t\tstruct FCastRow { const TCHAR* Name; const TCHAR* Rank; const TCHAR* Temper; };
\t\tstatic const FCastRow Cast[4][6] = {
\t\t\t{ {TEXT("Tova Renn"), TEXT("Cutter-Captain"), TEXT("aggressive")},
\t\t\t  {TEXT("Bram Kessler"), TEXT("Quartermaster"), TEXT("patient")},
\t\t\t  {TEXT("Nix"), TEXT("Salvage-Runner"), TEXT("curious")},
\t\t\t  {TEXT("Domen Arclight"), TEXT("Cartographer"), TEXT("curious")},
\t\t\t  {TEXT("Kesh Varo"), TEXT("Hull-Wright"), TEXT("patient")},
\t\t\t  {TEXT("Sable Ith"), TEXT("Crew-Head"), TEXT("aggressive")} },
\t\t\t{ {TEXT("Odalys Crane"), TEXT("Watch-Captain"), TEXT("aggressive")},
\t\t\t  {TEXT("Ferris Holt"), TEXT("Keeper of the Ledger"), TEXT("patient")},
\t\t\t  {TEXT("Mirea"), TEXT("Shield-Aunt"), TEXT("patient")},
\t\t\t  {TEXT("Sera Vahl"), TEXT("Keeper of the First Beacon"), TEXT("curious")},
\t\t\t  {TEXT("Halden Rook"), TEXT("Lamp-Warden"), TEXT("curious")},
\t\t\t  {TEXT("Iset Marr"), TEXT("Watch-Sergeant"), TEXT("aggressive")} },
\t\t\t{ {TEXT("Vespera Dal"), TEXT("First Voice"), TEXT("patient")},
\t\t\t  {TEXT("Elias Brume"), TEXT("The Notary"), TEXT("curious")},
\t\t\t  {TEXT("Sahn"), TEXT("Cipher-Adjutant"), TEXT("curious")},
\t\t\t  {TEXT("Ilva Marrow"), TEXT("The Seer"), TEXT("patient")},
\t\t\t  {TEXT("Corin Vey"), TEXT("Quiet Bailiff"), TEXT("aggressive")},
\t\t\t  {TEXT("Adea Lune"), TEXT("Court Archivist"), TEXT("patient")} },
\t\t\t{ {TEXT("The Listener"), TEXT("First of the Hollow"), TEXT("curious")},
\t\t\t  {TEXT("The Pale Host"), TEXT("The Hollow's answer"), TEXT("aggressive")},
\t\t\t  {TEXT("Ash-Witness"), TEXT("Hollow Echo"), TEXT("patient")},
\t\t\t  {TEXT("The Second Question"), TEXT("Hollow Echo"), TEXT("curious")},
\t\t\t  {TEXT("Pale Chorus"), TEXT("Hollow Echo"), TEXT("patient")},
\t\t\t  {TEXT("The Answerer"), TEXT("Hollow Echo"), TEXT("aggressive")} }
\t\t};
\t\tstatic const TCHAR* FactionWords[4] = {
\t\t\tTEXT("Ashfall Consortium"), TEXT("Lantern Watch"), TEXT("Quiet Court"), TEXT("Hollow") };
\t\tconst int32 FIdx = FMath::Clamp((int32)Faction, 0, 3);
\t\tconst int32 CIdx = FMath::Abs((int32)(GetUniqueID() / 7)) % 6;
\t\tCastName = Cast[FIdx][CIdx].Name;
\t\tCastRank = Cast[FIdx][CIdx].Rank;
\t\tCastTemperament = Cast[FIdx][CIdx].Temper;
\t\tif (CastTemperament == TEXT("aggressive"))
\t\t{
\t\t\tMaxHealth *= 0.85f; AttackRange *= 1.15f; WeaponDamage *= 1.35f; FireInterval *= 0.80f;
\t\t}
\t\telse if (CastTemperament == TEXT("patient"))
\t\t{
\t\t\tMaxHealth *= 1.25f; AttackRange *= 0.85f; WeaponDamage *= 0.80f; SightRange *= 1.30f;
\t\t}
\t\telse
\t\t{
\t\t\tSightRange *= 1.10f; FireInterval *= 1.15f;
\t\t}
\t\tHealth = MaxHealth;
\t\tTArray<UStaticMeshComponent*> BodyMeshes;
\t\tGetComponents<UStaticMeshComponent>(BodyMeshes);
\t\tfor (UStaticMeshComponent* BM : BodyMeshes)
\t\t{
\t\t\tif (!BM || BM->GetFName() != FName(TEXT("MachineBody"))) { continue; }
\t\t\tconst float Wide = 0.60f + 0.05f * (float)CIdx;
\t\t\tBM->SetRelativeScale3D(FVector(Wide, Wide, 1.30f + 0.10f * (float)CIdx));
\t\t\tstatic const FLinearColor Hues[6] = {
\t\t\t\tFLinearColor(0.20f, 0.11f, 0.06f), FLinearColor(0.18f, 0.20f, 0.28f),
\t\t\t\tFLinearColor(0.07f, 0.18f, 0.20f), FLinearColor(0.22f, 0.17f, 0.07f),
\t\t\t\tFLinearColor(0.16f, 0.09f, 0.22f), FLinearColor(0.24f, 0.22f, 0.20f) };
\t\t\tPHITintComponent(BM, Hues[CIdx], 0.f);
\t\t}
\t\tUE_LOG(LogVEIL, Warning, TEXT("PHI CAST: %s, %s of the %s - %s - at %s."),
\t\t\t*CastName, *CastRank, FactionWords[FIdx], *CastTemperament,
\t\t\t*GetActorLocation().ToCompactString());
\t}
"""


def faction_header(s, n):
    if "CastName" in s:
        return s
    anchor = "\tUPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"VEIL|Faction\")\n\tEVEILFaction Faction = EVEILFaction::Void;".replace("\n", n)
    assert s.count(anchor) == 1, "header anchor"
    add = ("\t/** WHO THIS MACHINE IS. Not NPC_03: a named member of its faction, with a rank and a\n"
           "\t *  temperament that changes how it fights, picked from its own id so it is the same\n"
           "\t *  person every session and never the same person as its neighbour. */\n"
           "\tFString CastName;\n\tFString CastRank;\n\tFString CastTemperament;\n\n").replace("\n", n)
    return s.replace(anchor, add + anchor, 1)


def faction_cpp(s, n):
    if "PHI CAST:" in s:
        return s
    b = s.find("void AVEILFactionAgent::BeginPlay()")
    assert b > 0, "beginplay"
    ob = s.find("{", b)
    assert ob > b, "beginplay brace"
    return s[:ob + 1] + CAST_BLOCK.replace("\n", n) + s[ob + 1:]


def one_words(s, n):
    s = s.replace('TEXT("ready - hold B to speak")', 'TEXT("ready - click to speak or type")')
    s = s.replace("awake, ready - hold B to speak.", "awake, ready - click the ONE bar to speak or type.")
    return s


rw("Source/VEIL/Private/Game/VEILHudCanvas.cpp", hud)
rw("Source/VEIL/Public/AI/VEILFactionAI.h", faction_header)
rw("Source/VEIL/Private/AI/VEILFactionAI.cpp", faction_cpp)
rw("Source/VEIL/Private/Systems/PHIONE.cpp", one_words)
print("patched: hud, one wording, cast")
