/*
 * cels-cli Components
 *
 * App-specific types and re-export of shared widget components.
 */

#ifndef CELS_CLI_COMPONENTS_H
#define CELS_CLI_COMPONENTS_H

#include <cels-widgets/compositions.h>

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

#endif /* CELS_CLI_COMPONENTS_H */
