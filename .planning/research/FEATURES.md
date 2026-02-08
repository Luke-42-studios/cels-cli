# Feature Research: CLI Developer Toolkit

**Domain:** Framework CLI / Interactive Developer Toolkit
**Researched:** 2026-02-08
**Confidence:** HIGH (based on analysis of cargo, npm/npx, flutter CLI, vue-cli, platformio, conan, vcpkg, cmake-init)

---

## Methodology

Features catalogued by studying eight mature CLI developer toolkits across different ecosystems, then mapped to cels-cli's specific constraints (Node.js TUI, CMake/FetchContent build system, GitHub-based module registry, C99/C++17 framework).

| CLI Tool | Ecosystem | Key Strength Studied |
|----------|-----------|---------------------|
| **cargo** | Rust | Gold standard: init, build, run, test, add, publish, workspaces |
| **npm / npx** | Node.js | Package management, scripts, registry, npx zero-install distribution |
| **flutter CLI** | Dart/Flutter | Project creation, device management, multi-target build, doctor |
| **vue-cli** | Vue.js | Interactive scaffolding, plugin system, TUI dashboard (vue ui) |
| **platformio** | Embedded C/C++ | Board management, library registry, build targets, serial monitor |
| **conan** | C/C++ | Package management for native code, profiles, build integration |
| **vcpkg** | C/C++ | Manifest-based dependency management, triplets, CMake integration |
| **cmake-init** | C/C++ | Pure CMake project scaffolding, template generation |

---

## Feature Landscape

### Table Stakes (Users Leave Without These)

These are features that every framework CLI provides. Missing any of these makes cels-cli feel incomplete or broken.

#### 1. Project Scaffolding

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| `cels init <name>` creates working project | Every framework CLI: `cargo new`, `flutter create`, `vue create`, `npm init`. Users' first interaction with the framework | LOW | Generate directory with CMakeLists.txt, main.c, .gitignore. Must compile and run immediately |
| Generated project builds on first try | cargo, flutter, vue all produce immediately-buildable projects. "Hello world must work" | LOW | Integration test: `cels init foo && cd foo && cmake -B build && cmake --build build && ./build/foo` |
| Sensible defaults without flags | `cels init foo` with zero flags must produce a reasonable project. Cargo does this perfectly | LOW | Default: minimal template, source build, no modules. User can customize later |
| Template/example content | Generated main.c should demonstrate the framework's patterns, not be empty | LOW | Show CEL_Build, one composition, one component. Working example, not just boilerplate |

#### 2. Dependency / Module Management

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| `cels add <module>` adds a dependency | `cargo add`, `npm install`, `flutter pub add`, `vcpkg add port`. Core workflow | MEDIUM | Append FetchContent block to CMakeLists.txt, add target_link_libraries |
| `cels remove <module>` removes a dependency | `cargo remove`, `npm uninstall`. Inverse of add | MEDIUM | Remove FetchContent block and link from CMakeLists.txt. Must handle partial matches safely |
| Module name resolution | User types `cels add ncurses`, CLI resolves to `cels-ncurses` GitHub repo | LOW | Registry maps short names to full GitHub URLs. Like npm's registry but flat file |
| Source vs release version choice | Unique to C/CMake: FetchContent can use GIT_TAG (source) or URL (release tarball) | LOW | `cels add ncurses --source` (latest main) vs `cels add ncurses --release v1.0` |
| List installed modules | `cargo tree`, `npm list`, `vcpkg list`. User needs to see what's installed | LOW | Parse CMakeLists.txt FetchContent blocks, display as table |
| Module version pinning | `cargo update`, `npm install pkg@version`. Reproducible builds require pinned versions | LOW | Store GIT_TAG or release version in FetchContent declaration |

#### 3. Build Integration

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| `cels build` triggers cmake configure + build | `cargo build`, `flutter build`, `npm run build`. Single command for full build | LOW | Wraps `cmake -B build && cmake --build build`. Handles first-time configure automatically |
| Build error forwarding | Build errors must be readable, not swallowed. Cargo's colored error output is the bar | LOW | Stream cmake/compiler output to terminal. Don't buffer or reformat |
| Clean build option | `cargo clean`, `flutter clean`. Users need to nuke build artifacts | LOW | `rm -rf build/` essentially. Simple but expected |
| Build configuration (Debug/Release) | `cargo build --release`, `cmake -DCMAKE_BUILD_TYPE=Release`. Standard workflow | LOW | `cels build --release` maps to CMAKE_BUILD_TYPE |

