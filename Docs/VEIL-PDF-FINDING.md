# The PDF, and what actually exists — measured 2026-11-04

Tyree asked for the PDF file to be opened and executed. This is what the machine returned.

## The PDF is not on this machine

Searched, all with `find`, no results:

| Where | Result |
|---|---|
| `D:\VEIL-OMNI` (whole tree, incl. `Docs/refs`, `.cloud-attachments`) | 0 `.pdf` |
| `C:\Users\Admin\Desktop`, `Downloads`, `Documents`, `OneDrive` | 0 `.pdf` |
| `D:\` to depth 5 | 0 `.pdf` |
| The 13 MB imported transcript in `.cloud-attachments` | 0 `.pdf` names, 0 `application/pdf`, 0 base64 PDF headers |
| `~/.copilot/attachments` | 0 `.pdf` |

`Docs/VEIL-STATE-2026-11-04.md:73-76` already records this: `VEIL-OMNI-PHI-PROOF-KRAKEN.pdf` is
"not on disk" and must be placed in the project.

`pdftotext` **is** installed (`/ucrt64/bin/pdftotext`), so the moment the file is at
`D:\VEIL-OMNI\Docs\refs\*.pdf` it can be read.

## What was found instead — the reference itself, inside Copilot's own attachments

`C:\Users\Admin\.copilot\attachments\` holds the two documents that were recorded as missing:

| Bytes | File |
|---|---|
| 919,687 | `VEIL-OMNI-HUGE-KRAKEN (1).html` — the "899K" in the state doc |
| 756,658 | `VEIL-OMNI-JARVIS-MODE.html` |

Both are now in the project as `Docs/refs/VEIL-OMNI-HUGE-KRAKEN.html` and
`Docs/refs/VEIL-OMNI-JARVIS-MODE.html`, with plain-text extractions beside them
(`*.txt`, 328 and 153 lines).

The extraction contains, in Tyree's own words: `Make the game BE this file. Golden Spire
(0,3500,500), Hub (0,0), Nest (-450,780), Omni (2800,500), Deep (0,-5000), 50 games in 1, seed
1618, golden angle only, no random, grounded, no placeholders, dialogue [E] only NOT Space NOT
Enter, HUD gold ribbon ONE mic only...`, plus the physics figures (walk 439, sprint 750, jump 185,
glide −50, gravity −980) and the HUD geometry (objective 420×110, minimap 180×180, manual 320×90,
quest 480px, VIT 3570/4200, STM 117/180, LV 72).

It carries **0** occurrences of "CURRENT DIRECTIVE"; that label was on a different export.

## GitHub Copilot CLI is installed and working

```
C:\Users\Admin\AppData\Local\GitHubCopilotCLI\copilot
GitHub Copilot CLI 1.0.105
smoke test -> "COPILOT ONLINE"   exit 0
```

VS Code is **not** installed for this profile (no `code` on PATH, no `Code.exe`), so the
"VS Code / GitHub Copilot extension" route does not exist here — the CLI does.

## What the Copilot session did with the reference

Opened interactively in `D:\VEIL-OMNI`, it loaded `.github/copilot-instructions.md`
(`Loading: 1 instruction`, 2 skills) and, asked to quote the files without inventing, returned:

- the five zones: Hub `(0,0)`, Nest `(-450,780)`, Spire `(0,3500)`, Omni District `(2800,500)`,
  Megastructure Deep `(0,-5000)`
- the eleven authority coordinates, including Runner `(0,0,100)`, Vault `(0,0,-80.9)`,
  Countess `(809,-309,64.7)`, Spire `(0,3500,500)`
- the HUD rules: gold ribbon, centre 60% clear, one mic, quest 480px, minimap 180×180
- all four `Tools/veil-cli.sh` verbs
- and it stated plainly, rather than guessing: *"It does not identify the existing Unreal district
  map, HUD, or gameplay implementation files to edit."*

That last line is the honest answer, and it is the actual next piece of work: map the reference onto
the real `Source/VEIL` files.

## Known defect in the session

```
Failed to start MCP Servers: Error: mcpServers: Required
```

The CLI reads a config whose schema it does not accept. It is harmless for code work — the MCP
server it would start is the Unreal editor bridge, which must not be opened anyway.

## Still true

- `UnrealEditor.exe` running: **0**
- no cook, no package, no exe/apk/obb
- headless verdict from this session: `VERDICT: PASS — no placeholder stood at the end of the sweep.`
