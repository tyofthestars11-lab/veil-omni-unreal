#!/usr/bin/env bash
# Tools/veil-cli.sh - drive VEIL's own command line, headless.
#
# This is the connection to the game that does not require opening Unreal: UnrealEditor-Cmd
# runs the real game offscreen at 320x180 (VEIL_NULLRHI=1 for no RHI at all) with no editor
# UI and nothing on the desktop, then exits. Every PHI verdict the world prints lands
# in one log, and the verify verb turns that log into a pass or a fail an agent can trust.
#
#   ./Tools/veil-cli.sh build              compile VEILEditor (the binary the editor loads)
#   ./Tools/veil-cli.sh headless [secs]     run the game headless, then print the verdicts
#   ./Tools/veil-cli.sh cmd "<commands>"    run console commands headless (e.g. "stat fps")
#   ./Tools/veil-cli.sh verify             verdicts only, PASS/FAIL, exit 1 on FAIL
#   ./Tools/veil-cli.sh log [lines]        tail the headless log
#   ./Tools/veil-cli.sh bake <script.py>   run a project Python script headless
#
# Verbs never open a window and never touch the level unless the script you name does.

set -o pipefail

# Git Bash rewrites any argument that starts with a slash into a Windows path, so
# /Game/Maps/VEILDistrict arrived at Unreal as C:/.../git/Game/Maps/VEILDistrict and the
# map never loaded. Both switches turn that rewriting off for the whole script.
export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL='*'

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENGINE="/c/Program Files/Epic Games/UE_5.8/Engine"
CMD_BIN="$ENGINE/Binaries/Win64/UnrealEditor-Cmd.exe"
BUILD_BAT="$ENGINE/Build/BatchFiles/Build.bat"
PROJ="$ROOT/VEIL.uproject"
MAP="${VEIL_MAP:-/Game/Maps/VEILDistrict}"
LOG="$ROOT/Saved/Logs/veil_cli.log"
WLOG="$(cygpath -w "$LOG" 2>/dev/null || echo "$LOG")"
WPROJ="$(cygpath -w "$PROJ" 2>/dev/null || echo "$PROJ")"

# The lines that decide whether the world is real. Each one is printed by a pass that only
# reports what it measured, so a missing line means the pass did not run.
VERDICT_PATTERNS=(
  "PHI ATMOSPHERE"
  "PHI OPEN SKY"
  "PHI HUB MARKET"
  "PHI ENGINE SHAPE CLEANUP"
  "PHI FLAT CULL"
  "PHI NEAR"
  "PHI CROWD"
  "PHI KIT"
  "PHI TERRAIN"
  "PHI SEA"
)

