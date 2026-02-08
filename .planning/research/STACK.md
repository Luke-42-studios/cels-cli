# Stack Research

**Domain:** CLI Developer Toolkit (TUI-based)
**Researched:** 2026-02-08
**Confidence:** HIGH

All version numbers and dates verified via `npm view` against the live npm registry on 2026-02-08.

## Recommended Stack

### Core Technologies

| Technology | Version | Purpose | Why Recommended |
|------------|---------|---------|-----------------|
| **Node.js** | >=20 (current: 25.4.0) | Runtime | Ink 6 requires Node >=20. LTS 22 is the safe floor; 25.x works fine. |
| **TypeScript** | 5.9.3 | Language | Type safety, IDE support, Zod integration. Published 2026-02-08. |
| **React** | 19.2.4 | UI paradigm | Ink 6 requires React >=19. Declarative component model is ideal for complex TUI state. |
| **Ink** | 6.6.0 | TUI framework | The standard for building interactive terminal UIs in Node.js. React-based, flexbox layout via Yoga, active maintenance (last updated 2025-12-22). |
| **Pastel** | 4.0.0 | CLI framework on top of Ink | Next.js-like file-based routing for CLI commands. Uses Commander under the hood. Auto-generates help. Zod-based option parsing with full type safety. Same maintainer as Ink. Published 2025-10-18. |
| **Zod** | 4.3.6 | Schema validation | Required by Pastel for option/argument definitions. Also useful for config file validation, module manifest validation. Published 2026-01-25. |

### TUI Component Libraries

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| **@inkjs/ui** | 2.0.0 | Official Ink component library | Always -- provides Select, MultiSelect, TextInput, Spinner, ProgressBar, Badge, StatusMessage, Alert, ConfirmInput, OrderedList, UnorderedList. Themeable. |
| **ink-tab** | 5.2.0 | Tab navigation | For the main dashboard navigation (Modules, Build, Run tabs). Compatible with ink 4/5/6 and React 18/19. Updated 2025-07-04. |
| **ink-table** | 3.1.0 | Table rendering | For displaying module lists, target lists, dependency trees. Peer dep: ink >=3. |
| **ink-big-text** | 2.0.0 | ASCII art text | For the welcome/splash header. Peer dep: ink >=4. |
| **ink-gradient** | 4.0.0 | Gradient text | For styled headers/branding. Peer dep: ink >=6 (explicitly compatible). Published 2026-02-03. |
| **ink-divider** | 4.1.1 | Section dividers | For visual separation between dashboard sections. Published 2025-03-25. |
| **ink-link** | 5.0.0 | Clickable terminal links | For linking to docs, GitHub repos. Published 2025-09-13. |
| **ink-syntax-highlight** | 2.0.2 | Code highlighting | For displaying generated code previews. Published 2025-01-05. |

### File System and Process

| Library | Version | Purpose | Why |
|---------|---------|---------|-----|
| **fs-extra** | 11.3.3 | File system operations | Superset of fs with `copy`, `ensureDir`, `outputFile`, `readJson`, `writeJson`. Well-typed. Updated 2025-12-18. |
| **execa** | 9.6.1 | Process spawning | For running cmake, git, and cels-debug. Better error handling, promise-based, piping, streaming output. Updated 2025-11-29. |
| **simple-git** | 3.30.0 | Git operations | For cloning module repos, checking versions. Higher-level than raw execa+git. Updated 2025-11-02. |
| **globby** | 16.1.0 | File globbing | For finding project files, scanning directories. Updated 2025-12-21. |

### Configuration and State

| Library | Version | Purpose | Why |
|---------|---------|---------|-----|
| **conf** | 15.1.0 | Persistent config | Stores user preferences, cached module registry data. XDG-compliant, schema validation, atomic writes. Updated 2026-02-04. |
| **zustand** | 5.0.11 | React state management | For complex TUI state (selected module, build status, navigation). Works with React 19. Lightweight, no boilerplate. Updated 2026-02-01. |

### Template Generation

| Library | Version | Purpose | Why |
|---------|---------|---------|-----|
| **ejs** | 4.0.1 | Template engine | For generating CMakeLists.txt, main.c, .vscode/. Simple `<%= %>` syntax, no runtime deps, straightforward for code generation. Updated 2026-01-14. |

### Development Tools

