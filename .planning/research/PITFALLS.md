# Pitfalls Research

**Domain:** CLI Developer Toolkit (TUI-based, Node.js, managing C/C++ build tooling)
**Researched:** 2026-02-08
**Confidence:** HIGH (Node.js CLI + npx distribution well-understood), MEDIUM (Ink-specific TUI rendering, CMake process management)

---

## Critical Pitfalls

Mistakes that cause rewrites, broken user experience, or fundamental architecture failures.

---

### P-01: npx Cold Start Kills First Impressions

**What goes wrong:** Running `npx cels-cli` for the first time downloads the package, installs dependencies, and then boots the Node.js runtime. With a typical TUI framework (Ink + React), the cold start time can exceed 3-5 seconds before the user sees anything. Developers expect `cargo`/`flutter`-like responsiveness (<500ms to first paint). A 5-second blank terminal signals "broken" or "slow tool" and erodes trust before the tool does anything.

**Why it happens:** npx must: (1) resolve the package from the registry, (2) download and extract the tarball, (3) install dependencies if not cached, (4) require/import the entry point, (5) parse and JIT-compile the JavaScript. Steps 1-3 happen only on first run, but step 4-5 happen every time. A large dependency tree (React, Ink, chalk, etc.) means hundreds of modules to require on every invocation. Node.js module resolution is I/O-heavy -- each `require()` triggers filesystem lookups.

**How to avoid:**
1. Bundle the entire application into a single file using esbuild or tsup. This eliminates require-chain overhead and reduces cold start to ~200-500ms
2. Keep the dependency tree minimal -- every added package increases bundle size and parse time
3. Show a loading indicator (even a simple `process.stdout.write` spinner) BEFORE importing heavy dependencies like Ink/React
4. Measure cold start time in CI: `time npx cels-cli --version` must complete in <1s
5. Consider lazy-loading heavy features (e.g., only import the TUI when the user actually enters the dashboard, not for `cels init`)
6. Pre-build and publish compiled JavaScript (not TypeScript source) -- never rely on runtime compilation

**Warning signs:**
- `npx cels-cli` takes >2 seconds on a warm cache
- `node_modules/` has >200 packages
- Bundle size exceeds 5MB
- Users report "nothing happened" when they run the command

**Phase to address:** Phase 1 (project scaffolding / initial setup). The bundling strategy must be decided at project initialization. Retrofitting bundling into a project with complex import trees is painful.

**Confidence:** HIGH -- this is the #1 complaint about npx-distributed tools. Create React App, Angular CLI, and similar tools all faced this.

---

### P-02: TUI Process Blocks While Child Process Runs

**What goes wrong:** The TUI spawns `cmake --build .` which runs for 30-120 seconds. During this time, the TUI must remain responsive -- the user should see build output streaming, be able to cancel the build, and navigate between tabs. If the child process is spawned synchronously or if stdout/stderr are not handled correctly, the TUI freezes until the build completes.

**Why it happens:** The simplest Node.js API is `child_process.execSync()` which blocks the event loop entirely. Even `child_process.exec()` buffers all output and delivers it only when the process completes. For a TUI that needs streaming output, you need `child_process.spawn()` with proper event handling -- but integrating streaming child process output with React/Ink's rendering model is non-trivial.

**How to avoid:**
1. Always use `child_process.spawn()` (never `exec` or `execSync`) for long-running processes
2. Pipe stdout/stderr through `data` event handlers that update React state incrementally
3. Implement a build output buffer: collect lines, render the last N visible lines in the viewport
4. Provide a cancel mechanism: track the child process PID, send SIGTERM on user cancel (Escape/Ctrl+C)
5. Handle process cleanup: if the TUI exits while a build is running, kill the child process group (use `process.kill(-pid)` for process groups, or `spawn()` with `detached: false`)
6. Set `stdio: ['pipe', 'pipe', 'pipe']` explicitly -- never inherit stdio in a TUI context (it would corrupt the terminal)

**Warning signs:**
- TUI stops responding to keyboard input during builds
- Build output appears all at once after completion
- Ctrl+C kills the TUI but leaves cmake running as an orphan process
- Terminal is corrupted after a cancelled build

**Phase to address:** Phase where build integration is implemented. The child process management pattern must be correct from the first build command.

**Confidence:** HIGH -- standard Node.js child process management concern. Every CI/CD tool and build runner has faced this.

---

### P-03: Terminal State Corruption on Crash or Forced Exit

**What goes wrong:** Ink puts the terminal into raw mode (no echo, no line buffering, alternate screen buffer). If the process crashes, throws an unhandled exception, or the user hits Ctrl+C while the TUI is in a complex state, the terminal is left corrupted -- no echo, cursor invisible, alternate screen still active. The user's shell becomes unusable until they type `reset` blindly.

**Why it happens:** Ink uses raw mode via the `stdin.setRawMode(true)` API and switches to the alternate screen buffer with ANSI escape codes. These must be reversed on exit. Ink registers cleanup handlers, but: (1) unhandled promise rejections can bypass them, (2) spawned child processes that write directly to stdout can interfere, (3) SIGKILL cannot be caught, (4) some signals require explicit handling.

