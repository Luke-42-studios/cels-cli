/*
 * cels-cli Scaffold
 *
 * Generates a new CELS project directory with CMakeLists.txt,
 * src/main.c, and .gitignore. All generated files are self-contained
 * and compile immediately via cmake.
 */

#include "scaffold.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* ============================================================================
 * Templates
 * ============================================================================ */

static const char* TMPL_CMAKELISTS =
    "cmake_minimum_required(VERSION 3.21)\n"
    "project(%s LANGUAGES C)\n"
    "\n"
    "include(FetchContent)\n"
    "FetchContent_Declare(cels\n"
    "    GIT_REPOSITORY https://github.com/Luke-42-studios/cels.git\n"
    "    GIT_TAG v0.2.0\n"
    ")\n"
    "FetchContent_MakeAvailable(cels)\n"
    "\n"
    "include(CelsDeps)\n"
    "cels_require(ncurses)\n"
    "\n"
    "add_executable(%s src/main.c)\n"
    "target_link_libraries(%s PRIVATE cels cels-ncurses)\n"
    "set_target_properties(%s PROPERTIES C_STANDARD 99)\n";

static const char* TMPL_MAIN =
    "/*\n"
    " * %s -- A CELS Application\n"
    " */\n"
    "\n"
    "#include <cels/cels.h>\n"
    "#include <cels-ncurses/tui_engine.h>\n"
    "\n"
    "CEL_State(AppState, {\n"
    "    int counter;\n"
    "});\n"
    "\n"
    "CEL_Root(AppUI, TUI_EngineContext) {\n"
    "    TUI_WindowState_t* win = CEL_WatchId(ctx.windowState, TUI_WindowState_t);\n"
    "    if (win->state == WINDOW_STATE_READY) {\n"
    "        AppState_t* app = CEL_Watch(AppState);\n"
    "        (void)app;\n"
    "    }\n"
    "}\n"
    "\n"
    "CEL_Build(App) {\n"
    "    (void)props;\n"
    "    AppState = (AppState_t){ .counter = 0 };\n"
    "\n"
    "    TUI_Engine_use((TUI_EngineConfig){\n"
    "        .title = \"%s\",\n"
    "        .version = \"0.1.0\",\n"
    "        .fps = 30,\n"
    "        .root = AppUI\n"
    "    });\n"
    "}\n";

static const char* TMPL_GITIGNORE =
    "build/\n"
    ".cache/\n"
    "compile_commands.json\n"
    "*.o\n"
    "*.a\n";

/* ============================================================================
 * Helpers
 * ============================================================================ */

static int make_dir(const char* path) {
    if (mkdir(path, 0755) != 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "Error: directory '%s' already exists.\n", path);
        } else {
            fprintf(stderr, "Error: cannot create '%s': %s\n", path, strerror(errno));
        }
        return -1;
    }
    return 0;
}

static int write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "Error: cannot write '%s': %s\n", path, strerror(errno));
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

static int write_file_fmt(const char* path, const char* fmt, ...) {
    FILE* f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "Error: cannot write '%s': %s\n", path, strerror(errno));
        return -1;
    }
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fclose(f);
    return 0;
}

/* ============================================================================
 * cli_scaffold_project
 * ============================================================================ */

int cli_scaffold_project(const char* project_name) {
    if (!project_name || project_name[0] == '\0') {
        fprintf(stderr, "Error: project name cannot be empty.\n");
        return 1;
    }

    /* Create project directory */
    if (make_dir(project_name) != 0) return 1;

    /* Create src/ subdirectory */
    char src_dir[512];
    snprintf(src_dir, sizeof(src_dir), "%s/src", project_name);
    if (make_dir(src_dir) != 0) return 1;

    /* Write CMakeLists.txt */
    char path[512];
    snprintf(path, sizeof(path), "%s/CMakeLists.txt", project_name);
    if (write_file_fmt(path, TMPL_CMAKELISTS,
            project_name, project_name, project_name, project_name) != 0)
        return 1;

    /* Write src/main.c */
    snprintf(path, sizeof(path), "%s/src/main.c", project_name);
    if (write_file_fmt(path, TMPL_MAIN, project_name, project_name) != 0)
        return 1;

    /* Write .gitignore */
    snprintf(path, sizeof(path), "%s/.gitignore", project_name);
    if (write_file(path, TMPL_GITIGNORE) != 0) return 1;

    printf("Created CELS project '%s'\n", project_name);
    printf("\n");
    printf("  cd %s\n", project_name);
    printf("  cmake -B build\n");
    printf("  cmake --build build\n");
    printf("  ./build/%s\n", project_name);
    printf("\n");

    return 0;
}