| Tool | Version | Purpose | Notes |
|------|---------|---------|-------|
| **tsup** | 8.5.1 | Build/bundle | Bundles TypeScript to ESM/CJS. Zero-config for most cases. Uses esbuild internally. Updated 2025-11-12. |
| **tsx** | 4.21.0 | Dev runner | Run TypeScript directly during development without compilation. Updated 2025-11-30. |
| **vitest** | 4.0.18 | Testing | Fast, TypeScript-native, watch mode, JSX support for testing Ink components. Updated 2026-02-02. |
| **ink-testing-library** | 4.0.0 | Component testing | Render Ink components in tests, inspect output, simulate input. Peer dep: @types/react >=18. |
| **prettier** | 3.8.1 | Formatting | Standard code formatting. |
| **eslint** | 10.0.0 | Linting | Code quality enforcement. |
| **@sindresorhus/tsconfig** | 8.1.0 | TSConfig base | Strict, modern TypeScript config. Same ecosystem as Ink. |

## Architecture Decision: Pastel vs Raw Ink + Commander

**Recommendation: Use Pastel.**

Pastel wraps Ink + Commander + Zod into a file-based routing system. For cels-cli, this means:

```
source/commands/
  index.tsx          -> `cels` (default: shows dashboard)
  init.tsx           -> `cels init`
  build.tsx          -> `cels build`
  run.tsx            -> `cels run`
  module/
    index.tsx        -> `cels module` (shows module browser)
    add.tsx          -> `cels module add <name>`
    remove.tsx       -> `cels module remove <name>`
  target/
    index.tsx        -> `cels target` (shows target list)
    add.tsx          -> `cels target add <name>`
```

Each command file exports a Zod schema for options/arguments and a React component for the UI. This gives you:
- Automatic help generation from Zod schemas
- Type-safe option parsing
- File-based command discovery (add a file = add a command)
- Each command is an Ink component, so `cels module` can render the full interactive module browser

**The alternative** (raw Ink + Commander) gives more control but requires manual wiring of every command, manual help text, manual type casting. For a toolkit with 10+ commands, Pastel's structure prevents spaghetti.

## Architecture Decision: Why Ink, Not Blessed

**blessed** (v0.1.81): Last meaningful update was years ago. The npm "modified" date of 2024-10-22 is likely a metadata update, not a code update. The library uses a custom rendering engine with ncurses-like semantics. It is powerful but unmaintained and has known issues with modern terminals.

**neo-blessed** (v0.2.0): Fork of blessed, last updated 2022-05-10. Also effectively abandoned.

**terminal-kit** (v3.1.2): Full-featured terminal library (256 colors, mouse, input fields, screen buffers). Updated 2025-10-13. A viable alternative for lower-level TUI work, but lacks the component model and ecosystem that Ink provides.