**How to avoid:**
1. Wrap the entire application in a try/catch at the top level that calls Ink's `unmount()` / cleanup before re-throwing
2. Register handlers for `process.on('exit')`, `process.on('SIGINT')`, `process.on('SIGTERM')`, and `process.on('uncaughtException')`
3. In each handler: restore terminal (disable raw mode, show cursor, exit alternate screen), then exit
4. Write the alternate-screen-exit sequence directly: `process.stdout.write('\x1b[?1049l\x1b[?25h')` as a last resort in signal handlers
5. During development, test by: (a) throwing an unhandled error in a component, (b) sending SIGTERM, (c) pressing Ctrl+C during a build. Verify terminal state after each.
6. Never call `process.exit()` without cleanup -- use Ink's `app.waitUntilExit()` pattern

**Warning signs:**
- Terminal has no echo after the CLI exits
- Cursor is invisible after crash
- Terminal shows raw escape codes instead of rendered text
- User must type `reset` to recover

**Phase to address:** Phase 1 (TUI foundation). Must be correct from the first render.

**Confidence:** HIGH -- this is the exact analog of the ncurses endwin() pitfall documented in cels-debug (P4). Every TUI framework has this problem.

---

### P-04: CMake Not Found or Wrong Version

**What goes wrong:** The user runs `cels init` and creates a project, then `cels build` fails because cmake is not installed, is too old (CMake <3.14 does not support FetchContent), or is installed in a non-standard path. The CLI either crashes with an unhelpful error or shows a cmake error that means nothing to a user unfamiliar with CMake.

**Why it happens:** The CLI is a Node.js tool that generates and invokes CMake. It assumes cmake is in PATH and is recent enough. This assumption fails for: (1) macOS users who have Xcode but not standalone cmake, (2) Ubuntu/Debian users with cmake from apt which may be old, (3) Windows WSL users, (4) nix/guix users where cmake may be in a non-standard path.

**How to avoid:**
1. On first run (or `cels init`), check for cmake: `cmake --version`, parse the version number
2. If cmake is missing: provide a clear error with installation instructions per platform (apt, brew, pacman, etc.)
3. If cmake is too old: show the installed version, the required version (3.14+), and how to upgrade
4. Allow a `CELS_CMAKE` environment variable or config to specify a custom cmake path
5. Cache the cmake path after first successful detection
6. Test the cmake check with: cmake missing, cmake too old, cmake in custom path

**Warning signs:**
- Error messages show raw cmake output instead of actionable guidance
- Users file issues saying "cels build doesn't work" with no useful error
- `cels init` succeeds but `cels build` fails (should have checked prerequisites during init)

**Phase to address:** Phase where build integration is implemented. Prerequisite checking should be the first thing the build command does.

**Confidence:** HIGH -- every CMake-dependent tool faces this. Flutter's `flutter doctor` is the gold standard for prerequisite checking.

---

### P-05: FetchContent Downloads During Build Hang With No Progress

**What goes wrong:** `cmake --build .` triggers FetchContent to download CELS core, flecs, and any modules from GitHub. These downloads can take 10-60 seconds on slow connections. During this time, the user sees no output (cmake configure step is quiet about FetchContent downloads by default). It appears hung. Users cancel and retry, never getting past the download phase.

**Why it happens:** CMake's FetchContent downloads happen during the configure step (`cmake -S . -B build`), not the build step. The configure step has minimal output by default. FetchContent does not show download progress unless `FETCHCONTENT_QUIET OFF` is set. On slow/unstable connections, git clone operations can take minutes with no feedback.

**How to avoid:**
1. In generated CMakeLists.txt, set `set(FETCHCONTENT_QUIET OFF)` so cmake shows download progress
2. Separate the configure step from the build step in the TUI: show "Downloading dependencies..." during configure with a spinner
3. Parse cmake configure output for FetchContent progress lines and display them in the TUI
4. Set timeouts: if configure takes >5 minutes, suggest the user check their network
5. Consider pre-downloading modules via git clone before running cmake configure (gives the CLI full control over progress reporting)
6. Cache downloaded content: ensure `FETCHCONTENT_BASE_DIR` is set to a persistent location (not inside the build directory) so rebuilds don't re-download

**Warning signs:**
- Users report "cels build hangs" -- actually downloading dependencies
- Build works on fast connections but fails on slow ones
- Users cancel during download phase, leaving partial downloads that break subsequent builds

**Phase to address:** Build integration phase AND module management phase. The download UX must be designed explicitly, not left to cmake defaults.

**Confidence:** HIGH -- FetchContent download behavior is well-documented. The "hangs with no output" complaint is extremely common in FetchContent-based projects.

---

### P-06: Module Version Conflicts in CMake FetchContent

**What goes wrong:** User adds module A which depends on CELS v0.2.0. They also add module B which depends on CELS v0.2.1. CMake's `FetchContent_Declare()` uses first-one-wins semantics -- whichever `FetchContent_Declare(cels ...)` is processed first determines the version. The second declaration is silently ignored. The user gets a version they did not expect, and one module may break with a subtle API incompatibility.

