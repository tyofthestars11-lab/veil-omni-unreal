import time, subprocess
# DLAM ONE continuous autopilot, merged
# Loop, whole: pull -> install -> build -> if fail, report red, wait, retry, no reset, no stop
while True:
    print("DLAM ONE: pull, install, build VEILEditor")
    subprocess.run(["git", "-C", "C:\\Users\\Admin\\phi-ledger-cookie-chain", "pull", "origin", "main"])
    subprocess.run(["powershell", "-File", "C:\\Users\\Admin\\phi-ledger-cookie-chain\\veil\\VEIL\\Tools\\veil_omni_install_to_pc.ps1"])
    r = subprocess.run(["cmd", "/c", "\"C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Build\\BatchFiles\\Build.bat\" VEILEditor Win64 Development \"D:\\VEIL-OMNI\\VEIL.uproject\" -WaitMutex"])
    if r.returncode == 0:
        print("DLAM ONE: build succeeded, opening editor")
        subprocess.Popen(["C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor.exe", "D:\\VEIL-OMNI\\VEIL.uproject"])
        break
    print("DLAM ONE: build failed, reading build_errors, retrying in 60s, no reset")
    time.sleep(60)
