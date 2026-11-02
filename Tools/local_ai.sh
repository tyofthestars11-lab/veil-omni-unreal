#!/usr/bin/env bash
# Ask a local Ollama model, optionally with an image.
#   bash Tools/local_ai.sh llama3.1:8b "what is this error"
#   bash Tools/local_ai.sh llava-phi3:latest "are these squares or diamonds?" proof/minimap_diamonds.jpg
#
# The image is base64'd into a payload FILE, not an argument: a 110 KB frame becomes
# ~150 KB of base64 and Windows refuses an argument list that long ("Argument list too long").
set -u
MODEL="${1:?model name required}"
PROMPT="${2:?prompt required}"
IMAGE="${3:-}"
URL="${OLLAMA_URL:-http://127.0.0.1:11434}"

PAYLOAD="$(mktemp)"
if [ -n "$IMAGE" ] && [ -f "$IMAGE" ]; then
  # 640 px wide is enough for a vision check and keeps the request small.
  SMALL="$(mktemp --suffix=.jpg)"
  FF="/c/Users/Admin/voice-bridge/ffmpeg-9.0.2-essentials_build/bin/ffmpeg"
  if [ -x "$FF" ]; then "$FF" -y -i "$IMAGE" -vf "scale=640:-1" -q:v 4 "$SMALL" >/dev/null 2>&1; else cp "$IMAGE" "$SMALL"; fi
  {
    printf '{"model":"%s","stream":false,"images":["' "$MODEL"
    base64 -w0 "$SMALL"
    printf '"],"prompt":"%s"}' "$(printf '%s' "$PROMPT" | sed 's/"/\\"/g')"
  } > "$PAYLOAD"
  rm -f "$SMALL"
else
  printf '{"model":"%s","stream":false,"prompt":"%s"}' "$MODEL" \
    "$(printf '%s' "$PROMPT" | sed 's/"/\\"/g')" > "$PAYLOAD"
fi

curl -s --max-time 180 "$URL/api/generate" --data-binary "@$PAYLOAD" \
  | sed 's/.*"response":"//; s/","done.*//' \
  | sed 's/\\n/\n/g'
rm -f "$PAYLOAD"
