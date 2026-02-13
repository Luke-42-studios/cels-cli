/*
 * cels-cli Scaffold
 *
 * Generates a new CELS project directory with CMakeLists.txt,
 * src/main.c, and .gitignore. All generated files are self-contained
 * and compile immediately via cmake.
 *
 * Templates:
 *   hello (default) - Minimal hello world with centered text (single-file)
 *   menu            - Menu + settings with features/ directory convention
 *   game            - Game skeleton with features/ directory convention
 */

#include "scaffold.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* ============================================================================
 * Templates - CMakeLists.txt (single-file: hello)
 * ============================================================================ */

static const char* TMPL_CMAKELISTS_SINGLE =
    "cmake_minimum_required(VERSION 3.21)\n"
    "project(%s LANGUAGES C)\n"
    "\n"
    "include(FetchContent)\n"
    "FetchContent_Declare(cels\n"
    "    GIT_REPOSITORY https://github.com/Luke-42-studios/cels.git\n"
    "    GIT_TAG v0.3.9\n"
    ")\n"
    "FetchContent_MakeAvailable(cels)\n"
    "\n"
    "include(CelsDeps)\n"
    "cels_require(ncurses)\n"
    "cels_require(clay)\n"
    "cels_require(widgets)\n"
    "\n"
    "add_executable(%s src/main.c)\n"
    "target_link_libraries(%s PRIVATE cels cels-ncurses cels-clay cels-widgets)\n"
    "set_target_properties(%s PROPERTIES C_STANDARD 99)\n";

/* ============================================================================
 * Templates - CMakeLists.txt (multi-file: menu)
 * ============================================================================ */

static const char* TMPL_CMAKELISTS_MENU =
    "cmake_minimum_required(VERSION 3.21)\n"
    "project(%s LANGUAGES C)\n"
    "\n"
    "include(FetchContent)\n"
    "FetchContent_Declare(cels\n"
    "    GIT_REPOSITORY https://github.com/Luke-42-studios/cels.git\n"
    "    GIT_TAG v0.3.9\n"
    ")\n"
    "FetchContent_MakeAvailable(cels)\n"
    "\n"
    "include(CelsDeps)\n"
    "cels_require(ncurses)\n"
    "cels_require(clay)\n"
    "cels_require(widgets)\n"
    "\n"
    "add_executable(%s\n"
    "    src/main.c\n"
    "    src/features/menu/menu.c\n"
    "    src/features/settings/settings.c)\n"
    "target_include_directories(%s PRIVATE ${CMAKE_SOURCE_DIR}/src)\n"
    "target_link_libraries(%s PRIVATE cels cels-ncurses cels-clay cels-widgets)\n"
    "set_target_properties(%s PROPERTIES C_STANDARD 99)\n";

/* ============================================================================
 * Templates - CMakeLists.txt (multi-file: game)
 * ============================================================================ */

static const char* TMPL_CMAKELISTS_GAME =
    "cmake_minimum_required(VERSION 3.21)\n"
    "project(%s LANGUAGES C)\n"
    "\n"
    "include(FetchContent)\n"
    "FetchContent_Declare(cels\n"
    "    GIT_REPOSITORY https://github.com/Luke-42-studios/cels.git\n"
    "    GIT_TAG v0.3.9\n"
    ")\n"
    "FetchContent_MakeAvailable(cels)\n"
    "\n"
    "include(CelsDeps)\n"
    "cels_require(ncurses)\n"
    "cels_require(clay)\n"
    "cels_require(widgets)\n"
    "\n"
    "add_executable(%s\n"
    "    src/main.c\n"
    "    src/features/gameplay/gameplay.c)\n"
    "target_include_directories(%s PRIVATE ${CMAKE_SOURCE_DIR}/src)\n"
    "target_link_libraries(%s PRIVATE cels cels-ncurses cels-clay cels-widgets)\n"
    "set_target_properties(%s PROPERTIES C_STANDARD 99)\n";

/* ============================================================================
 * Templates - .gitignore
 * ============================================================================ */

static const char* TMPL_GITIGNORE =
    "build/\n"
    ".cache/\n"
    "compile_commands.json\n"
    "*.o\n"
    "*.a\n";

