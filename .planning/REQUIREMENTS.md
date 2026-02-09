# cels-cli Requirements

**Version:** 0.1.0
**Date:** 2026-02-08
**Design basis:** `../../.planning/design/dependency-system.md` (FINAL)

---

## Architecture Principle

The CLI is a **thin TypeScript wrapper** around CMake-native `CelsDeps.cmake`. All dependency logic (transitive resolution, system package discovery, version pinning) lives in `cels_require()` inside CELS core. The CLI writes text lines to CMakeLists.txt — it does NOT implement a dependency resolver, manifest parser, or CMake AST manipulator.

---

## Functional Requirements

### FR-01: Project Scaffolding (`cels init`)

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-01.1 | `cels init <name>` creates a project directory with CMakeLists.txt, src/main.c, .gitignore | P0 |
| FR-01.2 | Generated CMakeLists.txt fetches CELS core via FetchContent, includes CelsDeps | P0 |
| FR-01.3 | Generated main.c contains a working CEL_Build with one CEL_Composition | P0 |
| FR-01.4 | `cels init <name> && cd <name> && cmake -B build && cmake --build build` succeeds | P0 |
| FR-01.5 | `--backend tui` flag adds `cels_require(ncurses)` to CMakeLists.txt | P1 |
| FR-01.6 | Generated .gitignore excludes build/, .cache/, compile_commands.json | P0 |

### FR-02: Build Integration (`cels build`)

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-02.1 | `cels build` runs `cmake -B build && cmake --build build` in the current directory | P0 |
| FR-02.2 | Build output streams to terminal in real-time (not buffered) | P0 |
| FR-02.3 | `cels build --release` passes `-DCMAKE_BUILD_TYPE=Release` | P1 |
| FR-02.4 | `cels build --clean` removes build/ directory before building | P1 |
| FR-02.5 | Exit code matches cmake exit code (non-zero on failure) | P0 |

### FR-03: Run Integration (`cels run`)

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-03.1 | `cels run` builds (if needed) then executes the project binary | P0 |
| FR-03.2 | `cels run -- --arg1 --arg2` passes arguments after `--` to the binary | P0 |
| FR-03.3 | Binary name is derived from CMakeLists.txt project name | P0 |
| FR-03.4 | Exit code matches application exit code | P0 |

### FR-04: Module Management (`cels add` / `cels remove`)

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-04.1 | `cels add <name>` appends `cels_require(<name>)` line to CMakeLists.txt | P0 |
| FR-04.2 | `cels add --backend tui` maps to `cels_require(ncurses)` | P1 |
| FR-04.3 | `cels remove <name>` removes the `cels_require(<name>)` line | P0 |
| FR-04.4 | Duplicate `cels_require()` calls are prevented | P0 |
| FR-04.5 | `cels list` parses CMakeLists.txt and lists all `cels_require()` calls | P0 |

### FR-05: Local Development (`cels link` / `cels unlink`)

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-05.1 | `cels link <name> <path>` inserts `cels_local(<name> "<path>")` before the require line | P1 |
| FR-05.2 | `cels unlink <name>` removes the `cels_local(<name> ...)` line | P1 |

### FR-06: Maintenance Commands

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-06.1 | `cels update` fetches latest CelsDeps.cmake from CELS core repo | P2 |
| FR-06.2 | `cels doctor` checks: cmake >= 3.21, C compiler, git, pkg-config | P2 |
| FR-06.3 | `cels doctor` shows green checkmarks or red X with fix instructions | P2 |

### FR-07: CLI Hygiene

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-07.1 | `cels --help` lists all commands with one-line descriptions | P0 |
| FR-07.2 | `cels <command> --help` shows command-specific help | P0 |
| FR-07.3 | `cels --version` prints the CLI version | P0 |
| FR-07.4 | Colored terminal output (errors red, success green) | P0 |
| FR-07.5 | Actionable error messages with suggested fixes | P1 |

---

## Non-Functional Requirements

| ID | Requirement | Priority |
|----|-------------|----------|
| NF-01 | Installable via `npx cels` (zero pre-install) | P0 |
| NF-02 | Runtime: Node.js >= 20 | P0 |
| NF-03 | Language: TypeScript, ESM-only | P0 |
| NF-04 | CLI framework: Pastel 4.0.0 (Ink + Commander + Zod) | P0 |
| NF-05 | Bundle with tsup for single-file npx distribution | P0 |
| NF-06 | Linux first, macOS desirable | P0 |
| NF-07 | All commands work headless (no TUI required for any operation) | P0 |

---

## Out of Scope (v0.1)

- TUI interactive dashboard (Phase 5 stretch goal)
- Module authoring tools
- Cross-compilation targets
- Plugin/extension system
- Lock files (CMake GIT_TAG pins are sufficient)
- Central package registry server
- cels.json manifest file (CMakeLists.txt IS the manifest)
- VS Code config generation
- Debug integration with cels-debug
- Watch mode / file change detection

---

## Dependency on CELS Core

| CLI Feature | CELS Core Dependency | Can Start Now? |
|-------------|---------------------|----------------|
| `cels init` | None (scaffolds FetchContent for CELS) | Yes |
| `cels build` / `cels run` | None (wraps cmake) | Yes |
| `cels add` / `cels remove` | CelsDeps.cmake must exist (Phase 7) | CLI can write the text lines now; they execute when CelsDeps.cmake ships |
| `cels add --backend tui` | CELS_BackendDesc (Phase 7) | CLI can write `cels_require(ncurses)` now |
| `cels link` / `cels unlink` | CelsDeps.cmake `cels_local()` | Same as add/remove |
| `cels update` | CelsDeps.cmake in a released CELS version | Needs a CELS release with CelsDeps.cmake |

---

**Supersedes:** DEPENDENCY-PHASES.md (which assumed JSON manifests and a custom resolver — both eliminated by the CelsDeps.cmake design).
