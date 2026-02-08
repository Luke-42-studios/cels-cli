# Project Research Summary

**Project:** cels-cli
**Domain:** CLI Developer Toolkit (TUI-based)
**Researched:** 2026-02-08
**Confidence:** HIGH

## Executive Summary

cels-cli is an interactive TUI developer toolkit for the CELS framework, distributed via `npx`. The best framework CLIs (cargo, flutter, vue-cli) share a common pattern: a small set of core commands (`init`, `add`, `build`, `run`) that cover 95% of daily workflow, backed by a project manifest file as the single source of truth. cels-cli should follow this pattern exactly, using Pastel (file-based CLI routing on top of Ink/React) for command dispatch and an interactive TUI dashboard as its identity differentiator. The stack is mature and well-proven: TypeScript + Ink 6 + React 19 + Pastel 4 + Zod 4, bundled into a single file via tsup for fast npx distribution. All versions have been verified against the npm registry.

The recommended architecture cleanly separates CLI commands, TUI screens, and core business logic into three layers. Core logic (module management, CMake generation, config handling, process spawning) has zero TUI dependency, enabling both headless CLI usage and interactive TUI to share the same code paths. The CMake generator -- which produces FetchContent blocks and manages section-marker-based editing of CMakeLists.txt -- is the most critical and complex component, as every other feature depends on correct CMake output. The module registry is a static JSON manifest bundled with the CLI (no server infrastructure), with each module entry specifying its GitHub repo, CMake integration method, and version pins.

The primary risks are: (1) npx cold start time -- bundling the entire app into a single file via tsup is mandatory, not optional, to keep startup under 1 second; (2) terminal state corruption on crash -- Ink's raw mode must be cleaned up via explicit signal handlers on every exit path; (3) TUI freezing during cmake builds -- all child processes must use async `spawn()` with streaming output; and (4) CMakeLists.txt corruption from regex-based editing -- section-marker parsing is the only safe approach. A `cels doctor` command (modeled on `flutter doctor`) should be implemented early to catch cmake/compiler/git prerequisites before they become build failures.

## Key Findings

### Recommended Stack

The entire stack lives in the Sindre Sorhus/Ink ecosystem, which is ESM-only and actively maintained. Pastel 4 provides file-based command routing with Zod schema validation -- each CLI command is a file that exports a Zod schema and a React component. This eliminates manual command wiring and gives automatic help generation for free. The Ink component library (@inkjs/ui 2.0) provides Select, Spinner, ProgressBar, TextInput, and other widgets needed for the TUI. All dependencies are bundled into a single ESM file by tsup for npx distribution.

**Core technologies:**
- **Pastel 4 + Ink 6 + React 19:** CLI framework + TUI rendering -- file-based routing, declarative terminal UI, flexbox layout via Yoga
- **TypeScript 5.9 + Zod 4:** Language + validation -- type-safe options parsing, config file validation, module manifest validation
- **tsup 8 (esbuild):** Bundler -- single-file output for fast npx cold start (<1s target)
- **@inkjs/ui 2.0 + ink-tab + ink-table:** TUI components -- interactive widgets, tab navigation, data display
- **fs-extra + execa + simple-git:** I/O layer -- file operations, cmake process spawning, git operations
- **zustand 5:** State management -- cross-component TUI state (navigation, build status, module selection)
- **conf 15:** Persistent config -- XDG-compliant user preferences with schema validation
- **ejs 4:** Template engine -- scaffolding code generation (CMakeLists.txt, main.c, .vscode configs)
- **vitest 4 + ink-testing-library 4:** Testing -- component rendering tests, integration tests

**Critical constraint:** ESM-only (`"type": "module"`). The entire Ink ecosystem is ESM. Do not fight this.

### Expected Features

**Must have (table stakes) -- v1.0:**
- `cels init <name>` -- project scaffolding that builds on first try
- `cels.json` manifest -- single source of truth for project config
- `cels add <module>` / `cels remove <module>` -- FetchContent management (primary value prop)
- `cels build` / `cels run` -- cmake configure + build + execute in one command
- `cels doctor` -- prerequisite checking (cmake, compiler, git, pkg-config)
- `cels list` -- show installed modules
- `--help` / `--version` / colored output / actionable error messages
- Static JSON module registry -- maps names to GitHub repos
- `.vscode/` config generation -- eliminates IDE setup pain

