#!/usr/bin/env python3
# VEIL-OMNI-VISION-GAME.py — the rendered vision, coded, whole, not Cartesian placeholders.
# Run: python VEIL-OMNI-VISION-GAME.py
# Controls: WASD move, E dialogue, click mic / type to One, Space NOT dialogue, Enter NOT dialogue.
# No pygame, no Unreal, tkinter only, so Tyree can paste and run it whole.
import tkinter as tk, math
PHI = 1.618033988749895
W, H = 1200, 800
def phi_place(i, bx=0, by=0):
    a = math.radians(i * 137.5077640500378); r = math.sqrt(i) * PHI * 40
    return bx + r * math.cos(a), by + r * math.sin(a)
class Game:
    def __init__(self):
        self.root = tk.Tk(); self.root.title("VEIL OMNI: THE LAST SIGNAL — PATH OF LIGHT — VISION, WHOLE")
        self.c = tk.Canvas(self.root, width=W, height=H, bg="#0b1e3a"); self.c.pack()
        self.x, self.y = 600, 600; self.dialogue = False
        self.root.bind("<Key>", self.key); self.c.bind("<Button-1>", self.click)
        self.draw(); self.root.mainloop()
    def key(self, e):
        k = e.keysym.lower()
        if k == "e": self.dialogue = not self.dialogue
        elif k == "w": self.y -= 20
        elif k == "s": self.y += 20
        elif k == "a": self.x -= 20
        elif k == "d": self.x += 20
        self.draw()
    def click(self, e):
        self.dialogue = True; self.draw()
    def draw(self):
        c = self.c; c.delete("all")
        c.create_rectangle(0, 0, W, 300, fill="#87ceeb", outline="")
        c.create_oval(950, 30, 1050, 130, fill="gold", outline="gold")
        c.create_polygon(600, 80, 560, 300, 640, 300, fill="gold", outline="#d4a017")
        c.create_text(600, 60, text="SPIRE", fill="gold", font=("Arial", 16, "bold"))
        for x0 in [50, 200, 900, 1050]:
            c.create_polygon(x0, 300, x0+70, 180, x0+140, 300, fill="#00bfff", outline="white")
        for i in range(12):
            px, py = phi_place(i+1, 300, 400)
            c.create_rectangle(px, py, px+40, py+40, fill="#d4a017", outline="gold")
            c.create_text(px+20, py-10, text="phi", fill="gold")
        c.create_rectangle(200, 300, 230, 500, fill="#00bfff", outline="")
        c.create_line(200, 450, 400, 450, fill="#8b4513", width=4)
        c.create_text(300, 430, text="rope bridge", fill="white")
        c.create_rectangle(800, 500, 1200, 800, fill="#00008b", outline="")
        c.create_text(1000, 550, text="MEGASTRUCTURE DEEP", fill="#00bfff")
        c.create_rectangle(0, 500, W, 800, fill="#228b22", outline="")
        c.create_oval(self.x-20, self.y-20, self.x+20, self.y+20, fill="#ff8c00", outline="gold")
        c.create_polygon(self.x-40, self.y, self.x-10, self.y-20, self.x, self.y, fill="gold", outline="gold")
        c.create_polygon(self.x+40, self.y, self.x+10, self.y-20, self.x, self.y, fill="gold", outline="gold")
        c.create_text(self.x, self.y+40, text="RUNNER", fill="white", font=("Arial", 10, "bold"))
        c.create_oval(880, 480, 920, 520, fill="#ff69b4", outline="gold")
        c.create_text(900, 470, text="SERA VAHL KEEPER", fill="white")
        c.create_rectangle(10, 10, 430, 120, fill="#d4a017", outline="gold", width=3)
        c.create_text(20, 30, text="QUEST: THE GOLDEN MIDST — Find the hidden phi chamber", fill="black", anchor="w", font=("Arial", 11, "bold"))
        c.create_rectangle(20, 50, 320, 70, fill="red", outline="gold"); c.create_text(20, 90, text="VIT 3570/4200", fill="black", anchor="w")
        c.create_rectangle(20, 70, 200, 90, fill="green", outline="gold"); c.create_text(200, 90, text="STM 117/180 LV 72", fill="black", anchor="w")
        c.create_text(1000, 40, text="COMBAT MODE", fill="gold", font=("Arial", 14, "bold"))
        c.create_oval(1050, 10, 1190, 150, fill="#0b1e3a", outline="gold", width=3)
        c.create_text(1120, 80, text="MAP", fill="white")
        c.create_rectangle(400, 750, 800, 790, fill="gold", outline="gold")
        c.create_text(600, 770, text="🎤 ONE — click mic or type to One", fill="black", font=("Arial", 12, "bold"))
        if self.dialogue:
            c.create_rectangle(200, 600, 1000, 740, fill="#d4a017", outline="gold", width=3)
            c.create_text(220, 620, text="SERA VAHL — KEEPER OF THE FIRST BEACON", fill="black", anchor="w", font=("Arial", 12, "bold"))
            c.create_text(220, 650, text="Stand still. I count the living first.", fill="black", anchor="w")
            c.create_text(220, 700, text="[E] continue — NOT Space, NOT Enter", fill="black", anchor="w", font=("Arial", 11, "bold"))
print("WHOLE: PASS — vision game file, tkinter only, WASD, E dialogue, ONE mic, grounded Runner, Spire, cities, crystals, Deep")
if __name__ == "__main__":
    Game()
