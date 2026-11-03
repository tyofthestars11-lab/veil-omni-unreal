# VEIL OMNI — THE CAST

This is the readable mirror of [VEILCast.cpp](Source/VEIL/Private/Story/VEILCast.cpp). The game reads
that file; this page is for reading the world without a compiler. If the two ever disagree, the code
is the truth.

Everything below is authored — names, ranks, creeds, history. Nothing here is a placeholder token
like `NPC_03`.

---

## The four factions

| Faction | Epithet | Creed | Stronghold | Colour |
|---|---|---|---|---|
| **THE ASHFALL CONSORTIUM** | Salvage is a religion | "Everything ends. We are what remains — and we sell it." | The Nest, beneath the dead spires | amber |
| **THE LANTERN WATCH** | Keepers of the lit beacons | "A light is a promise kept, not a claim made." | The Hub, around the First Beacon | cold blue |
| **THE QUIET COURT** | Heirs of the last broadcast | "Silence is the oldest coin. We are its bankers." | The Spire, in the sealed upper rings | violet |
| **THE HOLLOW** | Not a faction. A condition | "We listened first. We are still listening." | The Deep, wherever the Signal pools | bone white |

The engine's faction ids are unchanged — `Scavenger`, `Guardian`, `Remnant`, `Void`. Ashfall is the
salvage crews, the Watch holds the lights, the Court owns the quiet, the Hollow is what answers.

---

## The people

### The Lantern Watch

**SERA VAHL** — *Keeper of the First Beacon* (statue at the Hub, 900, 0)
Four centuries ago she carried the first flame down the Spire while the Signal was still screaming.
Her body did not survive the descent; her pattern did. The statue is not a monument to her — it is
her, still keeping count of every lamp lit in her name.

**CAPTAIN ODALYS CRANE** — *Watch-Captain of the Hub*
Held the plaza through the Third Silence with forty volunteers and a broken lance. Has not slept a
full night since, and will not say why.

**DEACON FERRIS HOLT** — *Keeper of the Ledger*
Records every lit beacon in ink and every unlit one in his memory. Believes the Signal is a debt and
the Vault is the receipt.

**SHIELD-AUNT MIREA** — *Warden of the Nest Gate*
Walked out of the Hollow with her name intact and refuses to say what she traded for it.

**HALDEN ROOK** — *Lamp-Warden of the Spire Stair*
Keeps the two hundred and eleven lamps on the stair by hand, one at a time, because the last
time the Watch trusted a mechanism the stair went dark for a year. Counts them aloud on the way
up; it is the only prayer the Watch still says.

**ISET MARR** — *Watch-Sergeant, Night Ledger*
Commands the night shift — the only people awake when the Signal is loudest. Writes down what her
patrols hear and burns the pages that answer.

### The Quiet Court

**ILVA MARROW** — *The Seer of the Nest* (statue at -450, 780)
Reads ash the way the Court reads contracts: for what was left out. Bought her sight from the Hollow
and has been paying the interest ever since.

**LADY VESPERA DAL** — *First Voice of the Quiet Court*
Owns nine of the ten quietest rooms in the Spire. Has never raised her voice, and three districts
have gone silent on her word.

**ELIAS BRUME** — *The Notary*
Writes down what people intend, not what they say, and files it where it will hurt most.

**CIPHER-ADJUTANT SAHN** — *Court Signals Officer*
Decodes the Court's dead channels for fun. Two of them decoded back.

**CORIN VEY** — *Quiet Bailiff*
Enforces the Court's silence with a ledger and a key. Has never drawn a weapon in the Spire and
has never needed to: the rooms he locks stay locked because the people inside agree with him.

**ADEA LUNE** — *Court Archivist of the Sealed Rings*
Keeps every broadcast the world ever made, filed by what it was afraid of. She has read the last
one and has not slept in the Spire since, which the Court records as dedication.

### The Ashfall Consortium

**DOMEN ARCLIGHT** — *Cartographer of the Deep* (statue at -4150, -780)
Mapped the Megastructure by walking it wrong on purpose, so the corridors would show him their
second face. Half his maps are of places that only exist when nobody is looking.

**BRAM KESSLER** — *Quartermaster of Ashfall*
Counts what everyone else throws away, and has never once been wrong about what a thing is worth —
only about who would pay.

**TOVA RENN** — *Cutter-Captain*
Opens hulls with shaped charges and bad language. Has lost eleven people to the Deep and remembers
all eleven names, in order.

**NIX** — *Salvage-Runner, no surname offered*
Came up out of the Nest with no memory and a perfect ear for machine rhythm. Can hear a failing pump
through three walls.

**KESH VARO** — *Hull-Wright of the Nest Dry-Docks*
Welds hulls that were never built to hold air, well enough that the Consortium forgives the eleven
ships that came back empty. Keeps their manifest bolts in a jar and calls it inventory.

### The Hollow

**THE LISTENER** — *No name. No face. First of the Hollow*
The technician who first heard the Signal and answered it with a question. What came back wears them
like a coat, and still asks the question.

**THE PALE HOST** — *The Hollow's answer*
Where the Signal pools it makes a body out of whatever is nearby — dust, cable, the recently dead.
What the Deep sends when it wants to be understood.

---

**THE THIRD ECHO** — *Hollow Echo, three seconds late*
Every answer the Hollow gives arrives late, and the Echo is the lateness: it repeats the district
three seconds after it has finished speaking, in a tone that is almost right.

**ASH-WITNESS** — *The Hollow's memory*
Has watched the same square metre of the Deep for a century and remembers every foot that crossed
it, including the ones that never arrived.

**THE PALE CHOIR** — *Many voices, one throat*
Where the Signal pooled in a stairwell it learned harmony. A hymn that is also a census, sung at
whoever is closest.

**THE ANSWERER** — *Hollow Echo, speaking first*
The Listener asked; the Answerer is what came back early. It answers questions nobody has asked
yet, which is why the Watch fires on it before it finishes a sentence.

## Where you meet them

- **The three statues** speak their own character: Keeper → Sera Vahl, Seer → Ilva Marrow,
  Cartographer → Domen Arclight. Walk up, press **[E]**.
- **Every machine** in the field speaks as a named member of its faction, chosen deterministically
  from its id, so the same patrol is the same person each time you meet it.
- **Nameplates** float over people and machines in the world, fading with distance, stacking so two
  neighbours never print over each other.

- **Nameplates and voices are the same person.** The roster this page mirrors is what the
  machines claim: a patroller's plate, its rank and the lines it speaks under **[E]** all come
  from one cast row, and no two machines in a district claim the same person.

## The conversation box

`[E]` near someone opens it; `[SPACE]` finishes the arriving line and then moves to the next.

The box is a plate, not a chat log: faction colour spine, the speaker's name, their rank, their
faction's name and creed, then the line arriving at reading pace (42 characters a second), and a
prompt only once the line has landed.

Runner: [VEILDialogue.cpp](Source/VEIL/Private/Systems/VEILDialogue.cpp)
Pixels: [VEILHudCanvas.cpp](Source/VEIL/Private/Game/VEILHudCanvas.cpp)

## Proof switch

`-phitalk` opens Sera Vahl's scene through the same path `[E]` uses, so a live frame proves the cast,
the box and the typewriter without a keyboard:

```
UnrealEditor.exe D:\VEIL-OMNI\VEIL.uproject /Game/Maps/VEILDistrict -game -phitalk -phishow
```