/* ============================================================================
 * Templates - Hello World (single-file, unchanged)
 * ============================================================================ */

static const char* TMPL_MAIN_HELLO =
    "/*\n"
    " * %s -- A CELS Application\n"
    " */\n"
    "\n"
    "#include <cels/cels.h>\n"
    "#include <cels-widgets/compositions.h>\n"
    "#include <cels-ncurses/backend.h>\n"
    "\n"
    "/* Content area: centered greeting */\n"
    "#define HelloContent(...) CEL_Init(HelloContent, __VA_ARGS__)\n"
    "CEL_Composition(HelloContent) {\n"
    "    (void)props;\n"
    "    Widget_TabContent(.text = \"%s\",\n"
    "               .hint = \"Press q to quit\") {}\n"
    "}\n"
    "\n"
    "/* Status bar */\n"
    "#define AppStatus(...) CEL_Init(AppStatus, __VA_ARGS__)\n"
    "CEL_Composition(AppStatus) {\n"
    "    (void)props;\n"
    "    Widget_StatusBar(.left  = \"%s v0.1.0\",\n"
    "              .right = \"q:quit \") {}\n"
    "}\n"
    "\n"
    "CEL_Root(AppUI) {\n"
    "    (void)_props;\n"
    "    cels_entity_t window = CEL_Query(CEL_Window);\n"
    "    CEL_Window* win = CEL_Watch(window, CEL_Window);\n"
    "    if (win && win->ready) {\n"
    "        Layout_Surface(.width = (float)win->width / 2.0f,\n"
    "                    .height = (float)win->height) {\n"
    "            HelloContent() {}\n"
    "            AppStatus() {}\n"
    "        }\n"
    "    }\n"
    "}\n"
    "\n"
    "CEL_BuildComposition(App) {\n"
    "    CEL_BuildHas(CelsEngine);\n"
    "    CEL_BuildHas(CelsNcurses);\n"
    "    CEL_BuildHas(Widgets);\n"
    "}\n"
    "\n"
    "CEL_Run(.title = \"%s\", .version = \"0.1.0\", .fps = 30, .root = AppUI);\n";

/* ============================================================================
 * Templates - Menu: src/main.c
 * ============================================================================ */

static const char* TMPL_MENU_MAIN =
    "/*\n"
    " * %s -- A CELS Menu Application\n"
    " *\n"
    " * Multi-file project using the features/ directory convention.\n"
    " * Each feature has its own .h (CEL_Composition_Decl) and .c\n"
    " * (CEL_Composition_Impl + state + systems).\n"
    " *\n"
    " * Controls: Up/Down to navigate, Enter to select, Q to quit.\n"
    " */\n"
    "\n"
    "#include <cels/cels.h>\n"
    "#include <cels-widgets/compositions.h>\n"
    "#include <cels-ncurses/backend.h>\n"
    "\n"
    "#include \"features/menu/menu.h\"\n"
    "#include \"features/settings/settings.h\"\n"
    "\n"
    "CEL_Root(AppUI) {\n"
    "    (void)_props;\n"
    "    cels_entity_t window = CEL_Query(CEL_Window);\n"
    "    CEL_Window* win = CEL_Watch(window, CEL_Window);\n"
    "    if (win && win->ready) {\n"
    "        Layout_Surface(.width = (float)win->width / 2.0f,\n"
    "                       .height = (float)win->height) {\n"
    "            /* TODO: Add screen routing logic here.\n"
    "             * Use CEL_State for screen index, then:\n"
    "             *   if (screen == 0) MainMenu() {}\n"
    "             *   if (screen == 1) Settings() {}\n"
    "             */\n"
    "            MainMenu() {}\n"
    "        }\n"
    "    }\n"
    "}\n"
    "\n"
    "CEL_BuildComposition(App) {\n"
    "    CEL_BuildHas(CelsEngine);\n"
    "    CEL_BuildHas(CelsNcurses);\n"
    "    CEL_BuildHas(Widgets);\n"
    "}\n"
    "\n"
    "CEL_Run(.title = \"%s\", .version = \"0.1.0\", .fps = 30, .root = AppUI);\n";

/* ============================================================================
 * Templates - Menu: features/menu/menu.h
 * ============================================================================ */

