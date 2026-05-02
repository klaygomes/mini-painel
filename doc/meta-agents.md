# Meta: rules for editing agent docs

This document governs every other `*agents*.md` file in the repo. Read it before editing `AGENTS.md`, `CLAUDE.md`, or anything in `doc/*-agents.md`. Its purpose is to prevent the regressions a previous validation pass surfaced (stale heights, contradictory device descriptions, drifted file trees, fonts documented in the wrong header).

## Files this governs

- `AGENTS.md` — top-level agent context for the whole repo. **Single source of truth.**
- `CLAUDE.md` — symlink to `AGENTS.md`. Do not edit separately; do not let it diverge. Same applies to any future per-tool pointer (`.cursorrules`, `GEMINI.md`, etc.) — symlink, never copy.
- `doc/agents.md` — device specs, protocol, layout constants, page math.
- `doc/components-agents.md` — component module conventions.
- `doc/dashboard-agents.md` — dashboard module conventions.
- `doc/glossary.md` — naming conventions and terms.
- `doc/daemon-agents.md` — WebSocket daemon conventions and daemon op catalog.
- `doc/meta-agents.md` — this file.

If you create another `*-agents.md`, list it here in the same edit.

## Hard rules

1. **No restated source values.** Never inline a number, file tree, struct field list, or enum that already exists in source. Reference the symbol or file path and tell the reader to read it. Examples of forbidden duplication:
   - Component pixel heights (read `COMP_<NAME>_HEIGHT`).
   - Font sizes / row heights (read `src/theme/theme.h`, `src/draw/layout.h`).
   - File trees (instruct the reader to run `tree <path>`).
   - Theme field lists (read `src/theme/theme.h`).
   - Enum values (read the enum's header).
   If you find yourself copying a number out of source into a doc, stop and link instead.

2. **One fact, one home.** Every fact lives in exactly one agent doc. Cross-reference with a path; do not duplicate. The home for each topic:
   - Device / protocol / page math → `doc/agents.md`.
   - Dashboard internals → `doc/dashboard-agents.md`.
   - Components, draw API, theme rules → `doc/components-agents.md`.
   - Naming + glossary → `doc/glossary.md`.
   - Repo-wide overview → `AGENTS.md`.
   - Claude-only behavioural rules → `CLAUDE.md`.

3. **Source paths must resolve.** Every `src/...` or `tests/...` path mentioned in agent docs must exist at the time of the commit. If you rename or delete a file, grep all `*-agents.md` and `CLAUDE.md` and `AGENTS.md` for the old path before committing.

4. **No version-specific values.** Do not write "v1 used X, v2 uses Y" or "+2pt from v1". The current source is the source of truth; history belongs in `git log`.

5. **No duplicated install instructions.** Cairo install steps live in `AGENTS.md` only. Other docs link to that section.

## Pre-commit checklist for agent-doc edits

Before committing a change to any governed file, verify:

- [ ] No new component height, font size, or row height literal was added. (Heights are read from `COMP_*_HEIGHT`; fonts from `FONT_*`; rows from `LAY_*`.)
- [ ] No new file tree was hand-written. (The doc tells the reader to run `tree`.)
- [ ] Every `src/...` and `tests/...` path mentioned in the diff exists.
- [ ] Every symbol mentioned in the diff (`xf_*`, `comp_*`, `XF_*`, `COMP_*`, `LAY_*`, `FONT_*`) resolves with `grep -rn "<symbol>" src tests`.
- [ ] Top-line description in `AGENTS.md` still matches what the codebase actually does (active protocol, supported platforms, what the demo runs).
- [ ] No fact added here is also stated in another `*-agents.md`. If a topic crosses files, one file owns it and the other links.
- [ ] No emoji, no decorative dividers, no "what" comments leaked in (see `CLAUDE.md`).

## Validation commands

Run from the repo root.

```sh
# 1. List every component height macro the source declares.
grep -hn '^#define COMP_.*_HEIGHT' src/components/comp_*.h

# 2. Show every src/... path mentioned in agent docs (review for stale entries).
grep -hnoE '`src/[A-Za-z0-9_./-]+`' AGENTS.md CLAUDE.md doc/*-agents.md | sort -u

# 3. Check every such path actually exists.
#    The sed strips the doc shorthand "foo.h/.c" -> "foo.h" and drops glob/wildcard
#    paths so only concrete paths are checked.
for p in $(grep -hoE 'src/[A-Za-z0-9_./-]+' AGENTS.md CLAUDE.md doc/*-agents.md \
           | sed -E 's/\.h\/\.c$/.h/; s/\/\*.*$//' \
           | grep -vE '(\*|\.\.\.|/$|_$)' \
           | sort -u); do
    [ -e "$p" ] || echo "MISSING: $p"
done

# 4. Spot stale numeric heights in docs (anything matching "<digits> px" outside a code block is suspect).
grep -nE '\b[0-9]{2,3}[[:space:]]*px\b' AGENTS.md doc/*-agents.md
```

If commands 3 or 4 produce output, fix the doc — do not commit.

## When the codebase changes structurally

Bump the relevant agent doc in the same commit. Common triggers and what to update:

| Change | Update |
|---|---|
| New `LAY_*` or `FONT_*` constant | `doc/agents.md` "Layout Constants" section. Do not list the value — list the constant name and one-line purpose. |
| New component | `doc/components-agents.md` "Adding a new component" stays valid; no per-component listing. |
| New gfx primitive | Same — pattern is documented; no listing. |
| New device revision | `AGENTS.md` "Adding a new device revision" + the protocol family bullet at the top. |
| New daemon op | `doc/daemon-agents.md` daemon op table + dispatch internals section. |
| New theme field | `src/theme/theme.h` only. Do not enumerate fields in agent docs. |
| Renamed file | Run validation command 3 above and fix every match. |
| New `*-agents.md` | Add it to the "Files this governs" list in this file. |

## How to spot a regression in review

Two heuristics catch most drift:

1. **Numbers in prose.** Any digit in an agent doc that is not inside a code block, an example, or a count of items is suspect. Heights, sizes, byte counts, and offsets all belong in source.
2. **Lists that look like a directory.** Any `├──` / `└──` tree is a future stale doc. Replace it with "run `tree <path>`".

## Updating this file

This file is itself a governed doc. To change a rule here, also remove or rewrite any other agent doc that contradicts the new rule in the same commit.
