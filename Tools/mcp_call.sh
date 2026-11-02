#!/usr/bin/env bash
# Talk to the Unreal editor's own MCP server (ModelContextProtocol plugin).
# Usage: bash Tools/mcp_call.sh <method> '<params-json>'
#   bash Tools/mcp_call.sh tools/list '{}'
#   bash Tools/mcp_call.sh tools/call '{"name":"list_toolsets","arguments":{}}'
# Override the port with MCP_PORT=... ; the editor takes -ModelContextProtocolPort=N.
set -u
PORT="${MCP_PORT:-8765}"
BASE="http://127.0.0.1:${PORT}/mcp"
METHOD="${1:-tools/list}"
PARAMS="${2:-{\}}"

HDR="$(mktemp)"
curl -s --max-time 20 -D "$HDR" \
  -H "Content-Type: application/json" \
  -H "Accept: application/json, text/event-stream" \
  -X POST "$BASE" \
  -d '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{},"clientInfo":{"name":"buffy","version":"1.0"}}}' >/dev/null

SID="$(grep -i 'mcp-session-id' "$HDR" | tr -d '\r' | awk '{print $2}')"

# Streamable-HTTP servers want the initialized notification before real calls.
curl -s --max-time 20 \
  -H "Content-Type: application/json" \
  -H "Accept: application/json, text/event-stream" \
  -H "Mcp-Session-Id: ${SID}" \
  -X POST "$BASE" \
  -d '{"jsonrpc":"2.0","method":"notifications/initialized"}' >/dev/null 2>&1

curl -s --max-time 120 \
  -H "Content-Type: application/json" \
  -H "Accept: application/json, text/event-stream" \
  -H "Mcp-Session-Id: ${SID}" \
  -X POST "$BASE" \
  -d "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"${METHOD}\",\"params\":${PARAMS}}"

rm -f "$HDR"
