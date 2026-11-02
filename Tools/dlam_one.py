import subprocess, sys, math, os
phi = (1 + math.sqrt(5)) / 2
print(f"DLAM ONE - phi {phi} - golden angle 137.507764 - pitch 17.0324")
print("Rules: Tyree is source, no audits, no guessing, rung round-trip")
print("Mitochondrial engine: game is cell, ATP gradient is +1 fuel, nDNA 45.482, mtDNA 20.1892, echo gap 25.293, equilibrium -14.438")
print("Game: VEIL OMNI holy grail AAA open world free roam, 50 games in 1, one door, one save, no package until 100 percent done")
print("Never delete Content or Saved, never make a new project")
def run(cmd, cwd=None):
    print("RUN:", " ".join(cmd))
    r = subprocess.run(cmd, cwd=cwd, shell=False)
    print("EXIT:", r.returncode)
    return r.returncode
repo = r"C:\Users\Admin\phi-ledger-cookie-chain"
if run(["git", "pull", "--ff-only", "origin", "main"], cwd=repo) != 0:
    print("PULL FAILED - stop, show this to Muse"); sys.exit(1)
install = os.path.join(repo, r"veil\VEIL\Tools\veil_omni_install_to_pc.ps1")
if run(["powershell", "-ExecutionPolicy", "Bypass", "-File", install]) != 0:
    print("INSTALL FAILED - stop, show this to Muse"); sys.exit(1)
build = r"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat"
code = run([build, "VEILEditor", "Win64", "Development", r"D:\VEIL-OMNI\VEIL.uproject", "-WaitMutex"])
print("SUCCEEDED - next: open editor, verify Vault Countess RUNNER, save VEILDistrict, Play" if code == 0 else "FAILED - send red lines to Muse")