**Why it happens:** CMake FetchContent is designed for direct dependencies, not transitive dependency resolution. It has no semver awareness, no conflict resolution, no diamond dependency handling. Each `FetchContent_Declare()` for the same content name is ignored after the first. This means the order of `add_subdirectory()` calls determines which version wins.

**How to avoid:**
1. The CLI must be the single source of truth for CELS core version -- do not let individual modules declare their own CELS dependency
2. Generate a single `FetchContent_Declare(cels ...)` at the top of the project's CMakeLists.txt, with a version chosen by the CLI
3. Modules should declare their minimum required CELS version in their manifest (metadata only), not in their CMakeLists.txt
4. The CLI resolves versions: pick the highest minimum version that satisfies all modules
5. Display a warning if a module's minimum version exceeds the resolved version
6. Pin specific git tags/commits, not branches -- `GIT_TAG v0.2.1` not `GIT_TAG main`

**Warning signs:**
- "Works with module A alone, breaks when module B is added"
- Different build results depending on CMakeLists.txt ordering
- Module tests pass in isolation but fail in the user's project

**Phase to address:** Module management phase. This is a design-time decision about how the CLI generates CMakeLists.txt.

**Confidence:** HIGH -- this is the most common FetchContent pitfall, documented in CMake's own documentation. Already identified in CELS v0.2 PITFALLS.md (P-17).

---

### P-07: Ink/React Rendering Performance With Streaming Build Output

**What goes wrong:** Build output arrives at high frequency (100+ lines/second during compilation). Each new line triggers a React re-render in Ink. Ink re-renders the entire component tree, diffs the output, and writes to stdout. At high line rates, the re-rendering becomes the bottleneck -- the TUI lags, stutters, or consumes excessive CPU. The terminal scrolling falls behind the actual build progress.

**Why it happens:** Ink's rendering model is React's: any state change triggers a component re-render, virtual DOM diff, and terminal write. This model is designed for interactive UIs with infrequent updates, not for high-throughput log streaming. React's batching helps but does not solve the fundamental problem when state changes arrive faster than the render cycle.

**How to avoid:**
1. Throttle state updates: batch incoming lines and update state at most every 50-100ms (16-20fps), not on every line
2. Use a ring buffer for build output: keep only the last N lines (e.g., 200), discard older lines
3. Consider rendering build output outside of Ink's React tree -- write directly to a subregion of the terminal using ANSI escapes, bypassing React
4. Use `React.memo()` aggressively to prevent unrelated components from re-rendering when build output updates
5. Profile with `why-did-you-render` or manual logging to identify unnecessary re-renders
6. If performance is still insufficient, consider a non-React TUI library (blessed-contrib, terminal-kit) for the build output panel

**Warning signs:**
- CPU usage spikes during builds
- Build output scrolling is visibly laggy
- TUI keyboard input becomes unresponsive during fast build output
- `time cmake --build .` is significantly faster outside the TUI than inside it

**Phase to address:** Build integration phase. Must be designed when build output streaming is first implemented.

**Confidence:** MEDIUM -- Ink performance characteristics depend heavily on version and usage patterns. The throttling strategy is well-established but the specific thresholds need testing.

---

## Technical Debt Patterns

---

### P-08: Template Drift -- Scaffolded Code Becomes Stale

**What goes wrong:** `cels init` generates a starter project with a CMakeLists.txt, main.c, and possibly .vscode config. These templates reference a specific CELS version, specific CMake patterns, and specific API calls. As CELS evolves (v0.2, v0.3, etc.), the templates become outdated. New users get starter code that does not compile against the current CELS version.

**Why it happens:** Templates are written once during CLI development and forgotten. They are not part of the CELS test suite. Nobody runs `cels init` as part of CI. The template and the framework evolve independently.

**How to avoid:**
1. Add a CI job that runs `cels init test-project && cd test-project && cmake -B build && cmake --build build` to verify templates compile
2. Keep templates minimal: the less generated code, the less drift surface
3. Template should reference a CELS version tag, not `main` branch
4. Version the templates: when the CLI is released, templates should be tested against the CELS version the CLI was designed for
5. Consider generating templates dynamically based on the current CELS version, rather than embedding static files

**Warning signs:**
- User reports "cels init creates a project that doesn't compile"
- Templates reference macros/APIs that were renamed in a newer CELS version
- Nobody has run `cels init` manually in months

**Phase to address:** Scaffolding phase. Add template compilation to CI from day one.

**Confidence:** HIGH -- already identified in CELS v0.2 PITFALLS.md (P-16). Universal problem for project generators.

---

### P-09: Module Manifest Becomes Single Point of Failure

**What goes wrong:** The CLI uses a JSON manifest (likely hosted on GitHub or bundled in the package) that maps module names to GitHub repos. If this manifest is wrong, stale, or inaccessible, the user cannot add modules. A single typo in the manifest breaks `cels add module-name` for everyone.

**Why it happens:** Without a proper registry server, the manifest is a flat file. It has no validation, no automated testing, and no contributor process. Adding a new module means editing a JSON file and publishing a new CLI version.

