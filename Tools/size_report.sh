#!/usr/bin/env bash
# VEIL OMNI - size report. Measured, not estimated.
# Run after a cook: bash Tools/size_report.sh
set -u
cd "$(dirname "$0")/.." || exit 1

echo "VEIL OMNI - PACKAGE SIZE REPORT"
echo "-------------------------------"

if [ ! -d Saved/StagedBuilds/Windows ]; then
  echo "no staged build. run cook_and_package.bat first."
  exit 1
fi

echo "STAGED BUILD"
du -sh Saved/StagedBuilds/Windows/VEIL 2>/dev/null | awk '{print "  game content      " $1}'
du -sh Saved/StagedBuilds/Windows/Engine 2>/dev/null | awk '{print "  engine shaders    " $1}'
ls -l Saved/StagedBuilds/Windows/VEIL/Binaries/Win64/VEIL.exe 2>/dev/null |
  awk '{printf "  game binary       %.0f MB\n", $5/1048576}'
du -sh Saved/StagedBuilds 2>/dev/null | awk '{print "  TOTAL             " $1}'

echo
echo "CONTAINER"
for f in Saved/StagedBuilds/Windows/VEIL/Content/Paks/*.ucas \
         Saved/StagedBuilds/Windows/VEIL/Content/Paks/*.utoc \
         Saved/StagedBuilds/Windows/VEIL/Content/Paks/*.pak; do
  [ -e "$f" ] || continue
  ls -l "$f" | awk -v n="$(basename "$f")" '{printf "  %-24s %.1f MB\n", n, $5/1048576}'
done

echo
echo "SOURCE VS SHIPPED"
SRC=$(du -sm Content 2>/dev/null | cut -f1)
PK=$(du -sm Saved/StagedBuilds/Windows/VEIL/Content 2>/dev/null | cut -f1)
if [ -n "${SRC:-}" ] && [ -n "${PK:-}" ] && [ "$PK" -gt 0 ]; then
  echo "  source content    ${SRC} MB"
  echo "  shipped content   ${PK} MB"
  awk -v s="$SRC" -v p="$PK" 'BEGIN { printf "  ratio             %.2f : 1\n", s/p }'
fi

echo
echo "TARGET 15-20 GB shipped, ceiling 25 GB. See Docs/PACKAGING.md."
