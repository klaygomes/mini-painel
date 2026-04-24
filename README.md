# mini-painel: your stack's vital signs, always in view

[![CI](https://github.com/klaygomes/mini-painel/actions/workflows/ci.yml/badge.svg)](https://github.com/klaygomes/mini-painel/actions/workflows/ci.yml)

A cheap, tiny panel on your desk. Builds, incidents, on-call, metrics, live and at a glance, no tab-switching required.

---

mini-painel drives a 3.5-inch screen you plug into a Mac over USB. Once running, the screen shows build status, alerts, who is on call, sprint progress, or error rates. It updates in the background without any interaction.

No browser tab. No second monitor. A small panel on your desk that shows what is happening right now.

---

## What you can show on it

The library ships with components for what engineering teams watch:

- **Build & deploy:** last deploy time, environment, current CI status
- **Alerts & outages:** active incidents, severity, owner
- **Team:** who is on call, sprint health, PR review queue
- **Metrics:** sparklines, error rates, SLA gauges
- **Custom:** any image or graphic your code can draw

The screen cycles through pages automatically. You can fit more than one screen's worth of information.

---

## The hardware

Any **XuanFang 3.5" USB display** (Rev B or Flagship) works. Plug it in, run the program, and the display lights up. The Flagship model adds a backplate RGB LED the library can control.

---

## For developers

- [Getting started](doc/getting-started.md): build, run, and wire up your first component
- [Dashboard API](doc/dashboard.md): rows, components, pages, live data
- [Device API](doc/api.md): open, send bitmaps, control brightness and LED
- [Protocol & layout reference](doc/agents.md): hardware specs, pixel math, layout constants
