/*
 * cels-cli Components
 *
 * Atomic component definitions for the CLI TUI shell.
 * Used by app.c (compositions) and app.c (rendering).
 */

#ifndef CELS_CLI_COMPONENTS_H
#define CELS_CLI_COMPONENTS_H

#include <cels/cels.h>

/* ============================================================================
 * Enums
 * ============================================================================ */

typedef enum {
    TAB_PROJECTS = 0,
    TAB_PACKAGES,
    TAB_BUILD,
    TAB_TESTS,
    TAB_COUNT
} CliTab;

/* ============================================================================
 * Components
 * ============================================================================ */

/* TabBar: horizontal tab strip at top of screen */
CEL_Define(TabBar, {
    CliTab active;
    int count;
});

/* TabContent: placeholder content area for each tab */
CEL_Define(TabContent, {
    CliTab tab;
    const char* placeholder_text;
});

/* StatusBar: bottom status line */
CEL_Define(StatusBar, {
    const char* version;
    const char* cwd;
});

#endif /* CELS_CLI_COMPONENTS_H */
