# GitHub Monitor Script

This folder contains a polling monitor that fetches GitHub PR/build data via `gh` and emits JSON API ops for `pr_review` and `deploy` components.

## Files

- `monitor.py` - main monitor script
- `projects.json` - repository configuration

## Requirements

- Python 3
- GitHub CLI (`gh`)
- Authenticated `gh` session

Quick checks:

```bash
gh --version
gh auth status
```

## Configure Repositories

Edit `projects.json` with your repositories:

```json
{
  "owner/repo": {
    "branch": "main",
    "ignored_jobs": ["optional-docs-check"]
  }
}
```

## Run

From repository root:

```bash
python3 playground/github/monitor.py --config playground/github/projects.json --interval 30
```

## Self-check Mode

Run internal output contract checks without calling GitHub:

```bash
python3 playground/github/monitor.py --self-check
```

## WebSocket Pipe Tip (websocat)

You can stream monitor output directly to a WebSocket endpoint:

```bash
python3 playground/github/monitor.py --config playground/github/projects.json --interval 30 \
| websocat -t ws://127.0.0.1:9000
```

Optional line-buffered variant:

```bash
stdbuf -oL python3 playground/github/monitor.py --config playground/github/projects.json --interval 30 \
| websocat -t ws://127.0.0.1:9000
```

Install websocat on macOS if needed:

```bash
brew install websocat
```