#### 4. Run Integration

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| `cels run` builds and runs | `cargo run`, `flutter run`. Most-used command in any framework CLI | LOW | Build if needed, then execute the binary. Single command from edit to running |
| Pass arguments to target | `cargo run -- --arg`, `flutter run --dart-define`. Users need to pass args to their app | LOW | `cels run -- --my-flag` passes everything after `--` to the target binary |

#### 5. Project Configuration

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Project manifest file | `Cargo.toml`, `package.json`, `pubspec.yaml`. Every framework has one | LOW | `cels.json` or `cels.toml`: project name, version, modules list, build settings |
| Manifest drives behavior | CLI reads manifest for module list, build settings, targets. Single source of truth | MEDIUM | All `cels add/remove/build` operations read/write the manifest |

#### 6. Help and Discoverability

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| `cels --help` with clear command listing | Every CLI tool. Must be scannable in 5 seconds | LOW | Top-level help shows all commands with one-line descriptions |
| `cels <command> --help` per-command help | Standard CLI pattern. Users need flag documentation | LOW | Each subcommand documents its flags and arguments |
| `--version` flag | Standard. Users need to report which version they're running | LOW | `cels --version` prints version |
| Colored terminal output | cargo, flutter, npm all use color for readability. Monochrome feels broken in 2026 | LOW | Errors in red, success in green, warnings in yellow |
| Actionable error messages | cargo's error messages suggest fixes ("did you mean X?"). Flutter's errors include URLs to docs | MEDIUM | When a module isn't found, suggest similar names. When build fails, suggest `cels build --clean` |

---

### Differentiators (Competitive Advantage)

These features separate great framework CLIs from adequate ones. Not expected, but valued highly when present.

#### Tier 1: High Impact, Achievable

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| **Interactive TUI dashboard** | vue-cli had `vue ui` (web-based). cels-cli does this natively in the terminal. Browse modules, see project status, trigger builds -- all from one TUI | HIGH | This IS cels-cli's identity. No other C/C++ tool does this. Full ncurses-style dashboard with menus, module browser, build status |
| **Module browser with descriptions** | Interactive catalog of available CELS modules with descriptions, install status, compatibility info. Like `platformio lib search` but visual | MEDIUM | TUI panel showing all modules from registry. Select to install. Shows README excerpts, version, size |
| **`cels doctor` health check** | `flutter doctor` is universally praised. Check cmake version, compiler, pkg-config, system deps. Diagnose before user hits build errors | LOW | Check: cmake >= 3.21, gcc/clang with C++17, git, pkg-config, ncurses-dev. Green checkmarks or red X with fix instructions |
| **VS Code integration generation** | Generate `.vscode/launch.json`, `tasks.json`, `c_cpp_properties.json`. User opens VS Code and gets intellisense + debug without manual config | LOW | Template files with project-specific paths. `cels init` generates these. `cels vscode` regenerates if CMake config changes |
| **Debug integration** | `cels debug` launches app with cels-debug TUI attached. Auto-installs cels-debug if not present. One command to full debug session | MEDIUM | Detect cels-debug in modules, build with `-DCELS_DEBUG=ON`, launch both processes. Unique to CELS ecosystem |
| **Project status overview** | Dashboard shows: installed modules, build status (last build time, success/fail), CELS version, target info | LOW | Parse CMakeLists.txt + build directory state. Display in TUI dashboard. Quick health-at-a-glance |

#### Tier 2: Medium Impact, Nice to Have

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| **Template selection** | `cels init --template menu` / `--template game` / `--template minimal`. Pre-built starting points beyond hello world | MEDIUM | Multiple template directories. `menu` = full menu system, `game` = game loop with input, `minimal` = bare CEL_Build |
| **Module dependency resolution** | When user adds `cels-clay`, CLI also adds its transitive deps (if any). Like npm's dependency tree but much simpler | MEDIUM | Each module manifest lists its CELS module deps. CLI resolves and adds all. Warn on conflicts |
| **Upgrade/update commands** | `cels update` bumps module versions. `cels update cels` upgrades framework version in FetchContent | LOW | Fetch latest tags from GitHub repos. Update GIT_TAG in CMakeLists.txt. Show changelog summary |
| **Build target management** | `cels target add release-static` configures additional CMake targets. Multiple build configurations in one project | MEDIUM | Manage multiple CMAKE_BUILD_TYPE + linker configs. `cels build --target release-static` |
| **Scaffold components/compositions** | `cels generate component Position` / `cels generate composition MainMenu`. Code generation for framework patterns | LOW | Append struct to components.h, create composition boilerplate. Saves typing the macro pattern |
| **Watch mode (file change detection)** | `cels watch` rebuilds and reruns on file save. Like `cargo watch`, `nodemon`, `flutter run` hot reload | HIGH | File watcher (chokidar or similar in Node.js). On .c/.h change: rebuild + rerun. Very productive for TUI apps |
| **Offline module cache** | Cache downloaded module sources locally so `cels build` works offline after first fetch | LOW | CMake FetchContent already caches in `_deps/`. Document this, maybe add `cels cache clean` |

