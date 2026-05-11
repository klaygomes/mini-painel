#!/usr/bin/env bash
# Stop hook: fires when Claude Code stops and waits for user input.
# Reads hook context JSON from stdin, writes a non-blocking event to the FIFO.

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

sess = ctx.get("session_id", "unknown")
tmux = os.environ.get("TMUX_CTX", "")[:31]

ev = json.dumps({
    "event":      "stop",
    "session_id": sess,
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
