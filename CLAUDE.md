# mini-painel — Claude Rules

## Comments

**No journal/section decorator comments.** Do not write:
- Section dividers: `/* ── section ── */`, `/* === ... === */`
- Page labels: `/* page 1 */`, `/* page 2 */`
- Narrative "what" comments: `/* draw the circle */`, `/* loop over rows */`
- Block labels that describe structure: `/* mocked data */`, `/* render loop */`, `/* device */`

Only write a comment when it explains **why** something non-obvious is done. The code explains what.

## Reference

See `doc/agents.md` for device specs, protocol details, layout constants, and page math.
