# Architecture Research

**Domain:** CLI Developer Toolkit (TUI-based)
**Researched:** 2026-02-08
**Confidence:** MEDIUM (web tools unavailable; based on training data + direct codebase analysis of CELS ecosystem, cels-debug architecture, and existing module patterns)

## Standard Architecture

### System Overview

```
+-----------------------------------------------------------------------+
|                          cels-cli (npx entry)                         |
+-----------------------------------------------------------------------+
|                                                                       |
|  CLI Router (command parsing)                                         |
|    |                                                                  |
|    +-- "cels init"      --> Scaffolding Engine                        |
|    +-- "cels add <mod>" --> Module Manager                            |
|    +-- "cels remove"    --> Module Manager                            |
|    +-- "cels build"     --> Build Orchestrator                        |
|    +-- "cels run"       --> Run/Debug Launcher                        |
|    +-- "cels" (no args) --> TUI Dashboard                             |
|                                                                       |
+-----------------------------------------------------------------------+
|                                                                       |
|  +------------------+   +------------------+   +------------------+   |
|  | TUI Dashboard    |   | Scaffolding      |   | Build            |   |
|  |                  |   | Engine           |   | Orchestrator     |   |
|  | - Screen Router  |   |                  |   |                  |   |
|  | - Menu System    |   | - Template       |   | - cmake spawn    |   |
|  | - Status Bar     |   |   Renderer       |   | - output capture |   |
|  | - Output Panel   |   | - File Writer    |   | - error parse    |   |
|  +--------+---------+   +--------+---------+   +--------+---------+   |
|           |                      |                      |             |
|  +--------+---------+   +--------+---------+   +--------+---------+   |
|  | Module Manager   |   | CMake Generator  |   | Process Manager  |   |
|  |                  |   |                  |   |                  |   |
|  | - Registry fetch |   | - FetchContent   |   | - child_process  |   |
|  | - Version resolve|   |   block gen      |   | - stdout/stderr  |   |
|  | - Dependency     |   | - target_link    |   |   streaming      |   |
|  |   resolution     |   |   generation     |   | - exit code      |   |
|  +--------+---------+   +--------+---------+   +--------+---------+   |
|           |                      |                      |             |
+-----------------------------------------------------------------------+
|                                                                       |
|  +------------------+   +------------------+   +------------------+   |
|  | Module Registry  |   | Project Config   |   | Template Store   |   |
|  | (registry.json)  |   | (cels.json)      |   | (embedded)       |   |
|  +------------------+   +------------------+   +------------------+   |
|                                                                       |
+-----------------------------------------------------------------------+
```

### Component Responsibilities

| Component | Responsibility | Inputs | Outputs |
|-----------|---------------|--------|---------|
| **CLI Router** | Parse argv, dispatch to command handler or launch TUI | `process.argv` | Command context object |
| **TUI Dashboard** | Interactive screen management, input handling, render loop | User keystrokes, app state | Terminal output (ANSI/Ink) |
| **Scaffolding Engine** | Generate project skeleton from templates | Project name, options | Files on disk (CMakeLists.txt, main.c, etc.) |
| **Module Manager** | Add/remove CELS modules, resolve versions | Module name, registry data | Modified CMakeLists.txt, updated cels.json |
| **Build Orchestrator** | Spawn cmake configure + build, capture output | Project path, build config | Streamed build output, exit status |
| **Run/Debug Launcher** | Execute built binary, optionally with cels-debug | Build output path, debug flag | Running process, debug session |
| **CMake Generator** | Produce correct FetchContent blocks and link targets | Module metadata | CMakeLists.txt content |
| **Module Registry** | Map module names to GitHub repos + versions | Registry file (bundled or remote) | Module metadata objects |
| **Project Config** | Track installed modules, project settings | cels.json on disk | Config object |
| **Process Manager** | Spawn/manage child processes (cmake, built app) | Command + args | stdout/stderr streams, exit code |
| **Template Store** | Embedded file templates for scaffolding | Template name + variables | Rendered file content |