#### Tier 3: Aspirational, Future

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| **Module authoring tools** | `cels module init` scaffolds a new CELS module with correct structure, CMake config, GitHub Actions CI | MEDIUM | Separate workflow from consumer tools. Future feature for ecosystem growth |
| **Performance dashboard** | TUI panel showing benchmark results, comparing against baseline. Integration with CELS benchmark harness | HIGH | Parse benchmark JSON output, render charts/tables in TUI. Compare against stored baselines |
| **Cross-compilation targets** | `cels build --target wasm` / `--target arm64`. Like `flutter build ios/android/web` | HIGH | Requires CMake toolchain files per target. Platform-specific, complex |
| **Plugin/extension system** | Third-party plugins extend cels-cli with custom commands. Like vue-cli plugins | HIGH | Plugin API, command registration, lifecycle hooks. Adds significant complexity. Premature for v1 |

---

### Anti-Features (Deliberately NOT Building)

Features that are commonly requested or seen in other CLIs but would be harmful for cels-cli.

| Anti-Feature | Why Requested | Why Problematic | Alternative |
|--------------|---------------|-----------------|-------------|
| **Central package registry server** | npm has registry.npmjs.org, cargo has crates.io. "Real" package managers have servers | Server infrastructure, uptime, auth, moderation -- massive operational burden for a small ecosystem. CELS has <10 modules | GitHub repos + flat manifest file (JSON). Registry is a static file in cels-cli repo. Update it with PRs |
| **Dependency version resolution (semver solver)** | npm/cargo solve complex version graphs. "Professional" package managers do this | CELS modules are few (<10) and mostly independent. A SAT solver is overkill. Adds massive complexity for near-zero benefit | Flat module list. Each module pins a CELS version range. Warn if incompatible. User resolves manually |
| **Lock file** | `package-lock.json`, `Cargo.lock`. Reproducible builds | CMake FetchContent with GIT_TAG already pins exact commits. A separate lock file duplicates this and adds sync confusion | Pin GIT_TAG to specific commit SHAs or version tags in CMakeLists.txt. That IS the lock file |
| **Monorepo/workspace management** | Cargo workspaces, npm workspaces. Multi-project management | CELS projects are standalone apps, not multi-crate workspaces. Adding workspace support adds complexity for a use case that doesn't exist | One project per `cels init`. If user wants monorepo, they manage it with CMake directly |
| **Auto-update CLI** | npm's self-update, flutter's upgrade. CLI keeps itself current | npx always runs latest version. Self-update logic is fragile and a security concern | Distribute via npx (always latest) or npm global install (user manages updates) |
| **GUI configuration tool** | vue ui was a web-based GUI for vue-cli. Some tools have Electron GUIs | The TUI IS the GUI. A separate web/Electron GUI fragments the experience and adds a massive maintenance burden | TUI dashboard handles all interactive workflows. Terminal-native developers prefer this |
| **Hot module replacement / live reload** | Flutter's hot reload, webpack HMR. Instant code changes | C/C++ requires recompilation. "Hot reload" for compiled languages means rebuilding, which is what `cels watch` does. True HMR is impossible without a VM | Watch mode with fast rebuild. C compilation is fast enough for small-to-medium CELS projects |
| **Built-in test runner** | `cargo test`, `flutter test`. Framework-level test command | CELS uses utest.h (header-only). Users write and run tests with cmake/ctest. Adding a test runner wraps ctest with no added value | `cels build --test` builds test targets. Let users run `ctest --test-dir build` directly. Document the pattern |
| **Package publishing** | `cargo publish`, `npm publish`. Upload packages to registry | No central registry to publish to. Modules are GitHub repos. Publishing = pushing to GitHub | Document how to create a CELS module. Provide a module template. "Publishing" = creating a GitHub repo and adding it to the registry manifest |
| **Binary distribution of CLI** | Ship cels-cli as a compiled binary (like cargo, flutter) | Node.js + npx provides zero-install distribution. Binary distribution requires CI for every platform, code signing, update mechanisms | npx cels-cli runs without install. npm install -g cels-cli for frequent users. Both leverage Node.js ecosystem for cross-platform |
| **Project-level cels-cli version pinning** | `volta pin`, `.nvmrc` -- pin CLI version per project | Premature. The CLI should be backward-compatible. Version pinning adds complexity for a young tool | Maintain backward compatibility. If breaking changes are needed, use major version bumps with clear migration guides |
| **Interactive prompts during init** | Yeoman-style wizard: "What template? What backend? What version?" | Unix philosophy: flags > prompts. Interactive wizards are slow for experienced users and confusing for beginners who don't know the options yet | Sensible defaults for `cels init`. Flags for customization (`--template`, `--backend`). TUI dashboard for exploration |

