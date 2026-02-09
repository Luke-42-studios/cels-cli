# Dependency Management Phases

New phases for the module dependency system. These insert **before** the existing Phase 2 (Module Management) from SUMMARY.md, because the dependency system design must be finalized before module add/remove is implemented.

**Related design doc:** `../../.planning/design/dependency-system.md`

---

## Phase 2A: Module Manifest and Registry (insert before Phase 2)

**Objective:** Define the module manifest format, create the static registry, and implement manifest validation.

**Delivers:**
- `cels-module.json` schema (JSON Schema file, Zod validator)
- `registry.json` with entries for all known modules (cels-ncurses, cels-clay, cels-widgets, cels-sdl3)
- Registry loader in `core/registry.ts` -- reads bundled JSON, validates with Zod
- Manifest fetcher -- given a git repo URL and tag, fetches `cels-module.json` from the repo
- `cels.json` schema (Zod) -- project config with `modules`, `backend`, `cels_version` fields

**Rationale:** The registry and manifest format are prerequisites for all dependency operations. Getting the data model right first prevents rework in later phases.

**Plans:**
1. Define `cels-module.json` JSON Schema and Zod validator
2. Create `registry.json` with all known modules
3. Implement `core/registry.ts` (load, validate, query by name/tag/backend)
4. Implement manifest fetcher (git archive or raw file fetch from GitHub)
5. Define `cels.json` Zod schema and implement `core/config.ts` (read/write)

**Addresses pitfalls:** P-09 (manifest fragility -- Zod validation), P-16 (registry security -- commit SHA pinning)

---

## Phase 2B: Dependency Resolution Engine (insert before Phase 2)

**Objective:** Implement the dependency resolver -- the algorithm that takes a module name and produces a flat, ordered install list.

**Delivers:**
- `core/resolver.ts` -- BFS dependency resolution with version constraint checking
- Topological sort for CMake output ordering (deps before dependents)
- Backend renderer selection (clay's ncurses renderer if backend is tui)
- Conflict detection (version conflicts, backend conflicts, duplicate features)
- Orphan detection for `cels remove` (find modules no longer needed)

**Rationale:** The resolver is pure logic with no I/O (given manifests and config, produce an install plan). It should be implemented and tested in isolation before wiring it to CMake generation or CLI commands.

**Plans:**
1. Implement BFS resolver with transitive dependency expansion
2. Implement renderer selection based on active backend
3. Implement conflict detection (version, backend, feature)
4. Implement topological sort for CMake module ordering
5. Implement orphan detection (modules only installed as transitive deps)
6. Unit tests: linear chain, diamond dependency, missing module, version conflict, renderer selection, orphan detection

**Test cases:**
- `add widgets` with no backend -> warn about missing backend
- `add widgets` with tui backend -> resolves clay + ncurses renderer
- `add widgets` when clay is already installed -> skip clay, add widgets only
- `remove clay` when widgets depends on it -> refuse
- `remove widgets` -> offer to remove orphaned clay
- `add modA` requires `cels-clay >=0.3.0`, `add modB` requires `cels-clay <0.3.0` -> conflict

---

## Phase 2C: CMake Section-Marker Generator (insert before Phase 2)

**Objective:** Implement the CMake file generator that produces correct, parseable CMakeLists.txt content from the resolved dependency graph.

**Delivers:**
- `core/cmake-generator.ts` -- generates FetchContent blocks, link libraries, section markers
- Section parser -- reads existing CMakeLists.txt, identifies managed regions, preserves user content
- Module-order-aware generation (topological sort from 2B drives output order)
- Template for initial CMakeLists.txt (used by `cels init`)

**Rationale:** The CMake generator is the most critical component. A bug here produces build failures for every user. It must be built and tested before any command uses it.

**Plans:**
1. Define marker format: `# === CELS-CLI MANAGED: <Section> ===` / `# === END CELS-CLI MANAGED: <Section> ===`
2. Implement section parser (find markers, extract managed content, preserve everything else)
3. Implement FetchContent block generator (one block per module, ordered by dependency)
4. Implement link libraries generator (target_link_libraries with all modules)
5. Implement initial CMakeLists.txt template (for `cels init`, includes all marker sections)
6. Integration tests: parse -> modify -> render round-trip preserves user content

**Error cases to handle:**
- Missing start marker (file was edited, markers deleted)
- Missing end marker (same)
- Empty managed section (no modules installed)
- User content between managed sections (preserved)
- Module with custom CMake needs (find_package, compile_definitions)

---

## Updated Phase Order

The full phase order for cels-cli becomes:

1. **Phase 1: Project Foundation and Scaffolding** (unchanged)
2. **Phase 2A: Module Manifest and Registry** (new)
3. **Phase 2B: Dependency Resolution Engine** (new)
4. **Phase 2C: CMake Section-Marker Generator** (new)
5. **Phase 2: Module Management Commands** (existing, now depends on 2A-2C)
   - `cels add`, `cels remove`, `cels list`, `cels deps`
6. **Phase 3: Build and Run Integration** (unchanged)
7. **Phase 4: TUI Dashboard** (unchanged)
8. **Phase 5: Polish, Testing, and Release** (unchanged)

Phases 2A, 2B, 2C are the infrastructure. Phase 2 wires them to CLI commands. This separation means the resolver and generator can be fully unit-tested before any CLI integration.
