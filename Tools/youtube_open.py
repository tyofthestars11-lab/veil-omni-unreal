import subprocess, time, pyautogui
subprocess.Popen(["C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe", "https://www.youtube.com"])
time.sleep(3)
pyautogui.hotkey("ctrl", "l")
pyautogui.write("https://www.youtube.com", interval=0.02)
pyautogui.press("enter")
print("YouTube opened, actual browser, not narration")
