"""Synthesize real phi-tuned WAV cues. No placeholders, no downloads."""
import math
import os
import struct
import wave

PHI = 1.618033988749895
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "audio")
os.makedirs(OUT, exist_ok=True)
RATE = 44100


def write(name, base_hz, seconds, partials):
    frames = []
    n = int(RATE * seconds)
    for i in range(n):
        t = i / RATE
        env = math.exp(-t * (1.0 / PHI) * 4.0)
        v = 0.0
        for k, amp in enumerate(partials, start=1):
            v += amp * math.sin(2.0 * math.pi * base_hz * k * t)
        frames.append(int(max(-1.0, min(1.0, v * env * 0.5)) * 32767))
    path = os.path.join(OUT, name)
    with wave.open(path, "w") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(RATE)
        f.writeframes(b"".join(struct.pack("<h", s) for s in frames))
    print("wrote %s %d frames %.0f Hz" % (name, n, base_hz))


BASE = 220.0
write("veil_strike.wav", BASE * PHI, 0.45, [1.0, 0.5 / PHI, 0.25 / PHI ** 2, 0.12 / PHI ** 3])
write("veil_beacon.wav", BASE * PHI ** 2, 1.10, [1.0, 0.6 / PHI, 0.3 / PHI ** 2])
write("veil_step.wav", BASE / PHI, 0.18, [1.0, 0.4 / PHI ** 2])
print("audio cues ready in", OUT)