static const char* TMPL_MENU_FEATURE_H =
    "/*\n"
    " * Main Menu Feature\n"
    " *\n"
    " * Declares the MainMenu composition for cross-file use.\n"
    " * Implementation in menu.c.\n"
    " */\n"
    "#ifndef FEATURE_MENU_H\n"
    "#define FEATURE_MENU_H\n"
    "\n"
    "#include <cels/cels.h>\n"
    "\n"
    "CEL_Composition_Decl(MainMenu)\n"
    "#define MainMenu(...) CEL_Init(MainMenu, __VA_ARGS__)\n"
    "\n"
    "#endif /* FEATURE_MENU_H */\n";

/* ============================================================================
 * Templates - Menu: features/menu/menu.c
 * ============================================================================ */

static const char* TMPL_MENU_FEATURE_C =
    "/*\n"
    " * Main Menu Feature -- Implementation\n"
    " *\n"
    " * Contains the MainMenu composition, state, and systems.\n"
    " * All feature-local state stays private to this file.\n"
    " */\n"
    "#include \"menu.h\"\n"
    "#include <cels-widgets/compositions.h>\n"
    "\n"
    "CEL_Composition_Impl(MainMenu) {\n"
    "    (void)props;\n"
    "\n"
    "    Layout_VStack(.width = CEL_GROW(0), .height = CEL_GROW(0),\n"
    "                  .align_x = CEL_ALIGN_CENTER,\n"
    "                  .align_y = CEL_ALIGN_MIDDLE) {\n"
    "        Widget_Panel(.title = \"%s\") {\n"
    "            Widget_NavigationGroup(.wrap = true) {\n"
    "                Widget_Button(.label = \"Start\") {}\n"
    "                Widget_Button(.label = \"Settings\") {}\n"
    "                Widget_Button(.label = \"Quit\") {}\n"
    "            }\n"
    "        }\n"
    "        Widget_Hint(.text = \"[Up/Down: Navigate | Enter: Select | Q: Quit]\") {}\n"
    "    }\n"
    "}\n";

/* ============================================================================
 * Templates - Menu: features/settings/settings.h
 * ============================================================================ */

static const char* TMPL_SETTINGS_FEATURE_H =
    "/*\n"
    " * Settings Feature\n"
    " *\n"
    " * Declares the Settings composition for cross-file use.\n"
    " * Implementation in settings.c.\n"
    " */\n"
    "#ifndef FEATURE_SETTINGS_H\n"
    "#define FEATURE_SETTINGS_H\n"
    "\n"
    "#include <cels/cels.h>\n"
    "\n"
    "CEL_Composition_Decl(Settings)\n"
    "#define Settings(...) CEL_Init(Settings, __VA_ARGS__)\n"
    "\n"
    "#endif /* FEATURE_SETTINGS_H */\n";

/* ============================================================================
 * Templates - Menu: features/settings/settings.c
 * ============================================================================ */

static const char* TMPL_SETTINGS_FEATURE_C =
    "/*\n"
    " * Settings Feature -- Implementation\n"
    " *\n"
    " * Contains the Settings composition with toggles.\n"
    " * Feature-local state stays private to this file.\n"
    " */\n"
    "#include \"settings.h\"\n"
    "#include <cels-widgets/compositions.h>\n"
    "\n"
    "/* Private per-feature state */\n"
    "CEL_State(SettingsConfig, {\n"
    "    bool fullscreen;\n"
    "    bool vsync;\n"
    "});\n"
    "\n"
    "CEL_Composition_Impl(Settings) {\n"
    "    (void)props;\n"
    "    SettingsConfig_t* cfg = CEL_Watch(SettingsConfig);\n"
    "\n"
    "    Layout_VStack(.width = CEL_GROW(0), .height = CEL_GROW(0),\n"
    "                  .align_x = CEL_ALIGN_CENTER,\n"
    "                  .align_y = CEL_ALIGN_MIDDLE) {\n"
    "        Widget_Panel(.title = \"Settings\") {\n"
    "            Widget_NavigationGroup(.wrap = true) {\n"
    "                Widget_Toggle(.label = \"Fullscreen\", .value = cfg->fullscreen) {}\n"
    "                Widget_Toggle(.label = \"VSync\",      .value = cfg->vsync) {}\n"
    "                Widget_Button(.label = \"Back\") {}\n"
    "            }\n"
    "        }\n"
    "        Widget_Hint(.text = \"[Up/Down: Navigate | Enter: Toggle/Select | Q: Quit]\") {}\n"
    "    }\n"
    "}\n";

