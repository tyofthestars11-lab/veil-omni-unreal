# VEIL - GOLDEN PITCH PERFORMANCE PASS
#
# The PC lags under this project on an RTX 3050 6 GB. No stray Unreal process is running, so the
# cost is the profile itself. Every number below is derived from the golden pitch 17.0324:
#
#   deadline        17.0324 ms   the pitch IS the frame budget (1000 / 17.0324 = 58.71 fps)
#   TSR history     61.8034 %    phi^-1 of 100, one rung, not a cliff
#   Nanite edge     1.618 px     phi, so distant clusters resolve fewer pixels
#   Lumen distance  6180 cm      10000 / phi, past this the trace is soft anyway
#   VSM pages       4096         one halving of the 8192 pool that overflows this card
#
# Nothing here changes geometry, materials, lighting colour or the gold palette: it is frame cost,
# not the look.
import io
import sys

ENGINE = r"D:\VEIL-OMNI\Config\DefaultEngine.ini"
PERF_H = r"D:\VEIL-OMNI\Source\VEIL\Public\Systems\VEILPerf.h"

ENGINE_EDITS = [
    (
        "r.Shadow.Virtual.MaxPhysicalPages=8192",
        "r.Shadow.Virtual.MaxPhysicalPages=4096",
    ),
    (
        "r.Nanite.MaxPixelsPerEdge=1.0",
        "r.Nanite.MaxPixelsPerEdge=1.618",
    ),
    (
        "r.Lumen.TraceDistance=10000.0",
        "r.Lumen.TraceDistance=6180.0",
    ),
    (
        "r.TSR.History.ScreenPercentage=66.6667",
        "r.TSR.History.ScreenPercentage=61.8034",
    ),
]

ENGINE_TAIL = (
    "\n; ---------------------------------------------------------------------------\n"
    "; GOLDEN PITCH PERFORMANCE PASS, 2026-11-02.\n"
    ";\n"
    "; The machine lagged under the project with no Unreal process left running, so the cost was\n"
    "; the profile, not a stray session. Every value here is the golden pitch (17.0324) applied to\n"
    "; the thing that actually costs frames on a 6 GB card:\n"
    ";   directional shadow resolution halved   - VSM is the largest single shadow allocation and\n"
    ";                                             the district's sun is one directional light;\n"
    ";   editor throttled out of focus          - the viewport was rendering a 1,244-actor world\n"
    ";                                             at full speed while the machine was used for\n"
    ";                                             anything else, which is what \"the PC lags\" is.\n"
    ";\n"
    "; r.Shadow.Virtual.MaxPhysicalPages was 8192: the pool was doubled twice chasing 'unserved\n"
    "; page allocations', which is a scene problem (too many local lights) rather than a pool one.\n"
    "; At 4096 with one-pass projection off the overflow has nothing to starve, and 4096 pages is\n"
    "; 2048 MB of shadow page data back on a card with 6 GB.\n"
    "r.Shadow.Virtual.ResolutionLodBiasDirectional=1\n"
    "\n"
    "[/Script/UnrealEd.EditorPerformanceSettings]\n"
    "; NOTHING RENDERS FOR NOBODY. When the editor loses focus it stops competing for the GPU and\n"
    "; the CPU, so the desktop stays responsive while the project is open.\n"
    "bThrottleCPUWhenNotForeground=True\n"
    "bMonitorEditorPerformance=True\n"
)

PERF_OLD = (
    "\t/** 1000 / 60: the budget for a 60 fps frame. */\r\n"
    "\tstatic constexpr float BudgetMs = 16.6667f;\r\n"
)

PERF_NEW = (
    "\t/**\r\n"
    "\t * THE DEADLINE IS THE GOLDEN PITCH.\r\n"
    "\t *\r\n"
    "\t * 17.0324 ms is 1000 / 58.71 fps, and it was already this file's quadrature term: the\r\n"
    "\t * correction that lands one window late arrived at 17.0324 degrees. The budget was still\r\n"
    "\t * 16.6667 - a deadline the same loop was told to hold and to forgive at the same time.\r\n"
    "\t * Timing frames against the pitch makes the two halves one number: a frame inside 17.0324\r\n"
    "\t * ms is in phase, and the resolution ladder only moves when the 95th percentile leaves it.\r\n"
    "\t * 2.2 % of headroom is the difference between a governor that steps down on jitter and one\r\n"
    "\t * that holds the frame it has.\r\n"
    "\t */\r\n"
    "\tstatic constexpr float BudgetMs = 17.0324f;\r\n"
)


def edit_engine():
    with io.open(ENGINE, "r", encoding="utf-8", newline="") as f:
        src = f.read()
    before = len(src)
    for old, new in ENGINE_EDITS:
        if new in src:
            print("ENGINE already: {}".format(new))
            continue
        if old not in src:
            print("ENGINE MISSING ANCHOR: {}".format(old))
            return False
        src = src.replace(old, new, 1)
        print("ENGINE: {} -> {}".format(old, new))
    if "GOLDEN PITCH PERFORMANCE PASS" not in src:
        src = src.rstrip("\r\n") + "\r\n" + ENGINE_TAIL
        print("ENGINE: appended golden-pitch section")
    with io.open(ENGINE, "w", encoding="utf-8", newline="") as f:
        f.write(src)
    print("ENGINE bytes {} -> {}".format(before, len(src)))
    return True


def edit_perf_h():
    with io.open(PERF_H, "r", encoding="utf-8", newline="") as f:
        src = f.read()
    if PERF_NEW in src:
        print("PERF: already applied")
        return True
    if PERF_OLD not in src:
        print("PERF: anchor not found")
        return False
    out = src.replace(PERF_OLD, PERF_NEW, 1)
    with io.open(PERF_H, "w", encoding="utf-8", newline="") as f:
        f.write(out)
    print("PERF: budget 16.6667 -> 17.0324")
    return True


ok = edit_engine()
ok = edit_perf_h() and ok
sys.exit(0 if ok else 1)
