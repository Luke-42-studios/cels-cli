/*
 * cels-cli App -- Root composition, input system, state management
 *
 * Tab-based TUI with Projects | Packages | Build | Tests.
 * Input via arrow keys, number keys, Tab/Shift-Tab.
 * Status bar shows version and current working directory.
 *
 * Rendering is handled by Clay layout + clay_ncurses_renderer.
 * This file only defines compositions and state -- no raw ncurses code.
 */

#define _POSIX_C_SOURCE 200809L
#include "app.h"
#include "components.h"
#include <cels-ncurses/tui_engine.h>
#include <cels-clay/clay_engine.h>
#include <cels-clay/clay_ncurses_renderer.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

/* ============================================================================
 * Tab Names & Placeholders
 * ============================================================================ */

static const char* tab_labels[TAB_COUNT] = {
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
 * Compositions
 * ============================================================================ */

#define TabBarView(...) CEL_Init(TabBarView, __VA_ARGS__)
CEL_Composition(TabBarView, CliTab active;) {
    Widget_TabBar(.active = props.active,
            .count = TAB_COUNT,
            .labels = tab_labels) {}
}

#define TabContentView(...) CEL_Init(TabContentView, __VA_ARGS__)
CEL_Composition(TabContentView, CliTab tab;) {
    Widget_TabContent(.text = tab_placeholders[props.tab],
                .hint = tab_hints[props.tab]) {}
}

static char g_cwd[512] = {0};
static char g_status_left[600] = {0};

#define StatusBarView(...) CEL_Init(StatusBarView, __VA_ARGS__)
CEL_Composition(StatusBarView) {
    (void)props;
    Widget_StatusBar(.left = g_status_left,
               .right = "1-4:tab  </>:switch  q:quit ") {}
}

/* ============================================================================
 * Root Composition
 * ============================================================================ */

CEL_Root(AppUI, Engine_Context) {
    Engine_WindowState_t* win = CEL_WatchId(ctx.windowState, Engine_WindowState_t);

    if (win->state == WINDOW_STATE_READY) {
        AppState_t* app = CEL_Watch(AppState);

        CEL_Use(TabInputSystem);

        Layout_Surface(.width = (float)win->width / 2.0f,
                    .height = (float)win->height) {
            TabBarView(.active = app->active_tab) {}
            TabContentView(.tab = app->active_tab) {}
            StatusBarView() {}
        }
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

    snprintf(g_status_left, sizeof(g_status_left), "cels-cli v0.1.0 | %s",
             g_cwd[0] ? g_cwd : "unknown");
}

/* ============================================================================
 * CEL_Build -- TUI Application Setup
 * ============================================================================ */

CEL_Build(CelsCLI) {
    (void)props;

    Widget_init();
    Engine_use((Engine_Config){
        .title = "CELS CLI",
        .version = "0.1.0",
        .fps = 30,
        .root = AppUI
    });
    Clay_Engine_use(NULL);
    clay_ncurses_renderer_init(NULL);

    cli_app_init();
}