/* ============================================================================
 * Templates - Game: src/main.c
 * ============================================================================ */

static const char* TMPL_GAME_MAIN =
    "/*\n"
    " * %s -- A CELS Game\n"
    " *\n"
    " * Multi-file project using the features/ directory convention.\n"
    " * Game logic lives in features/gameplay/ with its own components\n"
    " * and systems.\n"
    " *\n"
    " * Controls: WASD to move, Q to quit.\n"
    " */\n"
    "\n"
    "#include <cels/cels.h>\n"
    "#include <cels-widgets/compositions.h>\n"
    "#include <cels-ncurses/backend.h>\n"
    "\n"
    "#include \"features/gameplay/gameplay.h\"\n"
    "\n"
    "CEL_Root(AppUI) {\n"
    "    (void)_props;\n"
    "    cels_entity_t window = CEL_Query(CEL_Window);\n"
    "    CEL_Window* win = CEL_Watch(window, CEL_Window);\n"
    "    if (win && win->ready) {\n"
    "        Layout_Surface(.width = (float)win->width / 2.0f,\n"
    "                       .height = (float)win->height) {\n"
    "            GameWorld() {}\n"
    "            Widget_StatusBar(.left  = \"%s\",\n"
    "                      .right = \"wasd:move  q:quit \") {}\n"
    "        }\n"
    "    }\n"
    "}\n"
    "\n"
    "CEL_BuildComposition(App) {\n"
    "    CEL_BuildHas(CelsEngine);\n"
    "    CEL_BuildHas(CelsNcurses);\n"
    "    CEL_BuildHas(Widgets);\n"
    "}\n"
    "\n"
    "CEL_Run(.title = \"%s\", .version = \"0.1.0\", .fps = 60, .root = AppUI);\n";

/* ============================================================================
 * Templates - Game: features/gameplay/gameplay.h
 * ============================================================================ */

static const char* TMPL_GAMEPLAY_FEATURE_H =
    "/*\n"
    " * Gameplay Feature\n"
    " *\n"
    " * Declares the GameWorld composition for cross-file use.\n"
    " * Implementation in gameplay.c.\n"
    " */\n"
    "#ifndef FEATURE_GAMEPLAY_H\n"
    "#define FEATURE_GAMEPLAY_H\n"
    "\n"
    "#include <cels/cels.h>\n"
    "\n"
    "CEL_Composition_Decl(GameWorld)\n"
    "#define GameWorld(...) CEL_Init(GameWorld, __VA_ARGS__)\n"
    "\n"
    "#endif /* FEATURE_GAMEPLAY_H */\n";

/* ============================================================================
 * Templates - Game: features/gameplay/gameplay.c
 * ============================================================================ */

