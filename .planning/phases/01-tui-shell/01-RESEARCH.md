# Phase 1: TUI Shell -- Research

**Date:** 2026-02-09
**Goal:** Create a CELS ncurses TUI app with tab navigation for the CLI tool.

---

## Architecture Decision: CELS App vs Raw ncurses

cels-debug is a **raw ncurses app** with manual event loop, window management, and tab dispatch. The CLI tool takes a different approach: it is a **CELS framework app** using CEL_Build, CEL_Root, CEL_Composition, CEL_Backend, and cels-ncurses as the TUI backend. This means:

- Framework provides `main()` via `src/backends/main.c` (linked into `cels` library)
- Entry point is `CEL_Build(App) { CEL_Backend(TUI_Engine, ...) }`
- Root composition receives `TUI_EngineContext` with window state ID
- UI is declared using compositions, state, and reactive recomposition
- Input handled by CEL_System declarations, not raw `getch()`
- Tab switching uses CEL_State + CEL_Observer + CEL_Lifecycle pattern from app.c

This is intentionally different from cels-debug -- the CLI is a **showcase of the CELS developer experience**.

---

## Reference Architecture: app.c Pattern

```
CEL_State(MenuState, { ScreenType screen; int selected; })
CEL_Observer(MainVisible, MenuState) { return MenuState.screen == SCREEN_MAIN; }
CEL_Lifecycle(MainLC, .systemVisibility = &MainVisible);

CEL_Root(AppUI, TUI_EngineContext) {
    TUI_WindowState_t* win = CEL_WatchId(ctx.windowState, TUI_WindowState_t);
    CEL_Init(MenuRouter) {}
}

CEL_Build(App) {
    CEL_Backend(TUI_Engine, .title = "CELS CLI", .fps = 30, .root = AppUI);
}
```

Key patterns from app.c:
1. `CEL_State` for global app state (tab index, selections)
2. `CEL_Observer` for lifecycle visibility predicates
3. `CEL_Lifecycle` binds entity visibility to observers
4. `CEL_Root` with `TUI_EngineContext` for window state access
5. `CEL_System` with `.with = { CEL_Require(Canvas) }` for input handling
6. `CEL_Backend(TUI_Engine, ...)` replaces manual engine init

---

## TUI_Engine API (cels-ncurses)

```c
typedef struct TUI_EngineConfig {
    const char* title;       // Window title (default: "CELS App")
    const char* version;     // App version string
    int fps;                 // Target FPS (default: 60)
    void (*root)(TUI_EngineContext ctx);  // Root composition init function
} TUI_EngineConfig;

typedef struct TUI_EngineContext {
    cels_entity_t windowState;  // ID for CEL_WatchId to observe window changes
} TUI_EngineContext;
```

Calling `CEL_Backend(TUI_Engine, .title = "CELS CLI", .fps = 30, .root = AppUI)` expands to `TUI_Engine_use((TUI_EngineConfig){ .title = "CELS CLI", .fps = 30, .root = AppUI })`.

---

## Build Setup: In-Repo Tool

cels-cli lives at `tools/cels-cli/` inside the CELS repo. Unlike an external project (which would use FetchContent), it uses `add_subdirectory` to reference the parent cels and cels-ncurses targets.

The root CMakeLists.txt already has the `CELS_BUILD_TOOLS` option pattern:
```cmake
option(CELS_BUILD_TOOLS "Build CELS development tools (cels-debug)" OFF)
if(CELS_BUILD_TOOLS)
    add_subdirectory(tools/cels-debug)
endif()
```

The CLI will be added alongside this under a new option or extend the existing one.

CMake structure for cels-cli:
```cmake
add_executable(cels-cli src/main.c)
target_link_libraries(cels-cli PRIVATE cels cels-ncurses flecs::flecs_static)
target_include_directories(cels-cli PRIVATE ${CMAKE_SOURCE_DIR}/examples)
set_target_properties(cels-cli PROPERTIES C_STANDARD 99)
```

No `components.h` include path needed initially since the CLI defines its own components inline. The `examples/` include provides access to the shared component definitions if needed (Button, Slider, Canvas, etc.), but for Phase 1 we use only raw Canvas text rendering.

---

## Tab Content Strategy

Phase 1 is **placeholder-only**. Each tab renders a centered title and a brief description. No interactive content until Phase 2+.

| Tab | Phase 1 Content | Future (Phase 2+) |
|-----|-----------------|-------------------|
| Projects | "No projects found. Press Enter to create one." | Interactive project scaffolding wizard |
| Packages | "No packages installed." | Module browser from CelsDeps registry |
| Build | "No project loaded." | cmake configure + build with streaming output |
| Tests | "No test results found." | Test runner with pass/fail display |

---

## Input Design

Tab switching uses number keys (1-4) and Tab key, matching cels-debug behavior.
Global keys: q to quit, 1-4 for direct tab selection, Tab for next tab.
Arrow keys reserved for future per-tab navigation.

Input is handled via `CEL_System` with `CEL_Require(Canvas)` to get access to the `CELS_Input` struct, following the app.c pattern.

---

## File Layout

```
tools/cels-cli/
  CMakeLists.txt
  src/
    main.c          # CEL_Build, CEL_Root, CEL_State, tab compositions, input system
```

Phase 1 is deliberately minimal -- a single source file containing everything. As the app grows in Phase 2+, compositions will be split into separate files.