---

## Feature Dependencies

```
cels init (scaffolding)
  |
  +-- Manifest file (cels.json) -----> cels add / cels remove (module management)
  |                                      |
  |                                      +-- Module registry (static JSON)
  |                                      |
  |                                      +-- CMakeLists.txt manipulation
  |                                      |
  +-- CMakeLists.txt generation --------+
  |
  +-- .vscode/ generation (optional)
  |
  v
cels build (build integration)
  |
  +-- cmake configure + build
  |
  +-- Build config (Debug/Release)
  |
  v
cels run (run integration)
  |
  +-- Build first (if needed)
  |
  +-- Argument pass-through (--)
  |
  v
cels debug (debug integration)
  |
  +-- Depends on: cels-debug module installed
  |
  +-- Depends on: build with -DCELS_DEBUG=ON
  |
  v
TUI Dashboard (interactive mode)
  |
  +-- Depends on: ALL above commands exist as non-interactive equivalents
  |
  +-- Module browser: depends on module registry
  |
  +-- Project status: depends on manifest + build state
  |
  +-- Build panel: depends on cels build
```

**Critical path:** `cels init` -> `cels add` -> `cels build` -> `cels run`. This is the "zero to running" workflow and must work end-to-end before anything else matters.

**TUI dashboard is an overlay, not a prerequisite.** Every TUI action must have a CLI command equivalent. The TUI provides discoverability and convenience, but the CLI commands provide scriptability and CI compatibility.

---

## MVP Definition

### Launch With (v1.0)

The minimum feature set that makes cels-cli worth using over manual CMake setup.

| Feature | Category | Rationale |
|---------|----------|-----------|
| `cels init <name>` | Table stakes | First touch. Must work flawlessly |
| `cels.json` manifest | Table stakes | Single source of truth for project config |
| `cels add <module>` | Table stakes | Primary value: managing FetchContent is painful manually |
| `cels remove <module>` | Table stakes | Inverse of add. Incomplete without it |
| `cels list` (installed modules) | Table stakes | User needs to see what they have |
| `cels build` | Table stakes | Core workflow |
| `cels run` | Table stakes | Core workflow |
| `cels --help` / `--version` | Table stakes | Basic CLI hygiene |
| Module registry (static JSON) | Table stakes | Maps names to GitHub repos |
| Colored terminal output | Table stakes | Professional feel |
| Actionable error messages | Table stakes | Reduces friction |
| `.vscode/` config generation | Differentiator | High value, low cost. Eliminates VS Code setup pain |
| `cels doctor` | Differentiator | Prevents support issues. Low cost, high trust |
| TUI dashboard (basic) | Differentiator | cels-cli's identity. Even basic version differentiates |

**v1.0 scope:** ~14 features. Enough to go from `npx cels-cli` to a running CELS project with modules, build, and run.

### Add After Validation (v1.x)

Features to add once the core workflow is validated with real users.

| Feature | Rationale for Deferral |
|---------|----------------------|
| `cels debug` integration | Requires cels-debug to be stable. Build on working `cels run` |
| Template selection (`--template`) | Need to see which templates users actually want. Ship `minimal` first |
| Module browser (interactive TUI) | Build on basic TUI. Needs registry to have enough modules to browse |
| `cels generate component/composition` | Nice DX but not blocking. Users can type macros manually |
| `cels update` (upgrade modules) | Needs module versioning to be established first |
| Watch mode | High complexity. Validate that users want TUI-based dev workflow first |
| Project status overview (TUI) | Build on basic TUI dashboard. Needs more state to display |
| Module dependency resolution | Only matters when modules have transitive deps. Most don't yet |