**Should have (differentiators) -- v1.0:**
- Interactive TUI dashboard -- cels-cli's identity. No other C/C++ tool does this
- Module browser with descriptions -- interactive catalog in the terminal
- Project status overview -- at-a-glance health in the dashboard

**Defer (v1.x):**
- `cels debug` integration -- depends on cels-debug maturity
- Template selection (`--template menu/game/minimal`) -- ship minimal first
- Watch mode (`cels watch`) -- high complexity, validate demand first
- `cels generate component/composition` -- nice DX, not blocking
- `cels update` -- needs module versioning to be established

**Defer (v2+):**
- Module authoring tools, performance dashboard, cross-compilation, plugin system

**Anti-features (deliberately not building):**
- Central package registry server (use GitHub repos + static manifest)
- Semver dependency solver (overkill for <10 modules)
- Lock file (CMake FetchContent GIT_TAG already pins exact versions)
- Interactive init wizard (flags > prompts; TUI for exploration)

### Architecture Approach

The architecture follows a three-layer model: CLI commands (thin dispatchers), TUI screens (Ink/React components), and core business logic (pure functions with no TUI dependency). Every TUI action has a CLI command equivalent -- the TUI is a convenience layer, not a requirement. The TUI uses a screen stack (push/pop navigation) rather than tabs, because the workflow is hierarchical (Home -> Modules -> Module Detail), not parallel. All child processes (cmake, git) use async `spawn()` with event-based streaming so the TUI remains responsive during long operations.

**Major components:**
1. **CLI Router (Pastel)** -- file-based command dispatch, Zod option parsing, auto-generated help
2. **Scaffolding Engine** -- EJS template rendering, project skeleton generation, cels.json creation
3. **Module Manager** -- registry lookup, CMakeLists.txt modification, cels.json sync, version resolution
4. **CMake Generator** -- section-marker-based CMakeLists.txt parsing and manipulation (most critical component)
5. **Build Orchestrator** -- async cmake spawning, streaming output capture, exit code handling
6. **Process Manager** -- child process lifecycle, stdout/stderr streaming, cancellation support
7. **TUI Dashboard** -- screen router, Ink component tree, zustand state, keyboard navigation
8. **Module Registry** -- static JSON manifest bundled with CLI, maps names to GitHub repos + CMake config
9. **Project Config** -- cels.json read/write, Zod schema validation

### Critical Pitfalls

1. **npx cold start kills first impressions (P-01)** -- Bundle everything into a single file via tsup. Measure: `time npx cels-cli --version` must complete in <1s. Lazy-load the TUI (only import Ink/React when entering the dashboard, not for `cels init`).

2. **TUI freezes during cmake builds (P-02)** -- Never use `execSync` or `exec`. Always use `spawn()` with pipe mode and event-based line streaming. Throttle React state updates to 50-100ms intervals to prevent render thrashing.

3. **Terminal corruption on crash (P-03)** -- Register cleanup handlers for `SIGINT`, `SIGTERM`, `uncaughtException`, and `process.on('exit')`. Always restore terminal state (disable raw mode, show cursor, exit alternate screen) before process exit.

4. **FetchContent downloads hang silently (P-05)** -- Set `FETCHCONTENT_QUIET OFF` in generated CMakeLists.txt. Separate cmake configure from build in the TUI. Show spinner with "Downloading dependencies..." during configure step.

5. **CMakeLists.txt corruption from editing (P-06 + Anti-Pattern 2)** -- Use section-marker-based parsing, never regex. Generated sections use `# === CELS-CLI MANAGED: ... === #` markers. Content outside markers is untouched.

## Implications for Roadmap

Based on combined research, the suggested phase structure follows the architecture's build order and the feature dependency chain: `init` -> `add` -> `build` -> `run` -> TUI.

### Phase 1: Project Foundation and Scaffolding
**Rationale:** Everything depends on the project structure, bundling strategy, and `cels init`. The bundling decision (tsup single-file) must happen at project creation -- retrofitting is painful. `cels init` is the user's first interaction with the framework.
**Delivers:** Working `npx cels-cli init <name>` that produces a compilable CELS project. Package.json with correct `files` field and tsup config. Terminal cleanup handlers. cels.json manifest schema.
**Addresses:** Project scaffolding (table stakes), manifest file, .vscode generation, `--help`/`--version`
**Avoids:** P-01 (npx cold start), P-03 (terminal corruption), P-04 (cmake not found), P-08 (template drift via CI test), P-12 (path handling), P-14 (package size), P-17 (path exposure)
**Stack:** Pastel, TypeScript, tsup, ejs, fs-extra, Zod, conf