static const char* TMPL_GAMEPLAY_FEATURE_C =
    "/*\n"
    " * Gameplay Feature -- Implementation\n"
    " *\n"
    " * Contains the GameWorld composition, game components, and systems.\n"
    " * All feature-local state stays private to this file.\n"
    " */\n"
    "#include \"gameplay.h\"\n"
    "#include <cels-widgets/compositions.h>\n"
    "#include <string.h>\n"
    "\n"
    "/* Game Components -- private to this feature */\n"
    "CEL_Component(Position, { float x; float y; });\n"
    "CEL_Component(Velocity, { float dx; float dy; });\n"
    "CEL_Component(Sprite,   { char ch[4]; int style_id; });\n"
    "\n"
    "/* Game State */\n"
    "CEL_State(GameState, {\n"
    "    int score;\n"
    "    int running;\n"
    "});\n"
    "\n"
    "/* Input System */\n"
    "static CELS_Input g_prev_input = {0};\n"
    "\n"
    "CEL_System(PlayerInputSystem, .phase = CELS_Phase_OnUpdate) {\n"
    "    (void)it;\n"
    "    CELS_Context* ctx = cels_get_context();\n"
    "    ecs_world_t* world = cels_get_world(ctx);\n"
    "    const CELS_Input* input = cels_input_get(ctx);\n"
    "    float dt = cels_get_delta_time(ctx);\n"
    "\n"
    "    if (PositionID == 0) goto done;\n"
    "\n"
    "    {\n"
    "        ecs_iter_t pit = ecs_each_id(world, PositionID);\n"
    "        while (ecs_each_next(&pit)) {\n"
    "            for (int i = 0; i < pit.count; i++) {\n"
    "                ecs_entity_t e = pit.entities[i];\n"
    "                Position* pos = (Position*)ecs_get_mut_id(world, e, PositionID);\n"
    "                if (!pos) continue;\n"
    "\n"
    "                if (input->axis_left[0] < -0.5f) pos->x -= 20.0f * dt;\n"
    "                if (input->axis_left[0] >  0.5f) pos->x += 20.0f * dt;\n"
    "                if (input->axis_left[1] < -0.5f) pos->y -= 10.0f * dt;\n"
    "                if (input->axis_left[1] >  0.5f) pos->y += 10.0f * dt;\n"
    "\n"
    "                if (pos->x < 0) pos->x = 0;\n"
    "                if (pos->y < 0) pos->y = 0;\n"
    "            }\n"
    "        }\n"
    "    }\n"
    "\n"
    "done:\n"
    "    memcpy((void*)&g_prev_input, input, sizeof(CELS_Input));\n"
    "}\n"
    "\n"
    "CEL_Composition_Impl(GameWorld) {\n"
    "    (void)props;\n"
    "    CEL_Use(PlayerInputSystem);\n"
    "\n"
    "    /* Spawn player entity */\n"
    "    CEL_Entity({.name = \"Player\"}) {\n"
    "        CEL_Has(Position, .x = 40.0f, .y = 12.0f);\n"
    "        CEL_Has(Velocity, .dx = 0, .dy = 0);\n"
    "        CEL_Has(Sprite,   .ch = \"@\", .style_id = 0);\n"
    "    }\n"
    "\n"
    "    /* TODO: add enemies, obstacles, rendering */\n"
    "}\n";

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

