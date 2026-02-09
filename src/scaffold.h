/*
 * cels-cli Scaffold
 *
 * Project scaffolding: generates a new CELS project directory
 * with CMakeLists.txt, src/main.c, and .gitignore.
 */

#ifndef CELS_CLI_SCAFFOLD_H
#define CELS_CLI_SCAFFOLD_H

/* Create a new CELS project directory with all boilerplate files.
 * Returns 0 on success, 1 on error (messages printed to stderr). */
int cli_scaffold_project(const char* project_name);

#endif /* CELS_CLI_SCAFFOLD_H */
