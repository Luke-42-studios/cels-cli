/*
 * cels-cli Scaffold
 *
 * Generates a new CELS project directory with CMakeLists.txt,
 * src/main.c, and .gitignore. All generated files are self-contained
 * and compile immediately via cmake.
 *
 * Templates:
 *   hello (default) - Minimal hello world with centered text
 *   menu            - Menu + settings screen with navigation
 *   game            - Game skeleton with Position/Velocity/Sprite
 */

#include "scaffold.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* ============================================================================
 * Templates - CMakeLists.txt
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
    "cels_require(widgets)\n"
    "\n"
    "add_executable(%s src/main.c)\n"
    "target_link_libraries(%s PRIVATE cels cels-ncurses cels-widgets)\n"
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
 * Templates - Hello World (default)
 * ============================================================================ */

static const char* TMPL_MAIN_HELLO =
    "/*\n"
    " * %s -- A CELS Application\n"
    " */\n"
    "\n"
    "#define _POSIX_C_SOURCE 200809L\n"
    "#include <cels/cels.h>\n"
    "#include <cels-ncurses/tui_engine.h>\n"
    "#include <cels-ncurses/tui_widgets.h>\n"
    "#include <cels-widgets/widgets.h>\n"
    "\n"
    "/* Content area: centered greeting */\n"
    "#define HelloContent(...) CEL_Init(HelloContent, __VA_ARGS__)\n"
    "CEL_Composition(HelloContent) {\n"
    "    (void)props;\n"
    "    CEL_Has(W_TabContent,\n"
    "        .text = \"%s\",\n"
    "        .hint = \"Press q to quit\"\n"
    "    );\n"
    "}\n"
    "\n"
    "/* Status bar */\n"
    "#define AppStatus(...) CEL_Init(AppStatus, __VA_ARGS__)\n"
    "CEL_Composition(AppStatus) {\n"
    "    (void)props;\n"
    "    CEL_Has(W_StatusBar,\n"
    "        .left  = \"%s v0.1.0\",\n"
    "        .right = \"q:quit \"\n"
    "    );\n"
    "}\n"
    "\n"
    "CEL_Root(AppUI, TUI_EngineContext) {\n"
    "    TUI_WindowState_t* win = CEL_WatchId(ctx.windowState, TUI_WindowState_t);\n"
    "    if (win->state == WINDOW_STATE_READY) {\n"
    "        HelloContent() {}\n"
    "        AppStatus() {}\n"
    "    }\n"
    "}\n"
    "\n"
    "CEL_Build(App) {\n"
    "    (void)props;\n"
    "    Widgets_init();\n"
    "    CEL_Backend(TUI_Engine,\n"
    "        .title   = \"%s\",\n"
    "        .version = \"0.1.0\",\n"
    "        .fps     = 30,\n"
    "        .root    = AppUI\n"
    "    );\n"
    "    tui_widgets_register();\n"
    "}\n";

/* ============================================================================
 * Templates - Menu + Settings
 * ============================================================================ */