static int make_dir_exist_ok(const char* path) {
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: cannot create '%s': %s\n", path, strerror(errno));
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
 * Template selection helpers
 * ============================================================================ */

static const char* template_name(ScaffoldTemplate tmpl) {
    switch (tmpl) {
        case SCAFFOLD_HELLO: return "hello";
        case SCAFFOLD_MENU:  return "menu";
        case SCAFFOLD_GAME:  return "game";
        default:             return "hello";
    }
}

/* ============================================================================
 * Scaffold: Hello template (single-file)
 * ============================================================================ */

static int scaffold_hello(const char* project_name) {
    char path[512];

    /* Write CMakeLists.txt */
    snprintf(path, sizeof(path), "%s/CMakeLists.txt", project_name);
    if (write_file_fmt(path, TMPL_CMAKELISTS_SINGLE,
            project_name, project_name, project_name, project_name) != 0)
        return 1;

    /* Write src/main.c (4 format args: header, text, status left, title) */
    snprintf(path, sizeof(path), "%s/src/main.c", project_name);
    if (write_file_fmt(path, TMPL_MAIN_HELLO,
            project_name, project_name, project_name, project_name) != 0)
        return 1;

    return 0;
}

/* ============================================================================
 * Scaffold: Menu template (multi-file with features/)
 * ============================================================================ */

static int scaffold_menu(const char* project_name) {
    char path[512];

    /* Create feature directories */
    snprintf(path, sizeof(path), "%s/src/features", project_name);
    if (make_dir_exist_ok(path) != 0) return 1;

    snprintf(path, sizeof(path), "%s/src/features/menu", project_name);
    if (make_dir_exist_ok(path) != 0) return 1;

    snprintf(path, sizeof(path), "%s/src/features/settings", project_name);
    if (make_dir_exist_ok(path) != 0) return 1;

    /* Write CMakeLists.txt (5 format args: project x2, target, include, link, props) */
    snprintf(path, sizeof(path), "%s/CMakeLists.txt", project_name);
    if (write_file_fmt(path, TMPL_CMAKELISTS_MENU,
            project_name, project_name, project_name, project_name, project_name) != 0)
        return 1;

    /* Write src/main.c (2 format args: header, title) */
    snprintf(path, sizeof(path), "%s/src/main.c", project_name);
    if (write_file_fmt(path, TMPL_MENU_MAIN, project_name, project_name) != 0)
        return 1;

    /* Write features/menu/menu.h */
    snprintf(path, sizeof(path), "%s/src/features/menu/menu.h", project_name);
    if (write_file(path, TMPL_MENU_FEATURE_H) != 0) return 1;

    /* Write features/menu/menu.c (1 format arg: panel title) */
    snprintf(path, sizeof(path), "%s/src/features/menu/menu.c", project_name);
    if (write_file_fmt(path, TMPL_MENU_FEATURE_C, project_name) != 0) return 1;

    /* Write features/settings/settings.h */
    snprintf(path, sizeof(path), "%s/src/features/settings/settings.h", project_name);
    if (write_file(path, TMPL_SETTINGS_FEATURE_H) != 0) return 1;

    /* Write features/settings/settings.c */
    snprintf(path, sizeof(path), "%s/src/features/settings/settings.c", project_name);
    if (write_file(path, TMPL_SETTINGS_FEATURE_C) != 0) return 1;

    return 0;
}

/* ============================================================================
 * Scaffold: Game template (multi-file with features/)
 * ============================================================================ */

static int scaffold_game(const char* project_name) {
    char path[512];

    /* Create feature directories */
    snprintf(path, sizeof(path), "%s/src/features", project_name);
    if (make_dir_exist_ok(path) != 0) return 1;

    snprintf(path, sizeof(path), "%s/src/features/gameplay", project_name);
    if (make_dir_exist_ok(path) != 0) return 1;

    /* Write CMakeLists.txt (5 format args) */
    snprintf(path, sizeof(path), "%s/CMakeLists.txt", project_name);
    if (write_file_fmt(path, TMPL_CMAKELISTS_GAME,
            project_name, project_name, project_name, project_name, project_name) != 0)
        return 1;

    /* Write src/main.c (3 format args: header, status left, title) */
    snprintf(path, sizeof(path), "%s/src/main.c", project_name);
    if (write_file_fmt(path, TMPL_GAME_MAIN,
            project_name, project_name, project_name) != 0)
        return 1;

    /* Write features/gameplay/gameplay.h */
    snprintf(path, sizeof(path), "%s/src/features/gameplay/gameplay.h", project_name);
    if (write_file(path, TMPL_GAMEPLAY_FEATURE_H) != 0) return 1;

    /* Write features/gameplay/gameplay.c */
    snprintf(path, sizeof(path), "%s/src/features/gameplay/gameplay.c", project_name);
    if (write_file(path, TMPL_GAMEPLAY_FEATURE_C) != 0) return 1;

    return 0;
}

/* ============================================================================
 * cli_scaffold_project_with_template
 * ============================================================================ */

int cli_scaffold_project_with_template(const char* project_name, ScaffoldTemplate tmpl) {
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

    /* Scaffold based on template */
    int rc = 0;
    switch (tmpl) {
        case SCAFFOLD_HELLO: rc = scaffold_hello(project_name); break;
        case SCAFFOLD_MENU:  rc = scaffold_menu(project_name);  break;
        case SCAFFOLD_GAME:  rc = scaffold_game(project_name);  break;
        default:             rc = scaffold_hello(project_name); break;
    }
    if (rc != 0) return rc;

    /* Write .gitignore */
    char path[512];
    snprintf(path, sizeof(path), "%s/.gitignore", project_name);
    if (write_file(path, TMPL_GITIGNORE) != 0) return 1;

    /* Print success message */
    printf("Created CELS project '%s' (template: %s)\n", project_name, template_name(tmpl));
    printf("\n");
    printf("  cd %s\n", project_name);
    printf("  cmake -B build\n");
    printf("  cmake --build build\n");
    printf("  ./build/%s\n", project_name);
    printf("\n");

    return 0;
}

/* ============================================================================
 * cli_scaffold_project (default = hello)
 * ============================================================================ */

int cli_scaffold_project(const char* project_name) {
    return cli_scaffold_project_with_template(project_name, SCAFFOLD_HELLO);
}
