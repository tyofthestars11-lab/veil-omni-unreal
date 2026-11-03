#!/usr/bin/env python3
# VEIL-OMNI-HUGE.py — the merged whole: ALL, PHYSICS + BLUEPRINTS, VISION GAME.
# Run: python VEIL-OMNI-HUGE.py          -> runs all three, opens the vision window
#      python VEIL-OMNI-HUGE.py --headless -> runs all three checks, no window
import importlib.util, sys
def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec); spec.loader.exec_module(mod); return mod
def main():
    ok = True
    for name, path, entry in (("veil_all", "VEIL-OMNI-ALL.py", "main"),
                              ("veil_phys", "VEIL-OMNI-PHYSICS-BLUEPRINTS.py", "main")):
        mod = load(name, path)
        ok = (getattr(mod, entry)() == 0) and ok
    vision = load("veil_vision", "VEIL-OMNI-VISION-GAME.py")
    if "--headless" in sys.argv:
        print("WHOLE: PASS — vision game loaded (window skipped: --headless)")
    else:
        vision.Game()
    return 0 if ok else 1
if __name__ == "__main__":
    raise SystemExit(main())