static const char* TMPL_MAIN_MENU =
    "/*\n"
    " * %s -- A CELS Menu Application\n"
    " *\n"
    " * Demonstrates menu navigation with settings screen.\n"
    " * Controls: W/S to navigate, Enter to select, Q to quit.\n"
    " */\n"
    "\n"
    "#define _POSIX_C_SOURCE 200809L\n"
    "#include <cels/cels.h>\n"
    "#include <cels-ncurses/tui_engine.h>\n"
    "#include <cels-ncurses/tui_widgets.h>\n"
    "#include <cels-widgets/widgets.h>\n"
    "#include <stdbool.h>\n"
    "#include <string.h>\n"
    "\n"
    "/* ============================================================================\n"
    " * State\n"
    " * ============================================================================ */\n"
    "\n"
    "typedef enum { SCREEN_MAIN_MENU = 0, SCREEN_SETTINGS = 1 } ScreenType;\n"
    "\n"
    "CEL_State(MenuState, {\n"
    "    ScreenType screen;\n"
    "    int selected;\n"
    "    int item_count;\n"
    "});\n"
    "\n"
    "CEL_State(SettingsConfig, {\n"
    "    bool fullscreen;\n"
    "    bool vsync;\n"
    "});\n"
    "\n"
    "/* ============================================================================\n"
    " * Lifecycles\n"
    " * ============================================================================ */\n"
    "\n"
    "CEL_Observer(MainMenuVisible, MenuState) {\n"
    "    return MenuState.screen == SCREEN_MAIN_MENU;\n"
    "}\n"
    "\n"
    "CEL_Observer(SettingsVisible, MenuState) {\n"
    "    return MenuState.screen == SCREEN_SETTINGS;\n"
    "}\n"
    "\n"
    "CEL_Observer(SettingsInactive, MenuState) {\n"
    "    return MenuState.screen != SCREEN_SETTINGS;\n"
    "}\n"
    "\n"
    "CEL_Lifecycle(MainMenuLC, .systemVisibility = &MainMenuVisible);\n"
    "CEL_Lifecycle(SettingsLC, .systemVisibility = &SettingsVisible, .destroy = &SettingsInactive);\n"
    "\n"
    "/* ============================================================================\n"
    " * Input Systems\n"
    " * ============================================================================ */\n"
    "\n"
    "static CELS_Input g_prev_input = {0};\n"
    "\n"
    "CEL_System(MainMenuInputSystem, .phase = CELS_Phase_OnUpdate) {\n"
    "    (void)it;\n"
    "    CELS_Context* ctx = cels_get_context();\n"
    "    const CELS_Input* input = cels_input_get(ctx);\n"
    "\n"
    "    if (input->axis_left[1] < -0.5f && g_prev_input.axis_left[1] >= -0.5f) {\n"
    "        CEL_Update(MenuState) {\n"
    "            MenuState.selected--;\n"
    "            if (MenuState.selected < 0)\n"
    "                MenuState.selected = MenuState.item_count - 1;\n"
    "        }\n"
    "    }\n"
    "\n"
    "    if (input->axis_left[1] > 0.5f && g_prev_input.axis_left[1] <= 0.5f) {\n"
    "        CEL_Update(MenuState) {\n"
    "            MenuState.selected++;\n"
    "            if (MenuState.selected >= MenuState.item_count)\n"
    "                MenuState.selected = 0;\n"
    "        }\n"
    "    }\n"
    "\n"
    "    if (input->button_accept && !g_prev_input.button_accept) {\n"
    "        switch (MenuState.selected) {\n"
    "            case 0: break; /* Start -- placeholder */\n"
    "            case 1:\n"
    "                CEL_Update(MenuState) {\n"
    "                    MenuState.screen = SCREEN_SETTINGS;\n"
    "                    MenuState.selected = 0;\n"
    "                    MenuState.item_count = 3;\n"
    "                }\n"
    "                break;\n"
    "            case 2: break; /* Quit -- Q key */\n"
    "        }\n"
    "    }\n"
    "\n"
    "    memcpy((void*)&g_prev_input, input, sizeof(CELS_Input));\n"
    "}\n"
    "\n"
    "CEL_System(SettingsInputSystem, .phase = CELS_Phase_OnUpdate) {\n"
    "    (void)it;\n"
    "    CELS_Context* ctx = cels_get_context();\n"
    "    const CELS_Input* input = cels_input_get(ctx);\n"
    "\n"
    "    if (input->axis_left[1] < -0.5f && g_prev_input.axis_left[1] >= -0.5f) {\n"
    "        CEL_Update(MenuState) {\n"
    "            MenuState.selected--;\n"
    "            if (MenuState.selected < 0)\n"
    "                MenuState.selected = MenuState.item_count - 1;\n"
    "        }\n"
    "    }\n"
    "\n"
    "    if (input->axis_left[1] > 0.5f && g_prev_input.axis_left[1] <= 0.5f) {\n"
    "        CEL_Update(MenuState) {\n"
    "            MenuState.selected++;\n"
    "            if (MenuState.selected >= MenuState.item_count)\n"
    "                MenuState.selected = 0;\n"
    "        }\n"
    "    }\n"
    "\n"
    "    if (input->axis_left[0] < -0.5f && g_prev_input.axis_left[0] >= -0.5f) {\n"
    "        CEL_Update(SettingsConfig) {\n"
    "            switch (MenuState.selected) {\n"
    "                case 0: SettingsConfig.fullscreen = !SettingsConfig.fullscreen; break;\n"
    "                case 1: SettingsConfig.vsync = !SettingsConfig.vsync; break;\n"
    "            }\n"
    "        }\n"
    "    }\n"
    "    if (input->axis_left[0] > 0.5f && g_prev_input.axis_left[0] >= 0.5f) {\n"
    "        CEL_Update(SettingsConfig) {\n"
    "            switch (MenuState.selected) {\n"
    "                case 0: SettingsConfig.fullscreen = !SettingsConfig.fullscreen; break;\n"
    "                case 1: SettingsConfig.vsync = !SettingsConfig.vsync; break;\n"
    "            }\n"
    "        }\n"
    "    }\n"
    "\n"
    "    if ((input->button_accept && !g_prev_input.button_accept && MenuState.selected == 2) ||\n"
    "        (input->button_cancel && !g_prev_input.button_cancel)) {\n"
    "        CEL_Update(MenuState) {\n"
    "            MenuState.screen = SCREEN_MAIN_MENU;\n"
    "            MenuState.selected = 1;\n"
    "            MenuState.item_count = 3;\n"
    "        }\n"
    "    }\n"
    "\n"
    "    memcpy((void*)&g_prev_input, input, sizeof(CELS_Input));\n"
    "}\n"
    "\n"
    "/* ============================================================================\n"
    " * Widget Wrappers\n"
    " * ============================================================================ */\n"
    "\n"
    "#define WButton(...) CEL_Init(WButton, __VA_ARGS__)\n"
    "CEL_Composition(WButton, const char* label; bool selected;) {\n"
    "    CEL_Has(W_Button, .label = props.label, .selected = props.selected);\n"
    "}\n"
    "\n"
    "#define WToggle(...) CEL_Init(WToggle, __VA_ARGS__)\n"
    "CEL_Composition(WToggle, const char* label; bool value; bool selected;) {\n"
    "    CEL_Has(W_Toggle, .label = props.label, .value = props.value, .selected = props.selected);\n"
    "}\n"
    "\n"
    "/* ============================================================================\n"
    " * Compositions\n"
    " * ============================================================================ */\n"
    "\n"
    "#define MainMenu(...) CEL_Init(MainMenu, __VA_ARGS__)\n"
    "CEL_Composition(MainMenu) {\n"
    "    (void)props;\n"
    "    CEL_Use(MainMenuInputSystem);\n"
    "    MenuState_t* menu = CEL_Watch(MenuState);\n"
    "\n"
    "    CEL_Has(W_Canvas, .title = \"%s\");\n"
    "    WButton(.label = \"Start\",    .selected = (menu->selected == 0)) {}\n"
    "    WButton(.label = \"Settings\", .selected = (menu->selected == 1)) {}\n"
    "    WButton(.label = \"Quit\",     .selected = (menu->selected == 2)) {}\n"
    "    CEL_Has(W_Hint, .text = \"[W/S: Navigate | Enter: Select | Q: Quit]\");\n"
    "    CEL_Has(W_StatusBar, .left = \"%s v0.1.0\", .right = \"q:quit \");\n"
    "}\n"
    "\n"
    "#define SettingsMenu(...) CEL_Init(SettingsMenu, __VA_ARGS__)\n"
    "CEL_Composition(SettingsMenu) {\n"
    "    (void)props;\n"
    "    CEL_Use(SettingsInputSystem);\n"
    "    MenuState_t* menu = CEL_Watch(MenuState);\n"
    "    SettingsConfig_t* cfg = CEL_Watch(SettingsConfig);\n"
    "\n"
    "    CEL_Has(W_Canvas, .title = \"Settings\");\n"
    "    WToggle(.label = \"Fullscreen\", .value = cfg->fullscreen, .selected = (menu->selected == 0)) {}\n"
    "    WToggle(.label = \"VSync\",      .value = cfg->vsync,      .selected = (menu->selected == 1)) {}\n"
    "    WButton(.label = \"Back\",       .selected = (menu->selected == 2)) {}\n"
    "    CEL_Has(W_Hint, .text = \"[W/S: Navigate | A/D: Toggle | Enter/Esc: Back]\");\n"
    "    CEL_Has(W_StatusBar, .left = \"settings\", .right = \"esc:back \");\n"
    "}\n"
    "\n"
    "#define MenuRouter(...) CEL_Init(MenuRouter, __VA_ARGS__)\n"
    "CEL_Composition(MenuRouter) {\n"
    "    (void)props;\n"
    "    MenuState_t* menu = CEL_Watch(MenuState);\n"
    "    if (menu->screen == SCREEN_MAIN_MENU) {\n"
    "        MainMenu(.lifecycle = MainMenuLC) {}\n"
    "    } else if (menu->screen == SCREEN_SETTINGS) {\n"
    "        SettingsMenu(.lifecycle = SettingsLC) {}\n"
    "    }\n"
    "}\n"
    "\n"
    "CEL_Root(AppUI, TUI_EngineContext) {\n"
    "    TUI_WindowState_t* win = CEL_WatchId(ctx.windowState, TUI_WindowState_t);\n"
    "    if (win->state == WINDOW_STATE_READY) {\n"
    "        MenuRouter() {}\n"
    "    }\n"
    "}\n"
    "\n"
    "CEL_Build(App) {\n"
    "    (void)props;\n"
    "    MenuState = (MenuState_t){ .screen = SCREEN_MAIN_MENU, .selected = 0, .item_count = 3 };\n"
    "    SettingsConfig = (SettingsConfig_t){ .fullscreen = true, .vsync = true };\n"
    "\n"
    "    Widgets_init();\n"
    "    CEL_Backend(TUI_Engine,\n"
    "        .title   = \"%s\",\n"
    "        .version = \"0.1.0\",\n"
    "        .fps     = 30,\n"
    "        .root    = AppUI\n"
    "    );\n"
    "    tui_widgets_register();\n"
    "}\n";