**How to avoid:**
1. Validate the manifest in CI: for each module entry, verify the GitHub repo exists and the specified tag/branch is accessible
2. Design for manifest updates without CLI updates: fetch the manifest from a GitHub raw URL at runtime (with a local cache fallback)
3. Allow users to add arbitrary modules by URL, not just those in the manifest: `cels add github:user/repo`
4. Use a schema for the manifest and validate it on load
5. Provide a way to refresh the manifest: `cels update-registry` or automatic refresh with TTL

**Warning signs:**
- User cannot add a module that was recently published
- Module names in the manifest do not match actual repo names
- Users must update the CLI just to see new modules

**Phase to address:** Module management phase. The manifest design is a critical early decision.

**Confidence:** MEDIUM -- depends on how the manifest is implemented. The "fetch from GitHub" approach is common but has its own failure modes (rate limiting, network issues).

---

## Integration Gotchas

---

### P-10: CMake Output Parsing is Fragile

**What goes wrong:** The CLI parses cmake's stdout/stderr to detect errors, progress, and status. But cmake's output format is not stable -- it varies by generator (Make vs Ninja vs MSVC), by cmake version, by platform, and by verbosity level. Parsing that works with GNU Make on Linux breaks with Ninja, or with a different cmake version.

**Why it happens:** CMake does not have a machine-readable output format for build progress. The human-readable output uses locale-dependent strings, platform-specific paths, and generator-specific formatting. Developers test with one generator and one platform, then discover their regex breaks elsewhere.

**How to avoid:**
1. Do NOT parse cmake output for critical logic (success/failure) -- use the exit code instead (0 = success, non-zero = failure)
2. For progress reporting, use cmake's built-in `CMAKE_PROGRESS` support or Ninja's status line format
3. For error extraction, look for patterns that are stable across versions: `error:` prefix from compilers (gcc, clang), not cmake's own formatting
4. Pass through cmake output to the user as-is in a build log panel -- do not try to "interpret" it
5. Use `cmake --build . -- -j$(nproc)` and parse only the compiler's output, which has stable error formats
6. Consider using `compile_commands.json` for structured build information instead of parsing stdout

**Warning signs:**
- Build "succeeds" in the TUI but the binary was not actually produced
- Error messages are garbled or truncated
- Progress percentage jumps erratically or gets stuck at 0%

**Phase to address:** Build integration phase. The output handling strategy must accommodate different generators and platforms.

**Confidence:** MEDIUM -- cmake output stability depends on specific version and generator combinations.

---

### P-11: Git Operations Fail Silently or Confusingly

**What goes wrong:** Module installation involves git operations (clone, fetch, checkout). These can fail for many reasons: git not installed, SSH vs HTTPS authentication, private repos, corporate firewalls, shallow clone limitations, submodule issues. The error messages from git are informative for git users but cryptic for C/C++ developers who just want to add a module.

**Why it happens:** The CLI delegates to git (either via cmake FetchContent or direct `git clone`). Git errors are passed through without translation. Different authentication configurations (SSH keys, credential helpers, personal access tokens) create a matrix of failure modes that are difficult to predict.

**How to avoid:**
1. Always use HTTPS URLs for public repos (not SSH) in the default module manifest -- SSH requires key setup that most users may not have
2. Translate common git errors into actionable messages: "Could not clone repo. Is your network connected? Is the repo public?"
3. Check `git --version` on first run, same as cmake
4. For private repos, document the authentication setup clearly and detect auth failures specifically
5. Set `GIT_TERMINAL_PROMPT=0` to prevent git from hanging waiting for password input in the TUI
6. Test with: no git installed, no network, private repo, corporate proxy

**Warning signs:**
- `cels add` hangs (git waiting for password input)
- Cryptic SSH errors when using HTTPS URLs (or vice versa)
- FetchContent fails with "unable to access" but no guidance on why

**Phase to address:** Module management phase. Git error handling should be designed alongside the module add workflow.

**Confidence:** HIGH -- git authentication failures are the #1 support issue for tools that clone repos.

---

### P-12: Platform-Specific Path Handling

**What goes wrong:** The CLI generates file paths in CMakeLists.txt, .vscode configs, and shell commands. Paths that work on Linux (forward slashes, case-sensitive, no spaces in common paths) break on macOS (case-insensitive filesystem, spaces in "Application Support") or fail when the project is in a path with spaces.

**Why it happens:** Node.js developers typically use `path.join()` which handles OS-specific separators. But the generated CMakeLists.txt is consumed by cmake, which has its own path handling. And .vscode launch.json paths must match the OS's expectations. Mixing `path.join()`, template literals, and cmake path variables creates subtle mismatches.

**How to avoid:**
1. Use `path.join()` for all Node.js filesystem operations -- never concatenate paths with `/`
2. In generated CMakeLists.txt, use cmake's `${CMAKE_SOURCE_DIR}` and `${CMAKE_BINARY_DIR}` instead of hardcoded paths
3. Quote all paths in generated shell commands and CMake commands
4. Test with a project path containing spaces: `~/my projects/test-cels/`
5. Use `path.resolve()` to convert relative paths to absolute before using them
6. For .vscode configs, use `${workspaceFolder}` variable, not absolute paths

