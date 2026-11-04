"""Look for the PDF inside the imported conversation, not just on disk.

A file pasted into a chat can live in the transcript as base64. If the bytes are there, this
writes them out so pdftotext can read the real document; if they are not, it says so plainly
instead of reporting a document nobody can open.
"""
import base64
import json
import os
import re

SRC = ".cloud-attachments/imported-conversation-47c112215752291e6c67eca591b10707aca39854d38380750ab6361d20dce003.json"
text = open(SRC, encoding="utf-8", errors="replace").read()
print("transcript chars: %d" % len(text))

names = sorted(set(re.findall(r"[A-Za-z0-9._\- ()]{0,60}\.pdf", text, re.I)))
print("pdf names mentioned: %d" % len(names))
for n in names[:20]:
    print("  " + n.strip())

print("application/pdf mentions: %d" % text.lower().count("application/pdf"))
b64_hits = len(re.findall(r"JVBERi0", text))
print("base64 PDF headers (JVBERi0 = %PDF): %d" % b64_hits)

print("copilot mentions: %d" % text.lower().count("copilot"))

if b64_hits:
    i = text.find("JVBERi0")
    start = i
    end = i
    ok = set("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=\n\r")
    while end < len(text) and text[end] in ok:
        end += 1
    raw = text[start:end].replace("\n", "").replace("\r", "")
    try:
        data = base64.b64decode(raw + "=" * (-len(raw) % 4))
    except Exception as e:
        print("decode failed: %s" % e)
    else:
        if data[:4] == b"%PDF":
            os.makedirs("Docs/refs", exist_ok=True)
            out = "Docs/refs/transcript_attachment.pdf"
            open(out, "wb").write(data)
            print("WROTE %s (%d bytes) from the transcript" % (out, len(data)))
        else:
            print("decoded bytes are not a PDF (first 8: %r)" % data[:8])
