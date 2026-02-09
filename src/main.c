/*
 * cels-cli -- Interactive TUI toolkit for the CELS framework
 *
 * Custom main() handles subcommands (init, --help, --version)
 * before optionally launching the TUI. Overrides the universal
 * main() from libcels (static linker skips it when we define ours).
 *
 * Subcommands:
 *   cels-cli                               Launch TUI
 *   cels-cli init <name>                   Create a new CELS project (hello template)
 *   cels-cli init <name> --template <type> Create project with template (hello|menu|game)
 *   cels-cli --help                        Show help
 *   cels-cli --version                     Show version
 */

#include "scaffold.h"
#include <cels/cels.h>
#include <stdio.h>
#include <string.h>

#define CLI_VERSION "0.1.0"

static void print_help(void) {
    printf("cels-cli %s -- CELS project toolkit\n", CLI_VERSION);
    printf("\n");
    printf("Usage:\n");
    printf("  cels-cli                               Launch interactive TUI\n");
    printf("  cels-cli init <name>                   Create a new CELS project\n");
    printf("  cels-cli init <name> --template <type> Create project with template\n");
    printf("  cels-cli --help                        Show this help\n");
    printf("  cels-cli --version                     Show version\n");
    printf("\n");
    printf("Templates:\n");
    printf("  hello  (default)  Minimal app with centered text and status bar\n");
    printf("  menu              Menu + settings screen with navigation\n");
    printf("  game              Game skeleton with Position/Velocity/Sprite components\n");
}

/* Parse template name to enum. Returns -1 on invalid name. */
static int parse_template(const char* name) {
    if (strcmp(name, "hello") == 0) return SCAFFOLD_HELLO;
    if (strcmp(name, "menu") == 0)  return SCAFFOLD_MENU;
    if (strcmp(name, "game") == 0)  return SCAFFOLD_GAME;
    return -1;
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

    /* init <name> [--template <type>] */
    if (strcmp(cmd, "init") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: cels-cli init <name> [--template <type>]\n");
            return 1;
        }

        const char* name = argv[2];
        ScaffoldTemplate tmpl = SCAFFOLD_HELLO;

        /* Parse optional --template flag */
        for (int i = 3; i < argc - 1; i++) {
            if (strcmp(argv[i], "--template") == 0 || strcmp(argv[i], "-t") == 0) {
                int t = parse_template(argv[i + 1]);
                if (t < 0) {
                    fprintf(stderr, "Unknown template: '%s'\n", argv[i + 1]);
                    fprintf(stderr, "Available templates: hello, menu, game\n");
                    return 1;
                }
                tmpl = (ScaffoldTemplate)t;
                break;
            }
        }

        return cli_scaffold_project_with_template(name, tmpl);
    }

    /* Unknown command */
    fprintf(stderr, "Unknown command: %s\n", cmd);
    fprintf(stderr, "Run 'cels-cli --help' for usage.\n");
    return 1;
}