### Future Consideration (v2+)

| Feature | Why Wait |
|---------|----------|
| Module authoring tools | Ecosystem needs consumers first, then creators |
| Performance dashboard | Requires benchmark harness integration, cels-debug maturity |
| Cross-compilation targets | Platform-specific complexity. Desktop-only for now |
| Plugin/extension system | Premature. Need to see what extensions users would write |
| Build target management (multi-target) | Most users have one target. Add when multi-target demand emerges |

---

## Feature Prioritization Matrix

High value + low complexity = build first. Low value + high complexity = build never.

```
                    VALUE
              LOW         HIGH
         +----------+----------+
    LOW  | --version | cels init|
         | --help    | cels add |
  C      | .gitignore| cels run |
  O      | clean     | cels build|
  M      |           | doctor   |
  P  MED | generate  | TUI dash |
  L      | cache     | .vscode/ |
  E      | update    | manifest |
  X      | targets   | module   |
  I      |           | browser  |
  T HIGH | plugin sys| debug    |
  Y      | cross-comp| watch    |
         | perf dash |          |
         +----------+----------+
```

**Build order priority (highest to lowest):**
1. `cels init` + manifest -- foundation everything else builds on
2. `cels add` / `cels remove` -- primary value proposition (FetchContent management)
3. `cels build` / `cels run` -- complete the core workflow loop
4. `--help` / `--version` / colors / errors -- CLI hygiene (do alongside 1-3)
5. `cels doctor` -- cheap, high trust, prevents support burden
6. `.vscode/` generation -- cheap, high value for primary IDE
7. TUI dashboard (basic) -- identity feature, builds on all above
8. Module browser -- TUI content, builds on dashboard + registry
9. Debug integration -- high value but depends on cels-debug maturity
10. Everything else -- after validation

---

## Competitor Feature Analysis

### Feature Matrix

| Feature | cargo | npm | flutter | vue-cli | platformio | conan | vcpkg | cmake-init | **cels-cli** |
|---------|-------|-----|---------|---------|------------|-------|-------|------------|-------------|
| Project init/scaffolding | Y | Y | Y | Y | Y | N | N | Y | **Y** |
| Dependency add/remove | Y | Y | Y | Y | Y | Y | Y | N | **Y** |
| Build command | Y | scripts | Y | Y | Y | Y | N | N | **Y** |
| Run command | Y | scripts | Y | Y | N | N | N | N | **Y** |
| Test command | Y | scripts | Y | Y | Y | N | N | N | N (ctest) |
| Interactive TUI | N | N | N | vue ui (web) | N | N | N | N | **Y** |
| Module/package browser | crates.io (web) | npmjs.com (web) | pub.dev (web) | plugin search | lib search | conan center | vcpkg search | N | **Y (in-TUI)** |
| Doctor/health check | N | doctor | Y | N | N | N | N | N | **Y** |
| IDE config generation | N | N | Y (partial) | N | Y | N | N | N | **Y** |
| Debug integration | N | N | Y | N | Y | N | N | N | **Y** |
| Template selection | N | Y (create-X) | Y | Y | Y | N | N | Y | **Y (v1.x)** |
| Watch/hot reload | cargo-watch | nodemon | Y | Y | N | N | N | N | **v1.x** |
| Lock file | Y | Y | Y | Y | N | Y | Y (manifest) | N | N (CMake pins) |
| Central registry | crates.io | npmjs.com | pub.dev | npm | registry | conan center | vcpkg | N | N (GitHub) |
| Plugin system | Y (proc macros) | N | N | Y | N | N | N | N | N |
| Cross-compile | Y | N/A | Y | N/A | Y | Y | Y (triplets) | N | N (future) |
| Manifest file | Cargo.toml | package.json | pubspec.yaml | package.json | platformio.ini | conanfile | vcpkg.json | N | **cels.json** |
| Colored output | Y | Y | Y | Y | Y | Y | Y | N | **Y** |
| npx/zero-install | N | Y | N | Y | N | N | N | N | **Y** |

### Key Insights from Competitor Analysis

**1. cargo is the gold standard for DX.** `cargo new`, `cargo add`, `cargo build`, `cargo run`, `cargo test` -- five commands that cover 95% of daily workflow. cels-cli should match this simplicity for its core commands.

**2. flutter doctor is universally praised.** It catches problems before they become errors. Every developer who has used flutter remembers `flutter doctor` positively. Low implementation cost, massive user trust benefit.

