#!/usr/bin/env bash
# VEIL - ONE COMMAND GATE.
#
# The proof discipline for this project is: build line, bake audit, save values,
# and a frame, every time. Doing that by hand means remembering six commands and
# reading six logs, which is exactly where "looks done" creeps in. So it is a
# program: build both targets, bake the district, photograph it from the air,
# walk all five regions, play the full loop, and write ONE verdict to
# Saved/verify_report.txt with the lines it based that verdict on.
#
# Exit code 0 means every gate passed. Anything else means a gate failed and the
# reason is in the report.
#
#   bash Tools/verify_all.sh              # full gate
#   bash Tools/verify_all.sh skipbuild    # re-use the last binaries
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
UE="/c/Program Files/Epic Games/UE_5.8"
PROJ="$ROOT/VEIL.uproject"
OUT="$ROOT/Saved/verify"
REPORT="$ROOT/Saved/verify_report.txt"
mkdir -p "$OUT"
: > "$REPORT"

FAIL=0
say() { echo "$*" | tee -a "$REPORT"; }
lines() { tr -d '\000' < "$1" 2>/dev/null; }
count() { lines "$1" | grep -a -c "$2"; }

gate() { # gate <name> <condition-result> <detail>
  if [ "$2" -eq 0 ]; then say "PASS  $1"; else say "FAIL  $1 - $3"; FAIL=1; fi
}

say "VEIL VERIFY $(date '+%Y-%m-%d %H:%M:%S')"
say "project $PROJ"

# ---------------------------------------------------------------- 1. BUILD
if [ "${1:-}" != "skipbuild" ]; then
  for T in VEIL VEILEditor; do
    "$UE/Engine/Build/BatchFiles/Build.bat" "$T" Win64 Development -Project="$PROJ" \
      -WaitMutex -FromMsBuild -MaxParallelActions=12 > "$OUT/build_$T.log" 2>&1
    ERRS=$(lines "$OUT/build_$T.log" | grep -a -c -E "error C|error LNK|warning C")
    OK=1
    lines "$OUT/build_$T.log" | grep -aq "Result: Succeeded" && [ "${ERRS:-1}" -eq 0 ] && OK=0
    gate "build $T" "$OK" "$(lines "$OUT/build_$T.log" | grep -a -m1 -E "error C|error LNK|warning C")"
  done
else
  say "SKIP  build (re-using last binaries)"
fi

# ---------------------------------------------------------------- 2. BAKE
timeout 200 "$UE/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$PROJ" -run=pythonscript \
  -script="$ROOT/proof/bake_level.py" -unattended -nosplash -nullrhi -stdout \
  -FullStdOutLogOutput > "$OUT/bake.log" 2>&1
OK=1
lines "$OUT/bake.log" | grep -aq "CHECK RESULT PASS" && OK=0
gate "bake audit" "$OK" "no CHECK RESULT PASS"
say "      $(lines "$OUT/bake.log" | grep -a 'AFTER BAKE 2' | tail -1 | sed 's/.*\[VEIL CHECK\] //')"
PLACEHOLDERS=$(lines "$OUT/bake.log" | grep -a 'AFTER BAKE 2' | tail -1 | sed -n 's/.*, \([0-9]*\) engine placeholders.*/\1/p')
OK=1; [ "${PLACEHOLDERS:-1}" = "0" ] && OK=0
gate "no engine placeholders" "$OK" "placeholders=${PLACEHOLDERS:-?}"

# ------------------------------------------------------- 3. AERIAL FRAME
timeout 90 "$UE/Engine/Binaries/Win64/UnrealEditor.exe" "$PROJ" -game -windowed \
  -ResX=1280 -ResY=720 -phiair -log -abslog="$OUT/air.log" > /dev/null 2>&1
OK=1
[ "$(count "$OUT/air.log" "frame written from the air")" -ge 1 ] && OK=0
gate "aerial frames" "$OK" "no frame written"
say "      $(lines "$OUT/air.log" | grep -a 'VEIL AIR: the hub' | tail -1 | sed 's/.*VEIL AIR: //')"

# --------------------------------------------------- 4. FIVE REGION TOUR
timeout 70 "$UE/Engine/Binaries/Win64/UnrealEditor.exe" "$PROJ" -game -windowed \
  -ResX=1280 -ResY=720 -phitour -log -abslog="$OUT/tour.log" > /dev/null 2>&1
REGIONS=$(count "$OUT/tour.log" "VEIL TOUR [0-9]/5")
NOFLOOR=$(count "$OUT/tour.log" "floor NONE")
OK=1
[ "${REGIONS:-0}" -ge 5 ] && [ "${NOFLOOR:-1}" -eq 0 ] && OK=0
gate "five regions standable" "$OK" "reached=${REGIONS:-0}/5 floorless=${NOFLOOR:-?}"
lines "$OUT/tour.log" | grep -a "VEIL TOUR [0-9]/5" | sed 's/.*VEIL TOUR/     VEIL TOUR/' | tee -a "$REPORT" > /dev/null

# ------------------------------------------------------ 5. THE FULL LOOP
timeout 220 "$UE/Engine/Binaries/Win64/UnrealEditor.exe" "$PROJ" -game -windowed \
  -ResX=1280 -ResY=720 -phiomni -log -abslog="$OUT/loop.log" > /dev/null 2>&1
OK=1
lines "$OUT/loop.log" | grep -aq "whole session survived" \
  && lines "$OUT/loop.log" | grep -aq "PHI OMNI SAVE RESULT True" && OK=0
gate "full loop (walk/beacons/fight/death/respawn/save)" "$OK" "session did not complete"
lines "$OUT/loop.log" | grep -a "PHI OMNI PROOF: SAVE RESULT" | sed 's/.*PHI OMNI/     PHI OMNI/' | tee -a "$REPORT" > /dev/null

# ---------------------------------------------------------------- VERDICT
if [ "$FAIL" -eq 0 ]; then
  say "VERDICT PASS - built, baked, photographed, all five regions stood on, loop closed."
else
  say "VERDICT FAIL - see the FAIL lines above."
fi
exit "$FAIL"