**Warning signs:**
- "Works on my machine" reports
- Builds fail when the project is in a path with spaces
- CMakeLists.txt contains hardcoded absolute paths from the developer's machine

**Phase to address:** Scaffolding phase (path generation) and build integration phase (cmake invocation). Use `path.join()` and cmake variables from the start.

**Confidence:** HIGH -- universal cross-platform path handling concern.

---

## Performance Traps

---

### P-13: Node.js Event Loop Blocked by Synchronous Operations

**What goes wrong:** The TUI becomes unresponsive during operations that seem harmless: reading a large project config file, scanning a directory tree for modules, parsing a large CMakeLists.txt, or computing a diff. These synchronous operations block the Node.js event loop, freezing the TUI for 100ms-2s. Ink's rendering stops, keyboard input is dropped, and the tool feels janky.

**Why it happens:** Node.js is single-threaded. Any synchronous I/O (`fs.readFileSync`, `execSync`, `JSON.parse` on a large file) blocks the event loop. React/Ink's rendering relies on the event loop being free. Developers use sync APIs during prototyping and forget to migrate to async versions.

**How to avoid:**
1. Use async APIs everywhere: `fs.promises.readFile()`, not `fs.readFileSync()`
2. For CPU-intensive work (parsing, diffing), use `setImmediate()` to yield back to the event loop periodically
3. Never use `execSync` or `spawnSync` once the TUI is running
4. Profile with `--prof` or `clinic.js` to find event loop blocking during TUI operations
5. Consider using `worker_threads` for heavy computation (CMakeLists.txt parsing, module dependency resolution) if needed
6. Set a performance budget: no single synchronous operation should take >16ms (one frame at 60fps)

**Warning signs:**
- TUI "freezes" briefly during navigation or file operations
- Keyboard input is dropped or delayed
- Ink renders a partial frame (half the screen updates, then the rest catches up)

**Phase to address:** Every phase. This is a discipline that must be maintained throughout development, not a one-time fix.

**Confidence:** HIGH -- fundamental Node.js architecture constraint.

---

### P-14: Excessive Package Size Bloats npx Install

**What goes wrong:** The published npm package includes unnecessary files (TypeScript source, tests, documentation, example projects, build artifacts, .git directory). The package download is 10-50MB instead of 1-2MB. npx install time is proportional to download size. Users on slow connections wait 30+ seconds for the first `npx cels-cli`.

**Why it happens:** `npm publish` by default includes everything not in `.npmignore` or excluded by the `files` field in `package.json`. Developers add files during development and forget to exclude them from the published package. Dev dependencies bundled into the production build add weight.

**How to avoid:**
1. Use the `files` field in `package.json` (whitelist approach) instead of `.npmignore` (blacklist). Only include: `dist/`, `bin/`, and essential assets
2. Bundle the application into a single file with esbuild -- the published package should contain one JS file plus a bin stub
3. Run `npm pack --dry-run` before publishing to see exactly what will be included
4. Set a size budget: the published package should be <2MB
5. Add `npm pack --dry-run | wc -l` to CI as a check
6. Mark all build tools as `devDependencies`, not `dependencies`
7. If bundling, the `dependencies` field in `package.json` should be nearly empty (everything is bundled)

**Warning signs:**
- `npm pack` shows dozens of files
- Package size exceeds 5MB
- `node_modules/` is published (catastrophic but it happens)
- Users complain about slow `npx` startup

**Phase to address:** Phase 1 (project setup). The `package.json` files field and bundling strategy must be configured from the start.

**Confidence:** HIGH -- well-documented npm publishing concern.

---

### P-15: Memory Leaks in Long-Running TUI Sessions

**What goes wrong:** The TUI dashboard is designed to run for hours during a development session. React/Ink components that capture closures, accumulate state, or attach event listeners without cleanup slowly consume memory. After 2-4 hours, the process uses 500MB+ and becomes sluggish.

**Why it happens:** React's component lifecycle manages rendering but not arbitrary resource cleanup. Event listeners on process.stdin, child process event handlers, filesystem watchers, and accumulated log buffers all contribute. In a short-lived CLI this does not matter. In a long-running TUI dashboard, it is critical.

**How to avoid:**
1. Use React `useEffect` cleanup functions to remove event listeners and close watchers
2. Bound all accumulating data structures: build log ring buffer (max 1000 lines), module cache (TTL-based), error history (max 50 entries)
3. Avoid capturing large objects in closures -- particularly build output and cmake log data
4. Profile memory periodically during development: `process.memoryUsage()` logged every 60 seconds
5. Test: run the TUI for 30+ minutes with periodic build/module operations. Monitor RSS growth.
6. If using child processes, ensure event listeners are removed when the process exits

**Warning signs:**
- RSS memory grows steadily over time (leak)
- RSS spikes during builds and never fully recovers (accumulation)
- Garbage collector pauses cause visible TUI stuttering after extended use

**Phase to address:** Every phase -- but specifically build integration (build output accumulation) and module management (module data caching).

**Confidence:** MEDIUM -- depends on specific implementation patterns. React/Ink's own memory management is generally good; the risks come from user code.

---

## Security Mistakes

---

### P-16: Module Registry Integrity -- Typosquatting and Repo Hijacking