verdicts() {
  echo "=== VEIL verdicts from $(basename "$LOG") ==="
  if [ ! -f "$LOG" ]; then echo "  no log yet - run: $0 headless"; return 2; fi
  local pat found=0
  local -a silent=()
  for pat in "${VERDICT_PATTERNS[@]}"; do
    local hits
    hits=$(grep -ahoE "${pat}[^\"]{0,170}" "$LOG" 2>/dev/null | sort -u | head -4)
    if [ -n "$hits" ]; then
      echo "$hits" | sed 's/^/  /'
      found=$((found + 1))
    else
      silent+=("$pat")
    fi
  done
  if [ ${#silent[@]} -gt 0 ]; then
    echo "  silent this run: ${silent[*]}"
    echo "  (silent is not the same as failed: the passes in AVEILPlayerController::BeginPlay do"
    echo "   not begin play without a local player. Only the placeholder verdict decides.)"
  fi
  local still swap
  still=$(grep -ahoE "PHI (PLACEHOLDER SWAP|ENGINE SHAPE CLEANUP): .*" "$LOG" 2>/dev/null | tail -1)
  echo
  if [ -z "$still" ]; then
    echo "  VERDICT: INCOMPLETE - the world never reported; $found of ${#VERDICT_PATTERNS[@]} passes spoke."
    return 2
  fi
  if echo "$still" | grep -q "NO PLACEHOLDER IN THE FRAME"; then
    echo "  VERDICT: PASS - no placeholder stood at the end of the sweep."
    return 0
  fi
  echo "  VERDICT: FAIL - placeholders still stand: $still"
  return 1
}

case "${1:-help}" in
  build)
    shift
    # -f, NOT -x. Build.bat is a batch file, so MSYS reports it as not executable and the
    # verb refused to build at all: "no Build.bat at ..." on a path that exists. The test
    # was checking a permission that a .bat can never have.
    [ -f "$BUILD_BAT" ] || { echo "no Build.bat at $BUILD_BAT"; exit 2; }
    "$BUILD_BAT" VEILEditor Win64 Development -Project="$WPROJ" \
      -WaitMutex -FromMsBuild -MaxParallelActions=12 -NoUBA "$@" > "$ROOT/Saved/Logs/veil_cli_build.log" 2>&1
    grep -aE "Result: " "$ROOT/Saved/Logs/veil_cli_build.log" | tail -1 | sed 's/^/  /'
    grep -aE "error C[0-9]+|fatal error|Unable to build" "$ROOT/Saved/Logs/veil_cli_build.log" | head -5 | cut -c1-190 | sed 's/^/  /'
    grep -aq "Result: Succeeded" "$ROOT/Saved/Logs/veil_cli_build.log"
    ;;

  headless)
    SECS="${2:-45}"
    [ -f "$CMD_BIN" ] || { echo "no UnrealEditor-Cmd at $CMD_BIN"; exit 2; }
    rm -f "$LOG"
    echo "=== headless game, ${SECS}s, no window, no RHI: $MAP"
    if [ "${VEIL_NULLRHI:-0}" = "1" ]; then
      RHIFLAGS="-nullrhi"
      echo "  mode: null RHI (GameMode only - controller-side passes stay silent)"
    else
      RHIFLAGS="-RenderOffscreen -windowed -ResX=320 -ResY=180 -ForceRes"
      echo "  mode: offscreen 320x180 (a real local player, nothing on screen)"
    fi
    timeout "$SECS" "$CMD_BIN" "$WPROJ" \
      "$MAP" -game $RHIFLAGS -nosplash -unattended -NoSound -stdout -FullStdOutLogOutput \
      -log -abslog="$WLOG" > /dev/null 2>&1
    echo "  process exit=$? (124 = the timeout stopped it, which is the normal ending)"
    grep -ahoE "Fatal error[^\"]{0,120}|Video memory[^\"]{0,60}" "$LOG" 2>/dev/null | head -3 | sed 's/^/  /'
    echo
    verdicts
    ;;

  cmd)
    shift
    [ -n "${1:-}" ] || { echo "usage: $0 cmd \"stat unit; stat fps\""; exit 2; }
    [ -f "$CMD_BIN" ] || { echo "no UnrealEditor-Cmd at $CMD_BIN"; exit 2; }
    rm -f "$LOG"
    echo "=== console, headless: $*"
    timeout 180 "$CMD_BIN" "$WPROJ" \
      "$MAP" -game -nullrhi -nosplash -unattended -NoSound -log -abslog="$WLOG" \
      -ExecCmds="$*" > /dev/null 2>&1
    echo "  process exit=$?"
    grep -ahoE "LogTemp[^\"]{0,160}" "$LOG" 2>/dev/null | tail -12 | sed 's/^/  /'
    ;;

  bake)
    shift
    SCRIPT="${1:-}"
    # A REPO-RELATIVE PATH IS NOT A PATH UNREAL CAN OPEN. -script is resolved against
    # Engine/Binaries/Win64, so "Tools/bake_vision.py" became
    # "Binaries/Win64/Tools/bake_vision.py" and every bake died at load with no VEIL BAKE
    # line in the log. The verb now resolves the file itself and hands Unreal an absolute
    # Windows path, which is the only form it opens reliably.
    case "$SCRIPT" in
      /*|[A-Za-z]:*) SRC="$SCRIPT" ;;
      *) SRC="$ROOT/$SCRIPT" ;;
    esac
    [ -n "$SCRIPT" ] && [ -f "$SRC" ] || { echo "usage: $0 bake Tools/bake_world.py"; exit 2; }
    WSCRIPT=$(cygpath -w "$SRC")
    [ -f "$CMD_BIN" ] || { echo "no UnrealEditor-Cmd at $CMD_BIN"; exit 2; }
    rm -f "$LOG"
    echo "=== headless Python: $SCRIPT"
    timeout 300 "$CMD_BIN" "$WPROJ" \
      -run=pythonscript -script="$WSCRIPT" -nosplash -unattended -NoSound -nullrhi \
      -log -abslog="$WLOG" > /dev/null 2>&1
    echo "  process exit=$?"
    grep -ahoE "LogPython[^\"]{0,160}|BAKE[^\"]{0,160}" "$LOG" 2>/dev/null | tail -14 | sed 's/^/  /'
    ;;

  verify) verdicts ;;

  log)
    N="${2:-40}"
    [ -f "$LOG" ] || { echo "no log yet - run: $0 headless"; exit 2; }
    tail -n "$N" "$LOG" | cut -c1-200
    ;;

  shot)
    # One rendered frame, no editor: the game runs offscreen at 1280x720, the project's own
    # -phishot harness writes a HighresScreenshot, and the newest one is copied into Docs/shots.
    SECS="${2:-60}"
    [ -f "$CMD_BIN" ] || { echo "no UnrealEditor-Cmd at $CMD_BIN"; exit 2; }
    rm -f "$LOG"
    echo "=== rendering one frame offscreen, ${SECS}s, 1280x720"
    timeout "$SECS" "$CMD_BIN" "$WPROJ" "$MAP" -game -RenderOffscreen -windowed -ResX=1280 -ResY=720 \
      -ForceRes -nosplash -unattended -NoSound -phishot -log -abslog="$WLOG" > /dev/null 2>&1
    echo "  process exit=$? (124 = timeout, the normal ending)"
    # SORTED BY TIME, NOT BY PATH. `| sort | tail -1` orders the file NAMES, so the frame this verb
# copied as proof was whichever path sorted last - repeatedly an older beat than the shutter that
# had just fired, which is how a hub-view capture that wrote nothing went unnoticed for hours.
local_new=$(find "$ROOT/Saved/Screenshots" -name '*.png' -newermt '-6 minutes' -printf '%T@ %p\n' 2>/dev/null | sort -n | tail -1 | cut -d' ' -f2-)
    if [ -n "$local_new" ]; then
      mkdir -p "$ROOT/Docs/shots"
      cp "$local_new" "$ROOT/Docs/shots/_cli_shot.png"
      echo "  frame: $local_new"
      echo "  copied: Docs/shots/_cli_shot.png ($(wc -c < "$ROOT/Docs/shots/_cli_shot.png") bytes)"
    else
      echo "  no frame was written - the world rendered nothing in that window"
    fi
    verdicts
    ;;

  help|*)
    # Only the comment header, whatever its length: print from line 2 until the first line
    # that is not a comment, so internal switches never leak into the help text.
    awk 'NR>1 && /^#/ { sub(/^# ?/, ""); print; next } NR>1 { exit }' "${BASH_SOURCE[0]}"
    ;;
esac
