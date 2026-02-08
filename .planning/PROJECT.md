# cels-cli

## What This Is

An interactive TUI-based developer toolkit for the CELS framework, installable via `npx cels-cli`. It provides project scaffolding, module/package management, build orchestration, run/debug integration, and target management — all through a full ncurses-style interactive dashboard rendered in the terminal. It's the single entry point for developers working with CELS.

## Core Value

A developer can go from zero to a running CELS project with the right modules, build config, and debug tooling in one interactive session — no manual CMake wrangling.

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] Developer can install and launch via `npx cels-cli`
- [ ] Interactive TUI dashboard with menus, navigation, module browser
- [ ] `cels init` scaffolds a CELS project (CMakeLists.txt, main.c, .vscode/ config)
- [ ] Module registry: maps module names to GitHub repos
- [ ] `cels add <module>` adds a CELS module (source or release) via FetchContent
- [ ] `cels remove <module>` removes a module and cleans up CMakeLists.txt
- [ ] Module browsing: interactive UI to discover and select available modules
- [ ] CELS core pulled via CMake FetchContent at build time
- [ ] Build integration: trigger cmake configure + build from CLI
- [ ] Run integration: run the built app from CLI
- [ ] Debug integration: auto-install cels-debug if missing, launch debug sessions
- [ ] VS Code debug config generation (launch.json, tasks.json)
- [ ] Target management: build for different targets
- [ ] Project status overview in dashboard

### Out of Scope

- Native ncurses binary for TUI — using Node.js TUI library instead (simplicity, cross-platform)
- Central package registry server — using GitHub repos + manifest for now
- VS Code extension — generating .vscode/ config files is sufficient for v1
- Mobile/embedded target cross-compilation — desktop targets only for v1
- Module authoring tools — this is for consumers, not module creators

## Context

- CELS is a declarative ECS framework (C99 public API, C++17 internals)
- CELS uses CMake with FetchContent for dependencies (flecs, yyjson)
- Existing modules: cels-ncurses (TUI backend), cels-clay (UI layout), widgets
- cels-debug is a separate TUI debugger tool in the CELS repo at `tools/cels-debug/`
- CELS v0.1 is shipped and stable on `main` branch
- CELS v0.2 ("Descriptive Cels") is active development on `v2` branch
- This tool lives at `tools/cels-cli/` inside the CELS repo
- Developers' projects use CMake to build — the CLI must generate correct CMakeLists.txt with FetchContent blocks

## Constraints

- **Runtime**: Node.js (TypeScript) — must work via npx without pre-install
- **TUI library**: Node-based (ink, blessed, or similar) — not native ncurses
- **Build system**: Must generate CMake/FetchContent configs — that's what CELS uses
- **Module source**: GitHub repos — each module is a git repository
- **IDE support**: VS Code first (.vscode/ config generation)
- **Platform**: Linux first (developer's primary platform), macOS support desirable

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Node.js + npx distribution | Simplest cross-platform install, no binary distribution | — Pending |
| Node TUI library over native ncurses | Avoids binary compilation/distribution complexity | — Pending |
| GitHub repos for modules | Simplest registry — no server infrastructure needed | — Pending |
| CMake FetchContent for CELS core | Consistent with existing CELS build system | — Pending |
| VS Code as first IDE target | Developer's primary environment | — Pending |
| Source vs release choice per module | Gives developers flexibility without complexity | — Pending |

---
*Last updated: 2026-02-08 after initialization*
