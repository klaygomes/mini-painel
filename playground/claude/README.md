# Claude Code Monitor

Displays Claude Code session state on the mini-painel dashboard in near real-time. Multiple Claude Code sessions are supported simultaneously — each gets its own pair of rows on the panel.

## How It Works

Claude Code fires hook scripts (`pre_tool.sh`, `stop.sh`) on key events. Each hook writes a small JSON event to a FIFO (`/tmp/claude-panel.fifo`). `monitor.py` reads that FIFO and emits JSON API ops to stdout, which you pipe to the panel daemon.

```
Claude Code  →  hook scripts  →  FIFO  →  monitor.py  →  websocat  →  panel_daemon
```

Hooks are non-blocking: if the monitor is not running, the write silently drops — Claude Code is never stalled.

## Files

- `monitor.py` — FIFO reader and JSON op emitter
- `hooks/pre_tool.sh` — PreToolUse hook (fires before each tool call)
- `hooks/stop.sh` — Stop hook (fires when Claude waits for user input)

## Requirements

- Python 3 (stdlib only — no extra packages)
- `websocat` to pipe output to the WebSocket daemon
- `tmux` (optional) — if Claude runs in a tmux session, the pane context appears on the panel

## Run

From the repository root, start the monitor and pipe its output to the daemon:

```bash
python3 playground/claude/monitor.py | websocat -t ws://127.0.0.1:8765
```

Line-buffered variant (more reliable on some shells):

```bash
stdbuf -oL python3 playground/claude/monitor.py | websocat -t ws://127.0.0.1:8765
```

The monitor runs until you press Ctrl-C or send SIGTERM. Sessions that go quiet for more than 5 minutes have their rows removed automatically.

## Panel Layout

Each active Claude Code session is displayed as two rows:

| Component | Kind | Shows |
|-----------|------|-------|
| `component.build_status.claude.<slug>` | `build_status` | State (Running / Waiting) · tool name · elapsed time |
| `component.deploy.claude.<slug>` | `deploy` | tmux context · elapsed time · tool input summary |

`<slug>` is the first 8 characters of the Claude session UUID.

State colours:
- **Blue** — tool is running
- **Amber** — waiting for user input

## Configure Claude Code Hooks

Add the following `"hooks"` key to `~/.claude/settings.json` (alongside any existing keys such as `"statusLine"`):

```json
"hooks": {
  "PreToolUse": [
    {
      "matcher": "",
      "hooks": [
        {
          "type": "command",
          "command": "bash /path/to/playground/claude/hooks/pre_tool.sh"
        }
      ]
    }
  ],
  "Stop": [
    {
      "matcher": "",
      "hooks": [
        {
          "type": "command",
          "command": "bash /path/to/playground/claude/hooks/stop.sh"
        }
      ]
    }
  ]
}
```

Replace `/path/to/` with the absolute path to this repository. Restart any open Claude Code sessions for the hooks to take effect.

## Test Without Hardware

Start the daemon in no-device mode (writes a PPM after each render):

```bash
# Terminal 1
./bin/panel_daemon --no-device

# Terminal 2
python3 playground/claude/monitor.py | websocat -t ws://127.0.0.1:8765

# Terminal 3 — inject test events for two sessions
python3 - <<'EOF'
import os, json, time

events = [
    {"event": "pre_tool", "session_id": "aaaa-0001-bbbb", "tool": "Bash",
     "input": "ls -la /tmp", "tmux": "work:editor", "ts": int(time.time())},
    {"event": "pre_tool", "session_id": "cccc-0002-dddd", "tool": "Read",
     "input": "/etc/hosts", "tmux": "work:monitor", "ts": int(time.time())},
    {"event": "stop",     "session_id": "aaaa-0001-bbbb",
     "tmux": "work:editor", "ts": int(time.time())},
]
for ev in events:
    line = (json.dumps(ev) + "\n").encode()
    fd = os.open("/tmp/claude-panel.fifo", os.O_WRONLY | os.O_NONBLOCK)
    os.write(fd, line)
    os.close(fd)
    time.sleep(0.2)
EOF
```

Inspect the rendered frame:

```bash
open bin/panel_daemon_last.ppm     # macOS Preview
# or convert to PNG:
sips -s format png bin/panel_daemon_last.ppm --out /tmp/panel_last.png && open /tmp/panel_last.png
```
