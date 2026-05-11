#!/usr/bin/env bash
# PreToolUse hook: fires before every Claude Code tool call.
# Reads hook context JSON from stdin, writes a non-blocking event to the FIFO.
# If the monitor is not running the write silently fails — Claude Code is never stalled.

TMUX_CTX=""
if [ -n "$TMUX" ]; then
    TMUX_CTX=$(tmux display-message -p '#S:#W' 2>/dev/null || true)
fi
export TMUX_CTX

python3 - <<'EOF'
import sys, json, os, time

try:
    ctx = json.loads(sys.stdin.read())
except Exception:
    raise SystemExit(0)

tool    = ctx.get("tool_name", "unknown")
sess    = ctx.get("session_id", "unknown")
inp     = ctx.get("tool_input", {})
summary = str(next(iter(inp.values()), "") if isinstance(inp, dict) else inp)[:60]
tmux    = os.environ.get("TMUX_CTX", "")[:31]

ev = json.dumps({
    "event":      "pre_tool",
    "session_id": sess,
    "tool":       tool,
    "input":      summary,
    "tmux":       tmux,
    "ts":         int(time.time()),
}) + "\n"

try:
    fd = os.open("/tmp/claude-panel.fifo", os.O_WRONLY | os.O_NONBLOCK)
    os.write(fd, ev.encode())
    os.close(fd)
except OSError:
    pass
EOF
