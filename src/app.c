/*
 * cels-cli App -- Root composition, input system, renderer
 *
 * Tab-based TUI with Projects | Packages | Build | Tests.
 * Input via arrow keys, number keys, Tab/Shift-Tab.
 * Status bar shows version and current working directory.
 */

#define _POSIX_C_SOURCE 200809L
#include "app.h"
#include "components.h"
#include <cels-ncurses/tui_engine.h>
#include <cels-ncurses/tui_window.h>
#include <flecs.h>
#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

/* ============================================================================
 * Feature Definition
 * ============================================================================ */

_CEL_DefineFeature(Renderable, .phase = CELS_Phase_OnRender);

/* ============================================================================
 * Tab Names & Placeholders
 * ============================================================================ */

static const char* tab_names[TAB_COUNT] = {
    "Projects",
    "Packages",
    "Build",
    "Tests"
};

static const char* tab_placeholders[TAB_COUNT] = {
    "Projects tab",
    "Packages tab",
    "Build tab",
    "Tests tab"
};

static const char* tab_hints[TAB_COUNT] = {
    "Create and manage CELS projects",
    "Browse and install CELS modules",
    "Build and run your project",
    "Discover and run test suites"
};

/* ============================================================================
 * State
 * ============================================================================ */

CEL_State(AppState, {
    CliTab active_tab;
});

/* ============================================================================
 * Input System
 * ============================================================================ */

static CELS_Input g_prev_input = {0};

CEL_System(TabInputSystem,
    .phase = CELS_Phase_OnUpdate
) {
    (void)it;
    CELS_Context* ctx = cels_get_context();
    const CELS_Input* input = cels_input_get(ctx);

    /* Number keys 1-4 switch tabs directly */
    if (input->has_number && !g_prev_input.has_number) {
        int num = input->key_number;
        if (num >= 1 && num <= TAB_COUNT) {
            CEL_Update(AppState) {
                AppState.active_tab = (CliTab)(num - 1);
            }
        }
    }

    /* Right arrow / Tab: next tab */
    if ((input->axis_left[0] > 0.5f && g_prev_input.axis_left[0] <= 0.5f) ||
        (input->key_tab && !g_prev_input.key_tab)) {
        CEL_Update(AppState) {
            AppState.active_tab = (CliTab)((AppState.active_tab + 1) % TAB_COUNT);
        }
    }

    /* Left arrow / Shift-Tab: previous tab */
    if ((input->axis_left[0] < -0.5f && g_prev_input.axis_left[0] >= -0.5f) ||
        (input->key_shift_tab && !g_prev_input.key_shift_tab)) {
        CEL_Update(AppState) {
            AppState.active_tab = (CliTab)((AppState.active_tab + TAB_COUNT - 1) % TAB_COUNT);
        }
    }

    memcpy((void*)&g_prev_input, input, sizeof(CELS_Input));
}

/* ============================================================================
 * Render Callbacks
 * ============================================================================ */

static void render_tab_bar(CELS_Iter* it) {
    int count = cels_iter_count(it);
    TabBar* bars = (TabBar*)cels_iter_column(it, TabBarID, sizeof(TabBar));
    if (!bars || count == 0) return;

    TabBar* bar = &bars[0];

    attron(A_BOLD);
    move(0, 0);
    clrtoeol();

    int x = 1;
    for (int i = 0; i < bar->count && i < TAB_COUNT; i++) {
        if (i == (int)bar->active) {
            attron(A_REVERSE);
        }

        const char* name = tab_names[i];
        int label_len = (int)strlen(name);

        if (x + label_len + 5 < COLS) {
            mvprintw(0, x, " %d:%s ", i + 1, name);
            x += label_len + 5;
        }

        if (i == (int)bar->active) {
            attroff(A_REVERSE);
        }

        if (i < bar->count - 1 && x < COLS) {
            mvaddch(0, x, ACS_VLINE);
            x += 1;
        }
    }

    attroff(A_BOLD);
}

