#!/usr/bin/env python3
"""Claude Code state monitor for the mini-painel dashboard.

Reads session events from a FIFO written by Claude Code hooks and emits
JSON API ops to stdout.  Pipe stdout to the panel daemon:

    python3 playground/claude/monitor.py | websocat -t ws://127.0.0.1:8765
"""

from __future__ import annotations

import json
import os
import select
import signal
import stat
import sys
import time
from dataclasses import dataclass, field
from typing import Any

FIFO_PATH = "/tmp/claude-panel.fifo"
HEARTBEAT_INTERVAL = 5.0    # seconds between elapsed-time refreshes
STALE_TIMEOUT = 300.0       # seconds of inactivity before a session row is removed

STATE_RUNNING = "running"
STATE_WAITING = "waiting"

# RGB565-safe colours from the project palette
COLOR_RUNNING_BG = "#2464B4"
COLOR_RUNNING_FG = "#FFFFFF"
COLOR_WAITING_BG = "#B88800"
COLOR_WAITING_FG = "#FFFFFF"


def _slug(session_id: str) -> str:
    return session_id[:8].replace("-", "")


def _elapsed(since: float) -> str:
    secs = int(time.monotonic() - since)
    if secs < 60:
        return f"{secs}s"
    return f"{secs // 60}m{secs % 60}s"


@dataclass
class SessionState:
    state: str = STATE_WAITING
    tool: str = "—"
    input_summary: str = "—"
    tmux: str = ""
    since: float = field(default_factory=time.monotonic)
    last_seen: float = field(default_factory=time.monotonic)
    rows_created: bool = False

    def apply(self, ev: dict[str, Any]) -> None:
        self.last_seen = time.monotonic()
        kind = ev.get("event", "")
        self.tmux = ev.get("tmux", self.tmux)
        if kind == "pre_tool":
            self.state = STATE_RUNNING
            self.tool = ev.get("tool", "unknown")[:15]
            self.input_summary = ev.get("input", "")[:63]
            self.since = time.monotonic()
        elif kind == "stop":
            self.state = STATE_WAITING
            self.tool = "—"
            self.since = time.monotonic()

    def ops(self, slug: str) -> list[dict[str, Any]]:
        op = "row.update" if self.rows_created else "row.add"
        elapsed = _elapsed(self.since)
        context = self.tmux if self.tmux else "no tmux"

        if self.state == STATE_RUNNING:
            bg, fg, deploy_status, label = COLOR_RUNNING_BG, COLOR_RUNNING_FG, 0, "running"
        else:
            bg, fg, deploy_status, label = COLOR_WAITING_BG, COLOR_WAITING_FG, 1, "waiting"

        return [
            {
                "op": op,
                "id": f"component.build_status.claude.{slug}",
                "data": {
                    "branch":       self.state.capitalize(),
                    "build_id":     self.tool,
                    "duration":     elapsed,
                    "status":       label,
                    "status_color": bg,
                    "status_fg":    fg,
                },
            },
            {
                "op": op,
                "id": f"component.deploy.claude.{slug}",
                "data": {
                    "branch":   context[:31],
                    "time_ago": elapsed,
                    "label":    self.input_summary,
                    "status":   deploy_status,
                },
            },
            {"op": "page.render", "page": 0},
        ]

    def remove_ops(self, slug: str) -> list[dict[str, Any]]:
        return [
            {"op": "row.remove", "id": f"component.build_status.claude.{slug}"},
            {"op": "row.remove", "id": f"component.deploy.claude.{slug}"},
            {"op": "page.render", "page": 0},
        ]


def _emit(ops: list[dict[str, Any]]) -> None:
    line = json.dumps(ops, separators=(",", ":"))
    sys.stdout.write(line + "\n")
    sys.stdout.flush()


def _ensure_fifo(path: str) -> None:
    if os.path.exists(path):
        if not stat.S_ISFIFO(os.stat(path).st_mode):
            raise RuntimeError(f"{path} exists but is not a FIFO")
        return
    os.mkfifo(path, 0o600)


def _open_fifo(path: str) -> int:
    return os.open(path, os.O_RDONLY | os.O_NONBLOCK)


def main() -> int:
    _ensure_fifo(FIFO_PATH)
    fifo_fd = _open_fifo(FIFO_PATH)
    buf = b""
    sessions: dict[str, SessionState] = {}
    stop = False

    def _handle_signal(sig: int, frame: Any) -> None:
        nonlocal stop
        stop = True

    signal.signal(signal.SIGINT, _handle_signal)
    signal.signal(signal.SIGTERM, _handle_signal)

    last_heartbeat = time.monotonic()

    while not stop:
        timeout = max(0.0, HEARTBEAT_INTERVAL - (time.monotonic() - last_heartbeat))
        try:
            readable, _, _ = select.select([fifo_fd], [], [], min(timeout, 1.0))
        except (ValueError, OSError):
            break

        if readable:
            try:
                chunk = os.read(fifo_fd, 4096)
            except BlockingIOError:
                chunk = b""

            if not chunk:
                os.close(fifo_fd)
                fifo_fd = _open_fifo(FIFO_PATH)
            else:
                buf += chunk
                while b"\n" in buf:
                    line, buf = buf.split(b"\n", 1)
                    line = line.strip()
                    if not line:
                        continue
                    try:
                        ev = json.loads(line)
                    except json.JSONDecodeError:
                        continue

                    raw_sess = ev.get("session_id", "unknown")
                    slug = _slug(raw_sess)
                    if slug not in sessions:
                        sessions[slug] = SessionState()

                    sess = sessions[slug]
                    sess.apply(ev)
                    ops = sess.ops(slug)
                    sess.rows_created = True
                    _emit(ops)

        now = time.monotonic()
        if now - last_heartbeat >= HEARTBEAT_INTERVAL:
            last_heartbeat = now
            stale = [s for s, st in sessions.items()
                     if now - st.last_seen > STALE_TIMEOUT]
            for slug in stale:
                sess = sessions.pop(slug)
                if sess.rows_created:
                    _emit(sess.remove_ops(slug))

            for slug, sess in sessions.items():
                if sess.rows_created:
                    _emit(sess.ops(slug))

    os.close(fifo_fd)
    print("Claude panel monitor stopped", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