/* ============================================================================
 * Templates - Game Skeleton
 * ============================================================================ */

static const char* TMPL_MAIN_GAME =
    "/*\n"
    " * %s -- A CELS Game\n"
    " *\n"
    " * Game skeleton with Position, Velocity, and Sprite components.\n"
    " * Controls: WASD to move, Q to quit.\n"
    " */\n"
    "\n"
    "#define _POSIX_C_SOURCE 200809L\n"
    "#include <cels/cels.h>\n"
    "#include <cels-ncurses/tui_engine.h>\n"
    "#include <cels-ncurses/tui_widgets.h>\n"
    "#include <cels-widgets/widgets.h>\n"
    "#include <string.h>\n"
    "\n"
    "/* ============================================================================\n"
    " * Game Components\n"
    " * ============================================================================ */\n"
    "\n"
    "CEL_Component(Position, { float x; float y; });\n"
    "CEL_Component(Velocity, { float dx; float dy; });\n"
    "CEL_Component(Sprite,   { char ch[4]; int style_id; });\n"
    "\n"
    "/* ============================================================================\n"
    " * State\n"
    " * ============================================================================ */\n"
    "\n"
    "CEL_State(GameState, {\n"
    "    int score;\n"
    "    int running;\n"
    "});\n"
    "\n"
    "/* ============================================================================\n"
    " * Input System\n"
    " * ============================================================================ */\n"
    "\n"
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
    "/* ============================================================================\n"
    " * Game World Composition\n"
    " * ============================================================================ */\n"
    "\n"
    "#define GameWorld(...) CEL_Init(GameWorld, __VA_ARGS__)\n"
    "CEL_Composition(GameWorld) {\n"
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
    "    /* TODO: add enemies, obstacles, etc. */\n"
    "}\n"
    "\n"
    "/* Status bar */\n"
    "#define GameStatus(...) CEL_Init(GameStatus, __VA_ARGS__)\n"
    "CEL_Composition(GameStatus) {\n"
    "    (void)props;\n"
    "    GameState_t* gs = CEL_Watch(GameState);\n"
    "    (void)gs;\n"
    "    CEL_Has(W_StatusBar,\n"
    "        .left  = \"%s\",\n"
    "        .right = \"wasd:move  q:quit \"\n"
    "    );\n"
    "}\n"
    "\n"
    "CEL_Root(AppUI, TUI_EngineContext) {\n"
    "    TUI_WindowState_t* win = CEL_WatchId(ctx.windowState, TUI_WindowState_t);\n"
    "    if (win->state == WINDOW_STATE_READY) {\n"
    "        GameWorld() {}\n"
    "        GameStatus() {}\n"
    "    }\n"
    "}\n"
    "\n"
    "CEL_Build(App) {\n"
    "    (void)props;\n"
    "    GameState = (GameState_t){ .score = 0, .running = 1 };\n"
    "\n"
    "    Widgets_init();\n"
    "    CEL_Backend(TUI_Engine,\n"
    "        .title   = \"%s\",\n"
    "        .version = \"0.1.0\",\n"
    "        .fps     = 60,\n"
    "        .root    = AppUI\n"
    "    );\n"
    "    tui_widgets_register();\n"
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