static void render_tab_content(CELS_Iter* it) {
    int count = cels_iter_count(it);
    TabContent* contents = (TabContent*)cels_iter_column(it, TabContentID, sizeof(TabContent));
    if (!contents || count == 0) return;

    TabContent* content = &contents[0];
    int center_y = LINES / 2;

    /* Centered placeholder */
    int text_len = (int)strlen(content->placeholder_text);
    int center_x = (COLS - text_len) / 2;
    if (center_x < 0) center_x = 0;

    attron(A_DIM);
    mvprintw(center_y, center_x, "%s", content->placeholder_text);
    attroff(A_DIM);

    /* Hint below */
    if (content->tab < TAB_COUNT) {
        const char* hint = tab_hints[content->tab];
        int hint_len = (int)strlen(hint);
        int hint_x = (COLS - hint_len) / 2;
        if (hint_x < 0) hint_x = 0;
        mvprintw(center_y + 2, hint_x, "%s", hint);
    }
}

static void render_status_bar(CELS_Iter* it) {
    int count = cels_iter_count(it);
    StatusBar* bars = (StatusBar*)cels_iter_column(it, StatusBarID, sizeof(StatusBar));
    if (!bars || count == 0) return;

    StatusBar* bar = &bars[0];
    int y = LINES - 1;

    attron(A_REVERSE);
    move(y, 0);
    clrtoeol();

    /* Left: version + cwd */
    if (bar->cwd) {
        mvprintw(y, 1, " %s | %s ",
            bar->version ? bar->version : "cels-cli",
            bar->cwd);
    } else {
        mvprintw(y, 1, " %s ", bar->version ? bar->version : "cels-cli");
    }

    /* Right: keybind hints */
    const char* help = "1-4:tab  </>:switch  q:quit ";
    int help_len = (int)strlen(help);
    if (COLS > help_len + 2) {
        mvprintw(y, COLS - help_len - 1, "%s", help);
    }

    attroff(A_REVERSE);
}

/* ============================================================================
 * Provider Registration
 * ============================================================================ */

static void cli_renderer_init(void) {
    _CEL_Feature(TabBar, Renderable);
    _CEL_Feature(TabContent, Renderable);
    _CEL_Feature(StatusBar, Renderable);

    _CEL_Provides(TUI, Renderable, TabBar, render_tab_bar);
    _CEL_Provides(TUI, Renderable, TabContent, render_tab_content);
    _CEL_Provides(TUI, Renderable, StatusBar, render_status_bar);

    _CEL_ProviderConsumes(TabBar, TabContent, StatusBar);
}

/* ============================================================================
 * Compositions
 * ============================================================================ */

#define TabBarView(...) CEL_Init(TabBarView, __VA_ARGS__)
CEL_Composition(TabBarView, CliTab active;) {
    CEL_Has(TabBar, .active = props.active, .count = TAB_COUNT);
}

#define TabContentView(...) CEL_Init(TabContentView, __VA_ARGS__)
CEL_Composition(TabContentView, CliTab tab;) {
    CEL_Has(TabContent,
        .tab = props.tab,
        .placeholder_text = tab_placeholders[props.tab]
    );
}

static char g_cwd[512] = {0};

#define StatusBarView(...) CEL_Init(StatusBarView, __VA_ARGS__)
CEL_Composition(StatusBarView) {
    (void)props;
    CEL_Has(StatusBar,
        .version = "cels-cli v0.1.0",
        .cwd = g_cwd[0] ? g_cwd : NULL
    );
}

/* ============================================================================
 * Root Composition
 * ============================================================================ */

CEL_Root(AppUI, TUI_EngineContext) {
    TUI_WindowState_t* win = CEL_WatchId(ctx.windowState, TUI_WindowState_t);

    if (win->state == WINDOW_STATE_READY) {
        AppState_t* app = CEL_Watch(AppState);

        CEL_Use(TabInputSystem);

        TabBarView(.active = app->active_tab) {}
        TabContentView(.tab = app->active_tab) {}
        StatusBarView() {}
    }
}

/* ============================================================================
 * App Init
 * ============================================================================ */

static void cli_app_init(void) {
    AppState = (AppState_t){
        .active_tab = TAB_PROJECTS
    };

    if (getcwd(g_cwd, sizeof(g_cwd)) == NULL) {
        g_cwd[0] = '\0';
    }
}

/* ============================================================================
 * CEL_Build -- TUI Application Setup
 * ============================================================================
 *
 * Generates CELS_BuildInit() which main.c calls in TUI mode.
 * Must be in this file because AppUI is static (from CEL_Root).
 */

CEL_Build(CelsCLI) {
    (void)props;

    TUI_Engine_use((TUI_EngineConfig){
        .title = "CELS CLI",
        .version = "0.1.0",
        .fps = 30,
        .root = AppUI
    });

    cli_renderer_init();
    cli_app_init();
}
