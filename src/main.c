/*
 * cels-cli -- Interactive TUI toolkit for the CELS framework
 *
 * Custom main() handles subcommands (init, --help, --version)
 * before optionally launching the TUI. Overrides the universal
 * main() from libcels (static linker skips it when we define ours).
 *
 * Subcommands:
 *   cels-cli              Launch TUI
 *   cels-cli init <name>  Create a new CELS project
 *   cels-cli --help       Show help
 *   cels-cli --version    Show version
 */

#include "scaffolding.h"
#include <cels/cels.h>
#include <stdio.h>
#include <string.h>

#define CLI_VERSION "0.1.0"

static void print_help(void) {
    printf("cels-cli %s -- CELS project toolkit\n", CLI_VERSION);
    printf("\n");
    printf("Usage:\n");
    printf("  cels-cli              Launch interactive TUI\n");
    printf("  cels-cli init <name>  Create a new CELS project\n");
    printf("  cels-cli --help       Show this help\n");
    printf("  cels-cli --version    Show version\n");
}

int main(int argc, char** argv) {
    /* No arguments: launch TUI */
    if (argc < 2) {
        Engine_Startup();
        CELS_BuildInit();
        cels_run_providers();
        Engine_Shutdown();
        return 0;
    }

    const char* cmd = argv[1];

    /* --help */
    if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
        print_help();
        return 0;
    }

    /* --version */
    if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0) {
        printf("cels-cli %s\n", CLI_VERSION);
        return 0;
    }

    /* init <name> */
    if (strcmp(cmd, "init") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: cels-cli init <name>\n");
            return 1;
        }
        return scaffold_project(argv[2]);
    }

    /* Unknown command */
    fprintf(stderr, "Unknown command: %s\n", cmd);
    fprintf(stderr, "Run 'cels-cli --help' for usage.\n");
    return 1;
}
