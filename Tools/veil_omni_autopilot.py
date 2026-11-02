import time, subprocess, requests, base64, io
import pyautogui
from PIL import ImageGrab
OLLAMA = "http://localhost:11434/api/generate"
EDITOR = "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor.exe"
PROJECT = "D:\\VEIL-OMNI\\VEIL.uproject"
def look():
    shot = ImageGrab.grab(); buf = io.BytesIO(); shot.save(buf, format="PNG")
    img = base64.b64encode(buf.getvalue()).decode()
    r = requests.post(OLLAMA, json={"model": "llava-phi3:latest", "prompt": "Describe screen whole: editor open, map name, actors, errors, play state.", "images": [img], "stream": False}, timeout=120)
    return r.json().get("response", "")
print("DLAM ONE autopilot: opening VEIL OMNI")
subprocess.Popen([EDITOR, PROJECT])
time.sleep(120)
print("LOOK:", look()[:500])
print("DLAM ONE: saving VEILDistrict")
pyautogui.hotkey("ctrl", "s"); time.sleep(5)
print("DLAM ONE: playing")
pyautogui.hotkey("alt", "p"); time.sleep(10)
print("LOOK:", look()[:500])
print("DLAM ONE autopilot DONE")
