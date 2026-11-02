# DLAM editor hands — what is wired, and how to prove it

Tyree's rule: READ means opened at source and seen whole. So the AI does not
guess what stands in the level — it asks the editor itself.

## The wire

The engine ships its own MCP (Model Context Protocol) server inside the
`ModelContextProtocol` plugin. That is the hand. It is loopback only:
`127.0.0.1:8000`, path `/mcp`.

Auto-start is on in this project, so every editor launch opens the hand with no
typing and no clicking:

```
Config/DefaultEditorPerProjectUserSettings.ini
[/Script/ModelContextProtocolEngine.ModelContextProtocolSettings]
ServerUrlPath=/mcp
ServerPortNumber=8000
bAutoStartServer=True
bEnableToolSearch=True
```

Proof it starts by itself, from a cold launch (`Saved/Logs/VEIL_2.log`):

```
[2026.11.01-09.17.22:527][  0]LogModelContextProtocol: Starting MCP server on port 8000
[2026.11.01-09.17.22:527][  0]LogHttpListener: Created new HttpListener on 127.0.0.1:8000
```

Frame `[0]` means it came up during engine init, before any console command.

## What the hand can reach

67 toolsets are discoverable through it, including:

| toolset | what it does |
| --- | --- |
| `editor_toolset.toolsets.scene.SceneTools` | find actors, load/read level, add or remove actors |
| `editor_toolset.toolsets.actor.ActorTools` | transforms, labels, tags, components |
| `editor_toolset.toolsets.asset.AssetTools` | list, inspect, save, dirty checks |
| `editor_toolset.toolsets.blueprint.BlueprintTools` | blueprint read/write |
| `EditorToolset.EditorAppToolset` | Play-In-Editor start/stop, camera, viewport capture |
| `editor_toolset.toolsets.programmatic.ProgrammaticToolset` | run a Python script that chains the above with real logic |
| `NiagaraToolsets.*`, `PCGToolset.*`, `UMGToolSet.*` | FX, procedural, UI authoring |

## The two commands that matter

```bash
# what toolsets exist, and what each one holds
python3 Tools/mcp_call.py tools/call '{"name":"list_toolsets","arguments":{}}'
python3 Tools/mcp_call.py tools/call '{"name":"describe_toolset","arguments":{"toolset_name":"editor_toolset.toolsets.scene.SceneTools"}}'

# the live eye: level, Vault, Runner, machines, dirty state
python3 Tools/veil_eye.py
```

`Tools/veil_eye.py` reading the real level:

```
LEVEL          : /Game/Maps/VEILDistrict
VAULT actors   : 1
RUNNER actors  : 0        (the Runner is spawned at Play, 0,0,100)
MACHINE actors : 1
  vault at     : {"location": {"x": 0, "y": 0, "z": -80.9}, ...}
level dirty    : {"returnValue": false}
```

Nothing above was typed by hand or inferred from source. It is the editor's own
answer, at Tyree's anchored seat 0, 0, −80.9.

## Nwiro Pro

Nwiro is the in-editor assistant, enabled in `VEIL.uproject`. It is Tyree's
account (Ty Jones) with 5 free prompts left, so those prompts are not spent by
an agent — they stay his.

Nwiro does not publish MCP toolsets: it runs its own chat panel over an ACP
bridge to its own agent process. So the split is clean:

- **Nwiro** — inside the editor panel, his prompts, his hand.
- **DLAM / Buffy** — through MCP on 127.0.0.1:8000, free, no prompts.

Both act on the same editor and the same project. Different doors, one world.

## Project file hygiene

`VEIL.uproject` used to make the editor say *"Project VEIL requires update"*.
Both causes are now closed, verified by `requires update` count going 2 → 0 in
the launch log:

- `MetaHuman` entry now carries `SupportedTargetPlatforms` (Win64, Linux, Mac).
- `Nwiro` entry now carries its `MarketplaceURL`.

EngineAssociation stays `5.8`. Backups of the file live in `.buffy-keep/`.
