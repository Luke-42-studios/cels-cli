/*
 * cels-cli Render Provider
 *
 * TUI rendering callbacks for CLI components using the CELS
 * feature/provider system. Renders TabBar, TabContent, and StatusBar
 * via ncurses.
 */

#ifndef CELS_CLI_RENDER_PROVIDER_H
#define CELS_CLI_RENDER_PROVIDER_H

#include "components.h"
#include <cels-ncurses/tui_window.h>
#include <cels-ncurses/tui_draw.h>
#include <ncurses.h>
#include <string.h>

/* ============================================================================
 * Feature Definition
 * ============================================================================ */

_CEL_DefineFeature(Renderable, .phase = CELS_Phase_OnRender);

/* ============================================================================
 * Tab Names
 * ============================================================================ */

static const char* cli_tab_names[TAB_COUNT] = {
    "Projects",
    "Packages",
    "Build",
    "Tests"
};

/* ============================================================================
 * Render Callbacks
 * ============================================================================ */

static void render_tab_bar(CELS_Iter* it) {
    int count = cels_iter_count(it);
    TabBar* bars = (TabBar*)cels_iter_column(it, TabBarID, sizeof(TabBar));
    if (!bars || count == 0) return;

    TabBar* bar = &bars[0];
    int max_x = COLS;

    /* Draw tab bar background */
    attron(A_BOLD);
    move(0, 0);
    clrtoeol();

    int x = 1;
    for (int i = 0; i < bar->count && i < TAB_COUNT; i++) {
        if (i == (int)bar->active) {
            attron(A_REVERSE);
        }

        const char* name = cli_tab_names[i];
        int label_len = (int)strlen(name);

        /* Format: " N:Name " */
        if (x + label_len + 5 < max_x) {
            mvprintw(0, x, " %d:%s ", i + 1, name);
            x += label_len + 5;
        }

        if (i == (int)bar->active) {
            attroff(A_REVERSE);
        }

        /* Separator */
        if (i < bar->count - 1 && x < max_x) {
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
    int text_len = (int)strlen(content->placeholder_text);
    int center_x = (COLS - text_len) / 2;
    if (center_x < 0) center_x = 0;

    /* Draw centered placeholder text */
    attron(A_DIM);
    mvprintw(center_y, center_x, "%s", content->placeholder_text);
    attroff(A_DIM);

    /* Draw tab-specific hint */
    const char* hint = NULL;
    switch (content->tab) {
        case TAB_PROJECTS: hint = "Create and manage CELS projects"; break;
        case TAB_PACKAGES: hint = "Browse and install CELS modules"; break;
        case TAB_BUILD:    hint = "Build and run your project"; break;
        case TAB_TESTS:    hint = "Discover and run test suites"; break;
        default: break;
    }
    if (hint) {
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

    /* Draw status bar background */
    attron(A_REVERSE);
    move(y, 0);
    clrtoeol();

    /* Left: version */
    mvprintw(y, 1, " %s ", bar->version ? bar->version : "cels-cli");

    /* Right: help hint */
    const char* help = "Tab:switch  q:quit ";
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

#endif /* CELS_CLI_RENDER_PROVIDER_H */
