import base64, io, time, re, sys, requests, subprocess
import pyautogui
from PIL import ImageGrab
OLLAMA = "http://localhost:11434/api/generate"
CHROME = "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"
def look():
    shot = ImageGrab.grab(); buf = io.BytesIO(); shot.save(buf, format="PNG")
    img = base64.b64encode(buf.getvalue()).decode()
    r = requests.post(OLLAMA, json={"model": "llava-phi3:latest", "prompt": "Describe screen, buttons, text, whole.", "images": [img], "stream": False}, timeout=120)
    return r.json().get("response", "")
def run_chain(chain):
    for part in re.split(r"[|\n]", chain):
        p = part.strip().strip('"').strip()
        if not p: continue
        if p.startswith("OPEN"):
            m = re.search(r"https?://\S+", p)
            if m: subprocess.Popen([CHROME, m.group(0)])
        elif p.startswith("TYPE"):
            pyautogui.write(p[5:].strip().strip('"'), interval=0.02)
        elif p.startswith("PRESS"):
            pyautogui.press(p[6:].strip().lower())
        elif p.startswith("CLICK"):
            n = re.findall(r"\d+", p)
            if len(n) >= 2: pyautogui.click(int(n[0]), int(n[1]))
        time.sleep(1)
def act(task):
    print("ONE: opening, no look first")
    run_chain("OPEN chrome https://www.youtube.com/results?search_query=tyofthestarz9535 | DONE")
    for _ in range(10):
        seen = look()
        r = requests.post(OLLAMA, json={"model": "one", "prompt": "Task: " + task + "\nScreen: " + seen + "\nReply ONLY one chain: OPEN chrome https://www.youtube.com | TYPE tyofthestarz9535 | PRESS enter | DONE", "stream": False}, timeout=120)
        cmd = r.json().get("response", "").strip()
        print("ONE:", cmd)
        low = cmd.lower()
        if "large language model" in low or "cannot" in low:
            print("ONE: refusal filtered, retrying"); continue
        run_chain(cmd)
        if cmd.strip().endswith("DONE"): break
        time.sleep(2)
if __name__ == "__main__":
    act(" ".join(sys.argv[1:]) or "Go to youtube.com and search tyofthestarz9535")
