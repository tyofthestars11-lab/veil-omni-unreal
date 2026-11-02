# VEIL OMNI — Epic Launcher registration status

Date: 2026-10-31 (session ended by Tyree: "move on from Epic screen size")

## What was actually wrong (root cause)

`Tools/veil_omni_register_project.ps1` appended `CreatedProjectPaths` on every run instead of
replacing it. The launcher's config uses CRLF (and some lines CRCRLF), so the regex
`(?m)^CreatedProjectPaths=.*` never matched a whole line. Over several runs the `[Launcher]`
section grew five malformed, merged entries:

```
CreatedProjectPaths=D: CreatedProjectPaths=D: CreatedProjectPaths=D: HasMigratedFabIsFabMarkers=True
```

A second, bigger finding: **the Epic Launcher rewrites that key itself**, and it does not keep the
backslash form. After writing `CreatedProjectPaths=D:\`, the launcher came back with:

```
CreatedProjectPaths=D: HasMigratedFabIsFabMarkers=True
```

`D:` is a drive-relative path, not the drive root, so the scan has nothing valid to walk.
The forward-slash form is the one the launcher keeps intact, and it matches the shape of the
working default entry (`C:/Users/Admin/Documents/Unreal Projects`).

## Fix applied

1. `Tools/veil_omni_register_project.ps1` — replaced the regex edit with a line-based,
   idempotent normaliser: it pulls every `CreatedProjectPaths` line out, keeps the real values,
   re-emits exactly one Documents entry plus `D:\`, and preserves `HasMigratedFabIsFabMarkers`.
   Re-running it can no longer stack or merge entries.
2. Launcher config (`%LocalAppData%\EpicGamesLauncher\Saved\Config\WindowsEditor\GameUserSettings.ini`),
   final state — written as `D:/` because the launcher preserves that form:

```
CreatedProjectPaths=C:/Users/Admin/Documents/Unreal Projects
CreatedProjectPaths=D:/
HasMigratedFabIsFabMarkers=True
```

Backups kept next to the file: `.veil_backup_2026-10-31_2129`, `_2134`, `_2144`, `_2147`,
`_2148`, `.veil_backup_scale`, `.veil_backup_fwdslash`.

Nothing was moved, no project was created, `Content` and `Saved` were not touched.

## Verified (from the launcher's own log, `Saved/Logs/EpicGamesLauncher.log`)

```
LogDesktopPlatform: Found project "(ProjectName="D:/VEIL-OMNI/VEIL.uproject",
                    LastOpenTime=2026.11.01-03.43.17)" in recently opened files
```

The same scan also lists the stale `D:/VEIL/VEIL.uproject` from 2026.10.28 — the old-project
entry that caused the earlier "old game vs new launcher" confusion.

## NOT verified (honest gap)

No legible screenshot of the My Projects list showing **VEIL** as the project name was captured.

Why: this desktop is 3840x2160 at **300% scaling**. The launcher is DPI aware, so it lays its UI
out for a ~1034x581 viewport; only a fraction is visible at a time, the window hides itself when
it loses focus, and injected input only started working once `SendInput` was given the correct
40-byte `INPUT` struct (legacy `mouse_event` clicks were being swallowed). One frame did show a
project card with the VEIL district thumbnail, a 5.8 badge and a label ending in `...EIL`, but the
label was cut off, so it is not offered as proof.

Work was stopped at Tyree's instruction, before a clean My Projects frame and before the
Fab > My Library > Add to Project picker check.

## Recapture recipe, when wanted

- Make the capturing process per-monitor DPI aware first
  (`SetProcessDpiAwarenessContext(-4)`), otherwise it only sees a 1280x720 virtualised desktop.
- Then `CopyFromScreen` at 3840x2160 and the whole launcher UI is legible.
- Helpers used: `proof/phys_do.ps1` (DPI-aware capture/scroll/click), `proof/final_proof.ps1`,
  `proof/topwins.ps1` (window enumeration), `proof/click_test.ps1` (SendInput).