**3. vue ui proved interactive dashboards are valued but web UIs are awkward.** vue-cli's `vue ui` launched a browser-based dashboard. Users liked the concept but found it clunky (separate browser tab, port conflicts). A terminal-native TUI avoids all these problems.

**4. platformio's library manager is the closest analog.** platformio manages C/C++ libraries from a registry, integrates with build systems, and generates project files. cels-cli's module management is architecturally similar but much simpler (fewer modules, single build system).

**5. npx distribution is a genuine advantage.** No other C/C++ development tool can be run with zero install. `npx cels-cli` puts CELS one command away from any developer with Node.js. This is a significant competitive advantage for onboarding.

**6. Nobody does in-terminal module browsing well.** This is a genuine differentiator. The closest is `platformio lib search` which outputs a text list. An interactive TUI browser with descriptions, install status, and one-key install would be novel.

**7. Lock files are unnecessary when CMake FetchContent pins commits.** cargo, npm, flutter all need lock files because their registries serve version ranges. cels-cli uses GIT_TAG which is already an exact pin. Don't add a lock file -- it duplicates what CMake already does.

**8. Test runners are table stakes for general-purpose CLIs but not for framework CLIs.** cargo needs `cargo test` because Rust's test framework is built into the language. CELS uses utest.h (external, user-managed). Wrapping ctest adds no value over documenting `ctest --test-dir build`.

---

## Sources

### Primary Analysis Sources (from training data -- MEDIUM confidence)

These are mature, stable CLIs whose feature sets have been consistent for years. Core features documented here are unlikely to have changed significantly.

- **cargo**: https://doc.rust-lang.org/cargo/ -- Rust's build system and package manager. Commands: new, init, build, run, test, bench, add, remove, update, search, publish, install, tree, clean, doc, fix
- **npm**: https://docs.npmjs.com/cli -- Node.js package manager. Commands: init, install, uninstall, update, list, run, test, publish, search, audit, doctor, cache
- **flutter CLI**: https://docs.flutter.dev/reference/flutter-cli -- Flutter's development CLI. Commands: create, run, build, test, analyze, doctor, pub, devices, config, clean, upgrade
- **vue-cli**: https://cli.vuejs.org/ -- Vue.js scaffolding tool. Commands: create, add, invoke, inspect, serve, build, ui. Plugin system with @vue/cli-plugin-X
- **platformio CLI**: https://docs.platformio.org/en/latest/core/userguide/ -- Embedded development platform. Commands: init, run, test, device, lib, boards, settings, check, debug
- **conan**: https://docs.conan.io/2/ -- C/C++ package manager. Commands: create, install, build, export, upload, search, list, profile, remote
- **vcpkg**: https://learn.microsoft.com/en-us/vcpkg/ -- Microsoft's C/C++ package manager. Commands: install, remove, list, search, update, integrate, export. Manifest mode with vcpkg.json
- **cmake-init**: https://github.com/cginternals/cmake-init -- CMake project template generator. Interactive questionnaire, generates CMakeLists.txt + project structure

### Codebase Sources (HIGH confidence)

- `/home/cachy/workspaces/libs/cels/tools/cels-cli/.planning/PROJECT.md` -- cels-cli project definition, constraints, key decisions
- `/home/cachy/workspaces/libs/cels/.planning/research/FEATURES.md` -- CELS v0.2 feature research (CLI scaffolding section)
- `/home/cachy/workspaces/libs/cels/.planning/research/STACK.md` -- CELS v0.2 stack research (CLI scaffolding section)

### Confidence Notes

- Feature lists for cargo, npm, flutter, vue-cli are **MEDIUM-HIGH** confidence. These tools are extremely well-documented and their core features have been stable for 3+ years. Minor additions may exist post-training but core feature sets are accurate.
- Anti-feature analysis is **HIGH** confidence. These are architectural decisions, not version-specific features. The reasoning (e.g., "lock files are unnecessary with FetchContent pinning") is based on how CMake FetchContent works, which is stable and well-documented.
- Complexity estimates are **MEDIUM** confidence. Actual implementation complexity depends on Node.js TUI library choice and CMakeLists.txt manipulation approach, which are stack decisions not yet finalized.
- WebSearch and WebFetch were unavailable during this research session. All findings are based on training data analysis of well-established CLIs. The features documented here represent stable, long-standing capabilities of these tools, not recent additions.

---
*Feature research for: CLI Developer Toolkit (cels-cli)*
*Researched: 2026-02-08*
*Researcher: Features dimension*