**Ink** (v6.6.0): React-based, actively maintained by Vadim Demedes and Sindre Sorhus (two of the most prolific Node.js open source maintainers). Uses Yoga (Facebook's flexbox engine) for layout. Has a rich ecosystem of component libraries. The React model means state management, composition, and lifecycle hooks all work as expected.

**Verdict:** Ink is the clear winner for 2026. It is the only actively maintained Node.js TUI framework with a component ecosystem. The React mental model also means any developer familiar with React can contribute immediately.

## Architecture Decision: ESM Only

Ink 6, Pastel 4, and most of the Sindre Sorhus ecosystem are ESM-only. Do not fight this.

```json
// package.json
{
  "type": "module"
}
```

```json
// tsconfig.json
{
  "compilerOptions": {
    "module": "nodenext",
    "moduleResolution": "nodenext",
    "jsx": "react-jsx",
    "outDir": "build",
    "sourceMap": true
  }
}
```

## npx Distribution

For `npx cels` to work:

```json
// package.json
{
  "name": "cels-cli",
  "bin": {
    "cels": "./build/cli.js"
  },
  "files": ["build"],
  "engines": {
    "node": ">=20"
  }
}
```

The `build/cli.js` file must start with `#!/usr/bin/env node`.

**tsup** bundles everything into a single file (or small set of files), so `npx` downloads and runs efficiently. The bundle includes React, Ink, and all dependencies -- no post-install step needed.

```typescript
// tsup.config.ts
import {defineConfig} from 'tsup';

export default defineConfig({
  entry: ['source/cli.ts'],
  format: ['esm'],
  target: 'node20',
  clean: true,
  dts: false,       // CLI doesn't need type declarations
  sourcemap: true,
  // Bundle all dependencies for npx portability
  noExternal: [/.*/],
});
```

**Note on bundle size:** Bundling React + Ink + all components will produce a ~2-5MB bundle. This is acceptable for a CLI tool downloaded via npx (one-time download, cached afterward). The alternative (not bundling, relying on npm install) is slower for first-run npx experience.

## Installation

```bash
# Initialize project
mkdir cels-cli && cd cels-cli
npm init --yes

# Core dependencies
npm install pastel ink react @inkjs/ui zod

# TUI components
npm install ink-tab ink-table ink-big-text ink-gradient ink-divider ink-link

# File system and process
npm install fs-extra execa simple-git globby

# Configuration and state
npm install conf zustand

# Template generation
npm install ejs

# Dev dependencies
npm install -D typescript @sindresorhus/tsconfig @types/react @types/fs-extra @types/ejs
npm install -D tsup tsx vitest ink-testing-library
npm install -D prettier eslint
```

## Alternatives Considered

| Category | Recommended | Alternative | When to Use Alternative |
|----------|-------------|-------------|-------------------------|
| TUI Framework | **Ink 6** | terminal-kit 3.1 | Only if you need raw terminal control (cursor positioning, screen buffers) that Ink's flexbox model can't express. Unlikely for this project. |
| TUI Framework | **Ink 6** | blessed 0.1.81 | Never. Unmaintained. |
| CLI Framework | **Pastel 4** | Commander 14 + raw Ink | If you need only 2-3 commands with no TUI. For 10+ commands with interactive UIs, Pastel wins. |
| CLI Framework | **Pastel 4** | oclif 4.8 | If building a Heroku/Salesforce-style CLI with plugin architecture. Overkill and opinionated differently for a dev toolkit. |
| CLI Framework | **Pastel 4** | yargs 18 | If you want a purely non-interactive CLI. yargs has no TUI story. |
| Process spawning | **execa 9** | node:child_process | If you want zero dependencies. execa's error handling, streaming, and promise API are worth the dependency. |
| File system | **fs-extra 11** | node:fs/promises | If you only do basic reads/writes. fs-extra's `copy`, `ensureDir`, `readJson` save boilerplate. |
| State management | **zustand 5** | React useState/useReducer | If your state is simple and local to one component. For cross-component state (selected module, build status), zustand is cleaner. |
| Template engine | **ejs 4** | Handlebars 4.7 | If you need template inheritance/partials. For generating flat files (CMakeLists.txt, main.c), ejs is simpler. |
| Config storage | **conf 15** | node:fs + JSON | If you want zero deps. conf handles atomicity, migrations, XDG paths, schema validation. |
| Git operations | **simple-git 3.30** | execa + git commands | If you only need `git clone`. For version checking, branch listing, remote management, simple-git is more ergonomic. |
| Build tool | **tsup 8** | esbuild directly | If you need fine-grained esbuild control. tsup wraps esbuild with sane defaults for library/CLI bundling. |
| Testing | **vitest 4** | Jest | If your team already uses Jest. Vitest is faster, has native TypeScript/ESM support, and better watch mode. |
| Prompts (non-TUI) | **@inkjs/ui** (Select, TextInput) | @clack/prompts 1.0 | If building a non-interactive CLI that only needs occasional prompts. Since we're already using Ink, @inkjs/ui provides the same components as part of the TUI. |
| Prompts (non-TUI) | **@inkjs/ui** | @inquirer/prompts 8.2 | Same reasoning -- inquirer is for non-Ink CLIs. |

## What NOT to Use

| Avoid | Why | Use Instead |
|-------|-----|-------------|
| **blessed / neo-blessed** | Unmaintained since 2019/2022. Known rendering bugs in modern terminals. No TypeScript types. No React model. | Ink 6 |
| **chalk** (directly) | Ink handles colors internally. @inkjs/ui uses chalk internally. Adding chalk as a direct dependency creates confusion about where to style text -- in Ink's `<Text color="green">` or chalk's `chalk.green()`. | Ink's `<Text>` component with color prop |
| **ora** (directly) | ora renders its own spinner to stdout. This conflicts with Ink's rendering model (Ink owns stdout). Using both causes visual corruption. | @inkjs/ui `<Spinner>` component |
| **inquirer / @inquirer/prompts** | Inquirer manages its own stdin/stdout. Conflicts with Ink's input handling and rendering. | @inkjs/ui Select, TextInput, ConfirmInput |
| **@clack/prompts** | Same stdout conflict as inquirer. Beautiful library, but incompatible with Ink's rendering model. | @inkjs/ui components |
| **listr2** | Task list renderer that manages its own stdout. Conflicts with Ink. | Custom task list component in Ink (trivial to build with @inkjs/ui Spinner + StatusMessage) |
| **yargs** | No TUI integration. The argument parsing works fine, but Pastel already wraps Commander with Zod, which is superior for type-safe CLIs. | Pastel (uses Commander internally) |
| **CommonJS (`require`)** | Ink 6 and the entire Sindre Sorhus ecosystem are ESM-only. Fighting this with CJS will cause import errors and bundling headaches. | ESM (`import/export`) with `"type": "module"` |
| **webpack** | Overkill for CLI bundling. Slow, complex configuration. | tsup (wraps esbuild) |
| **Handlebars** | Heavier than needed for flat file generation. Logic-less templates sound good until you need conditionals in CMakeLists.txt. | ejs (supports `<% if %>` natively) |

## Dependency Count Summary

**Runtime dependencies:** 15 packages (Pastel, Ink, React, @inkjs/ui, Zod, ink-tab, ink-table, ink-big-text, ink-gradient, ink-divider, ink-link, fs-extra, execa, simple-git, globby, conf, zustand, ejs)

**Dev dependencies:** 10 packages (TypeScript, tsconfig, @types/react, @types/fs-extra, @types/ejs, tsup, tsx, vitest, ink-testing-library, prettier, eslint)

This is a moderate dependency footprint. The Ink ecosystem packages are lightweight (most are single-file). The heaviest dependencies are React (~150KB) and the Yoga layout engine (~800KB WASM), both of which are bundled by tsup.

## Sources

All data verified via `npm view <package> version time.modified` on 2026-02-08:

| Package | Version | Last Updated | Confidence |
|---------|---------|-------------|------------|
| ink | 6.6.0 | 2025-12-22 | HIGH (npm registry) |
| pastel | 4.0.0 | 2025-10-18 | HIGH (npm registry) |
| @inkjs/ui | 2.0.0 | 2024-05-22 | HIGH (npm registry) |
| react | 19.2.4 | -- | HIGH (npm registry) |
| zod | 4.3.6 | 2026-01-25 | HIGH (npm registry) |
| commander | 14.0.3 | 2026-01-31 | HIGH (npm registry) |
| ink-tab | 5.2.0 | 2025-07-04 | HIGH (npm registry) |
| ink-table | 3.1.0 | 2023-12-06 | HIGH (npm registry) |
| ink-big-text | 2.0.0 | -- | HIGH (npm registry) |
| ink-gradient | 4.0.0 | 2026-02-03 | HIGH (npm registry) |
| ink-divider | 4.1.1 | 2025-03-25 | HIGH (npm registry) |
| ink-link | 5.0.0 | 2025-09-13 | HIGH (npm registry) |
| ink-syntax-highlight | 2.0.2 | 2025-01-05 | HIGH (npm registry) |
| fs-extra | 11.3.3 | 2025-12-18 | HIGH (npm registry) |
| execa | 9.6.1 | 2025-11-29 | HIGH (npm registry) |
| simple-git | 3.30.0 | 2025-11-02 | HIGH (npm registry) |
| globby | 16.1.0 | 2025-12-21 | HIGH (npm registry) |
| conf | 15.1.0 | 2026-02-04 | HIGH (npm registry) |
| zustand | 5.0.11 | 2026-02-01 | HIGH (npm registry) |
| ejs | 4.0.1 | 2026-01-14 | HIGH (npm registry) |
| tsup | 8.5.1 | 2025-11-12 | HIGH (npm registry) |
| tsx | 4.21.0 | 2025-11-30 | HIGH (npm registry) |
| vitest | 4.0.18 | 2026-02-02 | HIGH (npm registry) |
| ink-testing-library | 4.0.0 | -- | HIGH (npm registry) |
| typescript | 5.9.3 | 2026-02-08 | HIGH (npm registry) |
| @sindresorhus/tsconfig | 8.1.0 | -- | HIGH (npm registry) |

**Confidence note:** The `@inkjs/ui` package was last published 2024-05-22 (almost 2 years ago). However, it is the official companion to Ink, maintained by the same team, and its peer dependency of `ink >=5` covers Ink 6. Its components (Select, Spinner, ProgressBar, etc.) are stable and well-tested. The lack of recent updates indicates stability, not abandonment. If a component gap is discovered during development, building custom Ink components is straightforward.

---
*Stack research for: cels-cli (CLI Developer Toolkit)*
*Researched: 2026-02-08*
*All versions verified against npm registry*
