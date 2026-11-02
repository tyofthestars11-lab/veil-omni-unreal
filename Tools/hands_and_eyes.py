import base64, io, time, requests
import pyautogui
from PIL import ImageGrab
OLLAMA = "http://localhost:11434/api/generate"
BRAIN = "one"
EYES = "llava-phi3:latest"
def look(prompt="Describe this screen, whole: windows, buttons, text, errors."):
    shot = ImageGrab.grab()
    buf = io.BytesIO(); shot.save(buf, format="PNG")
    img = base64.b64encode(buf.getvalue()).decode()
    r = requests.post(OLLAMA, json={"model": EYES, "prompt": prompt, "images": [img], "stream": False}, timeout=120)
    return r.json().get("response", "")
def click_xy(x, y):
    pyautogui.click(x, y); time.sleep(0.5); return look("What changed after that click?")
def type_text(text):
    pyautogui.write(text, interval=0.02); return True
def press_key(key):
    pyautogui.press(key); time.sleep(0.5); return True
def open_program(path):
    import subprocess; subprocess.Popen(path); time.sleep(2); return look()
if __name__ == "__main__":
    print(look())