**What goes wrong:** The module registry maps names to GitHub repos. An attacker could: (1) register a similarly-named module (`cels-ncruses` vs `cels-ncurses`), (2) take over an abandoned GitHub repo that a module points to, (3) push malicious code to a module repo between version tags. Since FetchContent clones and builds the code, a compromised module runs arbitrary code during cmake configure/build.

**Why it happens:** The trust model is "GitHub repos are legitimate because they are listed in our manifest." But the manifest does not verify content, only location. Git tags can be moved. Repos can change ownership. There is no signature verification.

**How to avoid:**
1. Pin module references to specific git commit SHAs, not tags (tags can be moved): `GIT_TAG abc123def` not `GIT_TAG v1.0`
2. The CLI should show the user what will be cloned and built before doing it
3. Maintain the manifest in a repo with signed commits and restricted push access
4. Consider adding a checksum/hash for each module version in the manifest
5. Warn users when a module has not been updated in >1 year (potential abandoned repo)
6. Never add `execute_process()` or custom `ExternalProject` commands in module CMakeLists.txt templates -- restrict to `add_library` and `target_link_libraries`
7. For v1, keep the module list small and manually curated -- do not accept arbitrary community submissions without review

**Warning signs:**
- Module names that are similar to official modules
- Module repos with sudden ownership changes
- Module CMakeLists.txt with `execute_process()` calls

**Phase to address:** Module management phase. Security model should be designed when the module registry is created.

**Confidence:** MEDIUM -- the risk is real but the attack surface is smaller for a niche framework. Still, supply chain attacks are increasingly common.

---

### P-17: Generated Config Files Expose Machine-Specific Paths

**What goes wrong:** `cels init` generates .vscode/launch.json or CMake presets with absolute paths from the developer's machine. These get committed to git and fail on other developers' machines. Worse, paths can leak usernames, home directory structure, or internal network paths into version-controlled files.

**How to avoid:**
1. Generated configs must use only relative paths and tool-specific variables (`${workspaceFolder}`, `${CMAKE_SOURCE_DIR}`)
2. If absolute paths are needed at runtime, resolve them at build time, not generation time
3. Add generated paths to .gitignore templates
4. Review: does any generated file contain the user's home directory path? If yes, it is a bug.

**Warning signs:**
- Generated files contain `/home/username/` or `/Users/username/`
- CI builds fail because paths do not exist on the build machine
- `.vscode/launch.json` committed to git with absolute paths

**Phase to address:** Scaffolding phase. Path generation logic must use variables, not literals.

**Confidence:** HIGH -- extremely common scaffolding mistake.

---

## UX Pitfalls

---

### P-18: TUI Colors Unreadable on Light Terminal Themes

**What goes wrong:** The TUI is designed and tested on a dark terminal theme (common for developers). Colors that look great on dark backgrounds (yellow text, light blue, light green) become invisible or unreadable on light backgrounds. The tool is unusable for the ~20% of developers who use light themes.

**Why it happens:** Ink/Node.js TUI libraries use ANSI color codes. The "default" colors (0-7, 8-15) are interpreted differently by every terminal and theme. "Yellow" on a dark theme is a visible highlight; on a light theme it is invisible pale text. Developers test only on their own theme.