/* Returns the format string for the selected template's main.c */
static const char* template_main(ScaffoldTemplate tmpl) {
    switch (tmpl) {
        case SCAFFOLD_HELLO: return TMPL_MAIN_HELLO;
        case SCAFFOLD_MENU:  return TMPL_MAIN_MENU;
        case SCAFFOLD_GAME:  return TMPL_MAIN_GAME;
        default:             return TMPL_MAIN_HELLO;
    }
}

/* Returns the number of %s format args in each template's main.c */
static int template_arg_count(ScaffoldTemplate tmpl) {
    switch (tmpl) {
        case SCAFFOLD_HELLO: return 4; /* header, text, status left, build title */
        case SCAFFOLD_MENU:  return 4; /* header, canvas title, status left, build title */
        case SCAFFOLD_GAME:  return 3; /* header, status left, build title */
        default:             return 4;
    }
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

    /* Write CMakeLists.txt */
    char path[512];
    snprintf(path, sizeof(path), "%s/CMakeLists.txt", project_name);
    if (write_file_fmt(path, TMPL_CMAKELISTS,
            project_name, project_name, project_name, project_name) != 0)
        return 1;

    /* Write src/main.c based on template */
    snprintf(path, sizeof(path), "%s/src/main.c", project_name);
    int argc = template_arg_count(tmpl);
    const char* tmpl_str = template_main(tmpl);

    if (argc == 4) {
        if (write_file_fmt(path, tmpl_str,
                project_name, project_name, project_name, project_name) != 0)
            return 1;
    } else {
        /* 3 args: header comment, one middle use, build title */
        if (write_file_fmt(path, tmpl_str,
                project_name, project_name, project_name) != 0)
            return 1;
    }

    /* Write .gitignore */
    snprintf(path, sizeof(path), "%s/.gitignore", project_name);
    if (write_file(path, TMPL_GITIGNORE) != 0) return 1;

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