## Recommended Project Structure

```
cels-cli/
  package.json              # name: "cels-cli", bin: { "cels": "./dist/cli.js" }
  tsconfig.json
  src/
    cli.ts                  # Entry point: parse args, dispatch
    commands/               # One file per command
      init.ts               # cels init
      add.ts                # cels add <module>
      remove.ts             # cels remove <module>
      build.ts              # cels build
      run.ts                # cels run
      dashboard.ts          # cels (no args) -> launch TUI
    tui/                    # TUI dashboard (Ink components)
      App.tsx               # Root Ink component, screen router
      screens/              # Full-screen views
        HomeScreen.tsx       # Main menu + project status
        ModuleBrowser.tsx    # Browse/search/add modules
        BuildScreen.tsx      # Build output + status
        ProjectStatus.tsx    # Overview of installed modules, config
      components/            # Reusable UI pieces
        Menu.tsx             # Selectable list
        StatusBar.tsx        # Bottom bar with shortcuts
        Header.tsx           # Top bar with project name + version
        OutputPanel.tsx      # Scrollable text output (build logs)
        Spinner.tsx          # Activity indicator
        ModuleCard.tsx       # Module info display
        ProgressBar.tsx      # Build progress
    core/                   # Business logic (no TUI dependency)
      registry.ts           # Module registry loading + querying
      config.ts             # cels.json read/write
      cmake-generator.ts    # Generate CMakeLists.txt content
      module-manager.ts     # Add/remove/update module logic
      scaffolder.ts         # Project scaffolding logic
      process.ts            # Child process spawning + streaming
      build.ts              # Build orchestration logic
    templates/              # Embedded file templates
      CMakeLists.txt.ejs    # CMakeLists template (or Handlebars/mustache)
      main.c.ejs            # Starter main.c
      vscode/
        launch.json.ejs     # VS Code debug config
        tasks.json.ejs      # VS Code build tasks
        c_cpp_properties.json.ejs
    data/                   # Static data bundled with package
      registry.json         # Module registry manifest
  dist/                     # Compiled output (gitignored)
  tests/
    core/                   # Unit tests for core logic
    commands/               # Integration tests for commands
```

**Rationale for this structure:**

1. **`commands/` separate from `core/`**: Commands are thin -- they parse CLI args, call core logic, and format output. Core logic is testable without CLI concerns.

2. **`tui/` with screens + components**: Mirrors the cels-debug tab system pattern (screens = tabs, components = reusable widgets). Screens are full layouts; components are composable pieces.

3. **`core/` has no TUI dependency**: The module manager, cmake generator, config reader, and process spawner are pure logic. Both CLI commands and TUI screens call into core. This means `cels add <module>` (headless) and the TUI module browser share the same `module-manager.ts`.

4. **Templates embedded, not fetched**: Templates ship with the npm package. No network calls for scaffolding. Versioned with the CLI.

5. **Registry as static JSON initially**: Ships bundled in `data/registry.json`. Can later evolve to fetch from GitHub releases or a registry API without changing the `registry.ts` interface.

## Architectural Patterns

### Pattern 1: Command/TUI Duality

Every feature works both as a CLI command AND through the TUI dashboard. The core logic is shared.

```
CLI Command Path:        TUI Dashboard Path:
$ cels add cels-ncurses  [User selects module in browser]
       |                          |
       v                          v
  commands/add.ts            ModuleBrowser.tsx
       |                          |
       +----> core/module-manager.ts <----+
                     |
                     v
              core/cmake-generator.ts
                     |
                     v
              Write CMakeLists.txt
```

**Why:** Users who prefer typing get commands. Users who want discovery get the TUI. Shared core prevents divergence.

### Pattern 2: Screen Router (TUI Navigation)