**How to avoid:**
1. Use semantic colors that adapt: prefer "bold" and "dim" attributes over specific color numbers for emphasis
2. Avoid hardcoding background colors -- use the terminal's default background
3. Test with both dark and light terminal themes (at minimum: test with `COLORFGBG` set to different values)
4. Provide a `--no-color` flag and respect the `NO_COLOR` environment variable (see https://no-color.org/)
5. Use chalk's level detection to gracefully degrade (true color -> 256 -> 16 -> none)
6. Never use light colors (bright yellow, bright white, bright cyan) for text on unknown backgrounds
7. For critical status information, use text decoration (bold, underline, inverse) rather than color alone

**Warning signs:**
- Bug reports saying "I can't read the text"
- Screenshots showing invisible or low-contrast elements
- All testing done on a single terminal/theme

**Phase to address:** TUI foundation phase. Color strategy should be decided early and tested on multiple themes.

**Confidence:** HIGH -- universal TUI/CLI color accessibility concern.

---

### P-19: Terminal Resize Causes Layout Corruption

**What goes wrong:** The user resizes their terminal while the TUI is running. The layout does not adapt: components overlap, text is truncated, scrollbars disappear, or the entire screen goes blank. Ink handles terminal resize via `SIGWINCH`, but components must be designed to work at any size.

**Why it happens:** Components with hardcoded widths/heights or minimum size assumptions break at smaller terminal sizes. Ink's flexbox layout model handles most resize cases, but components that calculate positions manually or use absolute positioning will corrupt on resize.

**How to avoid:**
1. Use Ink's `<Box>` flexbox layout exclusively -- avoid manual position calculation
2. Use `useStdout()` hook to get current terminal dimensions and react to changes
3. Set minimum terminal size requirements (e.g., 80x24) and show an error message if the terminal is too small
4. Test at extreme sizes: 40x10 (very small), 200x60 (very large), and 80x24 (minimum common)
5. Use percentage-based sizing where possible (`width="50%"`) instead of fixed widths
6. Handle edge cases: what happens when the terminal is so small that the tab bar itself does not fit?

**Warning signs:**
- Layout breaks when terminal is resized
- Text overflows component boundaries
- Components overlap after resize
- Application crashes with "out of bounds" errors on very small terminals

**Phase to address:** TUI foundation phase. Layout system must be responsive from the start.

**Confidence:** HIGH -- direct analog of cels-debug P8 (terminal resize crashes). The same problem exists regardless of TUI framework.

---

### P-20: Ctrl+C Ambiguity -- Cancel Operation vs Exit Application

**What goes wrong:** During a build, Ctrl+C should cancel the build but keep the TUI running. At the main menu, Ctrl+C should exit the application. But the default behavior of Node.js is to exit the process on SIGINT. Without careful signal handling, Ctrl+C always kills the entire application, including during operations the user wanted to cancel.

**Why it happens:** SIGINT is the universal "cancel" signal, but its scope is ambiguous. Shell users expect Ctrl+C to cancel the current operation (like in bash). Ink captures SIGINT for its own exit handling. This conflicts with using Ctrl+C as an in-app cancel.

**How to avoid:**
1. Use Escape (not Ctrl+C) as the in-app cancel key -- less ambiguous
2. If using Ctrl+C for cancel: disable Ink's default SIGINT handling and manage it manually
3. Implement a state machine: if a child process is running, Ctrl+C sends SIGTERM to the child; if no operation is running, Ctrl+C exits the app
4. Double-Ctrl+C to force exit: first Ctrl+C cancels/warns, second Ctrl+C exits
5. Show clear footer hints: "Esc: cancel build | q: quit" or "Ctrl+C: exit"
6. Test: press Ctrl+C during a build, during a module download, and at the idle dashboard. Verify each behaves correctly.

**Warning signs:**
- Users lose their TUI session when they meant to cancel a build
- Builds cannot be cancelled (Ctrl+C kills everything)
- Orphan cmake processes after cancellation

**Phase to address:** Build integration phase (cancel behavior) and TUI foundation phase (exit behavior).

**Confidence:** HIGH -- this is the #1 UX complaint about CLI tools that have both "cancel operation" and "exit" behaviors.

---

### P-21: No Feedback During Network Operations

**What goes wrong:** `cels add some-module` triggers a network fetch (checking the manifest, cloning a repo, verifying versions). If the network is slow, the user sees no indication of progress. The TUI appears frozen. There is no way to tell if it is downloading, computing, or stuck.

**How to avoid:**
1. Show a spinner/progress indicator for every operation that takes >200ms
2. For downloads, show bytes/percentage if possible
3. Display the current step: "Fetching module manifest..." -> "Cloning cels-ncurses..." -> "Verifying compatibility..."
4. Use timeouts: if an operation takes >30 seconds, suggest network troubleshooting
5. Make all network operations cancellable (Escape to cancel)

**Warning signs:**
- Users report "cels add hangs"
- No visual difference between "working" and "stuck"
- Users cancel operations that were actually making progress

**Phase to address:** Module management phase. Every network operation must have a progress indicator.

**Confidence:** HIGH -- universal UX requirement for network-dependent tools.

---

## "Looks Done But Isn't" Checklist

These are features that appear complete in demo/testing but fail in real-world usage.

| Feature | Looks Done When... | Actually Broken When... |
|---------|-------------------|------------------------|
| `cels init` scaffolding | Generated project compiles on developer's machine | User has different cmake version, missing compiler, or path with spaces |
| Module installation | Module installs successfully in test | Module repo is private, network is slow, manifest is stale |
| Build integration | Build runs and shows output | Build is cancelled mid-way, cmake errors are cryptic, TUI freezes during configure |
| TUI dashboard | All panels render correctly | Terminal is resized, user has light theme, terminal does not support 256 colors |
| `npx cels-cli` | Works when installed globally | Cold start takes 5+ seconds, postinstall scripts fail, wrong Node.js version |
| Cross-platform | Works on developer's Linux | macOS has different cmake paths, zsh vs bash differences, case-insensitive filesystem |
| CMakeLists.txt generation | Generated file builds the project | User adds manual edits, CLI overwrites them on next `cels add`, merge conflicts |
| .vscode config | Debug works from VS Code | launch.json has absolute paths, user has different VS Code extensions, tasks.json incompatible |

---

## Recovery Strategies

When a pitfall is hit despite prevention:

| Situation | Recovery |
|-----------|----------|
| Terminal corrupted after crash | Add `reset` suggestion to error output. Consider shipping a `cels reset-terminal` command |
| Build hangs | Add `--timeout` flag for build operations. Show "Press Escape to cancel" in build UI |
| Wrong CELS version installed | `cels doctor` command that shows all dependency versions and conflicts |
| Template produces broken project | `cels doctor` or `cels verify` that checks prerequisites and generated file integrity |
| Module installation broke CMakeLists.txt | Keep a backup of CMakeLists.txt before modification: `.CMakeLists.txt.bak`. Offer `cels restore` |
| Memory leak in long session | Add `--max-memory` flag that auto-restarts the TUI if RSS exceeds threshold |
| npx cold start too slow | Recommend `npm install -g cels-cli` for frequent users. Optimize bundle size. |
| Color scheme unreadable | `--theme dark|light|none` flag. Respect `NO_COLOR` and `COLORFGBG` env vars |

---

## Pitfall-to-Phase Mapping

| Phase | Pitfalls to Address | Priority |
|-------|-------------------|----------|
| **Project Setup / Foundation** | P-01 (npx cold start), P-03 (terminal corruption), P-14 (package size), P-18 (color themes), P-19 (resize), P-13 (event loop blocking) | CRITICAL -- these are architectural decisions |
| **TUI Dashboard** | P-07 (rendering performance), P-15 (memory leaks), P-18 (colors), P-19 (resize), P-20 (Ctrl+C ambiguity) | HIGH -- defines the user experience |
| **Scaffolding** | P-04 (cmake not found), P-08 (template drift), P-12 (path handling), P-17 (path exposure) | HIGH -- first user interaction |
| **Module Management** | P-06 (version conflicts), P-09 (manifest fragility), P-11 (git failures), P-16 (registry security), P-21 (network feedback) | HIGH -- complex integration point |
| **Build Integration** | P-02 (process blocking), P-05 (FetchContent progress), P-07 (streaming output), P-10 (cmake output parsing), P-20 (cancel vs exit) | CRITICAL -- two worlds colliding (Node.js + cmake) |
| **Polish / Release** | P-01 (cold start optimization), P-14 (bundle size), P-18 (accessibility) | MEDIUM -- quality-of-life improvements |

---

## Risk Matrix

| Pitfall | Probability | Impact | Phase | Priority |
|---------|-------------|--------|-------|----------|
| P-01: npx cold start | CERTAIN | HIGH | Foundation | MUST address |
| P-02: TUI blocks during build | HIGH | CRITICAL | Build | MUST address |
| P-03: Terminal corruption on crash | HIGH | HIGH | Foundation | MUST address |
| P-04: CMake not found / wrong version | HIGH | HIGH | Scaffolding | MUST address |
| P-05: FetchContent hangs silently | HIGH | HIGH | Build | MUST address |
| P-06: Module version conflicts | MEDIUM | HIGH | Modules | Design early |
| P-07: Ink rendering perf with streaming | MEDIUM | MEDIUM | Build | Throttle strategy |
| P-08: Template drift | CERTAIN | MEDIUM | Scaffolding | CI test |
| P-09: Manifest single point of failure | LOW | HIGH | Modules | Design early |
| P-10: CMake output parsing fragile | MEDIUM | MEDIUM | Build | Exit code only |
| P-11: Git operations fail | HIGH | MEDIUM | Modules | Error translation |
| P-12: Path handling cross-platform | MEDIUM | MEDIUM | All | path.join() discipline |
| P-13: Event loop blocking | MEDIUM | HIGH | All | Async-only discipline |
| P-14: Package size bloat | MEDIUM | HIGH | Foundation | Bundle + files field |
| P-15: Memory leaks in long sessions | LOW | MEDIUM | All | Bound all buffers |
| P-16: Module registry security | LOW | CRITICAL | Modules | Pin commits, not tags |
| P-17: Path exposure in generated files | MEDIUM | LOW | Scaffolding | Variables only |
| P-18: Colors unreadable on light themes | HIGH | MEDIUM | Foundation | Semantic colors |
| P-19: Resize layout corruption | MEDIUM | MEDIUM | Foundation | Flexbox layout |
| P-20: Ctrl+C ambiguity | HIGH | MEDIUM | Build + Foundation | State machine |
| P-21: No network progress feedback | HIGH | MEDIUM | Modules | Spinners everywhere |

---

## Sources

### Direct Project Experience
- cels-debug PITFALLS.md -- TUI-specific pitfalls for ncurses (terminal corruption, resize, flicker, memory management). Many pitfalls transfer directly to Node.js TUI context (P-03, P-19 are direct analogs of cels-debug P4, P8).
- CELS v0.2 PITFALLS.md -- FetchContent version conflicts (P-17), template drift (P-16). Both directly applicable to cels-cli.
- cels-debug STATE.md -- Lessons learned from building a TUI tool (ncurses decisions, threading model, polling strategy).

### Node.js CLI Ecosystem (from training data -- MEDIUM confidence)
- npm `files` field documentation for package size control
- Node.js `child_process.spawn()` documentation for streaming process management
- Ink library design -- React rendering model for terminals
- chalk color level detection for terminal color compatibility

### CMake (from training data -- HIGH confidence)
- CMake FetchContent `FETCHCONTENT_QUIET` variable documentation
- CMake FetchContent first-one-wins semantics for `FetchContent_Declare`
- CMake generator differences (Make vs Ninja output formats)

### Standards and Conventions
- https://no-color.org/ -- NO_COLOR environment variable standard
- Node.js signal handling documentation for SIGINT/SIGTERM
- npm publishing best practices for package size

---
*Pitfalls research for: CLI Developer Toolkit (TUI-based, Node.js, managing C/C++ build tooling)*
*Researched: 2026-02-08*