### Phase 2: Module Management
**Rationale:** `cels add` is the primary value proposition -- managing FetchContent manually is painful. Depends on cels.json from Phase 1 and the CMake generator component.
**Delivers:** `cels add <module>`, `cels remove <module>`, `cels list`. Static module registry (registry.json). CMake section-marker parser and generator.
**Addresses:** Module add/remove (table stakes), module registry, CMakeLists.txt manipulation, version pinning
**Avoids:** P-06 (version conflicts -- single CELS FetchContent at project top), P-09 (manifest fragility -- schema validation), P-11 (git failures -- HTTPS default, error translation, GIT_TERMINAL_PROMPT=0), P-16 (registry security -- pin commit SHAs), P-21 (network feedback -- spinners)
**Stack:** simple-git, execa, Zod (manifest validation)

### Phase 3: Build and Run Integration
**Rationale:** Completes the core workflow loop: init -> add -> build -> run. The build orchestrator is the most complex integration point (Node.js spawning cmake). Must get child process management right.
**Delivers:** `cels build`, `cels run`, `cels doctor`. Streaming build output. Build cancellation. Prerequisite checking.
**Addresses:** Build command (table stakes), run command (table stakes), doctor (differentiator), argument pass-through
**Avoids:** P-02 (process blocking -- async spawn only), P-05 (FetchContent progress -- FETCHCONTENT_QUIET OFF + spinner), P-07 (render perf -- throttled updates), P-10 (cmake output -- exit code for success/fail, passthrough for display), P-20 (Ctrl+C -- Escape to cancel build, Ctrl+C to exit)
**Stack:** execa, Ink (for TUI output streaming in later phase)

### Phase 4: TUI Dashboard
**Rationale:** The TUI is cels-cli's identity differentiator, but it is an overlay on top of working CLI commands. Every TUI action calls the same core logic from Phases 1-3. Building the TUI last ensures the core is solid.
**Delivers:** Interactive dashboard with Home screen, Module Browser, Build screen, Project Status. Screen stack navigation. Keyboard shortcuts. Status bar.
**Addresses:** Interactive TUI dashboard (differentiator), module browser (differentiator), project status (differentiator)
**Avoids:** P-07 (rendering performance -- ring buffer for build output, throttled state updates, React.memo), P-13 (event loop blocking -- async I/O only), P-15 (memory leaks -- bounded buffers, useEffect cleanup), P-18 (color themes -- semantic colors, NO_COLOR support), P-19 (resize -- flexbox layout, minimum size check), P-20 (Ctrl+C -- state machine for cancel vs exit)
**Stack:** Ink, React, @inkjs/ui, ink-tab, ink-table, ink-big-text, ink-gradient, ink-divider, zustand

### Phase 5: Polish, Testing, and Release
**Rationale:** Integration testing, bundle optimization, cross-platform validation, and CI pipeline. Template compilation test in CI prevents drift.
**Delivers:** CI pipeline that runs `cels init` + build. Bundle size optimization (<2MB target). Cross-platform path testing. Vitest test suite for core logic and Ink components. npm publish config.
**Addresses:** Template drift CI (P-08), package size budget (P-14), cross-platform validation (P-12)
**Avoids:** P-01 (final cold start optimization), P-08 (template drift -- CI test), P-14 (bundle size -- `npm pack --dry-run` check)
**Stack:** vitest, ink-testing-library, GitHub Actions

### Phase Ordering Rationale

- **Foundation first:** The bundling strategy (tsup single-file), ESM configuration, terminal cleanup, and cels.json schema are architectural decisions that cannot be changed later. Phase 1 locks these in.
- **Module management before build:** `cels add` is more complex than `cels build` (CMake generation, registry, version resolution) and is a dependency for meaningful build testing (a project with modules exercises more cmake paths than an empty project).
- **Build before TUI:** The build orchestrator's async process management pattern must be proven before the TUI wraps it. A broken build stream is harder to debug through a TUI layer.
- **TUI last:** The TUI calls into core logic. If core logic changes during TUI development, every screen must be updated. Building the TUI on stable core prevents churn.
- **Pitfall-driven ordering:** The most critical pitfalls (P-01 cold start, P-03 terminal corruption) are addressed in Phase 1. Build-related pitfalls (P-02, P-05, P-07) are addressed in Phase 3 before the TUI amplifies them.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 2 (Module Management):** CMake section-marker parsing needs a concrete implementation design. The exact marker format, parser behavior on malformed input, and handling of user edits between markers need specification. Also: git submodule vs FetchContent strategy per module needs a clear decision matrix.
- **Phase 4 (TUI Dashboard):** Ink 6 rendering performance with streaming output needs prototyping. The throttle interval (50ms? 100ms?) and ring buffer size need empirical tuning. Screen stack navigation pattern needs validation against Ink's component lifecycle.