The TUI uses a screen stack, not a tab bar. This is appropriate because the CLI workflow is hierarchical (Home -> Module Browser -> Module Detail), not parallel (like cels-debug's tabs which show different views of the same data).

```typescript
// Screen router state
type Screen =
  | { type: 'home' }
  | { type: 'modules' }
  | { type: 'module-detail'; module: ModuleInfo }
  | { type: 'build'; output: string[] }
  | { type: 'project-status' }

// Navigation: push/pop screen stack
const [screenStack, setScreenStack] = useState<Screen[]>([{ type: 'home' }]);

function navigate(screen: Screen) {
  setScreenStack(prev => [...prev, screen]);
}

function goBack() {
  setScreenStack(prev => prev.length > 1 ? prev.slice(0, -1) : prev);
}
```

**Why:** Unlike cels-debug (which monitors a live system and needs parallel tabs), the CLI workflow is task-oriented. You go somewhere, do something, come back. A stack matches this model.

### Pattern 3: Process Streaming to TUI

Build output must stream in real-time to the TUI output panel. Use Node.js child_process with event-based streaming, not buffered exec.

```typescript
// core/process.ts
import { spawn } from 'child_process';
import { EventEmitter } from 'events';

interface ProcessEvents {
  stdout: (line: string) => void;
  stderr: (line: string) => void;
  exit: (code: number) => void;
}

export function spawnProcess(
  cmd: string,
  args: string[],
  cwd: string
): EventEmitter & { kill: () => void } {
  const emitter = new EventEmitter();
  const proc = spawn(cmd, args, { cwd, stdio: 'pipe' });

  proc.stdout.on('data', (chunk) => {
    chunk.toString().split('\n').filter(Boolean).forEach(line => {
      emitter.emit('stdout', line);
    });
  });

  proc.stderr.on('data', (chunk) => {
    chunk.toString().split('\n').filter(Boolean).forEach(line => {
      emitter.emit('stderr', line);
    });
  });

  proc.on('close', (code) => emitter.emit('exit', code ?? 1));

  return Object.assign(emitter, { kill: () => proc.kill() });
}
```

**Why:** cmake output can be large and slow. Buffering it all then displaying creates a bad UX. Streaming line-by-line to the TUI gives real-time feedback.

### Pattern 4: Registry as Data, Not Code

The module registry is a static JSON manifest, not a service. Modules self-describe through a standard schema.

```json
// data/registry.json
{
  "version": 1,
  "modules": {
    "cels-ncurses": {
      "description": "ncurses-based TUI providers (Window, Input, Renderer)",
      "repo": "https://github.com/Luke-42-studios/cels-ncurses.git",
      "tags": ["tui", "rendering", "provider"],
      "requires": [],
      "cmake": {
        "type": "interface",
        "target": "cels-ncurses",
        "system_deps": ["Curses"],
        "fetch_method": "submodule"
      },
      "versions": {
        "latest": "main",
        "stable": "v0.1.0"
      }
    },
    "cels-clay": {
      "description": "Clay layout engine integration",
      "repo": "https://github.com/Luke-42-studios/cels-clay.git",
      "tags": ["layout", "ui"],
      "requires": [],
      "cmake": {
        "type": "interface",
        "target": "cels-clay",
        "alias": "cels::clay",
        "fetch_method": "fetchcontent"
      },
      "versions": {
        "latest": "main",
        "stable": "v0.1.0"
      }
    }
  }
}
```

**Why:** No server infrastructure needed. The registry ships with the CLI and can be updated independently. The `cmake` field tells the generator exactly how to integrate each module, because different modules use different CMake patterns (submodule vs FetchContent, INTERFACE vs STATIC, system dependencies vs none).

### Pattern 5: CMake Generation as AST Manipulation

Do NOT use string concatenation or regex to modify CMakeLists.txt. Parse it into sections, manipulate sections, then render back.

```typescript
// core/cmake-generator.ts

interface CMakeSection {
  type: 'dependencies' | 'library' | 'modules' | 'executable' | 'other';
  content: string;
  marker?: string; // e.g., "# ============ Dependencies ============"
}

// Parse CMakeLists.txt into sections using marker comments
function parseCMakeSections(content: string): CMakeSection[];

// Add a FetchContent block to the dependencies section
function addFetchContent(sections: CMakeSection[], module: ModuleConfig): CMakeSection[];

// Add target_link_libraries entry
function addLinkLibrary(sections: CMakeSection[], target: string, library: string): CMakeSection[];

// Render sections back to string
function renderCMake(sections: CMakeSection[]): string;
```

**Why:** CELS CMakeLists.txt files use clear section markers (visible in the existing `CMakeLists.txt` -- `# ============ Dependencies ============`, `# ============ Provider Modules ============`). Parsing by section markers is robust and preserves user customizations outside the managed sections.

### Pattern 6: Project Config as Source of Truth

`cels.json` in the project root tracks everything the CLI manages. It is the equivalent of `package.json` for Node or `Cargo.toml` for Rust.

```json
// cels.json (in user's CELS project)
{
  "name": "my-cels-app",
  "version": "0.1.0",
  "cels": {
    "version": "v0.2.0",
    "branch": "v2",
    "repo": "https://github.com/Luke-42-studios/cels.git"
  },
  "modules": {
    "cels-ncurses": {
      "version": "v0.1.0",
      "method": "submodule"
    },
    "cels-clay": {
      "version": "v0.1.0",
      "method": "fetchcontent"
    }
  },
  "build": {
    "type": "Debug",
    "options": {
      "CELS_DEBUG": true
    },
    "targets": ["app"]
  }
}
```

**Why:** The CLI needs to know what modules are installed, what CELS version is targeted, and what build options are set. Scanning CMakeLists.txt for this information is fragile. `cels.json` is the canonical source; CMakeLists.txt is generated/updated to match.

## Data Flow

### Flow 1: Project Initialization (`cels init`)

```
User input (project name, options)
    |
    v
commands/init.ts
    |
    +---> core/scaffolder.ts
    |         |
    |         +---> templates/CMakeLists.txt.ejs  --> render --> write to disk
    |         +---> templates/main.c.ejs          --> render --> write to disk
    |         +---> templates/vscode/*.ejs        --> render --> write to .vscode/
    |         +---> core/config.ts                --> write cels.json
    |
    v
Project directory created with:
    my-project/
      CMakeLists.txt
      src/main.c
      cels.json
      .vscode/
        launch.json
        tasks.json
        c_cpp_properties.json
```

### Flow 2: Module Addition (`cels add cels-ncurses`)

```
Module name
    |
    v
commands/add.ts (or TUI ModuleBrowser)
    |
    +---> core/registry.ts        --> lookup module metadata
    |         |
    |         v
    |     Module metadata object
    |
    +---> core/module-manager.ts
    |         |
    |         +---> core/config.ts          --> read cels.json
    |         +---> core/cmake-generator.ts --> modify CMakeLists.txt
    |         |         |
    |         |         +---> Parse sections
    |         |         +---> Insert FetchContent / add_subdirectory
    |         |         +---> Insert target_link_libraries
    |         |         +---> Render back to file
    |         |
    |         +---> core/config.ts          --> update cels.json
    |
    v
CMakeLists.txt updated + cels.json updated
```

### Flow 3: Build (`cels build`)

```
Build trigger (CLI command or TUI button)
    |
    v
commands/build.ts (or TUI BuildScreen)
    |
    +---> core/config.ts        --> read build options from cels.json
    |
    +---> core/build.ts
    |         |
    |         +---> core/process.ts --> spawn: cmake -B build -DCMAKE_BUILD_TYPE=Debug ...
    |         |         |
    |         |     stdout/stderr lines stream
    |         |         |
    |         |         v
    |         |     TUI OutputPanel (or CLI stdout)
    |         |
    |         +---> core/process.ts --> spawn: cmake --build build
    |                   |
    |               stdout/stderr lines stream
    |                   |
    |                   v
    |               TUI OutputPanel (or CLI stdout)
    |
    v
Build result (success/failure + output)
```

### Flow 4: TUI Dashboard Navigation

```
Launch: $ cels (no args)
    |
    v
commands/dashboard.ts
    |
    +---> Ink render(<App />)
              |
              v
          App.tsx (Screen Router)
              |
              +---> HomeScreen
              |         |
              |         +---> [1] Init Project  --navigate--> (runs init flow)
              |         +---> [2] Modules        --navigate--> ModuleBrowser
              |         +---> [3] Build          --navigate--> BuildScreen
              |         +---> [4] Run            --navigate--> (runs app)
              |         +---> [5] Project Status --navigate--> ProjectStatus
              |
              +---> ModuleBrowser
              |         |
              |         +---> Registry query --> module list
              |         +---> Select module  --navigate--> ModuleDetail
              |         +---> [Enter] Add    --> module-manager.ts
              |         +---> [Esc] Back     --> HomeScreen
              |
              +---> BuildScreen
                        |
                        +---> Start build --> build.ts stream
                        +---> OutputPanel shows lines as they arrive
                        +---> [Esc] Back --> HomeScreen
```

## Anti-Patterns

### Anti-Pattern 1: Monolithic CLI Handler

**What:** Single file with a giant switch statement for all commands, TUI mixed with business logic.

**Why bad:** Untestable, hard to extend. Every new command modifies the same file. TUI rendering coupled to logic means you cannot run headless tests.

**Instead:** Commands are thin dispatchers in `commands/`. Core logic in `core/`. TUI components in `tui/`. Each layer depends only downward.

### Anti-Pattern 2: Regex-Based CMakeLists.txt Editing

**What:** Using regex to find/replace content in CMakeLists.txt.

**Why bad:** CMake has complex syntax. Regex breaks on comments, multi-line strings, nested parentheses, user customizations. One wrong match and the build file is corrupted.

**Instead:** Use section-marker-based parsing. The generated CMakeLists.txt has clear `# ============ Section ============` markers (matching the existing CELS convention). Parse between markers, manipulate, render back. Content outside markers is preserved untouched.

### Anti-Pattern 3: TUI as the Only Interface

**What:** Every operation requires launching the TUI dashboard.

**Why bad:** Breaks CI/CD pipelines, scripts, and users who prefer typing commands. Cannot be composed with other tools.

**Instead:** Every operation works as a CLI command (`cels add`, `cels build`, etc.). The TUI is a convenience layer that calls the same core logic. This is the Command/TUI Duality pattern.

### Anti-Pattern 4: Fetching Registry from Network on Every Operation

**What:** Every `cels add` queries a remote API or GitHub for the module list.

**Why bad:** Slow, requires network, fails offline. For an initial set of known modules (cels-ncurses, cels-clay, widgets), this overhead is unnecessary.

**Instead:** Bundle `registry.json` with the CLI package. Add a `cels registry update` command for explicitly refreshing. Later, support a remote registry URL in cels.json for custom registries.

### Anti-Pattern 5: Generating Files Without Markers

**What:** Generating CMakeLists.txt content with no way to distinguish generated vs. user-written sections.

**Why bad:** User adds custom CMake logic. Next `cels add` overwrites the file. User loses work. Alternatively, the CLI refuses to touch the file because it cannot identify its own sections.

**Instead:** All generated sections use clear start/end markers:
```cmake
# === CELS-CLI MANAGED: Dependencies === #
# Do not edit between these markers
FetchContent_Declare(...)
# === END CELS-CLI MANAGED: Dependencies === #
```
The generator only touches content between its own markers. Everything else is the user's domain.

### Anti-Pattern 6: Synchronous Process Execution in TUI

**What:** Using `execSync` or `spawnSync` for cmake/build commands while the TUI is active.

**Why bad:** Blocks the entire Node.js event loop. TUI freezes, no input handling, no spinner, no progress updates. User thinks the tool crashed.

**Instead:** Always use async `spawn` with streaming. TUI remains responsive, can show a spinner/progress, and the user can cancel with Ctrl+C.

## Integration Points

### Integration 1: Ink (TUI Framework)

**What Ink provides:** React-like component model for terminal UIs. JSX components render to ANSI terminal output. Built-in `<Box>`, `<Text>`, hooks like `useInput`, `useApp`.

**How we use it:** Every TUI screen is an Ink component. The screen router is React state. Input handling uses `useInput` hook. Output streaming uses `useState` to accumulate lines.

**Why Ink over alternatives:**
- **blessed:** Abandoned (last release 2017), complex API, memory issues with long-running sessions
- **blessed-contrib:** Same abandoned foundation
- **raw ANSI:** No component model, manual cursor management, error-prone
- **Ink:** Active maintenance (vadimdemedes), React mental model, npm ecosystem, works with npx distribution, TypeScript support

**Confidence:** MEDIUM (based on training data, could not verify current Ink version/status via web)

### Integration 2: CMake (Build System)

**What we generate:** CMakeLists.txt files compatible with CELS's existing patterns:
- `FetchContent_Declare` for remote dependencies (cels core, yyjson, flecs)
- `add_subdirectory` for local module directories
- `target_link_libraries` for linking
- `set_target_properties` for C/C++ standards

**The CELS CMake conventions we must follow** (derived from actual codebase analysis):
- FetchContent with `GIT_REPOSITORY` + `GIT_TAG` for pinned versions
- INTERFACE libraries for modules (cels-ncurses, cels-clay pattern)
- Section markers with `# ============ ... ============`
- C99 standard for main targets, C++17 for CELS internals
- `cmake_minimum_required(VERSION 3.21)`

**Confidence:** HIGH (directly verified from codebase)

### Integration 3: Git (Module Fetching)

Two module integration methods observed in the CELS codebase:

1. **Git submodules** (cels-ncurses pattern): Module lives in `modules/` directory, referenced via `add_subdirectory`. The `.git` file in the module dir confirms submodule usage.

2. **FetchContent** (cels-clay's Clay dependency pattern): Module fetched at cmake configure time. No local clone needed.

The CLI should support both, with the registry specifying which method each module uses.

**Confidence:** HIGH (directly verified from codebase)

### Integration 4: VS Code Configuration

Generated `.vscode/` files must match the CELS build system:
- `launch.json`: C/C++ debug configs with `cmake --build build` as preLaunchTask
- `tasks.json`: cmake configure + build tasks
- `c_cpp_properties.json`: Include paths for CELS headers, module headers, FetchContent build directories

**Confidence:** HIGH (standard VS Code C/C++ workflow)

### Integration 5: cels-debug (Debug Tool)

The CLI can launch `cels-debug` alongside the built app. Requirements:
- Check if cels-debug is built (look for binary in CELS build directory)
- If not built, offer to build it (`-DCELS_BUILD_TOOLS=ON`)
- Launch app and cels-debug in parallel (debug connects to app via REST)

**Confidence:** HIGH (directly verified from cels-debug architecture -- it uses HTTP client to poll a flecs REST endpoint)

## Build Order (Component Dependencies)

Components should be built in this order, because each layer depends on layers below:

```
Phase 1: Foundation (no dependencies)
  +-- Project Config (cels.json read/write)
  +-- Template Store (embedded templates)
  +-- Module Registry (static JSON)
  +-- Process Manager (child_process wrapper)

Phase 2: Core Logic (depends on Phase 1)
  +-- CMake Generator (depends on: Registry, Config)
  +-- Module Manager (depends on: Registry, Config, CMake Generator)
  +-- Scaffolder (depends on: Config, Template Store, CMake Generator)
  +-- Build Orchestrator (depends on: Config, Process Manager)

Phase 3: CLI Commands (depends on Phase 2)
  +-- commands/init.ts (depends on: Scaffolder)
  +-- commands/add.ts (depends on: Module Manager)
  +-- commands/remove.ts (depends on: Module Manager)
  +-- commands/build.ts (depends on: Build Orchestrator)
  +-- commands/run.ts (depends on: Process Manager)

Phase 4: TUI Dashboard (depends on Phase 2 + 3)
  +-- TUI components (Header, Menu, StatusBar, OutputPanel)
  +-- TUI screens (Home, ModuleBrowser, BuildScreen)
  +-- Screen Router (App.tsx)
  +-- commands/dashboard.ts (wires TUI to core)

Phase 5: Polish (depends on everything)
  +-- cels-debug integration
  +-- VS Code config generation
  +-- Target management
```

**Build order rationale:**
- Phase 1 components are leaf nodes with no internal dependencies. They can be built and tested in isolation.
- Phase 2 components compose Phase 1 pieces into business logic. Each has clear inputs/outputs.
- Phase 3 wires core logic to CLI argument parsing. Thin layer, mostly glue code.
- Phase 4 adds the TUI on top. It calls the same core logic as Phase 3 commands, so core must exist first.
- Phase 5 is integration and polish that touches multiple layers.

**Critical dependency:** The CMake Generator is the most complex and most important component. It must correctly produce CMakeLists.txt files that match CELS conventions. It should be built and thoroughly tested early (Phase 2) because the Module Manager, Scaffolder, and TUI all depend on it.

## Scalability Considerations

| Concern | Now (5 modules) | Later (50 modules) | Future (community registry) |
|---------|-----------------|--------------------|-----------------------------|
| Registry | Bundled JSON | Bundled JSON (still small) | Remote JSON endpoint, local cache |
| Module discovery | Scrollable list | Searchable list with categories | Full-text search, tags, popularity |
| CMake generation | Template per module type | Template per module type (still works) | Module-provided cmake snippets |
| Build output | Simple streaming | Filtered streaming (errors highlighted) | Build output parsing, error links |

## Key Decision: Ink vs. Alternatives

| Criterion | Ink | blessed | raw ANSI | prompts/enquirer |
|-----------|-----|---------|----------|-----------------|
| Component model | React-like (JSX) | Widget-based | None | Question-based |
| Maintenance | Active | Abandoned | N/A | Active but limited |
| Full-screen TUI | Yes | Yes | Manual | No (prompts only) |
| TypeScript | Yes | Community types | N/A | Yes |
| npx compatible | Yes | Yes | Yes | Yes |
| Streaming output | useState + effect | Custom scrollbar | Manual | No |
| Learning curve | Low (if React-familiar) | High | Very high | Low |

**Recommendation:** Use Ink. It aligns with the TypeScript/React mental model, is actively maintained, handles full-screen rendering, and works well with npx distribution. The component model (JSX) makes the TUI code readable and composable.

**Confidence:** MEDIUM (based on training data; could not verify Ink's current release status via web)

## Sources

- CELS `CMakeLists.txt` at `/home/cachy/workspaces/libs/cels/CMakeLists.txt` -- verified FetchContent patterns, section markers, module integration (HIGH confidence)
- cels-ncurses `CMakeLists.txt` at `modules/cels-ncurses/CMakeLists.txt` -- verified INTERFACE library pattern, system deps (HIGH confidence)
- cels-clay `CMakeLists.txt` at `modules/cels-clay/CMakeLists.txt` -- verified FetchContent_Populate pattern, alias targets (HIGH confidence)
- cels-debug source at `tools/cels-debug/src/` -- verified tab system architecture (vtable pattern), screen/component separation, app state management (HIGH confidence)
- cels-debug `tab_system.h` / `tab_system.c` -- verified navigation and dispatch patterns (HIGH confidence)
- Ink framework knowledge -- React-like terminal rendering (MEDIUM confidence, from training data)
- oclif framework knowledge -- command architecture patterns (MEDIUM confidence, from training data)
- Node.js `child_process` API -- process spawning, streaming (HIGH confidence, well-established API)

---
*Architecture research for: CLI Developer Toolkit*
*Researched: 2026-02-08*
