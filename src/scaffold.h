/*
 * Copyright 2026 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * cels-cli Scaffold
 *
 * Project scaffolding: generates a new CELS project directory
 * with CMakeLists.txt, src/main.c, and .gitignore.
 */

#ifndef CELS_CLI_SCAFFOLD_H
#define CELS_CLI_SCAFFOLD_H

/* Template types for project scaffolding */
typedef enum {
    SCAFFOLD_HELLO = 0,   /* Default: hello world with centered text */
    SCAFFOLD_MENU,        /* Menu + settings screen pattern */
    SCAFFOLD_GAME         /* Game skeleton with Position/Velocity/Sprite */
} ScaffoldTemplate;

/* Create a new CELS project directory with all boilerplate files.
 * Returns 0 on success, 1 on error (messages printed to stderr). */
int cli_scaffold_project(const char* project_name);

/* Create a new CELS project directory using the specified template.
 * Returns 0 on success, 1 on error (messages printed to stderr). */
int cli_scaffold_project_with_template(const char* project_name, ScaffoldTemplate tmpl);

#endif /* CELS_CLI_SCAFFOLD_H */