Phases with standard patterns (skip research-phase):
- **Phase 1 (Foundation):** Pastel project setup, tsup bundling, EJS templating, and cels.json schema are all well-documented patterns with existing examples in the Ink ecosystem.
- **Phase 3 (Build/Run):** Node.js child process spawning with streaming is a solved problem. `cels doctor` follows flutter doctor's well-documented pattern.
- **Phase 5 (Polish):** Standard CI/CD, testing, and npm publishing workflows.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | All versions verified against npm registry on 2026-02-08. Ink 6, Pastel 4, React 19 are current and actively maintained. |
| Features | HIGH | Feature landscape derived from 8 mature CLI tools (cargo, npm, flutter, vue-cli, platformio, conan, vcpkg, cmake-init). Core feature set is stable across all. |
| Architecture | MEDIUM-HIGH | Architecture patterns verified against CELS codebase (CMake conventions, module patterns). Ink-specific patterns (screen router, process streaming) based on training data, not live verification. |
| Pitfalls | HIGH | 21 pitfalls identified. Most map directly to documented cels-debug pitfalls or well-known Node.js CLI patterns. CMake FetchContent pitfalls verified against CMake documentation. |

**Overall confidence:** HIGH

### Gaps to Address

- **Ink 6 + Pastel 4 integration testing:** The stack researcher verified versions exist on npm but could not run an integration test. A "hello world" Pastel app should be spiked in Phase 1 to validate the framework combination before committing to the full architecture.
- **@inkjs/ui 2.0 component coverage:** Last published 2024-05-22. Components listed (Select, Spinner, ProgressBar) are documented but should be verified as working with Ink 6 / React 19. If gaps exist, custom Ink components are straightforward to build.
- **CMake section-marker parser robustness:** No existing library for this. The parser must handle: empty sections, missing end markers (recovery mode), user content between markers (preserve), and nested cmake syntax within sections. This needs design-level planning in Phase 2.
- **Ink rendering performance ceiling:** The throttle-and-ring-buffer approach for build streaming is theoretically sound but has not been benchmarked. If Ink cannot sustain 20fps with a 200-line scrollable buffer, a fallback approach (direct ANSI writes for the build panel, bypassing React) should be prepared.
- **Cross-platform cmake path resolution:** The CLI will initially target Linux. macOS and Windows/WSL support should be validated but need not block v1.0 launch.

## Sources

### Primary (HIGH confidence)
- npm registry (live queries 2026-02-08) -- all package versions, publication dates, peer dependencies
- CELS `CMakeLists.txt` -- FetchContent patterns, section markers, module integration conventions
- cels-ncurses and cels-clay `CMakeLists.txt` -- INTERFACE library pattern, submodule vs FetchContent methods
- cels-debug source code -- TUI architecture patterns (tab system, screen/component separation, state management)
- cels-debug PITFALLS.md -- terminal corruption, resize, memory management (direct analogs)
- CELS v0.2 PITFALLS.md -- FetchContent version conflicts, template drift

### Secondary (MEDIUM confidence)
- cargo, npm, flutter, vue-cli, platformio feature analysis -- stable CLI feature sets from training data
- Ink framework architecture -- React rendering model for terminals (training data)
- Pastel framework -- file-based CLI routing (training data, npm registry confirmed v4.0.0)
- Node.js child_process, signal handling -- well-established APIs

### Tertiary (needs validation)
- @inkjs/ui 2.0 component inventory -- last published 2024-05-22, needs runtime verification with Ink 6
- Ink 6 rendering performance characteristics -- throttle thresholds need empirical testing
- tsup bundle size for full Ink+React+Yoga stack -- estimated 2-5MB, needs measurement

---
*Research completed: 2026-02-08*
*Ready for roadmap: yes*
