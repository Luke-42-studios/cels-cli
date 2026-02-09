/*
 * cels-cli -- Interactive TUI toolkit for the CELS framework
 *
 * Built with CELS itself using cels-ncurses backend.
 * Tab-based navigation: Projects | Packages | Build | Tests
 */

#include "components.h"
#include "render_provider.h"
#include <cels-ncurses/tui_engine.h>
#include <flecs.h>
#include <stdbool.h>
#include <string.h>

/* ============================================================================
 * State
 * ============================================================================ */

CEL_State(AppState, {
    CliTab active_tab;
});

/* ============================================================================
 * Input System -- Tab switching via number keys and Tab key
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

    /* Tab key cycles to next tab */
    if (input->key_tab && !g_prev_input.key_tab) {
        CEL_Update(AppState) {
            AppState.active_tab = (CliTab)((AppState.active_tab + 1) % TAB_COUNT);
        }
    }

    /* Shift+Tab cycles to previous tab */
    if (input->key_shift_tab && !g_prev_input.key_shift_tab) {
        CEL_Update(AppState) {
            AppState.active_tab = (CliTab)((AppState.active_tab + TAB_COUNT - 1) % TAB_COUNT);
        }
    }

    memcpy((void*)&g_prev_input, input, sizeof(CELS_Input));
}

/* ============================================================================
 * Placeholder text for each tab
 * ============================================================================ */

static const char* tab_placeholders[TAB_COUNT] = {
    "[ Projects ]",
    "[ Packages ]",
    "[ Build ]",
    "[ Tests ]"
};

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

#define StatusBarView(...) CEL_Init(StatusBarView, __VA_ARGS__)
CEL_Composition(StatusBarView) {
    (void)props;
    CEL_Has(StatusBar,
        .version = "cels-cli v0.1.0",
        .project_path = NULL
    );
}

/* ============================================================================
 * Root Composition
 * ============================================================================ */

CEL_Root(CliUI, TUI_EngineContext) {
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
 * Init
 * ============================================================================ */

static void InitConfig(void) {
    AppState = (AppState_t){
        .active_tab = TAB_PROJECTS
    };
}

/* ============================================================================
 * Entry Point
 * ============================================================================ */

CEL_Build(CLI) {
    (void)props;

    TUI_Engine_use((TUI_EngineConfig){
        .title = "cels-cli",
        .version = "0.1.0",
        .fps = 30,
        .root = CliUI
    });

    cli_renderer_init();
    InitConfig();
}
