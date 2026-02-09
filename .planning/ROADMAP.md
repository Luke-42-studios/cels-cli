# cels-cli Roadmap

**Version:** 0.1.0
**Date:** 2026-02-09
**Architecture:** CELS ncurses TUI app, distributed via npm (prebuilt binaries)

---

## Architecture Decision

The CLI tool is a **CELS app built with cels-ncurses** — eating our own dog food. It's an interactive TUI for managing CELS projects, packages, builds, and tests. Distributed via npx with platform-specific prebuilt binaries (like esbuild/turbo).

**Why CELS + ncurses:**
- Proves the framework works for real tools
- Flagship demo of the developer experience
- Consistent with cels-debug (also ncurses TUI)
- No Node.js/TypeScript/Python runtime dependency for end users

**Distribution:** `npx cels-cli` downloads a prebuilt binary for the user's platform. Supports Linux and macOS (Windows via WSL until SDL3 backend ships).

**Depends on:** CELS core Phase 7 (Backend Abstraction Interface) — needs `CEL_Backend` macro, `CelsDeps.cmake`, and `CELS_BACKEND` CMake flag.

---

## Phase Overview

| Phase | Name | Plans | Depends On | Status |
|-------|------|-------|------------|--------|
| 1 | TUI Shell | TBD | CELS Phase 7 | Blocked |
| 2 | Project Scaffolding | TBD | Phase 1 | Pending |
| 3 | Module Management | TBD | Phase 2 | Pending |
| 4 | Build & Run | TBD | Phase 2 | Pending |
| 5 | npm Distribution | TBD | Phase 4 | Pending |

---

## Phase 1: TUI Shell

**Objective:** Create the CLI as a CELS ncurses app with tab-based navigation, similar to cels-debug. The shell provides the framework for all subsequent features.

**Delivers:**
- CELS app with `CEL_Backend(.renderer = CEL_NCURSES, ...)`
- Tab bar: Projects | Packages | Build | Tests
- Keyboard navigation (arrow keys, tab, enter, q to quit)
- Status bar showing version and current project path
- CMakeLists.txt linking cels + cels-ncurses

**Exit criteria:** `./cels-cli` launches a tabbed TUI that responds to keyboard input and quits cleanly.

**Blocked by:** CELS core Phase 7 completion.

---

## Phase 2: Project Scaffolding

**Objective:** The "Projects" tab lets developers create new CELS projects interactively.

**Delivers:**
- `cels init <name>` CLI arg creates project non-interactively
- TUI wizard: enter project name, select backend (tui/sdl3/headless)
- Generates: CMakeLists.txt (with FetchContent + `include(CelsDeps)` + `cels_require()`), src/main.c (CEL_Build template), components.h (example CEL_Atom), .gitignore
- Generated project compiles and runs immediately

**Exit criteria:** Create a project via TUI, `cd <name> && cmake -B build && cmake --build build && ./build/<name>` succeeds.

---

## Phase 3: Module Management

**Objective:** The "Packages" tab shows available modules and lets developers add/remove them.

**Delivers:**
- Module browser listing official registry from CelsDeps.cmake
- Add module: writes `cels_require(<name>)` to project CMakeLists.txt
- Remove module: removes the line (warns about dependents)
- Link/unlink: `cels_local()` for local development
- Show installed modules with dependency tree

**Exit criteria:** Add `widgets` via TUI, see it in the installed list with `clay` as transitive dependency.

---

## Phase 4: Build & Run

**Objective:** The "Build" and "Tests" tabs provide build orchestration and test running.

**Delivers:**
- Build tab: cmake configure + build with streaming output
- Run button: execute the built binary
- Tests tab: discover and run test targets, show pass/fail
- Error highlighting in build output
- `cels build` and `cels run` CLI args for non-interactive use

**Exit criteria:** Build a project from the TUI, see output, run the binary, run tests.

---

## Phase 5: npm Distribution

**Objective:** Distribute the CLI via npx with prebuilt binaries.

**Delivers:**
- Cross-compile for Linux x86_64, Linux aarch64, macOS x86_64, macOS aarch64
- Platform-specific npm packages (`@cels-cli/linux-x64`, etc.)
- Wrapper `cels-cli` npm package that downloads correct binary
- `npx cels-cli` works on fresh machines (with cmake + gcc installed)
- CI/CD pipeline for building and publishing

**Exit criteria:** `npx cels-cli` on a fresh Linux machine downloads the binary and launches the TUI.

---

## Timeline Dependencies

```
CELS Core Phase 5.1 (Perf Fix)
    |
    v
CELS Core Phase 6 (CEL_Remember)
    |
    v
CELS Core Phase 7 (Backend Abstraction)  <-- MUST complete first
    |
    v
CLI Phase 1 (TUI Shell)
    |
    v
CLI Phase 2 (Project Scaffolding)
    |
    +---> CLI Phase 3 (Module Management)
    |
    +---> CLI Phase 4 (Build & Run)
              |
              v
         CLI Phase 5 (npm Distribution)
```

Detailed plans will be created after CELS core Phase 7 ships, when the full API surface is stable.

---
*Created: 2026-02-09*
*Supersedes: 2026-02-08 TypeScript/Pastel roadmap (scrapped)*
